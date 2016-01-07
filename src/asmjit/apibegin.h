// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - AsmJit]
#if !defined(_ASMJIT_BUILD_H)
#include "./build.h"
#endif // !_ASMJIT_BUILD_H

// [Guard]
#if !defined(ASMJIT_API_SCOPE)
# define ASMJIT_API_SCOPE
#else
# error "[asmjit] Api-Scope is already active, previous scope not closed by apiend.h?"
#endif // ASMJIT_API_SCOPE

// ============================================================================
// [NoExcept]
// ============================================================================

#if !ASMJIT_CC_HAS_NOEXCEPT && !defined(noexcept)
# define noexcept ASMJIT_NOEXCEPT
# define ASMJIT_UNDEF_NOEXCEPT
#endif // !ASMJIT_CC_HAS_NOEXCEPT && !noexcept

// ============================================================================
// [NullPtr]
// ============================================================================

#if !ASMJIT_CC_HAS_NULLPTR && !defined(nullptr)
# define nullptr NULL
# define ASMJIT_UNDEF_NULLPTR
#endif // !ASMJIT_CC_HAS_NULLPTR && !nullptr

// ============================================================================
// [Override]
// ============================================================================

#if !ASMJIT_CC_HAS_OVERRIDE && !defined(override)
# define override
# define ASMJIT_UNDEF_OVERRIDE
#endif // !ASMJIT_CC_HAS_OVERRIDE && !override

// ============================================================================
// [MSC]
// ============================================================================

#if defined(_MSC_VER)

# pragma warning(push)
# pragma warning(disable: 4127) // conditional expression is constant
# pragma warning(disable: 4201) // nameless struct/union
# pragma warning(disable: 4244) // '+=' : conversion from 'int' to 'x', possible
                                // loss of data
# pragma warning(disable: 4251) // struct needs to have dll-interface to be used
                                // by clients of struct ...
# pragma warning(disable: 4275) // non dll-interface struct ... used as base for
                                // dll-interface struct
# pragma warning(disable: 4355) // this used in base member initializer list
# pragma warning(disable: 4480) // specifying underlying type for enum
# pragma warning(disable: 4800) // forcing value to bool 'true' or 'false'

// TODO: Check if these defines are needed and for which version of MSC. There are
// news about these as they are part of C99.
# if !defined(vsnprintf)
#  define ASMJIT_UNDEF_VSNPRINTF
#  define vsnprintf _vsnprintf
# endif // !vsnprintf
# if !defined(snprintf)
#  define ASMJIT_UNDEF_SNPRINTF
#  define snprintf _snprintf
# endif // !snprintf

#endif // _MSC_VER

// ============================================================================
// [CLang]
// ============================================================================

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunnamed-type-template-args"
#endif // __clang__

// ============================================================================
// [GCC]
// ============================================================================

#if defined(__GNUC__) && !defined(__clang__)
# if __GNUC__ >= 4 && !defined(__MINGW32__)
#  pragma GCC visibility push(hidden)
# endif // GCC 4+
#endif // __GNUC__
