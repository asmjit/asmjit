// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BUILD_H
#define _ASMJIT_BUILD_H

// [Include]
#if !defined(ASMJIT_CONFIG_FILE)
#include "config.h"
#endif // !ASMJIT_CONFIG_FILE

// Turn of deprecation warnings for this compiler when compiling AsmJit.
#if defined(ASMJIT_EXPORTS) && defined(_MSC_VER)
# if !defined(_CRT_SECURE_NO_DEPRECATE)
#  define _CRT_SECURE_NO_DEPRECATE
# endif // !_CRT_SECURE_NO_DEPRECATE
# if !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
# endif // !_CRT_SECURE_NO_WARNINGS
#endif // ASMJIT_EXPORTS

// Default includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <new>

// ============================================================================
// [ASMJIT_OS]
// ============================================================================

#if defined(WINDOWS) || defined(_WINDOWS) || defined(__WINDOWS__) || defined(_WIN32) || defined(_WIN64)
# define ASMJIT_OS_WINDOWS
#elif defined(__linux__)     || defined(__unix__)    || \
      defined(__OpenBSD__)   || defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__DragonFly__) || defined(__BSD__)     || defined(__FREEBSD__) || \
      defined(__APPLE__)
# define ASMJIT_OS_POSIX
#else
# warning "AsmJit - Can't match host operating system, using ASMJIT_OS_POSIX"
# define ASMJIT_OS_POSIX
#endif

// ============================================================================
// [ASMJIT_HOST]
// ============================================================================

// Define it only if it's not defined. On some systems -D command can be passed
// to the compiler to bypass this autodetection.
#if !defined(ASMJIT_HOST_X86) && !defined(ASMJIT_HOST_X64)
# if defined(__x86_64__) || defined(__LP64) || defined(__IA64__) || defined(_M_X64) || defined(_WIN64)
#  define ASMJIT_HOST_X64
#  define ASMJIT_HOST_LE
# else
// _M_IX86, __INTEL__, __i386__
#  define ASMJIT_HOST_X86
#  define ASMJIT_HOST_LE
# endif
#endif

// ============================================================================
// [ASMJIT_BUILD]
// ============================================================================

// Build host architecture if no architecture is selected.
#if !defined(ASMJIT_BUILD_HOST) && \
    !defined(ASMJIT_BUILD_X86) && \
    !defined(ASMJIT_BUILD_X64)
# define ASMJIT_BUILD_HOST
#endif

// Autodetect host architecture if enabled.
#if defined(ASMJIT_BUILD_HOST)
# if defined(ASMJIT_HOST_X86) && !defined(ASMJIT_BUILD_X86)
#  define ASMJIT_BUILD_X86
# endif // ASMJIT_HOST_X86 && !ASMJIT_BUILD_X86
# if defined(ASMJIT_HOST_X64) && !defined(ASMJIT_BUILD_X64)
#  define ASMJIT_BUILD_X64
# endif // ASMJIT_HOST_X64 && !ASMJIT_BUILD_X64
#endif // ASMJIT_BUILD_HOST

// ============================================================================
// [ASMJIT_API]
// ============================================================================

#if !defined(ASMJIT_API)
# if defined(ASMJIT_STATIC)
#  define ASMJIT_API
# elif defined(ASMJIT_OS_WINDOWS)
#  if defined(__GNUC__)
#   if defined(ASMJIT_EXPORTS)
#    define ASMJIT_API __attribute__((dllexport))
#   else
#    define ASMJIT_API __attribute__((dllimport))
#   endif // ASMJIT_EXPORTS
#  else
#   if defined(ASMJIT_EXPORTS)
#    define ASMJIT_API __declspec(dllexport)
#   else
#    define ASMJIT_API __declspec(dllimport)
#   endif // ASMJIT_EXPORTS
#  endif // __GNUC__
# else
#  if defined(__GNUC__)
#   if __GNUC__ >= 4
#    define ASMJIT_API __attribute__((visibility("default")))
#    define ASMJIT_VAR extern ASMJIT_API
#   endif // __GNUC__ >= 4
#  endif // __GNUC__
# endif
#endif // ASMJIT_API

#if !defined(ASMJIT_VAR)
# define ASMJIT_VAR extern ASMJIT_API
#endif // !ASMJIT_VAR

// ============================================================================
// [ASMJIT_INLINE]
// ============================================================================

#if defined(_MSC_VER)
# define ASMJIT_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__) && !defined(__MINGW32__)
# define ASMJIT_INLINE inline __attribute__((always_inline))
#else
# define ASMJIT_INLINE inline
#endif

// ============================================================================
// [ASMJIT_ENUM]
// ============================================================================

#if defined(_MSC_VER)
# define ASMJIT_ENUM(_Name_) enum _Name_ : uint32_t
#else
# define ASMJIT_ENUM(_Name_) enum _Name_
#endif

// ============================================================================
// [_ASMJIT_HOST_INDEX]
// ============================================================================

#if defined(ASMJIT_HOST_LE)
# define _ASMJIT_HOST_INDEX(_Total_, _Index_) (_Index_)
#else
# define _ASMJIT_HOST_INDEX(_Total_, _Index_) ((_Total_) - 1 - (_Index_)
#endif

// ============================================================================
// [ASMJIT_ARRAY_SIZE]
// ============================================================================

#define ASMJIT_ARRAY_SIZE(_Array_) (sizeof(_Array_) / sizeof(*_Array_))

// ============================================================================
// [ASMJIT_NO_COPY]
// ============================================================================

#define ASMJIT_NO_COPY(_Type_) \
private: \
  ASMJIT_INLINE _Type_(const _Type_& other); \
  ASMJIT_INLINE _Type_& operator=(const _Type_& other); \
public:

// ============================================================================
// [ASMJIT_DEBUG]
// ============================================================================

// If ASMJIT_DEBUG and ASMJIT_RELEASE is not defined ASMJIT_DEBUG will be
// detected using the compiler specific macros. This enables to set the build
// type using IDE.
#if !defined(ASMJIT_DEBUG) && !defined(ASMJIT_RELEASE)
# if defined(_DEBUG)
#  define ASMJIT_DEBUG
# endif // _DEBUG
#endif // !ASMJIT_DEBUG && !ASMJIT_RELEASE

// ============================================================================
// [ASMJIT_UNUSED]
// ============================================================================

#if !defined(ASMJIT_UNUSED)
# define ASMJIT_UNUSED(_Var_) ((void)_Var_)
#endif // ASMJIT_UNUSED

// ============================================================================
// [ASMJIT_NOP]
// ============================================================================

#if !defined(ASMJIT_NOP)
# define ASMJIT_NOP() ((void)0)
#endif // ASMJIT_NOP

// ============================================================================
// [ASMJIT_CCONV]
// ============================================================================

#if defined(ASMJIT_HOST_X86)
# if defined(__GNUC__)
#  define ASMJIT_REGPARM_1 __attribute__((regparm(1)))
#  define ASMJIT_REGPARM_2 __attribute__((regparm(2)))
#  define ASMJIT_REGPARM_3 __attribute__((regparm(3)))
#  define ASMJIT_FASTCALL  __attribute__((fastcall))
#  define ASMJIT_STDCALL   __attribute__((stdcall))
#  define ASMJIT_CDECL     __attribute__((cdecl))
# else
#  define ASMJIT_FASTCALL   __fastcall
#  define ASMJIT_STDCALL    __stdcall
#  define ASMJIT_CDECL      __cdecl
# endif
#else
# define ASMJIT_FASTCALL
# define ASMJIT_STDCALL
# define ASMJIT_CDECL
#endif // ASMJIT_HOST

// ============================================================================
// [IntTypes]
// ============================================================================

#if defined(__MINGW32__) || defined(__MINGW64__)
# include <sys/types.h>
#endif // __MINGW32__ || __MINGW64__

#if defined(_MSC_VER) && (_MSC_VER < 1600)
# if !defined(ASMJIT_SUPRESS_STD_TYPES)
#  if (_MSC_VER < 1300)
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed __int64 int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
#  else
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#  endif // _MSC_VER
# endif // ASMJIT_SUPRESS_STD_TYPES
#else
# include <stdint.h>
# include <limits.h>
#endif

#if defined(_MSC_VER)
# define ASMJIT_INT64_C(_Num_) _Num_##i64
# define ASMJIT_UINT64_C(_Num_) _Num_##ui64
#else
# define ASMJIT_INT64_C(_Num_) _Num_##LL
# define ASMJIT_UINT64_C(_Num_) _Num_##ULL
#endif

// ============================================================================
// [OS Support]
// ============================================================================

#if defined(ASMJIT_OS_WINDOWS) && !defined(ASMJIT_SUPRESS_WINDOWS_H)
#include <windows.h>
#endif // ASMJIT_OS_WINDOWS  && !ASMJIT_SUPRESS_WINDOWS_H

// [Guard]
#endif // _ASMJIT_BUILD_H
