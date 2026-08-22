# Vix Language v0.5.0 Release Notes

## Release Date: August 22, 2026

## Overview

Vix 0.5.0 fixes Windows linking by bundling the MinGW runtime libraries into the distributed package, and fixes the `-obj` / `-S` output path derivation.

## Bug Fixes

### Windows MinGW runtime linking

- **Bundled MinGW runtime**: the Windows release package now ships with the MinGW-w64 runtime libraries (`libmingw32.a`, `libgcc.a`, `libgcc_eh.a`, CRT objects, and import libraries) under `libc/`. The linker auto-detects this bundled directory, so `vixc main.vix` links on a clean Windows machine without requiring a separate MinGW toolchain installation.
- **CI**: `.github/workflows/build.yml` now automatically installs a MinGW-w64 toolchain, collects the runtime libraries, and packages them into `dist/libc/` for the Windows zip artifact.

### CLI output paths

- **`-obj` / `-S` without `-o`**: `vixc main.vix -obj` and `vixc main.vix -S` previously failed with `Error: could not determine LLVM IR output path` because the intermediate `.ll` path was only derived when `save_c` was set. The LLVM IR output path is now derived for object and assembly emission modes as well.

## Version

- Bumped compiler version to `0.5.0` (reported by `vixc -v`).
