cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Don't create a project if it was already created by another CMakeLists.txt.
# This allows one library to embed another library without a project collision.
if(NOT CMAKE_PROJECT_NAME OR "${CMAKE_PROJECT_NAME}" STREQUAL "asmjit")
  project(asmjit CXX)
endif()

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

# =============================================================================
# [AsmJit - Configuration]
# =============================================================================

set(ASMJIT_DIR            "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "Location of 'asmjit'")
set(ASMJIT_BUILD_EMBED    ${ASMJIT_BUILD_EMBED}       CACHE BOOLEAN "Embed 'asmjit' library (no targets)")
set(ASMJIT_BUILD_STATIC   ${ASMJIT_BUILD_STATIC}      CACHE BOOLEAN "Build 'asmjit' library as static")
set(ASMJIT_BUILD_SANITIZE ${ASMJIT_BUILD_SANITIZE}    CACHE BOOLEAN "Use C/C++ sanitizers if possible")
set(ASMJIT_BUILD_TEST     ${ASMJIT_BUILD_TEST}        CACHE BOOLEAN "Build 'asmjit_test' applications")
set(ASMJIT_BUILD_X86      ${ASMJIT_BUILD_X86}         CACHE BOOLEAN "Build X86 backends (X86 and X86_64)")
set(ASMJIT_BUILD_ARM      ${ASMJIT_BUILD_ARM}         CACHE BOOLEAN "Build ARM backends")

# =============================================================================
# [AsmJit - Project]
# =============================================================================

include("${ASMJIT_DIR}/CxxProject.cmake")

cxx_project(asmjit)
cxx_detect_standard(ASMJIT_PRIVATE_CFLAGS)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(ASMJIT_PRIVATE_LFLAGS "/OPT:REF /OPT:ICF")

  list(APPEND ASMJIT_PRIVATE_CFLAGS /MP)            # [+] Multi-process compilation
  list(APPEND ASMJIT_PRIVATE_CFLAGS /GF)            # [+] Eliminate duplicate strings.
  list(APPEND ASMJIT_PRIVATE_CFLAGS /GR-)           # [-] Runtime type information.

  list(APPEND ASMJIT_PRIVATE_CFLAGS_DBG /GS)        # [+] Buffer security-check.
  list(APPEND ASMJIT_PRIVATE_CFLAGS_REL /GS-)       # [-] Buffer security-check.

  list(APPEND ASMJIT_PRIVATE_CFLAGS_REL /Ox)        # [+] Global optimization.
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang)$")
  cxx_detect_cflags(ASMJIT_PRIVATE_CFLAGS
    "-fvisibility=hidden"
    "-fno-tree-vectorize"
    "-Wconversion"
    "-Wall"
    "-Wextra")
  cxx_detect_cflags(ASMJIT_PRIVATE_CFLAGS_REL
    "-O2" # CMake by default uses -O3, which does nothing useful.
    "-fno-keep-static-consts"
    "-fmerge-all-constants")
endif()

if(ASMJIT_BUILD_SANITIZE AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang)$")
  cxx_detect_cflags(ASMJIT_SANITIZE_FLAGS "-fsanitize=undefined")
  list(APPEND ASMJIT_PRIVATE_CFLAGS ${ASMJIT_SANITIZE_FLAGS})
  list(APPEND ASMJIT_PRIVATE_LFLAGS ${ASMJIT_SANITIZE_FLAGS})
endif()

if(WIN32)
  list(APPEND ASMJIT_PRIVATE_CFLAGS "${CXX_DEFINE}_UNICODE")
else()
  list(APPEND ASMJIT_DEPS pthread)
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  list(APPEND ASMJIT_DEPS rt)
endif()

set(ASMJIT_LIBS ${ASMJIT_DEPS})
if(NOT ASMJIT_BUILD_EMBED)
  list(INSERT ASMJIT_LIBS 0 asmjit)
endif()

foreach(BUILD_OPTION
    ASMJIT_BUILD_X86
    ASMJIT_BUILD_ARM
    ASMJIT_BUILD_A64
    ASMJIT_DISABLE_BUILDER
    ASMJIT_DISABLE_COMPILER
    ASMJIT_DISABLE_TEXT
    ASMJIT_DISABLE_LOGGING
    ASMJIT_DISABLE_INST_API)
  if(${BUILD_OPTION})
    List(APPEND ASMJIT_CFLAGS         "${CXX_DEFINE}${BUILD_OPTION}")
    List(APPEND ASMJIT_PRIVATE_CFLAGS "${CXX_DEFINE}${BUILD_OPTION}")
  endif()
endforeach()

cxx_project_info(asmjit)

# =============================================================================
# [AsmJit - Source]
# =============================================================================

set(ASMJIT_SRC "")

cxx_add_source(asmjit ASMJIT_SRC asmjit
  asmjit.h
  arm.h
  core.h
  x86.h
)

cxx_add_source(asmjit ASMJIT_SRC asmjit/core
  build.h

  arch.cpp
  arch.h
  assembler.cpp
  assembler.h
  builder.cpp
  builder.h
  callconv.cpp
  callconv.h
  codebufferwriter_p.h
  codeholder.cpp
  codeholder.h
  compiler.cpp
  compiler.h
  constpool.cpp
  constpool.h
  cpuinfo.cpp
  cpuinfo.h
  datatypes.h
  emitter.cpp
  emitter.h
  features.h
  func.cpp
  func.h
  globals.cpp
  globals.h
  inst.cpp
  inst.h
  jitallocator.cpp
  jitallocator.h
  jitruntime.cpp
  jitruntime.h
  logging.cpp
  logging.h
  misc_p.h
  operand.cpp
  operand.h
  osutils.cpp
  osutils.h
  raassignment_p.h
  rabuilders_p.h
  radefs_p.h
  ralocal.cpp
  ralocal_p.h
  rapass.cpp
  rapass_p.h
  rastack.cpp
  rastack_p.h
  string.cpp
  string.h
  support.cpp
  support.h
  target.cpp
  target.h
  type.cpp
  type.h
  virtmem.cpp
  virtmem.h
  zone.cpp
  zone.h
  zonehash.cpp
  zonehash.h
  zonelist.cpp
  zonelist.h
  zonestack.cpp
  zonestack.h
  zonestring.h
  zonetree.cpp
  zonetree.h
  zonevector.cpp
  zonevector.h
)

cxx_add_source(asmjit ASMJIT_SRC asmjit/arm
  armassembler.cpp
  armassembler.h
  armbuilder.cpp
  armbuilder.h
  armcallconv.cpp
  armcallconv_p.h
  armcompiler.cpp
  armcompiler.h
  armemitter.h
  armfeatures.cpp
  armfeatures.h
  armglobals.h
  arminternal.cpp
  arminternal_p.h
  arminstdb.cpp
  arminstdb.h
  armlogging.cpp
  armlogging_p.h
  armoperand.cpp
  armoperand.h
  armrapass.cpp
  armrapass_p.h
)

cxx_add_source(asmjit ASMJIT_SRC asmjit/x86
  x86assembler.cpp
  x86assembler.h
  x86builder.cpp
  x86builder.h
  x86callconv.cpp
  x86callconv_p.h
  x86compiler.cpp
  x86compiler.h
  x86emitter.h
  x86features.cpp
  x86features.h
  x86globals.h
  x86internal.cpp
  x86internal_p.h
  x86instdb.cpp
  x86instdb.h
  x86instdb_p.h
  x86logging.cpp
  x86logging_p.h
  x86operand.cpp
  x86operand.h
  x86rapass.cpp
  x86rapass_p.h
  x86ssetoavx.cpp
  x86ssetoavx_p.h
)

if(MSVC)
  cxx_add_source(asmjit ASMJIT_SRC .. asmjit.natvis)
endif()

# =============================================================================
# [AsmJit - Targets]
# =============================================================================

if(NOT ASMJIT_BUILD_EMBED)
  # Add `asmjit` library.
  cxx_add_library(asmjit asmjit
    "${ASMJIT_SRC}"
    "${ASMJIT_DEPS}"
    "${ASMJIT_PRIVATE_CFLAGS}"
    "${ASMJIT_PRIVATE_CFLAGS_DBG}"
    "${ASMJIT_PRIVATE_CFLAGS_REL}")

  foreach(_src_file ${ASMJIT_SRC})
    get_filename_component(_src_dir ${_src_file} PATH)
    get_filename_component(_src_name ${_src_file} NAME)
    string(REGEX REPLACE "^${ASMJIT_SOURCE_DIR}/" "" targetpath "${_src_dir}")
    if("${_src_name}" MATCHES ".h$")
      if(NOT "${_src_name}" MATCHES "_p.h$")
        install(FILES ${_src_file} DESTINATION "include/${targetpath}")
      endif()
    endif()
  endforeach()

  # Add `asmjit` tests and samples.
  if(ASMJIT_BUILD_TEST)
    cxx_detect_standard(CXX_STD)
    cxx_add_source(asmjit ASMJIT_TEST_SRC ../test asmjit_test_unit.cpp broken.cpp broken.h)

    cxx_add_executable(asmjit asmjit_test_unit
      "${ASMJIT_SRC};${ASMJIT_TEST_SRC}"
      "${ASMJIT_DEPS}"
      "${ASMJIT_PRIVATE_CFLAGS};${CXX_DEFINE}ASMJIT_BUILD_TEST;${CXX_DEFINE}ASMJIT_BUILD_EMBED;${CXX_STD}"
      "${ASMJIT_PRIVATE_CFLAGS_DBG}"
      "${ASMJIT_PRIVATE_CFLAGS_REL}")

    foreach(_target asmjit_bench_x86 asmjit_test_opcode asmjit_test_x86_asm asmjit_test_x86_cc asmjit_test_x86_sections)
      cxx_add_executable(asmjit ${_target} "test/${_target}.cpp" "${ASMJIT_LIBS}" "${ASMJIT_CFLAGS};${CXX_STD}" "" "")
    endforeach()
  endif()
endif()
