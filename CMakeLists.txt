cmake_minimum_required(VERSION 3.20)
project(vixc LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(BISON 3.0 REQUIRED)
find_package(FLEX 2.6 REQUIRED)

if(APPLE)
    set(_brew_llvm_prefix "")
    set(_brew_lld_prefix "")
    foreach(_candidate "llvm" "llvm@20" "llvm@19" "llvm@18")
        execute_process(
            COMMAND brew --prefix "${_candidate}"
            OUTPUT_VARIABLE _prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE _brew_rc
        )
        if(_brew_rc EQUAL 0 AND EXISTS "${_prefix}/lib/cmake/llvm/LLVMConfig.cmake")
            set(_brew_llvm_prefix "${_prefix}")
            break()
        endif()
    endforeach()
    execute_process(
        COMMAND brew --prefix lld
        OUTPUT_VARIABLE _prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _brew_rc
    )
    if(_brew_rc EQUAL 0 AND EXISTS "${_prefix}/lib/cmake/lld/LLDConfig.cmake")
        set(_brew_lld_prefix "${_prefix}")
    endif()
    if(_brew_llvm_prefix)
        list(PREPEND CMAKE_PREFIX_PATH "${_brew_llvm_prefix}")
        message(STATUS "Homebrew LLVM prefix: ${_brew_llvm_prefix}")
    else()
        message(WARNING "No Homebrew LLVM found. Install via: brew install llvm")
    endif()
    if(_brew_lld_prefix)
        list(PREPEND CMAKE_PREFIX_PATH "${_brew_lld_prefix}")
        message(STATUS "Homebrew LLD prefix: ${_brew_lld_prefix}")
    endif()
endif()

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} at ${LLVM_DIR}")

get_filename_component(_llvm_cmake_dir "${LLVM_DIR}" ABSOLUTE)
set(_lld_cmake_candidates
    "${_llvm_cmake_dir}/../lld"
    "${_llvm_cmake_dir}/../../lld"
    "${_llvm_cmake_dir}/../../../lib/cmake/lld"
    "${_llvm_cmake_dir}/../lib/cmake/lld"
)
if(APPLE AND _brew_llvm_prefix)
    list(APPEND _lld_cmake_candidates "${_brew_llvm_prefix}/lib/cmake/lld")
endif()
if(APPLE AND _brew_lld_prefix)
    list(APPEND _lld_cmake_candidates "${_brew_lld_prefix}/lib/cmake/lld")
endif()
if(WIN32 AND CMAKE_PREFIX_PATH)
    foreach(_pp IN LISTS CMAKE_PREFIX_PATH)
        list(APPEND _lld_cmake_candidates "${_pp}/lib/cmake/lld")
    endforeach()
endif()

set(_lld_found OFF)
foreach(_lld_dir IN LISTS _lld_cmake_candidates)
    get_filename_component(_lld_dir "${_lld_dir}" ABSOLUTE)
    if(EXISTS "${_lld_dir}/LLDConfig.cmake")
        find_package(LLD REQUIRED CONFIG PATHS "${_lld_dir}" NO_DEFAULT_PATH)
        set(_lld_found ON)
        message(STATUS "Found LLD at ${_lld_dir}")
        break()
    endif()
endforeach()

if(NOT _lld_found)
    find_package(LLD REQUIRED CONFIG)
endif()

include_directories(${LLVM_INCLUDE_DIRS} ${LLD_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
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
set(C_SOURCES
    src/main.c
    src/ast/ast.c
    src/ast/typeinfer.c
    src/semantic/semantic.c
    src/utils/error.c
    ${BISON_vix_parser_OUTPUTS}
    ${FLEX_vix_lexer_OUTPUTS}
)
set(CXX_SOURCES
    src/compiler/CodeGen.cpp
    src/compiler/Passes.cpp
    src/compiler/Llc/Llc.cpp
    src/compiler/Linker/Linker.cpp
    src/Typeck/Typeck.cpp
    src/Typeck/TypeckInfer.cpp
    src/Typeck/LayOut.cpp
)
add_executable(vixc ${C_SOURCES} ${CXX_SOURCES})

target_include_directories(vixc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/parser
)
target_compile_options(vixc PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra -fno-stack-protector>
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -fno-stack-protector -fexceptions>
)

if(NOT WIN32)
    target_compile_definitions(vixc PRIVATE
        _POSIX_C_SOURCE=200809L
        _XOPEN_SOURCE=700
    )
endif()
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
install(TARGETS vixc RUNTIME DESTINATION bin)