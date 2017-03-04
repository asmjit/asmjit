cmake_minimum_required(VERSION 3.1)

# Don't create a project if it was already created by another CMakeLists.txt.
# This allows one library to embed another library without a project collision.
if(NOT CMAKE_PROJECT_NAME OR "${CMAKE_PROJECT_NAME}" STREQUAL "asmjit")
  project(asmjit C CXX)
endif()

if (NOT DEFINED ASMJIT_EMBED)
  set(ASMJIT_EMBED FALSE)
endif()

if (NOT DEFINED ASMJIT_STATIC)
  set(ASMJIT_STATIC ${ASMJIT_EMBED})
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

set(ASMJIT_DIR        "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "Location of 'asmjit'")
set(ASMJIT_EMBED      ${ASMJIT_EMBED}             CACHE BOOLEAN "Embed 'asmjit' library (no targets)")
set(ASMJIT_STATIC     ${ASMJIT_STATIC}            CACHE BOOLEAN "Build 'asmjit' library as static")
set(ASMJIT_BUILD_ARM  ${ASMJIT_BUILD_ARM}         CACHE BOOLEAN "Build ARM32/ARM64 backends")
set(ASMJIT_BUILD_X86  ${ASMJIT_BUILD_X86}         CACHE BOOLEAN "Build X86/X64 backends")
set(ASMJIT_BUILD_TEST ${ASMJIT_BUILD_TEST}        CACHE BOOLEAN "Build 'asmjit_test' applications")

# =============================================================================
# [AsmJit - Project]
# =============================================================================

include("${ASMJIT_DIR}/CxxProject.cmake")

cxx_project(asmjit)
cxx_detect_standard(ASMJIT_PRIVATE_CFLAGS)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(ASMJIT_PRIVATE_LFLAGS "/OPT:REF /OPT:ICF")

  list(APPEND ASMJIT_PRIVATE_CFLAGS /GF)
  list(APPEND ASMJIT_PRIVATE_CFLAGS_DBG /GS /GR-)
  list(APPEND ASMJIT_PRIVATE_CFLAGS_REL /Oi /Oy /GS- /GR-)

  # Enable multi-process compilation.
  if(NOT MSVC60 AND NOT MSVC70 AND NOT MSVC71)
    list(APPEND ASMJIT_PRIVATE_CFLAGS /MP)
  endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang)$")
  cxx_detect_cflags(ASMJIT_PRIVATE_CFLAGS
    "-fno-tree-vectorize"
    "-fvisibility=hidden"
    "-Winconsistent-missing-override")
  cxx_detect_cflags(ASMJIT_PRIVATE_CFLAGS_REL
    "-O2" # CMake by default uses -O3, which does nothing useful.
    "-fno-keep-static-consts"
    "-fmerge-all-constants")
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
if(NOT ASMJIT_EMBED)
  list(INSERT ASMJIT_LIBS 0 asmjit)
endif()

foreach(BUILD_OPTION
    ASMJIT_BUILD_ARM
    ASMJIT_BUILD_X86
    ASMJIT_DISABLE_BUILDER
    ASMJIT_DISABLE_COMPILER
    ASMJIT_DISABLE_TEXT
    ASMJIT_DISABLE_LOGGING
    ASMJIT_DISABLE_VALIDATION)
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
  asmjit_apibegin.h
  asmjit_apiend.h
  asmjit_build.h
  base.h
  arm.h
  x86.h
)

cxx_add_source(asmjit ASMJIT_SRC asmjit/base
  arch.cpp
  arch.h
  assembler.cpp
  assembler.h
  codebuilder.cpp
  codebuilder.h
  codecompiler.cpp
  codecompiler.h
  codeemitter.cpp
  codeemitter.h
  codeholder.cpp
  codeholder.h
  constpool.cpp
  constpool.h
  cpuinfo.cpp
  cpuinfo.h
  func.cpp
  func.h
  globals.cpp
  globals.h
  logging.cpp
  logging.h
  misc_p.h
  operand.cpp
  operand.h
  osutils.cpp
  osutils.h
  regalloc.cpp
  regalloc_p.h
  runtime.cpp
  runtime.h
  simdtypes.h
  string.cpp
  string.h
  utils.cpp
  utils.h
  vmem.cpp
  vmem.h
  zone.cpp
  zone.h
)

if(0)
cxx_add_source(asmjit ASMJIT_SRC asmjit/arm
  armassembler.cpp
  armassembler.h
  arminst.cpp
  arminst.h
  armoperand.cpp
  armoperand_regs.cpp
  armoperand.h
)
endif()

cxx_add_source(asmjit ASMJIT_SRC asmjit/x86
  x86assembler.cpp
  x86assembler.h
  x86builder.cpp
  x86builder.h
  x86compiler.cpp
  x86compiler.h
  x86emitter.h
  x86globals.h
  x86internal.cpp
  x86internal_p.h
  x86inst.cpp
  x86inst.h
  x86logging.cpp
  x86logging_p.h
  x86misc.h
  x86operand.cpp
  x86operand_regs.cpp
  x86operand.h
  x86regalloc.cpp
  x86regalloc_p.h
)

# =============================================================================
# [AsmJit - Targets]
# =============================================================================

if(NOT ASMJIT_EMBED)
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
    cxx_add_source(asmjit ASMJIT_TEST_SRC ../test asmjit_test_unit.cpp broken.cpp broken.h)
    cxx_add_executable(asmjit asmjit_test_unit
      "${ASMJIT_SRC};${ASMJIT_TEST_SRC}"
      "${ASMJIT_DEPS}"
      "${ASMJIT_PRIVATE_CFLAGS};${CXX_DEFINE}ASMJIT_TEST;${CXX_DEFINE}ASMJIT_EMBED"
      "${ASMJIT_PRIVATE_CFLAGS_DBG}"
      "${ASMJIT_PRIVATE_CFLAGS_REL}")

    foreach(_target asmjit_bench_x86 asmjit_test_opcode asmjit_test_x86_asm asmjit_test_x86_cc)
      cxx_add_executable(asmjit ${_target} "test/${_target}.cpp" "${ASMJIT_LIBS}" "${ASMJIT_CFLAGS}" "" "")
    endforeach()
  endif()
endif()
