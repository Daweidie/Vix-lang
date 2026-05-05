cmake_minimum_required(VERSION 3.20)
project(vixc LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Platform-specific tool names ─────────────────────────────────
if(WIN32)
    set(BISON_EXECUTABLE "win_bison" CACHE FILEPATH "Path to bison executable")
    set(FLEX_EXECUTABLE "win_flex" CACHE FILEPATH "Path to flex executable")
endif()

# ── Required tools ────────────────────────────────────────────────
find_package(BISON 3.0 REQUIRED)
find_package(FLEX 2.6 REQUIRED)
find_package(LLVM REQUIRED CONFIG)
find_package(LLD REQUIRED CONFIG
    HINTS "${LLVM_DIR}/../lib/cmake/lld"
)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using LLDConfig.cmake in: ${LLD_CMAKE_DIR}")

include_directories(${LLVM_INCLUDE_DIRS} ${LLD_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# ── Bison / Flex ─────────────────────────────────────────────────
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/parser)

BISON_TARGET(vix_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.tab.c
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.tab.h
)

FLEX_TARGET(vix_lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/parser/lex.yy.c
)
ADD_FLEX_BISON_DEPENDENCY(vix_lexer vix_parser)

# ── C sources ────────────────────────────────────────────────────
set(C_SOURCES
    src/main.c
    src/ast/ast.c
    src/ast/typeinfer.c
    src/semantic/semantic.c
    src/utils/error.c
    ${BISON_vix_parser_OUTPUTS}
    ${FLEX_vix_lexer_OUTPUTS}
)

# ── C++ sources ──────────────────────────────────────────────────
set(CXX_SOURCES
    src/compiler/CodeGen.cpp
    src/compiler/Passes.cpp
    src/compiler/Llc/Llc.cpp
    src/compiler/Linker/Linker.cpp
    src/Typeck/Typeck.cpp
    src/Typeck/TypeckInfer.cpp
    src/Typeck/LayOut.cpp
)

# ── Target ───────────────────────────────────────────────────────
add_executable(vixc ${C_SOURCES} ${CXX_SOURCES})

target_include_directories(vixc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/parser
)

if(MSVC)
    target_compile_options(vixc PRIVATE
        $<$<COMPILE_LANGUAGE:C>:/W4>
        $<$<COMPILE_LANGUAGE:CXX>:/W4 /EHsc>
    )
else()
    target_compile_options(vixc PRIVATE
        $<$<COMPILE_LANGUAGE_C>:-Wall -Wextra -fno-stack-protector>
        $<$<COMPILE_LANGUAGE_CXX>:-Wall -Wextra -fno-stack-protector -fexceptions>
    )
endif()

if(NOT WIN32)
    target_compile_definitions(vixc PRIVATE
        _POSIX_C_SOURCE=200809L
        _XOPEN_SOURCE=700
    )
endif()

# ── Link ─────────────────────────────────────────────────────────
if(TARGET LLVM)
    target_link_libraries(vixc PRIVATE LLVM)
else()
    llvm_map_components_to_libnames(LLVM_LIBS
        core support passes irreader target
        x86asmparser x86codegen x86desc x86disassembler x86info
        aarch64asmparser aarch64codegen aarch64desc aarch64disassembler aarch64info
        armasmparser armcodegen armdesc armdisassembler arminfo
        webassemblyasmparser webassemblycodegen webassemblydesc webassemblydisassembler webassemblyinfo
        mc mcparser mcdisassembler bitreader bitwriter
        linker instrumentation ipo codegen
    )
    target_link_libraries(vixc PRIVATE ${LLVM_LIBS})
endif()
target_link_libraries(vixc PRIVATE
    lldELF lldMachO lldCOFF lldWasm lldMinGW lldCommon
)

foreach(lib IN LISTS LLVM_SYSTEM_LIBS)
    target_link_libraries(vixc PRIVATE ${lib})
endforeach()

# ── Install ──────────────────────────────────────────────────────
install(TARGETS vixc RUNTIME DESTINATION bin)