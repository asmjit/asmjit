// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ZONESTACK_H
#define _ASMJIT_CORE_ZONESTACK_H

// [Dependencies]
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
//! \{

// ============================================================================
// [asmjit::ZoneStackBase]
// ============================================================================

class ZoneStackBase {
public:
  ASMJIT_NONCOPYABLE(ZoneStackBase)

  static constexpr uint32_t kBlockSize = ZoneAllocator::kHiMaxSize;

  struct Block {
    inline bool empty() const noexcept { return _start == _end; }
    inline Block* prev() const noexcept { return _link[Globals::kLinkLeft]; }
    inline Block* next() const noexcept { return _link[Globals::kLinkRight]; }

    inline void setPrev(Block* block) noexcept { _link[Globals::kLinkLeft] = block; }
    inline void setNext(Block* block) noexcept { _link[Globals::kLinkRight] = block; }

    template<typename T>
    inline T* start() const noexcept { return static_cast<T*>(_start); }
    template<typename T>
    inline void setStart(T* start) noexcept { _start = static_cast<void*>(start); }

    template<typename T>
    inline T* end() const noexcept { return (T*)_end; }
    template<typename T>
    inline void setEnd(T* end) noexcept { _end = (void*)end; }

    template<typename T>
    inline T* data() const noexcept { return (T*)((uint8_t*)(this) + sizeof(Block)); }

    template<typename T>
    inline bool canPrepend() const noexcept { return _start > data<void>(); }

    template<typename T>
    inline bool canAppend() const noexcept {
      size_t kNumBlockItems = (kBlockSize - sizeof(Block)) / sizeof(T);
      size_t kStartBlockIndex = sizeof(Block);
      size_t kEndBlockIndex = kStartBlockIndex + kNumBlockItems * sizeof(T);

      return (uintptr_t)_end <= ((uintptr_t)this + kEndBlockIndex - sizeof(T));
    }

    Block* _link[Globals::kLinkCount];   //!< Next and previous blocks.
    void* _start;                        //!< Pointer to the start of the array.
    void* _end;                          //!< Pointer to the end of the array.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline ZoneStackBase() noexcept {
    _allocator = nullptr;
    _block[0] = nullptr;
    _block[1] = nullptr;
  }
  inline ZoneStackBase(ZoneStackBase&& stack) noexcept {
    _allocator = stack._allocator;
    _block[0] = stack._block[0];
    _block[1] = stack._block[1];

    stack._allocator = nullptr;
    stack._block[0] = nullptr;
    stack._block[1] = nullptr;
  }
  inline ~ZoneStackBase() noexcept { reset(); }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  inline bool isInitialized() const noexcept { return _allocator != nullptr; }
  ASMJIT_API Error _init(ZoneAllocator* allocator, size_t middleIndex) noexcept;
  inline Error reset() noexcept { return _init(nullptr, 0); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get a `ZoneAllocator` attached to this container.
  inline ZoneAllocator* allocator() const noexcept { return _allocator; }

  inline bool empty() const noexcept {
    ASMJIT_ASSERT(isInitialized());
    return _block[0]->start<void>() == _block[1]->end<void>();
  }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  ASMJIT_API Error _prepareBlock(uint32_t side, size_t initialIndex) noexcept;
  ASMJIT_API void _cleanupBlock(uint32_t side, size_t middleIndex) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ZoneAllocator* _allocator;             //!< Allocator used to allocate data.
  Block* _block[Globals::kLinkCount];    //!< First and last blocks.
};

// ============================================================================
// [asmjit::ZoneStack<T>]
// ============================================================================

template<typename T>
class ZoneStack : public ZoneStackBase {
public:
  ASMJIT_NONCOPYABLE(ZoneStack<T>)

  enum : uint32_t {
    kNumBlockItems   = uint32_t((kBlockSize - sizeof(Block)) / sizeof(T)),
    kStartBlockIndex = uint32_t(sizeof(Block)),
    kMidBlockIndex   = uint32_t(kStartBlockIndex + (kNumBlockItems / 2) * sizeof(T)),
    kEndBlockIndex   = uint32_t(kStartBlockIndex + (kNumBlockItems    ) * sizeof(T))
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline ZoneStack() noexcept {}
  inline ZoneStack(ZoneStack&& stack) noexcept = default;
  inline ~ZoneStack() noexcept {}

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  inline Error init(ZoneAllocator* allocator) noexcept { return _init(allocator, kMidBlockIndex); }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  ASMJIT_FORCEINLINE Error prepend(T item) noexcept {
    ASMJIT_ASSERT(isInitialized());
    Block* block = _block[Globals::kLinkFirst];

    if (!block->canPrepend<T>()) {
      ASMJIT_PROPAGATE(_prepareBlock(Globals::kLinkFirst, kEndBlockIndex));
      block = _block[Globals::kLinkFirst];
    }

    T* ptr = block->start<T>() - 1;
    ASMJIT_ASSERT(ptr >= block->data<T>() && ptr <= block->data<T>() + (kNumBlockItems - 1));
    *ptr = item;
    block->setStart<T>(ptr);
    return kErrorOk;
  }

  ASMJIT_FORCEINLINE Error append(T item) noexcept {
    ASMJIT_ASSERT(isInitialized());
    Block* block = _block[Globals::kLinkLast];

    if (!block->canAppend<T>()) {
      ASMJIT_PROPAGATE(_prepareBlock(Globals::kLinkLast, kStartBlockIndex));
      block = _block[Globals::kLinkLast];
    }

    T* ptr = block->end<T>();
    ASMJIT_ASSERT(ptr >= block->data<T>() && ptr <= block->data<T>() + (kNumBlockItems - 1));

    *ptr++ = item;
    block->setEnd(ptr);
    return kErrorOk;
  }

  ASMJIT_FORCEINLINE T popFirst() noexcept {
    ASMJIT_ASSERT(isInitialized());
    ASMJIT_ASSERT(!empty());

    Block* block = _block[Globals::kLinkFirst];
    ASMJIT_ASSERT(!block->empty());

    T* ptr = block->start<T>();
    T item = *ptr++;

    block->setStart(ptr);
    if (block->empty())
      _cleanupBlock(Globals::kLinkFirst, kMidBlockIndex);

    return item;
  }

  ASMJIT_FORCEINLINE T pop() noexcept {
    ASMJIT_ASSERT(isInitialized());
    ASMJIT_ASSERT(!empty());

    Block* block = _block[Globals::kLinkLast];
    ASMJIT_ASSERT(!block->empty());

    T* ptr = block->end<T>();
    T item = *--ptr;
    ASMJIT_ASSERT(ptr >= block->data<T>());
    ASMJIT_ASSERT(ptr >= block->start<T>());

    block->setEnd(ptr);
    if (block->empty())
      _cleanupBlock(Globals::kLinkLast, kMidBlockIndex);

    return item;
  }
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ZONESTACK_H
