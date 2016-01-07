// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#if defined(ASMJIT_API_SCOPE)
# undef ASMJIT_API_SCOPE
#else
# error "[asmjit] Api-Scope not active, forgot to include apibegin.h?"
#endif // ASMJIT_API_SCOPE

// ============================================================================
// [NoExcept]
// ============================================================================

#if defined(ASMJIT_UNDEF_NOEXCEPT)
# undef noexcept
# undef ASMJIT_UNDEF_NOEXCEPT
#endif // ASMJIT_UNDEF_NOEXCEPT

// ============================================================================
// [NullPtr]
// ============================================================================

#if defined(ASMJIT_UNDEF_NULLPTR)
# undef nullptr
# undef ASMJIT_UNDEF_NULLPTR
#endif // ASMJIT_UNDEF_NULLPTR

// ============================================================================
// [Override]
// ============================================================================

#if defined(ASMJIT_UNDEF_OVERRIDE)
# undef override
# undef ASMJIT_UNDEF_OVERRIDE
#endif // ASMJIT_UNDEF_OVERRIDE

// ============================================================================
// [MSC]
// ============================================================================

#if defined(_MSC_VER)
# pragma warning(pop)

# if defined(ASMJIT_UNDEF_VSNPRINTF)
#  undef vsnprintf
#  undef ASMJIT_UNDEF_VSNPRINTF
# endif // ASMJIT_UNDEF_VSNPRINTF

# if defined(ASMJIT_UNDEF_SNPRINTF)
#  undef snprintf
#  undef ASMJIT_UNDEF_SNPRINTF
# endif // ASMJIT_UNDEF_SNPRINTF

#endif // _MSC_VER

// ============================================================================
// [CLang]
// ============================================================================

#if defined(__clang__)
# pragma clang diagnostic pop
#endif // __clang__

// ============================================================================
// [GCC]
// ============================================================================

#if defined(__GNUC__) && !defined(__clang__)
# if __GNUC__ >= 4 && !defined(__MINGW32__)
#  pragma GCC visibility pop
# endif // GCC 4+
#endif // __GNUC__
