# =============================================================================
# [AsmJit - CMakeLists.txt]
# =============================================================================

CMake_Minimum_Required(VERSION 2.8.12)

# =============================================================================
# [AsmJit - Configuration]
# =============================================================================

# Whether not to build anything (default FALSE).
# Set(ASMJIT_EMBED FALSE)

# Whether to build static library (default FALSE).
# Set(ASMJIT_STATIC FALSE)

# Whether to build tests (default FALSE).
# Set(ASMJIT_BUILD_TEST FALSE)

# Whether to build samples (default FALSE).
# Set(ASMJIT_BUILD_SAMPLES FALSE)

# =============================================================================
# [AsmJit - Build]
# =============================================================================

If(ASMJIT_EMBED)
  Set(ASMJIT_STATIC TRUE)
EndIf()

If(NOT CMAKE_PROJECT_NAME OR CMAKE_PROJECT_NAME MATCHES "^asmjit$")
  Project(asmjit C CXX)
  Set(ASMJIT_PROJECT_STR "Project")
Else()
  # Do not create a project if this CMakeLists.txt is included by a different
  # project. This allows easy static library build including debugger support.
  Set(ASMJIT_PROJECT_STR "Include")
EndIf()

If(ASMJIT_STATIC)
  Set(ASMJIT_PROJECT_STR "${ASMJIT_PROJECT_STR}|Static")
Else()
  Set(ASMJIT_PROJECT_STR "${ASMJIT_PROJECT_STR}|Shared")
EndIf()

Message("")
Message("== ====================================================")
Message("== [AsmJit ${ASMJIT_PROJECT_STR}]")
Message("== ====================================================")
Message("")

# =============================================================================
# [AsmJit - Directories]
# =============================================================================

If(NOT ASMJIT_DIR)
  Set(ASMJIT_DIR ${CMAKE_CURRENT_LIST_DIR})
  Message("-- Initializing ASMJIT_DIR=${ASMJIT_DIR}")
Else()
  Message("-- Using Custom ASMJIT_DIR=${ASMJIT_DIR}")
EndIf()

Set(ASMJIT_SRC_DIR "${ASMJIT_DIR}/src")
Set(ASMJIT_INC_DIR "${ASMJIT_SRC_DIR}")

Include_Directories(${ASMJIT_SRC_DIR})

# =============================================================================
# [AsmJit - Flags/Deps]
# =============================================================================

Set(ASMJIT_DEPS)
Set(ASMJIT_LFLAGS)

Set(ASMJIT_CFLAGS)
Set(ASMJIT_CFLAGS_DBG)
Set(ASMJIT_CFLAGS_REL)

Set(ASMJIT_DEFINE "-D")

# MSVC.
If("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  Message("-- Using MSVC")

  Set(ASMJIT_DEFINE "/D")
  Set(ASMJIT_LFLAGS "/OPT:REF /OPT:ICF")
  Set(ASMJIT_CFLAGS /GF)
  Set(ASMJIT_CFLAGS_DBG /DASMJIT_DEBUG /GS /GR-)
  Set(ASMJIT_CFLAGS_REL /DASMJIT_RELEASE /Oi /Oy /GS- /GR-)

  # Enable multi-process compilation.
  If(NOT MSVC60 AND NOT MSVC70 AND NOT MSVC71)
     List(APPEND ASMJIT_CFLAGS /MP)
  EndIf()
EndIf()

# GCC
If("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  Message("-- Using GCC")

  Set(ASMJIT_CFLAGS
    -fno-exceptions)
  Set(ASMJIT_CFLAGS_DBG
    -DASMJIT_DEBUG -O0)
  Set(ASMJIT_CFLAGS_REL
    -DASMJIT_RELEASE -O2
    -finline-functions
    -fomit-frame-pointer
    -fmerge-all-constants
    -fno-keep-static-consts)
EndIf()

# Clang.
If("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  Message("-- Using Clang")

  Set(ASMJIT_CFLAGS
    -fno-exceptions)
  Set(ASMJIT_CFLAGS_DBG
    -DASMJIT_DEBUG -O0)
  Set(ASMJIT_CFLAGS_REL
    -DASMJIT_RELEASE -O2
    -finline-functions
    -fomit-frame-pointer
    -fmerge-all-constants)
EndIf()

# Use Unicode by default on Windows target.
If(WIN32)
  List(APPEND ASMJIT_CFLAGS "${ASMJIT_DEFINE}_UNICODE")
EndIf()

# Static library.
If(ASMJIT_STATIC)
  List(APPEND ASMJIT_CFLAGS "${ASMJIT_DEFINE}ASMJIT_STATIC")
EndIf()

# Dependencies - pthread (Unix).
If(NOT WIN32)
  List(APPEND ASMJIT_DEPS pthread)
EndIf()

# Dependencies - librt (Linux).
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  LIST(APPEND ASMJIT_DEPS rt)
ENDIF()

Set(ASMJIT_CFLAGS_DBG ${ASMJIT_CFLAGS} ${ASMJIT_CFLAGS_DBG})
Set(ASMJIT_CFLAGS_REL ${ASMJIT_CFLAGS} ${ASMJIT_CFLAGS_REL})

# =============================================================================
# [AsmJit - Macros]
# =============================================================================

Macro(AsmJit_AddSource in_dst in_path)
  Set(__list "")
  Set(__path "${ASMJIT_SRC_DIR}/${in_path}")

  ForEach(__name ${ARGN})
    Set(__file "${__path}/${__name}")
    Set(__cflags ${ASMJIT_CFLAGS})

    If(__name MATCHES "\\.cpp|\\.h")
      If("${__cflags}")
        Set_Source_Files_Properties(${__name} PROPERTIES COMPILE_FLAGS ${__cflags})
      EndIf()
      List(APPEND __list ${__file})
    EndIf()
  EndForEach()

  List(APPEND "${in_dst}" ${__list})
  Source_Group(${in_path} FILES ${__list})
EndMacro()

Macro(AsmJit_AddLibrary in_name in_src in_deps in_cflags in_cflags_dbg in_cflags_rel)
  If(NOT ASMJIT_STATIC)
    Set(__type "SHARED")
  Else()
    Set(__type "STATIC")
  EndIf()

  Add_Library(${in_name} ${__type} ${in_src})

  # Dependencies.
  Target_Link_Libraries(${in_name} ${in_deps})

  # Compiler Flags.
  If(${CMAKE_BUILD_TYPE})
    If(${CMAKE_BUILD_TYPE} MATCHES "Debug")
      Set_Target_Properties(${in_name} PROPERTIES COMPILE_FLAGS ${in_cflags} ${in_cflags_dbg})
    Else()
      Set_Target_Properties(${in_name} PROPERTIES COMPILE_FLAGS ${in_cflags} ${in_cflags_rel})
    EndIf()
  Else()
    Target_Compile_Options(${in_name} PUBLIC ${in_cflags}
      $<$<CONFIG:Debug>:${in_cflags_dbg}>
      $<$<NOT:$<CONFIG:Debug>>:${in_cflags_rel}>)
  EndIf()

  # Linker Flags.
  Set_Target_Properties(${in_name} PROPERTIES LINK_FLAGS "${ASMJIT_LFLAGS}")

  # Install Instructions.
  If(NOT ASMJIT_EMBED)
    Install(TARGETS ${in_name} LIBRARY DESTINATION lib${LIB_SUFFIX}
                               ARCHIVE DESTINATION lib${LIB_SUFFIX}
                               RUNTIME DESTINATION bin)
  EndIf()

  Unset(__type)
EndMacro()

# =============================================================================
# [AsmJit - Source]
# =============================================================================

Set(ASMJIT_SRC "")

AsmJit_AddSource(ASMJIT_SRC asmjit
  apibegin.h
  apiend.h

  asmjit.h
  base.h
  build.h
  config.h
  host.h
  x86.h
)

AsmJit_AddSource(ASMJIT_SRC asmjit/base
  assembler.cpp
  assembler.h
  codegen.cpp
  codegen.h
  compiler.cpp
  compiler.h
  constpool.cpp
  constpool.h
  containers.cpp
  containers.h
  context.cpp
  context_p.h
  cpuinfo.cpp
  cpuinfo.h
  cputicks.cpp
  cputicks.h
  error.cpp
  error.h
  globals.cpp
  globals.h
  intutil.cpp
  intutil.h
  lock.h
  logger.cpp
  logger.h
  operand.cpp
  operand.h
  runtime.cpp
  runtime.h
  string.cpp
  string.h
  vectypes.h
  vmem.cpp
  vmem.h
  zone.cpp
  zone.h
)

AsmJit_AddSource(ASMJIT_SRC asmjit/x86
  x86assembler.cpp
  x86assembler.h
  x86compiler.cpp
  x86compiler.h
  x86context.cpp
  x86context_p.h
  x86cpuinfo.cpp
  x86cpuinfo.h
  x86inst.cpp
  x86inst.h
  x86operand.cpp
  x86operand_regs.cpp
  x86operand.h
  x86scheduler.cpp
  x86scheduler_p.h
)

# =============================================================================
# [AsmJit - Headers]
# =============================================================================

If(NOT ASMJIT_EMBED)
  ForEach(i ${ASMJIT_SRC})
    Get_Filename_Component(path ${i} PATH)
    Get_Filename_Component(name ${i} NAME)
    String(REGEX REPLACE "^${ASMJIT_SRC_DIR}/" "" targetpath "${path}")
    If(name MATCHES ".h$")
      If(NOT name MATCHES "_p.h$")
        Install(FILES ${i} DESTINATION "include/${targetpath}")
      EndIf()
    EndIf()
  EndForEach()
EndIf()

# =============================================================================
# [Asmjit - Library]
# =============================================================================

If(NOT ASMJIT_EMBED)
  AsmJit_AddLibrary(asmjit
    "${ASMJIT_SRC}"
    "${ASMJIT_DEPS}"
    "${ASMJIT_CFLAGS}"
    "${ASMJIT_CFLAGS_DBG}"
    "${ASMJIT_CFLAGS_REL}"
  )
EndIf()

# =============================================================================
# [Asmjit - Testing]
# =============================================================================

# AsmJit library is always embedded into the tests executable. This way it's
# much easier to test private functions compared to just linking to AsmJit.
If(ASMJIT_BUILD_TEST)
  AsmJit_AddSource(ASMJIT_TEST_SRC asmjit/test broken.cpp broken.h main.cpp)

  Set(ASMJIT_TEST_CFLAGS
    ${ASMJIT_CFLAGS}
    ${ASMJIT_DEFINE}ASMJIT_STATIC
    ${ASMJIT_DEFINE}ASMJIT_TEST)

  Add_Executable(asmjit_test ${ASMJIT_SRC} ${ASMJIT_TEST_SRC})
  Target_Link_Libraries(asmjit_test ${ASMJIT_DEPS})

  If(${CMAKE_BUILD_TYPE})
    If(${CMAKE_BUILD_TYPE} MATCHES "Debug")
      Set_Target_Properties(asmjit_test PROPERTIES COMPILE_FLAGS ${ASMJIT_TEST_CFLAGS} ${ASMJIT_CFLAGS_DBG})
    Else()
      Set_Target_Properties(asmjit_test PROPERTIES COMPILE_FLAGS ${ASMJIT_TEST_CFLAGS} ${ASMJIT_CFLAGS_REL})
    EndIf()
  Else()
    Target_Compile_Options(asmjit_test PUBLIC ${ASMJIT_TEST_CFLAGS}
      $<$<CONFIG:Debug>:${ASMJIT_CFLAGS_DBG}>
      $<$<NOT:$<CONFIG:Debug>>:${ASMJIT_CFLAGS_REL}>)
  EndIf()

  Set_Target_Properties(asmjit_test PROPERTIES LINK_FLAGS "${ASMJIT_LFLAGS}")
EndIf()

# =============================================================================
# [Asmjit - Samples]
# =============================================================================

If(ASMJIT_BUILD_SAMPLES)
  Set(ASMJIT_SRC_SAMPLES
    benchx86
    testopcode
    testx86
  )

  ForEach(file ${ASMJIT_SRC_SAMPLES})
    Add_Executable(${file} src/app/test/${file}.cpp)
    Target_Link_Libraries(${file} asmjit ${ASMJIT_DEPS})
  EndForEach(file)
EndIf()
