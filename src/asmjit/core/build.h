// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// AsmJit Static Builds and Embedding
// ----------------------------------
//
// These definitions can be used to enable static library build. Embed is used
// when AsmJit's source code is embedded directly in another project, implies
// static build as well.
//
// #define ASMJIT_BUILD_EMBED        // Asmjit is embedded (implies ASMJIT_BUILD_STATIC).
// #define ASMJIT_BUILD_STATIC       // Enable static-library build.

// AsmJit Build Mode
// -----------------
//
// These definitions control the build mode and tracing support. The build mode
// should be auto-detected at compile time, but it's possible to override it in
// case that the auto-detection fails.
//
// Tracing is a feature that is never compiled by default and it's only used to
// debug AsmJit itself.
//
// #define ASMJIT_BUILD_DEBUG        // Always use debug-mode   (ASMJIT_ASSERT enabled).
// #define ASMJIT_BUILD_RELEASE      // Always use release-mode (ASMJIT_ASSERT disabled).

// AsmJit Build Backends
// ---------------------
//
// These definitions control which backends to compile. If none of these is
// defined AsmJit will use host architecture by default (for JIT code generation).
//
// #define ASMJIT_BUILD_X86          // Enable X86 targets (X86 and X86_64).
// #define ASMJIT_BUILD_ARM          // Enable ARM targets (ARM and AArch64).
// #define ASMJIT_BUILD_HOST         // Enable targets based on target arch (default).

// AsmJit Build Options
// --------------------
//
// Flags can be defined to disable standard features. These are handy especially
// when building AsmJit statically and some features are not needed or unwanted
// (like BaseCompiler).
//
// AsmJit features are enabled by default.
// #define ASMJIT_DISABLE_BUILDER    // Disable Builder (completely).
// #define ASMJIT_DISABLE_COMPILER   // Disable Compiler (completely).
// #define ASMJIT_DISABLE_JIT        // Disable JIT memory manager and JitRuntime.
// #define ASMJIT_DISABLE_LOGGING    // Disable logging and formatting (completely).
// #define ASMJIT_DISABLE_TEXT       // Disable everything that contains text
//                                   // representation (instructions, errors, ...).
// #define ASMJIT_DISABLE_INST_API   // Disable API related to instruction database
//                                   // (validation, cpu features, rw-info, etc).

#ifndef _ASMJIT_CORE_BUILD_H
#define _ASMJIT_CORE_BUILD_H

// ============================================================================
// [asmjit::Version]
// ============================================================================

#define ASMJIT_LIBRARY_VERSION 0x010200 /* 1.2.0 */

// ============================================================================
// [asmjit::Dependencies]
// ============================================================================

// We really want std-types as globals.
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits>
#include <type_traits>
#include <utility>

#if defined(_WIN32)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #define ASMJIT_UNDEF_WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
    #define ASMJIT_UNDEF_NOMINMAX
  #endif
  #include <windows.h>
  #ifdef ASMJIT_UNDEF_WIN32_LEAN_AND_MEAN
    #undef WIN32_LEAN_AND_MEAN
    #undef ASMJIT_UNDEF_WIN32_LEAN_AND_MEAN
  #endif
  #ifdef ASMJIT_UNDEF_NOMINMAX
    #undef NOMINMAX
    #undef ASMJIT_UNDEF_NOMINMAX
  #endif
#else
  #include <pthread.h>
#endif

// ============================================================================
// [asmjit::Build - Globals - Deprecated]
// ============================================================================

// DEPRECATED: Will be removed from v2.0+.
#if defined(ASMJIT_EMBED)
  #pragma message("'ASMJIT_EMBED' is deprecated, use 'ASMJIT_BUILD_EMBED'")
  #define ASMJIT_BUILD_EMBED
#endif

#if defined(ASMJIT_STATIC)
  #pragma message("'ASMJIT_STATIC' is deprecated, use 'ASMJIT_BUILD_STATIC'")
  #define ASMJIT_BUILD_STATIC
#endif

#if defined(ASMJIT_DISABLE_LOGGER)
  #pragma message("'ASMJIT_DISABLE_LOGGER' is deprecated, use 'ASMJIT_DISABLE_LOGGING'")
  #define ASMJIT_DISABLE_LOGGING
#endif

// ============================================================================
// [asmjit::Build - Globals - Build Mode]
// ============================================================================

// Detect ASMJIT_BUILD_DEBUG and ASMJIT_BUILD_RELEASE if not defined.
#if !defined(ASMJIT_BUILD_DEBUG) && !defined(ASMJIT_BUILD_RELEASE)
  #if !defined(NDEBUG)
    #define ASMJIT_BUILD_DEBUG
  #else
    #define ASMJIT_BUILD_RELEASE
  #endif
#endif

// Prevent compile-time errors caused by misconfiguration.
#if defined(ASMJIT_DISABLE_TEXT) && !defined(ASMJIT_DISABLE_LOGGING)
  #error "[asmjit] ASMJIT_DISABLE_TEXT requires ASMJIT_DISABLE_LOGGING to be defined."
#endif

// ============================================================================
// [asmjit::Build - Globals - Target Architecture]
// ============================================================================

#if defined(_M_X64) || defined(__x86_64__)
  #define ASMJIT_ARCH_X86 64
#elif defined(_M_IX86) || defined(__X86__) || defined(__i386__)
  #define ASMJIT_ARCH_X86 32
#else
  #define ASMJIT_ARCH_X86 0
#endif

#if defined(__arm64__) || defined(__aarch64__)
# define ASMJIT_ARCH_ARM 64
#elif defined(_M_ARM) || defined(_M_ARMT) || defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
  #define ASMJIT_ARCH_ARM 32
#else
  #define ASMJIT_ARCH_ARM 0
#endif

#if defined(_MIPS_ARCH_MIPS64) || defined(__mips64)
  #define ASMJIT_ARCH_MIPS     64
#elif defined(_MIPS_ARCH_MIPS32) || defined(_M_MRX000) || defined(__mips__)
  #define ASMJIT_ARCH_MIPS     32
#else
  #define ASMJIT_ARCH_MIPS     0
#endif

#define ASMJIT_ARCH_BITS       (ASMJIT_ARCH_X86 | ASMJIT_ARCH_ARM | ASMJIT_ARCH_MIPS)
#if ASMJIT_ARCH_BITS == 0
  #undef ASMJIT_ARCH_BITS
  #if defined (__LP64__) || defined(_LP64)
    #define ASMJIT_ARCH_BITS   64
  #else
    #define ASMJIT_ARCH_BITS   32
  #endif
#endif

#if (defined(__ARMEB__))  || \
    (defined(__MIPSEB__)) || \
    (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
  #define ASMJIT_ARCH_LE       0
  #define ASMJIT_ARCH_BE       1
#else
  #define ASMJIT_ARCH_LE       1
  #define ASMJIT_ARCH_BE       0
#endif

// Build host architecture if no architecture is selected.
#if !defined(ASMJIT_BUILD_HOST) && \
    !defined(ASMJIT_BUILD_X86)  && \
    !defined(ASMJIT_BUILD_ARM)
  #define ASMJIT_BUILD_HOST
#endif

// Detect host architecture if building only for host.
#if ASMJIT_ARCH_X86 && defined(ASMJIT_BUILD_HOST) && !defined(ASMJIT_BUILD_X86)
  #define ASMJIT_BUILD_X86
#endif

#if ASMJIT_ARCH_ARM && defined(ASMJIT_BUILD_HOST) && !defined(ASMJIT_BUILD_ARM)
  #define ASMJIT_BUILD_ARM
#endif

// ============================================================================
// [asmjit::Build - Globals - C++ Compiler and Features Detection]
// ============================================================================

#define ASMJIT_CXX_CLANG 0
#define ASMJIT_CXX_GNU   0
#define ASMJIT_CXX_INTEL 0
#define ASMJIT_CXX_MSC   0
#define ASMJIT_CXX_MAKE_VER(MAJOR, MINOR, PATCH) ((MAJOR) * 10000000 + (MINOR) * 100000 + (PATCH))

// Intel Compiler [pretends to be GNU or MSC, so it must be checked first]:
//   - https://software.intel.com/en-us/articles/c0x-features-supported-by-intel-c-compiler
//   - https://software.intel.com/en-us/articles/c14-features-supported-by-intel-c-compiler
//   - https://software.intel.com/en-us/articles/c17-features-supported-by-intel-c-compiler
#if defined(__INTEL_COMPILER)

  #undef ASMJIT_CXX_INTEL
  #define ASMJIT_CXX_INTEL ASMJIT_CXX_MAKE_VER(__INTEL_COMPILER / 100, (__INTEL_COMPILER / 10) % 10, __INTEL_COMPILER % 10)

// MSC Compiler:
//   - https://msdn.microsoft.com/en-us/library/hh567368.aspx
//
// Version List:
//   - 16.00.0 == VS2010
//   - 17.00.0 == VS2012
//   - 18.00.0 == VS2013
//   - 19.00.0 == VS2015
//   - 19.10.0 == VS2017
#elif defined(_MSC_VER) && defined(_MSC_FULL_VER)

  #undef ASMJIT_CXX_MSC
  #if _MSC_VER == _MSC_FULL_VER / 10000
    #define ASMJIT_CXX_MSC ASMJIT_CXX_MAKE_VER(_MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 10000)
  #else
    #define ASMJIT_CXX_MSC ASMJIT_CXX_MAKE_VER(_MSC_VER / 100, (_MSC_FULL_VER / 100000) % 100, _MSC_FULL_VER % 100000)
  #endif

  // SEVERE: VS2015 handles constexpr's incorrectly in case a struct contains a
  //         union. There is no workaround known other than rewriting the whole
  //         code. VS2017 has a similar bug, but it can be workarounded.
  #if ASMJIT_CXX_MSC < ASMJIT_CXX_MAKE_VER(19, 10, 0)
    #error "[asmjit] At least VS2017 is required due to a severe bug in VS2015's constexpr implementation"
  #endif

// Clang Compiler [Pretends to be GNU, so it must be checked before]:
//   - https://clang.llvm.org/cxx_status.html
#elif defined(__clang_major__) && defined(__clang_minor__) && defined(__clang_patchlevel__)

  #undef ASMJIT_CXX_CLANG
  #define ASMJIT_CXX_CLANG ASMJIT_CXX_MAKE_VER(__clang_major__, __clang_minor__, __clang_patchlevel__)

// GNU Compiler:
//   - https://gcc.gnu.org/projects/cxx-status.html
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)

  #undef ASMJIT_CXX_GNU
  #define ASMJIT_CXX_GNU ASMJIT_CXX_MAKE_VER(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)

#endif

// Compiler features detection macros.
#if ASMJIT_CXX_CLANG && defined(__has_builtin)
  #define ASMJIT_CXX_HAS_BUILTIN(NAME, CHECK) (__has_builtin(NAME))
#else
  #define ASMJIT_CXX_HAS_BUILTIN(NAME, CHECK) (!(!(CHECK)))
#endif

#if ASMJIT_CXX_CLANG && defined(__has_extension)
  #define ASMJIT_CXX_HAS_FEATURE(NAME, CHECK) (__has_extension(NAME))
#elif ASMJIT_CXX_CLANG && defined(__has_feature)
  #define ASMJIT_CXX_HAS_FEATURE(NAME, CHECK) (__has_feature(NAME))
#else
  #define ASMJIT_CXX_HAS_FEATURE(NAME, CHECK) (!(!(CHECK)))
#endif

#if ASMJIT_CXX_CLANG && defined(__has_attribute)
  #define ASMJIT_CXX_HAS_ATTRIBUTE(NAME, CHECK) (__has_attribute(NAME))
#else
  #define ASMJIT_CXX_HAS_ATTRIBUTE(NAME, CHECK) (!(!(CHECK)))
#endif

#if ASMJIT_CXX_CLANG && defined(__has_cpp_attribute)
  #define ASMJIT_CXX_HAS_CPP_ATTRIBUTE(NAME, CHECK) (__has_cpp_attribute(NAME))
#else
  #define ASMJIT_CXX_HAS_CPP_ATTRIBUTE(NAME, CHECK) (!(!(CHECK)))
#endif

// Compiler features by vendor.
#if defined(_MSC_VER) && !defined(_NATIVE_WCHAR_T_DEFINED)
  #define ASMJIT_CXX_HAS_NATIVE_WCHAR_T 0
#else
  #define ASMJIT_CXX_HAS_NATIVE_WCHAR_T 1
#endif

#if ASMJIT_CXX_HAS_FEATURE(cxx_unicode_literals, ( \
                          (ASMJIT_CXX_INTEL >= ASMJIT_CXX_MAKE_VER(14, 0, 0)) || \
                          (ASMJIT_CXX_MSC   >= ASMJIT_CXX_MAKE_VER(19, 0, 0)) || \
                          (ASMJIT_CXX_GNU   >= ASMJIT_CXX_MAKE_VER(4 , 5, 0) && __cplusplus >= 201103L) ))
  #define ASMJIT_CXX_HAS_UNICODE_LITERALS 1
#else
  #define ASMJIT_CXX_HAS_UNICODE_LITERALS 0
#endif

// ============================================================================
// [asmjit::Build - Globals - API Decorators & Language Extensions]
// ============================================================================

// API (Export / Import).
#if !defined(ASMJIT_BUILD_EMBED) && !defined(ASMJIT_BUILD_STATIC)
  #if defined(_WIN32) && (defined(_MSC_VER) || defined(__MINGW32__))
    #if defined(ASMJIT_EXPORTS)
      #define ASMJIT_API __declspec(dllexport)
    #else
      #define ASMJIT_API __declspec(dllimport)
    #endif
  #elif defined(_WIN32) && defined(__GNUC__)
    #if defined(ASMJIT_EXPORTS)
      #define ASMJIT_API __attribute__((__dllexport__))
    #else
      #define ASMJIT_API __attribute__((__dllimport__))
    #endif
  #elif defined(__GNUC__)
    #define ASMJIT_API __attribute__((__visibility__("default")))
  #endif
#endif

#if !defined(ASMJIT_API)
  #define ASMJIT_API
#endif

#if !defined(ASMJIT_VARAPI)
  #define ASMJIT_VARAPI extern ASMJIT_API
#endif

// This is basically a workaround. When using MSVC and marking class as DLL
// export everything gets exported, which is unwanted in most projects. MSVC
// automatically exports typeinfo and vtable if at least one symbol of the
// class is exported. However, GCC has some strange behavior that even if
// one or more symbol is exported it doesn't export typeinfo unless the
// class itself is decorated with "visibility(default)" (i.e. ASMJIT_API).
#if !defined(_WIN32) && defined(__GNUC__)
  #define ASMJIT_VIRTAPI ASMJIT_API
#else
  #define ASMJIT_VIRTAPI
#endif

// Function attributes.
#if !defined(ASMJIT_BUILD_DEBUG) && defined(__GNUC__)
  #define ASMJIT_INLINE inline __attribute__((__always_inline__))
#elif !defined(ASMJIT_BUILD_DEBUG) && defined(_MSC_VER)
  #define ASMJIT_INLINE __forceinline
#else
  #define ASMJIT_INLINE inline
#endif

#if defined(__GNUC__)
  #define ASMJIT_NOINLINE __attribute__((__noinline__))
  #define ASMJIT_NORETURN __attribute__((__noreturn__))
#elif defined(_MSC_VER)
  #define ASMJIT_NOINLINE __declspec(noinline)
  #define ASMJIT_NORETURN __declspec(noreturn)
#else
  #define ASMJIT_NOINLINE
  #define ASMJIT_NORETURN
#endif

// Calling conventions.
#if ASMJIT_ARCH_X86 == 32 && defined(__GNUC__)
  #define ASMJIT_CDECL __attribute__((__cdecl__))
  #define ASMJIT_STDCALL __attribute__((__stdcall__))
  #define ASMJIT_FASTCALL __attribute__((__fastcall__))
  #define ASMJIT_REGPARM(N) __attribute__((__regparm__(N)))
#elif ASMJIT_ARCH_X86 == 32 && defined(_MSC_VER)
  #define ASMJIT_CDECL __cdecl
  #define ASMJIT_STDCALL __stdcall
  #define ASMJIT_FASTCALL __fastcall
  #define ASMJIT_REGPARM(N)
#else
  #define ASMJIT_CDECL
  #define ASMJIT_STDCALL
  #define ASMJIT_FASTCALL
  #define ASMJIT_REGPARM(N)
#endif

// Type alignment (not allowed by C++11 'alignas' keyword).
#if defined(__GNUC__)
  #define ASMJIT_ALIGN_TYPE(TYPE, N) __attribute__((__aligned__(N))) TYPE
#elif defined(_MSC_VER)
  #define ASMJIT_ALIGN_TYPE(TYPE, N) __declspec(align(N)) TYPE
#else
  #define ASMJIT_ALIGN_TYPE(TYPE, N) TYPE
#endif

// Annotations.
#if defined(__GNUC__)
  #define ASMJIT_LIKELY(...) __builtin_expect(!!(__VA_ARGS__), 1)
  #define ASMJIT_UNLIKELY(...) __builtin_expect(!!(__VA_ARGS__), 0)
#else
  #define ASMJIT_LIKELY(...) (__VA_ARGS__)
  #define ASMJIT_UNLIKELY(...) (__VA_ARGS__)
#endif

#if defined(__clang__) && __cplusplus >= 201103L
  #define ASMJIT_FALLTHROUGH [[clang::fallthrough]]
#elif ASMJIT_CXX_GNU >= ASMJIT_CXX_MAKE_VER(7, 0, 0)
  #define ASMJIT_FALLTHROUGH __attribute__((__fallthrough__))
#else
  #define ASMJIT_FALLTHROUGH ((void)0) /* fallthrough */
#endif

#define ASMJIT_UNUSED(X) (void)(X)

// Utilities.
#define ASMJIT_OFFSET_OF(STRUCT, MEMBER) ((int)(intptr_t)((const char*)&((const STRUCT*)0x100)->MEMBER) - 0x100)
#define ASMJIT_ARRAY_SIZE(X) uint32_t(sizeof(X) / sizeof(X[0]))

#if ASMJIT_CXX_HAS_ATTRIBUTE(attribute_deprecated_with_message, ASMJIT_CXX_GNU >= ASMJIT_CXX_MAKE_VER(4, 5, 0))
  #define ASMJIT_DEPRECATED(DECL, MESSAGE) DECL __attribute__((__deprecated__(MESSAGE)))
#elif ASMJIT_MSC
  #define ASMJIT_DEPRECATED(DECL, MESSAGE) __declspec(deprecated(MESSAGE)) DECL
#else
  #define ASMJIT_DEPRECATED(DECL, MESSAGE) DECL
#endif

#if ASMJIT_CXX_HAS_ATTRIBUTE(no_sanitize, 0)
  #define ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF __attribute__((__no_sanitize__("undefined")))
#elif ASMJIT_CXX_GNU >= ASMJIT_CXX_MAKE_VER(4, 9, 0)
  #define ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF __attribute__((__no_sanitize_undefined__))
#else
  #define ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF
#endif

// ============================================================================
// [asmjit::Build - Globals - Begin-Namespace / End-Namespace]
// ============================================================================

#if defined(__clang__)
  #define ASMJIT_BEGIN_NAMESPACE                                              \
    namespace asmjit {                                                        \
      _Pragma("clang diagnostic push")                                        \
      _Pragma("clang diagnostic ignored \"-Wconstant-logical-operand\"")      \
      _Pragma("clang diagnostic ignored \"-Wunnamed-type-template-args\"")
  #define ASMJIT_END_NAMESPACE                                                \
      _Pragma("clang diagnostic pop")                                         \
    }
#elif ASMJIT_CXX_GNU >= ASMJIT_CXX_MAKE_VER(4, 0, 0) && \
      ASMJIT_CXX_GNU <  ASMJIT_CXX_MAKE_VER(5, 0, 0)
  #define ASMJIT_BEGIN_NAMESPACE                                              \
    namespace asmjit {                                                        \
      _Pragma("GCC diagnostic push")                                          \
      _Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"")
  #define ASMJIT_END_NAMESPACE                                                \
      _Pragma("GCC diagnostic pop")                                           \
    }
#elif ASMJIT_CXX_GNU >= ASMJIT_CXX_MAKE_VER(8, 0, 0)
  #define ASMJIT_BEGIN_NAMESPACE                                              \
    namespace asmjit {                                                        \
      _Pragma("GCC diagnostic push")                                          \
      _Pragma("GCC diagnostic ignored \"-Wclass-memaccess\"")
  #define ASMJIT_END_NAMESPACE                                                \
      _Pragma("GCC diagnostic pop")                                           \
    }
#elif defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  #define ASMJIT_BEGIN_NAMESPACE                                              \
    namespace asmjit {                                                        \
      __pragma(warning(push))                                                 \
      __pragma(warning(disable: 4127)) /* conditional expression is constant*/\
      __pragma(warning(disable: 4201)) /* nameless struct/union             */
  #define ASMJIT_END_NAMESPACE                                                \
      __pragma(warning(pop))                                                  \
    }
#endif

#if !defined(ASMJIT_BEGIN_NAMESPACE) && !defined(ASMJIT_END_NAMESPACE)
  #define ASMJIT_BEGIN_NAMESPACE namespace asmjit {
  #define ASMJIT_END_NAMESPACE }
#endif

#define ASMJIT_BEGIN_SUB_NAMESPACE(NAMESPACE)                                 \
  ASMJIT_BEGIN_NAMESPACE                                                      \
  namespace NAMESPACE {

#define ASMJIT_END_SUB_NAMESPACE                                              \
  }                                                                           \
  ASMJIT_END_NAMESPACE

// ============================================================================
// [asmjit::Build - Globals - Utilities]
// ============================================================================

#define ASMJIT_NONCOPYABLE(...)                                               \
  private:                                                                    \
    __VA_ARGS__(const __VA_ARGS__& other) = delete;                           \
    __VA_ARGS__& operator=(const __VA_ARGS__& other) = delete;                \
  public:

#define ASMJIT_NONCONSTRUCTIBLE(...)                                          \
  private:                                                                    \
    __VA_ARGS__() = delete;                                                   \
    __VA_ARGS__(const __VA_ARGS__& other) = delete;                           \
    __VA_ARGS__& operator=(const __VA_ARGS__& other) = delete;                \
  public:

// ============================================================================
// [asmjit::Build - Globals - Build-Only]
// ============================================================================

// Internal macros that are only used when building AsmJit itself.
#ifdef ASMJIT_EXPORTS
  #if !defined(ASMJIT_BUILD_DEBUG) && ASMJIT_CXX_HAS_ATTRIBUTE(__optimize__, ASMJIT_CXX_GNU >= ASMJIT_CXX_MAKE_VER(4, 4, 0))
    #define ASMJIT_FAVOR_SIZE  __attribute__((__optimize__("Os")))
    #define ASMJIT_FAVOR_SPEED __attribute__((__optimize__("O3")))
  #else
    #define ASMJIT_FAVOR_SIZE
    #define ASMJIT_FAVOR_SPEED
  #endif

  // Only turn-off these warnings when building asmjit itself.
  #ifdef _MSC_VER
    #ifndef _CRT_SECURE_NO_DEPRECATE
      #define _CRT_SECURE_NO_DEPRECATE
    #endif
    #ifndef _CRT_SECURE_NO_WARNINGS
      #define _CRT_SECURE_NO_WARNINGS
    #endif
  #endif
#endif

// ============================================================================
// [asmjit::Build - Globals - Cleanup]
// ============================================================================

// Undefine everything that is not used by AsmJit outside of `build.h` and that
// is considered private.
#undef ASMJIT_CXX_CLANG
#undef ASMJIT_CXX_GNU
#undef ASMJIT_CXX_INTEL
#undef ASMJIT_CXX_MSC
#undef ASMJIT_CXX_MAKE_VER

// ============================================================================
// [asmjit::Build - Globals - Unit Testing Boilerplate]
// ============================================================================

// IDE: Make sure '#ifdef'ed unit tests are properly highlighted.
#if defined(__INTELLISENSE__) && !defined(ASMJIT_BUILD_TEST)
  #define ASMJIT_BUILD_TEST
#endif

// IDE: Make sure '#ifdef'ed unit tests are not disabled by IDE.
#if defined(ASMJIT_BUILD_TEST)
  #include "../../../test/broken.h"
#endif

#endif // _ASMJIT_CORE_BUILD_H
