cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

cmake_policy(PUSH)
cmake_policy(SET CMP0063 NEW) # Honor visibility properties.

include(CheckCXXCompilerFlag)

# Don't create a project if it was already created by another CMakeLists.txt.
# This allows one library to embed another library without a project collision.
if (NOT CMAKE_PROJECT_NAME OR "${CMAKE_PROJECT_NAME}" STREQUAL "asmjit")
  project(asmjit CXX)
endif()

# =============================================================================
# [AsmJit - Configuration]
# =============================================================================

if (NOT DEFINED ASMJIT_BUILD_EMBED)
  set(ASMJIT_BUILD_EMBED FALSE)
endif()

if (NOT DEFINED ASMJIT_BUILD_STATIC)
  set(ASMJIT_BUILD_STATIC ${ASMJIT_BUILD_EMBED})
endif()

if (NOT DEFINED ASMJIT_BUILD_ARM)
  set(ASMJIT_BUILD_ARM FALSE)
endif()

if (NOT DEFINED ASMJIT_BUILD_X86)
  set(ASMJIT_BUILD_X86 FALSE)
endif()

if (NOT DEFINED ASMJIT_BUILD_TEST)
  set(ASMJIT_BUILD_TEST FALSE)
endif()

set(ASMJIT_DIR            "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "Location of 'asmjit'")
set(ASMJIT_BUILD_EMBED    ${ASMJIT_BUILD_EMBED}       CACHE BOOL "Embed 'asmjit' library (no targets)")
set(ASMJIT_BUILD_STATIC   ${ASMJIT_BUILD_STATIC}      CACHE BOOL "Build 'asmjit' library as static")
set(ASMJIT_BUILD_SANITIZE ${ASMJIT_BUILD_SANITIZE}    CACHE BOOL "Use C/C++ sanitizers if possible")
set(ASMJIT_BUILD_TEST     ${ASMJIT_BUILD_TEST}        CACHE BOOL "Build 'asmjit_test' applications")
set(ASMJIT_BUILD_X86      ${ASMJIT_BUILD_X86}         CACHE BOOL "Build X86 backends (X86 and X86_64)")
set(ASMJIT_BUILD_ARM      ${ASMJIT_BUILD_ARM}         CACHE BOOL "Build ARM backends")

# =============================================================================
# [AsmJit - Utilities]
# =============================================================================

function(asmjit_detect_cflags out)
  set(out_array ${${out}})
  foreach(flag ${ARGN})
    string(REGEX REPLACE "[+]" "x" flag_signature "${flag}")
    string(REGEX REPLACE "[-=:;/.\]" "_" flag_signature "${flag_signature}")
    check_cxx_compiler_flag(${flag} "__CxxFlag_${flag_signature}")
    if (${__CxxFlag_${flag_signature}})
      list(APPEND out_array "${flag}")
    endif()
  endforeach()
  set(${out} "${out_array}" PARENT_SCOPE)
endfunction()

set(ASMJIT_MAKE_DEF "-D")  # Define a preprocessor macro: "${ASMJIT_MAKE_DEF}VAR=1"
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") OR "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
  set(ASMJIT_MAKE_DEF "/D")
endif()

function(asmjit_add_source out base_dir)
  set(src_array)
  foreach(file ${ARGN})
    set(src_file "${ASMJIT_DIR}/${base_dir}/${file}")
    list(APPEND src_array ${src_file})
  endforeach()
  source_group(TREE "${ASMJIT_DIR}" FILES ${src_array})

  set(out_tmp ${${out}})
  list(APPEND out_tmp ${src_array})
  set("${out}" "${out_tmp}" PARENT_SCOPE)
endfunction()

function(asmjit_add_target target target_type src deps cflags cflags_dbg cflags_rel)
  if ("${target_type}" STREQUAL "EXECUTABLE")
    add_executable(${target} ${src})
  else()
    add_library(${target} ${target_type} ${src})
  endif()

  target_link_libraries(${target} PRIVATE ${deps})

  # target_link_options was added in cmake 3.13, which doesn't work for us.
  # target_link_options(${target} PRIVATE ${ASMJIT_SANITIZE_FLAGS})
  foreach(link_flag ${ASMJIT_SANITIZE_FLAGS})
    set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " ${link_flag}")
  endforeach()

  set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS NO)
  set_property(TARGET ${target} PROPERTY CXX_VISIBILITY_PRESET hidden)
  target_compile_options(${target} PRIVATE ${cflags} ${ASMJIT_SANITIZE_FLAGS} $<$<CONFIG:Debug>:${cflags_dbg}> $<$<NOT:$<CONFIG:Debug>>:${cflags_rel}>)
  target_compile_features(${target} PUBLIC cxx_std_11)
endfunction()

# =============================================================================
# [AsmJit - Project]
# =============================================================================

set(ASMJIT_INCLUDE_DIRS "${ASMJIT_DIR}/src")     # Include directory is the same as source dir.
set(ASMJIT_DEPS "")                              # Blend2D dependencies (libraries) for the linker.
set(ASMJIT_LIBS "")                              # Dependencies of libs/apps that want to use Blend2D.
set(ASMJIT_CFLAGS "")                            # Public compiler flags.
set(ASMJIT_PRIVATE_CFLAGS "")                    # Private compiler flags independent of build type.
set(ASMJIT_PRIVATE_CFLAGS_DBG "")                # Private compiler flags used by debug builds.
set(ASMJIT_PRIVATE_CFLAGS_REL "")                # Private compiler flags used by release builds.
set(ASMJIT_SANITIZE_FLAGS "")                    # Sanitizer flags used as compile and link flags.

# TODO: Backward compatibility.
set(ASMJIT_INCLUDE_DIR "${ASMJIT_INCLUDE_DIRS}")

if (NOT ASMJIT_BUILD_USE_DEFAULT_FLAGS)
  if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") OR "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    list(APPEND ASMJIT_PRIVATE_CFLAGS     /GR-   # [-] Runtime type information.
                                          /GF)   # [+] Eliminate duplicate strings.
    list(APPEND ASMJIT_PRIVATE_CFLAGS_DBG /GS)   # [+] Buffer security-check.
    list(APPEND ASMJIT_PRIVATE_CFLAGS_REL /GS-   # [-] Buffer security-check.
                                          /Oi    # [+] Generate Intrinsic Functions.
                                          /GL)   # [+] Whole Program Optimization.
  endif()

  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang)$")
    asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -Wall -Wextra)
    asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS_REL -O2 -fmerge-all-constants)
  endif()
endif()

if (ASMJIT_BUILD_SANITIZE AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang)$")
  asmjit_detect_cflags(ASMJIT_SANITIZE_FLAGS "-fsanitize=undefined")
endif()

if (NOT WIN32)
  list(APPEND ASMJIT_DEPS pthread)
endif()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  list(APPEND ASMJIT_DEPS rt)
endif()

set(ASMJIT_LIBS ${ASMJIT_DEPS})
if (NOT ASMJIT_BUILD_EMBED)
  list(INSERT ASMJIT_LIBS 0 asmjit)
endif()

if (ASMJIT_BUILD_EMBED)
  set(ASMJIT_TARGET_TYPE "EMBED")
elseif (ASMJIT_BUILD_STATIC)
  set(ASMJIT_TARGET_TYPE "STATIC")
else()
  set(ASMJIT_TARGET_TYPE "SHARED")
endif()

foreach(build_option ASMJIT_BUILD_X86
                     ASMJIT_BUILD_ARM
                     ASMJIT_BUILD_A64
                     ASMJIT_BUILD_EMBED
                     ASMJIT_BUILD_STATIC
                     ASMJIT_DISABLE_BUILDER
                     ASMJIT_DISABLE_COMPILER
                     ASMJIT_DISABLE_TEXT
                     ASMJIT_DISABLE_LOGGING
                     ASMJIT_DISABLE_INST_API)
  if (${build_option})
    List(APPEND ASMJIT_CFLAGS         "${ASMJIT_MAKE_DEF}${build_option}")
    List(APPEND ASMJIT_PRIVATE_CFLAGS "${ASMJIT_MAKE_DEF}${build_option}")
  endif()
endforeach()

# =============================================================================
# [AsmJit - Source]
# =============================================================================

set(ASMJIT_SRC_LIST
  asmjit/asmjit.h
  asmjit/arm.h
  asmjit/core.h
  asmjit/x86.h

  asmjit/core/build.h
  asmjit/core/arch.cpp
  asmjit/core/arch.h
  asmjit/core/assembler.cpp
  asmjit/core/assembler.h
  asmjit/core/builder.cpp
  asmjit/core/builder.h
  asmjit/core/callconv.cpp
  asmjit/core/callconv.h
  asmjit/core/codebufferwriter_p.h
  asmjit/core/codeholder.cpp
  asmjit/core/codeholder.h
  asmjit/core/compiler.cpp
  asmjit/core/compiler.h
  asmjit/core/constpool.cpp
  asmjit/core/constpool.h
  asmjit/core/cpuinfo.cpp
  asmjit/core/cpuinfo.h
  asmjit/core/datatypes.h
  asmjit/core/emitter.cpp
  asmjit/core/emitter.h
  asmjit/core/features.h
  asmjit/core/func.cpp
  asmjit/core/func.h
  asmjit/core/globals.cpp
  asmjit/core/globals.h
  asmjit/core/inst.cpp
  asmjit/core/inst.h
  asmjit/core/jitallocator.cpp
  asmjit/core/jitallocator.h
  asmjit/core/jitruntime.cpp
  asmjit/core/jitruntime.h
  asmjit/core/logging.cpp
  asmjit/core/logging.h
  asmjit/core/misc_p.h
  asmjit/core/operand.cpp
  asmjit/core/operand.h
  asmjit/core/osutils.cpp
  asmjit/core/osutils.h
  asmjit/core/raassignment_p.h
  asmjit/core/rabuilders_p.h
  asmjit/core/radefs_p.h
  asmjit/core/ralocal.cpp
  asmjit/core/ralocal_p.h
  asmjit/core/rapass.cpp
  asmjit/core/rapass_p.h
  asmjit/core/rastack.cpp
  asmjit/core/rastack_p.h
  asmjit/core/string.cpp
  asmjit/core/string.h
  asmjit/core/support.cpp
  asmjit/core/support.h
  asmjit/core/target.cpp
  asmjit/core/target.h
  asmjit/core/type.cpp
  asmjit/core/type.h
  asmjit/core/virtmem.cpp
  asmjit/core/virtmem.h
  asmjit/core/zone.cpp
  asmjit/core/zone.h
  asmjit/core/zonehash.cpp
  asmjit/core/zonehash.h
  asmjit/core/zonelist.cpp
  asmjit/core/zonelist.h
  asmjit/core/zonestack.cpp
  asmjit/core/zonestack.h
  asmjit/core/zonestring.h
  asmjit/core/zonetree.cpp
  asmjit/core/zonetree.h
  asmjit/core/zonevector.cpp
  asmjit/core/zonevector.h

  asmjit/arm/armassembler.cpp
  asmjit/arm/armassembler.h
  asmjit/arm/armbuilder.cpp
  asmjit/arm/armbuilder.h
  asmjit/arm/armcallconv.cpp
  asmjit/arm/armcallconv_p.h
  asmjit/arm/armcompiler.cpp
  asmjit/arm/armcompiler.h
  asmjit/arm/armemitter.h
  asmjit/arm/armfeatures.cpp
  asmjit/arm/armfeatures.h
  asmjit/arm/armglobals.h
  asmjit/arm/arminternal.cpp
  asmjit/arm/arminternal_p.h
  asmjit/arm/arminstdb.cpp
  asmjit/arm/arminstdb.h
  asmjit/arm/armlogging.cpp
  asmjit/arm/armlogging_p.h
  asmjit/arm/armoperand.cpp
  asmjit/arm/armoperand.h
  asmjit/arm/armrapass.cpp
  asmjit/arm/armrapass_p.h

  asmjit/x86/x86assembler.cpp
  asmjit/x86/x86assembler.h
  asmjit/x86/x86builder.cpp
  asmjit/x86/x86builder.h
  asmjit/x86/x86callconv.cpp
  asmjit/x86/x86callconv_p.h
  asmjit/x86/x86compiler.cpp
  asmjit/x86/x86compiler.h
  asmjit/x86/x86emitter.h
  asmjit/x86/x86features.cpp
  asmjit/x86/x86features.h
  asmjit/x86/x86globals.h
  asmjit/x86/x86internal.cpp
  asmjit/x86/x86internal_p.h
  asmjit/x86/x86instdb.cpp
  asmjit/x86/x86instdb.h
  asmjit/x86/x86instdb_p.h
  asmjit/x86/x86logging.cpp
  asmjit/x86/x86logging_p.h
  asmjit/x86/x86operand.cpp
  asmjit/x86/x86operand.h
  asmjit/x86/x86rapass.cpp
  asmjit/x86/x86rapass_p.h
  asmjit/x86/x86ssetoavx.cpp
  asmjit/x86/x86ssetoavx_p.h
)

set(ASMJIT_SRC "")
asmjit_add_source(ASMJIT_SRC src ${ASMJIT_SRC_LIST})

# if (MSVC)
#   asmjit_add_source(ASMJIT_SRC .. asmjit.natvis)
# endif()

# =============================================================================
# [AsmJit - Summary]
# =============================================================================

message("** AsmJit Summary **")
message("   ASMJIT_DIR=${ASMJIT_DIR}")
message("   ASMJIT_TARGET_TYPE=${ASMJIT_TARGET_TYPE}")
message("   ASMJIT_BUILD_TEST=${ASMJIT_BUILD_TEST}")
message("   ASMJIT_DEPS=${ASMJIT_DEPS}")
message("   ASMJIT_LIBS=${ASMJIT_LIBS}")
message("   ASMJIT_CFLAGS=${ASMJIT_CFLAGS}")
message("   ASMJIT_PRIVATE_CFLAGS=")

foreach(_flag ${ASMJIT_PRIVATE_CFLAGS})
  message("     ${_flag}")
endforeach()

foreach(_flag ${ASMJIT_PRIVATE_CFLAGS_DBG})
  message("     ${_flag} [DEBUG]")
endforeach()

foreach(_flag ${ASMJIT_PRIVATE_CFLAGS_REL})
  message("     ${_flag} [RELEASE]")
endforeach()

# =============================================================================
# [AsmJit - Targets]
# =============================================================================

if (NOT ASMJIT_BUILD_EMBED)
  # Add 'asmjit' target.
  asmjit_add_target(asmjit "${ASMJIT_TARGET_TYPE}"
    "${ASMJIT_SRC}"
    "${ASMJIT_DEPS}"
    "${ASMJIT_PRIVATE_CFLAGS}"
    "${ASMJIT_PRIVATE_CFLAGS_DBG}"
    "${ASMJIT_PRIVATE_CFLAGS_REL}")
  target_include_directories(asmjit BEFORE INTERFACE ${ASMJIT_INCLUDE_DIRS})
  target_compile_options(asmjit INTERFACE ${ASMJIT_CFLAGS})

  # Add AsmJit::AsmJit target (alias to asmjit).
  add_library(AsmJit::AsmJit ALIAS asmjit)

  # Install 'asmjit' target (shared or static).
  install(TARGETS asmjit RUNTIME DESTINATION "bin"
                         LIBRARY DESTINATION "lib${LIB_SUFFIX}"
                         ARCHIVE DESTINATION "lib${LIB_SUFFIX}")

  # Install 'asmjit' header files (private headers are filtered out).
  foreach(_src_file ${ASMJIT_SRC_LIST})
    if ("${_src_file}" MATCHES ".h$" AND NOT "${_src_file}" MATCHES "_p.h$")
      get_filename_component(_src_dir ${_src_file} PATH)
      get_filename_component(_src_name ${_src_file} NAME)
      install(FILES "${ASMJIT_DIR}/src/${_src_file}" DESTINATION "include/${_src_dir}")
    endif()
  endforeach()

  # Add 'asmjit' tests.
  if (ASMJIT_BUILD_TEST)
    asmjit_add_source(ASMJIT_TEST_SRC test asmjit_test_unit.cpp broken.cpp broken.h)

    asmjit_add_target(asmjit_test_unit EXECUTABLE
      "${ASMJIT_SRC};${ASMJIT_TEST_SRC}"
      "${ASMJIT_DEPS}"
      "${ASMJIT_PRIVATE_CFLAGS}"
      "${ASMJIT_PRIVATE_CFLAGS_DBG}"
      "${ASMJIT_PRIVATE_CFLAGS_REL}")
    target_compile_definitions(asmjit_test_unit PRIVATE ASMJIT_BUILD_TEST ASMJIT_BUILD_EMBED)

    foreach(_target asmjit_bench_x86
                    asmjit_test_opcode
                    asmjit_test_x86_asm
                    asmjit_test_x86_cc
                    asmjit_test_x86_sections)
      asmjit_add_target(${_target} EXECUTABLE
        "test/${_target}.cpp"
        "${ASMJIT_LIBS}"
        "${ASMJIT_PRIVATE_CFLAGS}"
        "${ASMJIT_PRIVATE_CFLAGS_DBG}"
        "${ASMJIT_PRIVATE_CFLAGS_REL}")
    endforeach()
  endif()
endif()

cmake_policy(POP)
