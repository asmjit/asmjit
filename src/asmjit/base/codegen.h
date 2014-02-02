// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CODEGEN_H
#define _ASMJIT_BASE_CODEGEN_H

// [Dependencies - AsmJit]
#include "../base/defs.h"
#include "../base/error.h"
#include "../base/logger.h"
#include "../base/runtime.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_base
//! @{

// ============================================================================
// [asmjit::kCodeGen]
// ============================================================================

//! @brief @ref CodeGen features.
ASMJIT_ENUM(kCodeGen) {
  //! @brief Emit optimized code-alignment sequences.
  //!
  //! X86/X64:
  //!
  //! Default align sequence used by X86/X64 architecture is one-byte 0x90
  //! opcode that is mostly shown by disassemblers as nop. However there are
  //! more optimized align sequences for 2-11 bytes that may execute faster.
  //! If this feature is enabled asmjit will generate specialized sequences
  //! for alignment between 1 to 11 bytes. Also when @ref x86x64::Compiler
  //! is used, it may add rex prefixes into the code to make some instructions
  //! larger so no alignment sequences are needed.
  //!
  //! @default true.
  kCodeGenOptimizedAlign = 0,

  //! @brief Emit jump-prediction hints.
  //!
  //! X86/X64:
  //!
  //! Jump prediction is usually based on the direction of the jump. If the
  //! jump is backward it is usually predicted as taken; and if the jump is
  //! forward it is usually predicted as not-taken. The reason is that loops
  //! generally use backward jumps and conditions usually use forward jumps.
  //! However this behavior can be overridden by using instruction prefixes.
  //! If this option is enabled these hints will be emitted.
  //!
  //! @default true.
  kCodeGenPredictedJumps = 1
};

// ============================================================================
// [asmjit::CodeGen]
// ============================================================================

//! @brief Abstract class inherited by @ref Assembler and @ref Compiler.
struct CodeGen {
  ASMJIT_NO_COPY(CodeGen)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref CodeGen instance.
  ASMJIT_API CodeGen(BaseRuntime* runtime);
  //! @brief Destroy the @ref CodeGen instance.
  ASMJIT_API virtual ~CodeGen();

  // --------------------------------------------------------------------------
  // [Runtime]
  // --------------------------------------------------------------------------

  //! @brief Get runtime.
  ASMJIT_INLINE BaseRuntime* getRuntime() const { return _runtime; }

  // --------------------------------------------------------------------------
  // [Logger]
  // --------------------------------------------------------------------------

  //! @brief Get whether the code generator has a logger.
  ASMJIT_INLINE bool hasLogger() const { return _logger != NULL; }
  //! @brief Get logger.
  ASMJIT_INLINE BaseLogger* getLogger() const { return _logger; }
  //! @brief Set logger to @a logger.
  ASMJIT_API Error setLogger(BaseLogger* logger);

  // --------------------------------------------------------------------------
  // [Arch]
  // --------------------------------------------------------------------------

  //! @brief Get target architecture.
  ASMJIT_INLINE uint32_t getArch() const { return _arch; }

  //! @brief Get default register size (4 or 8 bytes).
  ASMJIT_INLINE uint32_t getRegSize() const { return _regSize; }

  // --------------------------------------------------------------------------
  // [Error]
  // --------------------------------------------------------------------------

  //! @brief Get last error code.
  ASMJIT_INLINE Error getError() const { return _error; }
  //! @brief Set last error code and propagate it through the error handler.
  ASMJIT_API Error setError(Error error, const char* message = NULL);
  //! @brief Clear the last error code.
  ASMJIT_INLINE void clearError() { _error = kErrorOk; }

  //! @brief Get error handler.
  ASMJIT_INLINE ErrorHandler* getErrorHandler() const { return _errorHandler; }
  //! @brief Set error handler.
  ASMJIT_API Error setErrorHandler(ErrorHandler* handler);
  //! @brief Clear error handler.
  ASMJIT_INLINE Error clearErrorHandler() { return setErrorHandler(NULL); }

  // --------------------------------------------------------------------------
  // [Features]
  // --------------------------------------------------------------------------

  //! @brief Get code-generator @a feature.
  ASMJIT_API bool hasFeature(uint32_t feature) const;
  //! @brief Set code-generator @a feature to @a value.
  ASMJIT_API Error setFeature(uint32_t feature, bool value);

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! @brief Get options.
  ASMJIT_INLINE uint32_t getOptions() const { return _options; }
  //! @brief Set options.
  ASMJIT_INLINE void setOptions(uint32_t options) { _options = options; }

  //! @brief Get options and clear them.
  ASMJIT_INLINE uint32_t getOptionsAndClear() {
    uint32_t options = _options;
    _options = 0;
    return options;
  };

  // --------------------------------------------------------------------------
  // [Purge]
  // --------------------------------------------------------------------------

  //! @brief Called by clear() and reset() to clear all data used by the code
  //! generator.
  virtual void _purge() = 0;

  // --------------------------------------------------------------------------
  // [Make]
  // --------------------------------------------------------------------------

  //! @brief Make is a convenience method to make and relocate the current code
  //! into the associated runtime.
  //!
  //! What is needed is only to cast the returned pointer to your function type
  //! and then use it. If there was an error during make() @c NULL is returned
  //! and the last error code can be obtained by calling @ref getError().
  virtual void* make() = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Runtime.
  BaseRuntime* _runtime;
  //! @brief Logger.
  BaseLogger* _logger;
  //! @brief Error handler, called by @ref setError().
  ErrorHandler* _errorHandler;

  //! @brief Target architecture.
  uint8_t _arch;
  //! @brief Get the default register size of the architecture (4 or 8 bytes).
  uint8_t _regSize;
  //! @brief Last error code.
  uint8_t _error;
  //! @brief Target features.
  uint8_t _features;
  //! @brief Options for the next generated instruction (only 8-bits used).
  uint32_t _options;

  //! @brief Zone memory allocator.
  Zone _zoneAllocator;
};

//! @}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CODEGEN_H
