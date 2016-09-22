// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CONTAINERS_H
#define _ASMJIT_BASE_CONTAINERS_H

// [Dependencies]
#include "../base/utils.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [Forward Declarations]
// ============================================================================

class ZoneHeap;

// ============================================================================
// [asmjit::ZoneList<T>]
// ============================================================================

//! \internal
template <typename T>
class ZoneList {
public:
  ASMJIT_NONCOPYABLE(ZoneList<T>)

  // --------------------------------------------------------------------------
  // [Link]
  // --------------------------------------------------------------------------

  struct Link {
    //! Get next node.
    ASMJIT_INLINE Link* getNext() const noexcept { return _next; }
    //! Get value.
    ASMJIT_INLINE T getValue() const noexcept { return _value; }
    //! Set value to `value`.
    ASMJIT_INLINE void setValue(const T& value) noexcept { _value = value; }

    Link* _next;
    T _value;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE ZoneList() noexcept : _first(nullptr), _last(nullptr) {}
  ASMJIT_INLINE ~ZoneList() noexcept {}

  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isEmpty() const noexcept { return _first != nullptr; }
  ASMJIT_INLINE Link* getFirst() const noexcept { return _first; }
  ASMJIT_INLINE Link* getLast() const noexcept { return _last; }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() noexcept {
    _first = nullptr;
    _last = nullptr;
  }

  ASMJIT_INLINE void prepend(Link* link) noexcept {
    link->_next = _first;
    if (!_first) _last = link;
    _first = link;
  }

  ASMJIT_INLINE void append(Link* link) noexcept {
    link->_next = nullptr;
    if (!_first)
      _first = link;
    else
      _last->_next = link;
    _last = link;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Link* _first;
  Link* _last;
};

// ============================================================================
// [asmjit::ZoneVectorBase]
// ============================================================================

//! \internal
class ZoneVectorBase {
public:
  ASMJIT_NONCOPYABLE(ZoneVectorBase)

protected:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `ZoneVectorBase`.
  explicit ASMJIT_INLINE ZoneVectorBase(ZoneHeap* heap) noexcept
    : _heap(heap),
      _length(0),
      _capacity(0),
      _data(nullptr) {}

  //! Destroy the `ZoneVectorBase` and its data.
  ASMJIT_INLINE ~ZoneVectorBase() noexcept {}

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset the vector data and set its `length` to zero.
  //!
  //! If `releaseMemory` is true the vector buffer will be released to the
  //! system.
  ASMJIT_API void _reset(size_t sizeOfT, ZoneHeap* heap) noexcept;

  // --------------------------------------------------------------------------
  // [Grow / Reserve]
  // --------------------------------------------------------------------------

  ASMJIT_API Error _grow(size_t sizeOfT, size_t n) noexcept;
  ASMJIT_API Error _resize(size_t sizeOfT, size_t n) noexcept;
  ASMJIT_API Error _reserve(size_t sizeOfT, size_t n) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ZoneHeap* _heap;                       //!< Zone heap used to allocate data.
  size_t _length;                        //!< Length of the vector.
  size_t _capacity;                      //!< Capacity of the vector.
  void* _data;                           //!< Vector data.
};

// ============================================================================
// [asmjit::ZoneVector<T>]
// ============================================================================

//! Template used to store and manage array of Zone allocated data.
//!
//! This template has these advantages over other std::vector<>:
//! - Always non-copyable (designed to be non-copyable, we want it).
//! - No copy-on-write (some implementations of STL can use it).
//! - Optimized for working only with POD types.
//! - Uses ZoneHeap, thus small vectors are basically for free.
template <typename T>
class ZoneVector : public ZoneVectorBase {
public:
  ASMJIT_NONCOPYABLE(ZoneVector<T>)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `ZoneVector<T>`.
  explicit ASMJIT_INLINE ZoneVector(ZoneHeap* heap = nullptr) noexcept
    : ZoneVectorBase(heap) {}

  //! Destroy the `ZoneVector<T>` and its data.
  ASMJIT_INLINE ~ZoneVector() noexcept { _reset(sizeof(T), nullptr); }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  //! Get if this `ZoneVectorBase` has been initialized.
  ASMJIT_INLINE bool isInitialized() const noexcept { return _heap != nullptr; }
  //! Reset this vector and initialize to use the given ZoneHeap (can be null).
  ASMJIT_INLINE void reset(ZoneHeap* heap) noexcept { _reset(sizeof(T), heap); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get whether the vector is empty.
  ASMJIT_INLINE bool isEmpty() const noexcept { return _length == 0; }

  //! Get length.
  ASMJIT_INLINE size_t getLength() const noexcept { return _length; }
  //! Get capacity.
  ASMJIT_INLINE size_t getCapacity() const noexcept { return _capacity; }

  //! Get data.
  ASMJIT_INLINE T* getData() noexcept { return static_cast<T*>(_data); }
  //! \overload
  ASMJIT_INLINE const T* getData() const noexcept { return static_cast<const T*>(_data); }

  // --------------------------------------------------------------------------
  // [Grow / Reserve]
  // --------------------------------------------------------------------------

  //! Called to grow the buffer to fit at least `n` elements more.
  ASMJIT_INLINE Error grow(size_t n) noexcept { return ZoneVectorBase::_grow(sizeof(T), n); }
  //! Resize the vector to hold `n` elements.
  //!
  //! If `n` is greater than the current length then the additional elements'
  //! content will be initialized to zero. If `n` is less than the current
  //! length then the vector will be truncated to exactly `n` elements.
  ASMJIT_INLINE Error resize(size_t n) noexcept { return ZoneVectorBase::_resize(sizeof(T), n); }
  //! Realloc internal array to fit at least `n` items.
  ASMJIT_INLINE Error reserve(size_t n) noexcept { return ZoneVectorBase::_reserve(sizeof(T), n); }

  ASMJIT_INLINE void truncate(size_t n) noexcept {
    ASMJIT_ASSERT(n <= _length);
    _length = n;
  }

  ASMJIT_INLINE Error willGrow(size_t n) noexcept {
    return _capacity - _length < n ? grow(n) : static_cast<Error>(kErrorOk);
  }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  //! Clears the vector without reseting \ref ZoneHeap.
  ASMJIT_INLINE void clear() noexcept { _length = 0; }

  //! Prepend `item` to the vector.
  Error prepend(const T& item) noexcept {
    if (ASMJIT_UNLIKELY(_length == _capacity))
      ASMJIT_PROPAGATE(grow(1));

    ::memmove(static_cast<T*>(_data) + 1, _data, _length * sizeof(T));
    ::memcpy(_data, &item, sizeof(T));

    _length++;
    return kErrorOk;
  }

  //! Insert an `item` at the specified `index`.
  Error insert(size_t index, const T& item) noexcept {
    ASMJIT_ASSERT(index <= _length);

    if (ASMJIT_UNLIKELY(_length == _capacity))
      ASMJIT_PROPAGATE(grow(1));

    T* dst = static_cast<T*>(_data) + index;
    ::memmove(dst + 1, dst, _length - index);
    ::memcpy(dst, &item, sizeof(T));

    _length++;
    return kErrorOk;
  }

  //! Append `item` to the vector.
  Error append(const T& item) noexcept {
    if (ASMJIT_UNLIKELY(_length == _capacity))
      ASMJIT_PROPAGATE(grow(1));

    ::memcpy(static_cast<T*>(_data) + _length, &item, sizeof(T));

    _length++;
    return kErrorOk;
  }

  //! Append `item` to the vector (unsafe case).
  //!
  //! Can only be used together with `willGrow()`. If `willGrow(N)` returns
  //! `kErrorOk` then N elements can be added to the vector without checking
  //! if there is a place for them. Used mostly internally.
  ASMJIT_INLINE void appendUnsafe(const T& item) noexcept {
    ASMJIT_ASSERT(_length < _capacity);

    ::memcpy(static_cast<T*>(_data) + _length, &item, sizeof(T));
    _length++;
  }

  //! Get index of `val` or `kInvalidIndex` if not found.
  ASMJIT_INLINE size_t indexOf(const T& val) const noexcept {
    const T* data = static_cast<const T*>(_data);
    size_t length = _length;

    for (size_t i = 0; i < length; i++)
      if (data[i] == val)
        return i;

    return kInvalidIndex;
  }

  //! Remove item at index `i`.
  ASMJIT_INLINE void removeAt(size_t i) noexcept {
    ASMJIT_ASSERT(i < _length);

    T* data = static_cast<T*>(_data) + i;
    _length--;
    ::memmove(data, data + 1, _length - i);
  }

  //! Swap this pod-vector with `other`.
  ASMJIT_INLINE void swap(ZoneVector<T>& other) noexcept {
    ASMJIT_ASSERT(_heap == other._heap);

    Utils::swap(_length, other._length);
    Utils::swap(_capacity, other._capacity);
    Utils::swap(_data, other._data);
  }

  //! Get item at index `i`.
  ASMJIT_INLINE T& operator[](size_t i) noexcept {
    ASMJIT_ASSERT(i < _length);
    return getData()[i];
  }

  //! Get item at index `i`.
  ASMJIT_INLINE const T& operator[](size_t i) const noexcept {
    ASMJIT_ASSERT(i < _length);
    return getData()[i];
  }
};

// ============================================================================
// [asmjit::ZoneHashNode]
// ============================================================================

//! Node used by \ref ZoneHash<> template.
//!
//! You must provide function `bool eq(const Key& key)` in order to make
//! `ZoneHash::get()` working.
class ZoneHashNode {
public:
  ASMJIT_INLINE ZoneHashNode(uint32_t hVal = 0) noexcept
    : _hashNext(nullptr),
      _hVal(hVal) {}

  //! Next node in the chain, null if it terminates the chain.
  ZoneHashNode* _hashNext;
  //! Key hash.
  uint32_t _hVal;
  //! Should be used by Node that inherits ZoneHashNode, it aligns ZoneHashNode.
  uint32_t _customData;
};

// ============================================================================
// [asmjit::ZoneHashBase]
// ============================================================================

class ZoneHashBase {
public:
  ASMJIT_NONCOPYABLE(ZoneHashBase)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE ZoneHashBase(ZoneHeap* heap) noexcept {
    _heap = heap;
    _size = 0;
    _bucketsCount = 1;
    _bucketsGrow = 1;
    _data = _embedded;
    _embedded[0] = nullptr;
  }
  ASMJIT_INLINE ~ZoneHashBase() noexcept { reset(nullptr); }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isInitialized() const noexcept { return _heap != nullptr; }
  ASMJIT_API void reset(ZoneHeap* heap) noexcept;

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE size_t getSize() const noexcept { return _size; }

  ASMJIT_API void _rehash(uint32_t newCount) noexcept;
  ASMJIT_API ZoneHashNode* _put(ZoneHashNode* node) noexcept;
  ASMJIT_API ZoneHashNode* _del(ZoneHashNode* node) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ZoneHeap* _heap;                       //!< ZoneHeap used to allocate data.
  size_t _size;                          //!< Count of records inserted into the hash table.
  uint32_t _bucketsCount;                //!< Count of hash buckets.
  uint32_t _bucketsGrow;                 //!< When buckets array should grow.

  ZoneHashNode** _data;                  //!< Buckets data.
  ZoneHashNode* _embedded[1];            //!< Embedded data, used by empty hash tables.
};

// ============================================================================
// [asmjit::ZoneHash<Key, Node>]
// ============================================================================

//! Low-level hash table specialized for storing string keys and POD values.
//!
//! This hash table allows duplicates to be inserted (the API is so low
//! level that it's up to you if you allow it or not, as you should first
//! `get()` the node and then modify it or insert a new node by using `put()`,
//! depending on the intention).
template<typename Node>
class ZoneHash : public ZoneHashBase {
public:
  explicit ASMJIT_INLINE ZoneHash(ZoneHeap* heap = nullptr) noexcept
    : ZoneHashBase(heap) {}
  ASMJIT_INLINE ~ZoneHash() noexcept {}

  template<typename Key>
  ASMJIT_INLINE Node* get(const Key& key) const noexcept {
    uint32_t hMod = key.hVal % _bucketsCount;
    Node* node = static_cast<Node*>(_data[hMod]);

    while (node && !key.matches(node))
      node = static_cast<Node*>(node->_hashNext);
    return node;
  }

  ASMJIT_INLINE Node* put(Node* node) noexcept { return static_cast<Node*>(_put(node)); }
  ASMJIT_INLINE Node* del(Node* node) noexcept { return static_cast<Node*>(_del(node)); }
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CONTAINERS_H
