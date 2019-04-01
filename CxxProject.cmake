# CxxProject 1.0.0
# ----------------

if (NOT __CXX_INCLUDED)
  set(__CXX_INCLUDED TRUE)
  include(CheckCXXCompilerFlag)

  # ---------------------------------------------------------------------------
  # C++ COMPILER SUPPORT:
  #
  #   * cxx_detect_cflags(out, ...)
  #   * cxx_detect_standard(out)
  # ---------------------------------------------------------------------------
  function(cxx_detect_cflags out)
    set(out_array ${${out}})

    list(GET ARGN 0 FIRST)
    if("${FIRST}" STREQUAL "FIRST")
      list(REMOVE_AT ARGN 0)
    endif()

    foreach(flag ${ARGN})
      string(REGEX REPLACE "[+]" "x" flag_signature "${flag}")
      string(REGEX REPLACE "[-=:;/.\]" "_" flag_signature "${flag_signature}")
      check_cxx_compiler_flag(${flag} "__CxxFlag_${flag_signature}")
      if(${__CxxFlag_${flag_signature}})
        list(APPEND out_array "${flag}")
        if("${FIRST}" STREQUAL "FIRST")
          break()
        endif()
      endif()
    endforeach()

    set(${out} "${out_array}" PARENT_SCOPE)
  endfunction()

  function(cxx_detect_standard out)
    set(out_array)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      cxx_detect_cflags(out_array FIRST "/std:c++latest" "/std:c++14")
    else()
      cxx_detect_cflags(out_array FIRST "-std=c++17" "-std=c++14" "-std=c++11")
    endif()

    set(out_array ${${out}} ${out_array})
    set(${out} "${out_array}" PARENT_SCOPE)
  endfunction()

  function(cxx_detect_cpu_features out)
    # Flags to be exported.
    set(cxx_cpu_features SSE2 SSE3 SSSE3 SSE4_1 SSE4_2 AVX AVX2)

    foreach(feature ${cxx_cpu_features})
      set("cxx_cpu_feature_${feature}" "")
    endforeach()

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      # AVX/AVX2 doesn't need custom defs as MSVC does define __AVX[2]__
      # similary to other compilers. In addition, we only detect the support
      # for AVX/AVX2 as if these are available all previous instruction sets
      # are also available. If such check fails it means that we are either
      # not compiling for X86/X64 or the compiler is very old, which cannot
      # be used anyway.
      cxx_detect_cflags(cxx_cpu_feature_AVX "/arch:AVX")
      cxx_detect_cflags(cxx_cpu_feature_AVX2 "/arch:AVX2")

      if(cxx_cpu_feature_AVX)
        # 64-bit MSVC compiler doesn't like /arch:SSE[2] as it's implicit.
        if(NOT CMAKE_CL_64)
          list(APPEND cxx_cpu_feature_SSE2 "/arch:SSE2")
          list(APPEND cxx_cpu_feature_SSE3 "/arch:SSE2")
          list(APPEND cxx_cpu_feature_SSSE3 "/arch:SSE2")
          list(APPEND cxx_cpu_feature_SSE4_1 "/arch:SSE2")
          list(APPEND cxx_cpu_feature_SSE4_2 "/arch:SSE2")
        endif()
        # MSVC doesn't provide any preprocessor definitions for SSE3 and higher, thus
        # we have to define these ourselves to match these defined by Intel|Clang|GCC.
        list(APPEND cxx_cpu_feature_SSE2 "/D__SSE2__")
        list(APPEND cxx_cpu_feature_SSE3 "/D__SSE3__")
        list(APPEND cxx_cpu_feature_SSSE3 "/D__SSSE3__")
        list(APPEND cxx_cpu_feature_SSE4_1 "/D__SSE4_1__")
        list(APPEND cxx_cpu_feature_SSE4_2 "/D__SSE4_2__")
      endif()
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" AND WIN32)
      # Intel deprecated /arch:SSE, so it's implicit. In contrast to MSVC, Intel
      # also provides /arch:SSE3+ options and uses the same definitions as GCC
      # and Clang, so no magic needed here.
      cxx_detect_cflags(cxx_cpu_feature_AVX "/arch:AVX")
      cxx_detect_cflags(cxx_cpu_feature_AVX2 "/arch:AVX2")
      if(cxx_cpu_feature_AVX)
        list(APPEND cxx_cpu_feature_SSE2 "/arch:SSE2")
        list(APPEND cxx_cpu_feature_SSE3 "/arch:SSE3")
        list(APPEND cxx_cpu_feature_SSSE3 "/arch:SSSE3")
        list(APPEND cxx_cpu_feature_SSE4_1 "/arch:SSE4.1")
        list(APPEND cxx_cpu_feature_SSE4_2 "/arch:SSE4.2")
      endif()
    else()
      # Assume all other compilers are compatible with GCC|Clang.
      cxx_detect_cflags(cxx_cpu_feature_AVX "-mavx")
      cxx_detect_cflags(cxx_cpu_feature_AVX2 "-mavx2")
      if(cxx_cpu_feature_AVX)
        list(APPEND cxx_cpu_feature_SSE2 "-msse2")
        list(APPEND cxx_cpu_feature_SSE3 "-msse3")
        list(APPEND cxx_cpu_feature_SSSE3 "-mssse3")
        list(APPEND cxx_cpu_feature_SSE4_1 "-msse4.1")
        list(APPEND cxx_cpu_feature_SSE4_2 "-msse4.2")
      endif()
    endif()

    foreach(feature ${cxx_cpu_features})
      set("${out}_${feature}" "${cxx_cpu_feature_${feature}}" PARENT_SCOPE)
    endforeach()
  endfunction()

  function(cxx_print_cflags cflags_any cflags_dbg cflags_rel)
    foreach(flag ${cflags_any})
      message("     ${flag}")
    endforeach()
    foreach(flag ${cflags_dbg})
      message("     ${flag} [DEBUG]")
    endforeach()
    foreach(flag ${cflags_rel})
      message("     ${flag} [RELEASE]")
    endforeach()
  endfunction()

  # ---------------------------------------------------------------------------
  # This part detects the c++ compiler and fills basic CXX_... variables to make
  # integration with that compiler easier. It provides the most common flags in
  # a cross-platform way.
  # ---------------------------------------------------------------------------
  set(CXX_DEFINE "-D")  # Define a preprocessor macro: "${CXX_DEFINE}VAR=1"
  set(CXX_INCLUDE "-I") # Define an include directory: "${CXX_INCLUDE}PATH"

  if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") OR
     ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" AND WIN32))
    set(CXX_DEFINE "/D")
    set(CXX_INCLUDE "/I")
  endif()

  # ---------------------------------------------------------------------------
  # Function
  #   cxx_project(product)
  #
  # Create a master project or embed other project in a master project.
  # ---------------------------------------------------------------------------
  function(cxx_project product)
    string(TOUPPER "${product}" PRODUCT)

    set(MODE_EMBED ${${PRODUCT}_BUILD_EMBED})
    set(MODE_STATIC ${${PRODUCT}_BUILD_STATIC})

    # EMBED implies STATIC.
    if(MODE_EMBED)
      set(MODE_STATIC TRUE)
      set(${PRODUCT}_BUILD_STATIC TRUE PARENT_SCOPE)
    endif()

    # Deduce source and include directories. By default CxxProject assumes that
    # both source and include files are located at './src'.
    set(SOURCE_DIR "${${PRODUCT}_SOURCE_DIR}")
    set(INCLUDE_DIR "${${PRODUCT}_INCLUDE_DIR}")

    if(NOT SOURCE_DIR)
      set(SOURCE_DIR "${${PRODUCT}_DIR}/src")
      set(${PRODUCT}_SOURCE_DIR "${SOURCE_DIR}" PARENT_SCOPE)
    endif()

    if(NOT INCLUDE_DIR)
      set(INCLUDE_DIR "${SOURCE_DIR}")
      set(${PRODUCT}_INCLUDE_DIR "${INCLUDE_DIR}" PARENT_SCOPE)
    endif()

    set(DEPS               "") # Dependencies (list of libraries) for the linker.
    set(LIBS               "") # Dependencies with project included, for consumers.
    set(CFLAGS             "") # Public compiler flags.
    set(PRIVATE_CFLAGS     "") # Private compiler flags independent of build type.
    set(PRIVATE_CFLAGS_DBG "") # Private compiler flags used by debug builds.
    set(PRIVATE_CFLAGS_REL "") # Private compiler flags used by release builds.
    set(PRIVATE_LFLAGS     "") # Private linker flags.

    if(MODE_EMBED)
      list(APPEND CFLAGS         "${CXX_DEFINE}${PRODUCT}_BUILD_EMBED")
      list(APPEND PRIVATE_CFLAGS "${CXX_DEFINE}${PRODUCT}_BUILD_EMBED")
    endif()

    if(MODE_STATIC)
      list(APPEND CFLAGS         "${CXX_DEFINE}${PRODUCT}_BUILD_STATIC")
      list(APPEND PRIVATE_CFLAGS "${CXX_DEFINE}${PRODUCT}_BUILD_STATIC")
    endif()

    # PUBLIC properties - usable by third parties.
    set(${PRODUCT}_DEPS   "${DEPS}"   PARENT_SCOPE)
    set(${PRODUCT}_LIBS   "${LIBS}"   PARENT_SCOPE)
    set(${PRODUCT}_CFLAGS "${CFLAGS}" PARENT_SCOPE)

    # PRIVATE properties - only used during build.
    set(${PRODUCT}_PRIVATE_CFLAGS     "${PRIVATE_CFLAGS}"     PARENT_SCOPE)
    set(${PRODUCT}_PRIVATE_CFLAGS_DBG "${PRIVATE_CFLAGS_DBG}" PARENT_SCOPE)
    set(${PRODUCT}_PRIVATE_CFLAGS_REL "${PRIVATE_CFLAGS_REL}" PARENT_SCOPE)
    set(${PRODUCT}_PRIVATE_LFLAGS     "${PRIVATE_LFLAGS}"     PARENT_SCOPE)
  endfunction()

  function(cxx_project_info product)
    string(TOUPPER "${product}" PRODUCT)

    set(BUILD_MODE "")
    set(BUILD_TEST "")

    if(${PRODUCT}_BUILD_EMBED)
      set(BUILD_MODE "Embed")
    elseif(${PRODUCT}_BUILD_STATIC)
      set(BUILD_MODE "Static")
    else()
      set(BUILD_MODE "Shared")
    endif()

    if(${PRODUCT}_BUILD_TEST)
      set(BUILD_TEST "On")
    else()
      set(BUILD_TEST "Off")
    endif()

    message("-- [${product}]")
    message("   BuildMode=${BUILD_MODE}")
    message("   BuildTest=${BUILD_TEST}")
    message("   ${PRODUCT}_DIR=${${PRODUCT}_DIR}")
    message("   ${PRODUCT}_DEPS=${${PRODUCT}_DEPS}")
    message("   ${PRODUCT}_LIBS=${${PRODUCT}_LIBS}")
    message("   ${PRODUCT}_CFLAGS=${${PRODUCT}_CFLAGS}")
    message("   ${PRODUCT}_SOURCE_DIR=${${PRODUCT}_SOURCE_DIR}")
    message("   ${PRODUCT}_INCLUDE_DIR=${${PRODUCT}_INCLUDE_DIR}")
    message("   ${PRODUCT}_PRIVATE_CFLAGS=")
    cxx_print_cflags(
      "${${PRODUCT}_PRIVATE_CFLAGS}"
      "${${PRODUCT}_PRIVATE_CFLAGS_DBG}"
      "${${PRODUCT}_PRIVATE_CFLAGS_REL}")
  endfunction()

  function(cxx_add_source product out src_dir)
    string(TOUPPER "${product}" PRODUCT)

    set(src_path "${${PRODUCT}_SOURCE_DIR}/${src_dir}")
    set(src_array)

    foreach(file ${ARGN})
      set(src_file "${src_path}/${file}")
      set(src_cflags "")

      foreach(feature sse sse2 sse3 ssse3 sse4_1 sse4_2 avx avx2)
        if(file MATCHES "_${feature}\\.(c|cc|cxx|cpp|m|mm)")
          string(TOUPPER "${feature}" FEATURE)
          if(NOT "${${PRODUCT}_CFLAGS_${FEATURE}}" STREQUAL "")
            list(APPEND src_cflags ${${PRODUCT}_CFLAGS_${FEATURE}})
            # message("[debug] ${file} [cflags] += ${${PRODUCT}_CFLAGS_${FEATURE}}")
          endif()
        endif()
      endforeach()

      # HACK: Setting `COMPILE_FLAGS` property cannot be used when your input
      # is LIST, even when you use `VALUE1 VALUE2 ...` as cmake would insert
      # escaped semicolons instead of spaces. So let's make it the cmake way:
      #   - nonintuitive, verbose, and idiotic.
      foreach(src_cflag ${src_cflags})
        set_property(SOURCE "${src_file}" APPEND_STRING PROPERTY COMPILE_FLAGS " ${src_cflag}")
      endforeach()
      list(APPEND src_array ${src_file})
    endforeach()
    source_group(${src_dir} FILES ${src_array})

    set(out_tmp ${${out}})
    list(APPEND out_tmp ${src_array})
    set("${out}" "${out_tmp}" PARENT_SCOPE)
  endfunction()

  function(cxx_add_library product target src deps cflags cflags_dbg cflags_rel)
    string(TOUPPER "${product}" PRODUCT)

    if(NOT ${PRODUCT}_BUILD_STATIC)
      add_library(${target} SHARED ${src})
    else()
      add_library(${target} STATIC ${src})
    endif()

    target_link_libraries(${target} ${deps})
    foreach(link_flag ${${PRODUCT}_PRIVATE_LFLAGS})
      set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " ${link_flag}")
    endforeach()

    if(CMAKE_BUILD_TYPE)
      if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        target_compile_options(${target} PRIVATE ${cflags} ${cflags_dbg})
      else()
        target_compile_options(${target} PRIVATE ${cflags} ${cflags_rel})
      endif()
    else()
      target_compile_options(${target} PRIVATE ${cflags} $<$<CONFIG:Debug>:${cflags_dbg}> $<$<NOT:$<CONFIG:Debug>>:${cflags_rel}>)
    endif()

    if(NOT ${PRODUCT}_BUILD_STATIC)
      install(TARGETS ${target} RUNTIME DESTINATION "bin"
                                LIBRARY DESTINATION "lib${LIB_SUFFIX}"
                                ARCHIVE DESTINATION "lib${LIB_SUFFIX}")
    endif()
  endfunction()

  function(cxx_add_executable product target src deps cflags cflags_dbg cflags_rel)
    string(TOUPPER "${product}" PRODUCT)
    add_executable(${target} ${src})

    target_link_libraries(${target} ${deps})
    foreach(link_flag ${${PRODUCT}_PRIVATE_LFLAGS})
      set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " ${link_flag}")
    endforeach()

    if(CMAKE_BUILD_TYPE)
      if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        target_compile_options(${target} PRIVATE ${cflags} ${cflags_dbg})
      else()
        target_compile_options(${target} PRIVATE ${cflags} ${cflags_rel})
      endif()
    else()
      target_compile_options(${target} PRIVATE ${cflags} $<$<CONFIG:Debug>:${cflags_dbg}> $<$<NOT:$<CONFIG:Debug>>:${cflags_rel}>)
    endif()

    if(NOT ${PRODUCT}_BUILD_STATIC)
      install(TARGETS ${target} DESTINATION "lib${LIB_SUFFIX}")
    endif()
  endfunction()
endif()
