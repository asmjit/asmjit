# =============================================================================
# [AsmJit - CMakeLists.txt]
# =============================================================================

CMake_Minimum_Required(VERSION 2.8.12)

# =============================================================================
# [AsmJit - Configuration]
# =============================================================================

# Whether to build static library (default FALSE).
# Set(ASMJIT_STATIC FALSE)

# Whether to build samples (default FALSE).
# Set(ASMJIT_BUILD_SAMPLES FALSE)

# =============================================================================
# [AsmJit - Build]
# =============================================================================

If(NOT CMAKE_PROJECT_NAME)
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

# MSVC.
If(MSVC)
  Message("-- Using MSVC")
  Set(ASMJIT_LFLAGS "/OPT:REF /OPT:ICF")
  Set(ASMJIT_CFLAGS /GF)
  Set(ASMJIT_CFLAGS_DBG /DASMJIT_DEBUG /GS /GR-)
  Set(ASMJIT_CFLAGS_REL /DASMJIT_RELEASE /Oi /Oy /GS- /GR-)

  # Use Unicode by default on Windows target.
  If(WIN32)
    List(APPEND ASMJIT_CFLAGS /D_UNICODE)
  EndIf()
  If(ASMJIT_STATIC)
    List(APPEND ASMJIT_CFLAGS /DASMJIT_STATIC)
  EndIf(ASMJIT_STATIC)
EndIf()

# GCC.
If(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  Message("-- Using GCC")
  Set(ASMJIT_CFLAGS
    -fno-exceptions)
  Set(ASMJIT_CFLAGS_DBG -DASMJIT_DEBUG -O0
    -fno-inline-functions)
  Set(ASMJIT_CFLAGS_REL -DASMJIT_RELEASE -O2
    -finline-functions
    -fomit-frame-pointer
    -fmerge-all-constants
    -fno-keep-static-consts)

  # Use Unicode by default on Windows target.
  If(WIN32)
    List(APPEND ASMJIT_CFLAGS -D_UNICODE)
  EndIf()
  If(ASMJIT_STATIC)
    List(APPEND ASMJIT_CFLAGS -DASMJIT_STATIC)
  EndIf(ASMJIT_STATIC)
EndIf()

# Dependencies - Base.
If(NOT WIN32)
  List(APPEND ASMJIT_DEPS pthread)
EndIf()

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
    Target_Compile_Options(${in_name} PUBLIC
      ${in_cflags}
      $<$<CONFIG:Debug>:${in_cflags_dbg}>
      $<$<NOT:$<CONFIG:Debug>>:${in_cflags_rel}>)
  EndIf()

  # Linker Flags.
  Set_Target_Properties(${in_name} PROPERTIES LINK_FLAGS "${ASMJIT_LFLAGS}")

  # Install Instructions.
  If(NOT ASMJIT_STATIC)
    Install(TARGETS ${in_name} DESTINATION lib)
  EndIf()

  Unset(__type)
EndMacro()

# =============================================================================
# [AsmJit - Source]
# =============================================================================

Set(ASMJIT_SRC "")

AsmJit_AddSource(ASMJIT_SRC asmjit
  asmjit.h
  base.h
  build.h
  config.h
  contrib.h
  host.h
  x86.h
)

AsmJit_AddSource(ASMJIT_SRC asmjit/base
  apibegin.h
  apiend.h

  assembler.cpp
  assembler.h
  codegen.cpp
  codegen.h
  compiler.cpp
  compiler.h
  context.cpp
  context_p.h
  cpu.cpp
  cpu.h
  defs.cpp
  defs.h
  error.cpp
  error.h
  func.cpp
  func.h
  globals.cpp
  globals.h
  intutil.h
  lock.h
  logger.cpp
  logger.h
  memorymanager.cpp
  memorymanager.h
  podlist.h
  podvector.cpp
  podvector.h
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
  x86cpu.cpp
  x86cpu.h
  x86defs.cpp
  x86defs.h
  x86func.cpp
  x86func.h
)

AsmJit_AddSource(ASMJIT_SRC asmjit/contrib
  winremoteruntime.cpp
  winremoteruntime.h
)

# =============================================================================
# [AsmJit - Headers]
# =============================================================================

If(NOT ASMJIT_STATIC)
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

AsmJit_AddLibrary(asmjit
  "${ASMJIT_SRC}"
  "${ASMJIT_DEPS}"
  "${ASMJIT_CFLAGS}"
  "${ASMJIT_CFLAGS_DBG}"
  "${ASMJIT_CFLAGS_REL}"
)

# =============================================================================
# [Asmjit - Samples]
# =============================================================================

If(ASMJIT_BUILD_SAMPLES)
  Set(ASMJIT_SRC_SAMPLES
    benchx86
    testcpu
    testdummy
    testmem
    testopcode
    testsizeof
    testx86
  )

  ForEach(file ${ASMJIT_SRC_SAMPLES})
    Add_Executable(${file} src/app/test/${file}.cpp)
    Target_Link_Libraries(${file} asmjit ${ASMJIT_DEPS})
  EndForEach(file)
EndIf()
