/*
 * Copyright (c) 2026 Vix Language Authors. All rights reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Linker.h"

#include <lld/Common/Driver.h>
LLD_HAS_DRIVER(elf)
LLD_HAS_DRIVER(macho)
LLD_HAS_DRIVER(coff)
LLD_HAS_DRIVER(mingw)
LLD_HAS_DRIVER(wasm)

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <string>
#include <vector>

using namespace llvm;

namespace {

enum class LinkFlavor { ELF, MachO, COFF, MinGW, Wasm };

LinkFlavor detectFlavor(const Triple &T) {
    if (T.isOSBinFormatMachO()) return LinkFlavor::MachO;
    if (T.isOSBinFormatWasm())  return LinkFlavor::Wasm;
    if (T.isOSBinFormatCOFF()) {
        if (T.isWindowsGNUEnvironment()) return LinkFlavor::MinGW;
        return LinkFlavor::COFF;
    }
    return LinkFlavor::ELF;
}

bool fileExists(const Twine &path) {
    return sys::fs::exists(path);
}

// ── Linux / ELF sysroot discovery ───────────────────────────────
struct SysPaths {
    std::string gccDir;
    std::string sysLibDir;
    std::string dynamicLinker;
};

SysPaths probeSysPaths(const Triple &T) {
    SysPaths sp;

    StringRef arch = T.isArch64Bit() ? "x86_64" : (T.getArch() == Triple::aarch64 ? "aarch64" : "x86_64");
    std::string gnuTuple = arch.str() + "-linux-gnu";

    std::string libDir = "/usr/lib/" + gnuTuple;
    if (!fileExists(libDir + "/crt1.o")) {
        std::string alt = "/usr/lib/" + std::string(T.isArch64Bit() ? "64" : "32");
        if (fileExists(alt + "/crt1.o"))
            libDir = alt;
    }
    sp.sysLibDir = libDir;

    SmallString<256> gccBase("/usr/lib/gcc/" + gnuTuple);
    if (sys::fs::is_directory(gccBase)) {
        std::string bestVer;
        std::error_code ec;
        for (sys::fs::directory_iterator it(gccBase, ec), end; it != end;
             it.increment(ec)) {
            StringRef name = sys::path::filename(it->path());
            if (name > bestVer)
                bestVer = name.str();
        }
        if (!bestVer.empty()) {
            std::string cand = (gccBase + "/" + bestVer).str();
            if (fileExists(cand + "/crtbegin.o"))
                sp.gccDir = cand;
        }
    }

    static const char *const ldCandidates[] = {
        "/lib64/ld-linux-x86-64.so.2",
        "/lib/ld-linux-aarch64.so.1",
        "/lib/ld-linux-armhf.so.3",
        "/lib/ld-linux.so.2",
        "/lib/ld-musl-x86_64.so.1",
    };
    for (auto *c : ldCandidates) {
        if (fileExists(c)) {
            sp.dynamicLinker = c;
            break;
        }
    }

    return sp;
}

// ── macOS SDK discovery ─────────────────────────────────────────
std::string findMacOSSDK() {
    static const char *const sdkCandidates[] = {
        "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk",
        "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk",
    };
    for (auto *c : sdkCandidates) {
        if (sys::fs::is_directory(c))
            return c;
    }
    return {};
}

// ── MinGW installation discovery ────────────────────────────────
struct MinGWPaths {
    std::string libDir;    // e.g. /usr/x86_64-w64-mingw32/lib
    std::string crtDir;    // e.g. /usr/lib/gcc/x86_64-w64-mingw32/13-posix
};

MinGWPaths probeMinGWPaths(const Triple &T) {
    MinGWPaths mp;
    std::string triple = T.getTriple(); // e.g. x86_64-w64-mingw32

    // Standard MinGW sysroot locations.
    std::string sysroot = "/usr/" + triple;
    if (sys::fs::is_directory(sysroot + "/lib")) {
        mp.libDir = sysroot + "/lib";
    }

    // GCC cross-compiler CRT directory.
    std::string gccBase = "/usr/lib/gcc/" + triple;
    if (sys::fs::is_directory(gccBase)) {
        std::string bestVer;
        std::error_code ec;
        for (sys::fs::directory_iterator it(gccBase, ec), end; it != end;
             it.increment(ec)) {
            StringRef name = sys::path::filename(it->path());
            if (name > bestVer)
                bestVer = name.str();
        }
        if (!bestVer.empty()) {
            std::string cand = gccBase + "/" + bestVer;
            // Prefer posix threading variant if available.
            if (sys::fs::is_directory(cand + "-posix"))
                cand += "-posix";
            if (fileExists(cand + "/crtbegin.o"))
                mp.crtDir = cand;
        }
    }

    return mp;
}

// ── Linker argument builders ────────────────────────────────────

void addBareArgs(std::vector<std::string> &args, const char *entry,
                 const char *script) {
    args.push_back("-static");
    args.push_back("-e");
    args.push_back(entry && entry[0] ? entry : "_start");
    if (script && script[0]) {
        args.push_back("-T");
        args.push_back(script);
    }
}

void buildElfArgs(std::vector<std::string> &args, bool bare,
                  const char *entry, const char *script) {
    if (bare) {
        args.push_back("--no-dynamic-linker");
        args.push_back("-z");
        args.push_back("max-page-size=0x1000");
        args.push_back("--build-id=none");
        addBareArgs(args, entry, script);
    }
}

void buildMachOArgs(std::vector<std::string> &args, const Triple &T,
                    bool bare, const char *entry, const char *script) {
    args.push_back("-arch");
    args.push_back(T.getArch() == Triple::aarch64 ? "arm64" : "x86_64");

    if (!bare) {
        std::string sdk = findMacOSSDK();
        if (!sdk.empty()) {
            args.push_back("-syslibroot");
            args.push_back(sdk);
        }
        args.push_back("-L/usr/lib");
        args.push_back("-L/usr/local/lib");
        if (!sdk.empty())
            args.push_back("-L" + sdk + "/usr/lib");

        args.push_back("-lSystem");
        args.push_back("-lc++");
    } else {
        addBareArgs(args, entry, script);
    }
}

void buildCoffArgs(std::vector<std::string> &args, bool bare,
                   const char *entry, const char *script,
                   const char *libc_dir) {
    args.push_back("/NOLOGO");
    if (bare) {
        std::string e = "/ENTRY:";
        e += (entry && entry[0]) ? entry : "main";
        args.push_back(e);
        args.push_back("/SUBSYSTEM:CONSOLE");
        addBareArgs(args, entry, script);
    } else {
        // Prefer bundled libc if available
        if (libc_dir && libc_dir[0]) {
            args.push_back("/LIBPATH:" + std::string(libc_dir));
        }
        args.push_back("msvcrt.lib");
        args.push_back("kernel32.lib");
    }
}

void buildMinGWArgs(std::vector<std::string> &args, const Triple &T,
                    bool bare, const char *entry, const char *script,
                    const char *libc_dir) {
    if (bare) {
        args.push_back("--no-dynamic-linker");
        args.push_back("-e");
        args.push_back(entry && entry[0] ? entry : "main");
        args.push_back("-static");
        if (script && script[0]) {
            args.push_back("-T");
            args.push_back(script);
        }
    } else {
        // Prefer bundled libc if available
        bool haveBundled = libc_dir && libc_dir[0] &&
                           sys::fs::is_directory(libc_dir);
        if (haveBundled) {
            args.push_back("-L" + std::string(libc_dir));
            // Try bundled CRT objects
            std::string crtbegin = std::string(libc_dir) + "/crtbegin.o";
            if (fileExists(crtbegin))
                args.push_back(crtbegin);
            std::string crt2 = std::string(libc_dir) + "/crt2.o";
            if (fileExists(crt2))
                args.push_back(crt2);
        } else {
            // Fall back to system MinGW paths
            MinGWPaths mp = probeMinGWPaths(T);
            if (!mp.libDir.empty())
                args.push_back("-L" + mp.libDir);
            if (!mp.crtDir.empty()) {
                args.push_back(mp.crtDir + "/crtbegin.o");
                args.push_back("-L" + mp.crtDir);
            }
        }

        args.push_back("-lmingw32");
        args.push_back("-lgcc");
        args.push_back("-lgcc_eh");
        args.push_back("-lmoldname");
        args.push_back("-lmingwex");
        args.push_back("-lmsvcrt");
        args.push_back("-lkernel32");
        args.push_back("-lpthread");
        args.push_back("-ladvapi32");
        args.push_back("-lshell32");
        args.push_back("-luser32");
        args.push_back("-lmingw32");
        args.push_back("-lgcc");
        args.push_back("-lgcc_eh");
        args.push_back("-lmsvcrt");

        if (haveBundled) {
            std::string crtend = std::string(libc_dir) + "/crtend.o";
            if (fileExists(crtend))
                args.push_back(crtend);
        } else {
            MinGWPaths mp = probeMinGWPaths(T);
            if (!mp.crtDir.empty())
                args.push_back(mp.crtDir + "/crtend.o");
        }
    }
}

} // namespace

extern "C" int vix_link(const char *obj_file, const char *output_file,
                        const VixLinkOptions *options, const char **error_msg) {
    static thread_local std::string lastError;

    if (!obj_file || !output_file) {
        lastError = "linker: missing input or output file";
        if (error_msg) *error_msg = lastError.c_str();
        return 0;
    }

    std::string tripleStr = (options && options->target_triple && options->target_triple[0])
                                ? options->target_triple
                                : sys::getDefaultTargetTriple();
    Triple T(tripleStr);
    LinkFlavor flavor = detectFlavor(T);
    bool bare = options && options->bare_mode;

    std::vector<std::string> args;
    const char *progName = nullptr;
    switch (flavor) {
        case LinkFlavor::ELF:   progName = "ld.lld";    break;
        case LinkFlavor::MachO: progName = "ld64.lld";  break;
        case LinkFlavor::COFF:  progName = "lld-link";  break;
        case LinkFlavor::MinGW: progName = "ld.lld";    break;
        case LinkFlavor::Wasm:  progName = "wasm-ld";   break;
    }
    args.push_back(progName);

    // ── Flavor-specific flags ──────────────────────────────────
    const char *entry = options ? options->entry_point : nullptr;
    const char *script = options ? options->linker_script : nullptr;
    const char *libc_dir = options ? options->libc_dir : nullptr;

    switch (flavor) {
        case LinkFlavor::ELF:
            buildElfArgs(args, bare, entry, script);
            break;
        case LinkFlavor::MachO:
            buildMachOArgs(args, T, bare, entry, script);
            break;
        case LinkFlavor::COFF:
            buildCoffArgs(args, bare, entry, script, libc_dir);
            break;
        case LinkFlavor::MinGW:
            buildMinGWArgs(args, T, bare, entry, script, libc_dir);
            break;
        case LinkFlavor::Wasm:
            if (bare)
                addBareArgs(args, entry, script);
            break;
    }

    // ── ELF sysroot (Linux only, non-bare) ────────────────────
    if (flavor == LinkFlavor::ELF && !bare) {
        SysPaths sp = probeSysPaths(T);

        if (!sp.gccDir.empty())
            args.push_back(sp.gccDir + "/crtbegin.o");
        if (!sp.sysLibDir.empty()) {
            args.push_back(sp.sysLibDir + "/crt1.o");
            args.push_back(sp.sysLibDir + "/crti.o");
        }
        if (!sp.dynamicLinker.empty()) {
            args.push_back("--dynamic-linker");
            args.push_back(sp.dynamicLinker);
        }
        if (!sp.sysLibDir.empty())
            args.push_back("-L" + sp.sysLibDir);
        if (!sp.gccDir.empty())
            args.push_back("-L" + sp.gccDir);
        args.push_back("-L/lib");
        args.push_back("-L/usr/lib");
    }

    // ── Input / output ────────────────────────────────────────
    args.push_back(obj_file);
    args.push_back("-o");
    args.push_back(output_file);

    // ── System libraries (ELF non-bare only, others handled above) ─
    if (!bare && flavor == LinkFlavor::ELF) {
        args.push_back("-lc");
        args.push_back("-lm");
        args.push_back("-ldl");
        args.push_back("-lpthread");
        args.push_back("-lstdc++");
        SysPaths sp = probeSysPaths(T);
        if (!sp.gccDir.empty())
            args.push_back(sp.gccDir + "/crtend.o");
        if (!sp.sysLibDir.empty())
            args.push_back(sp.sysLibDir + "/crtn.o");
        if (options && options->static_link)
            args.push_back("-static");
    }

    // ── Convert to const char* array for LLD API ──────────────
    std::vector<const char *> rawArgs;
    rawArgs.reserve(args.size());
    for (auto &s : args)
        rawArgs.push_back(s.c_str());

    std::string outStr, errStr;
    raw_string_ostream outOS(outStr);
    raw_string_ostream errOS(errStr);

    // ── Select LLD driver ─────────────────────────────────────
    std::vector<lld::DriverDef> drivers;
    switch (flavor) {
        case LinkFlavor::ELF:
            drivers.push_back({lld::Gnu, lld::elf::link});
            break;
        case LinkFlavor::MachO:
            drivers.push_back({lld::Darwin, lld::macho::link});
            break;
        case LinkFlavor::COFF:
            drivers.push_back({lld::WinLink, lld::coff::link});
            break;
        case LinkFlavor::MinGW:
            drivers.push_back({lld::MinGW, lld::mingw::link});
            break;
        case LinkFlavor::Wasm:
            drivers.push_back({lld::Wasm, lld::wasm::link});
            break;
    }

    lld::Result result = lld::lldMain(rawArgs, outOS, errOS, drivers);

    outOS.flush();
    errOS.flush();

    if (result.retCode != 0) {
        lastError = errStr.empty()
                        ? "linker failed with exit code " + std::to_string(result.retCode)
                        : errStr;
        if (error_msg) *error_msg = lastError.c_str();
        return 0;
    }

    if (error_msg) *error_msg = nullptr;
    return 1;
}
