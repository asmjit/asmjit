// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_VECTYPES_H
#define _ASMJIT_BASE_VECTYPES_H

// [Dependencies - AsmJit]
#include "../base/defs.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::Vec64Data]
// ============================================================================

//! @brief 64-bit vector register data.
union Vec64Data {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Set all eight signed 8-bit integers.
  static ASMJIT_INLINE Vec64Data fromSb(
    int8_t x0, int8_t x1, int8_t x2, int8_t x3, int8_t x4, int8_t x5, int8_t x6, int8_t x7)
  {
    Vec64Data self;
    self.setSb(x0, x1, x2, x3, x4, x5, x6, x7);
    return self;
  }

  //! @brief Set all eight signed 8-bit integers.
  static ASMJIT_INLINE Vec64Data fromSb(
    int8_t x0)
  {
    Vec64Data self;
    self.setSb(x0);
    return self;
  }

  //! @brief Set all eight unsigned 8-bit integers.
  static ASMJIT_INLINE Vec64Data fromUb(
    uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7)
  {
    Vec64Data self;
    self.setUb(x0, x1, x2, x3, x4, x5, x6, x7);
    return self;
  }

  //! @brief Set all eight unsigned 8-bit integers.
  static ASMJIT_INLINE Vec64Data fromUb(
    uint8_t x0)
  {
    Vec64Data self;
    self.setUb(x0);
    return self;
  }

  //! @brief Set all four signed 16-bit integers.
  static ASMJIT_INLINE Vec64Data fromSw(
    int16_t x0, int16_t x1, int16_t x2, int16_t x3)
  {
    Vec64Data self;
    self.setSw(x0, x1, x2, x3);
    return self;
  }

  //! @brief Set all four signed 16-bit integers.
  static ASMJIT_INLINE Vec64Data fromSw(
    int16_t x0)
  {
    Vec64Data self;
    self.setSw(x0);
    return self;
  }

  //! @brief Set all four unsigned 16-bit integers.
  static ASMJIT_INLINE Vec64Data fromUw(
    uint16_t x0, uint16_t x1, uint16_t x2, uint16_t x3)
  {
    Vec64Data self;
    self.setUw(x0, x1, x2, x3);
    return self;
  }

  //! @brief Set all four unsigned 16-bit integers.
  static ASMJIT_INLINE Vec64Data fromUw(
    uint16_t x0)
  {
    Vec64Data self;
    self.setUw(x0);
    return self;
  }

  //! @brief Set all two signed 32-bit integers.
  static ASMJIT_INLINE Vec64Data fromSd(
    int32_t x0, int32_t x1)
  {
    Vec64Data self;
    self.setSd(x0, x1);
    return self;
  }

  //! @brief Set all two signed 32-bit integers.
  static ASMJIT_INLINE Vec64Data fromSd(
    int32_t x0)
  {
    Vec64Data self;
    self.setSd(x0);
    return self;
  }

  //! @brief Set all two unsigned 32-bit integers.
  static ASMJIT_INLINE Vec64Data fromUd(
    uint32_t x0, uint32_t x1)
  {
    Vec64Data self;
    self.setUd(x0, x1);
    return self;
  }

  //! @brief Set all two unsigned 32-bit integers.
  static ASMJIT_INLINE Vec64Data fromUd(
    uint32_t x0)
  {
    Vec64Data self;
    self.setUd(x0);
    return self;
  }

  //! @brief Set signed 64-bit integer.
  static ASMJIT_INLINE Vec64Data fromSq(
    int64_t x0)
  {
    Vec64Data self;
    self.setSq(x0);
    return self;
  }

  //! @brief Set unsigned 64-bit integer.
  static ASMJIT_INLINE Vec64Data fromUq(
    uint64_t x0)
  {
    Vec64Data self;
    self.setUq(x0);
    return self;
  }

  //! @brief Set all two SP-FP values.
  static ASMJIT_INLINE Vec64Data fromSf(
    float x0, float x1)
  {
    Vec64Data self;
    self.setSf(x0, x1);
    return self;
  }

  //! @brief Set all two SP-FP values.
  static ASMJIT_INLINE Vec64Data fromSf(
    float x0)
  {
    Vec64Data self;
    self.setSf(x0);
    return self;
  }

  //! @brief Set all two SP-FP values.
  static ASMJIT_INLINE Vec64Data fromDf(
    double x0)
  {
    Vec64Data self;
    self.setDf(x0);
    return self;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Set all eight signed 8-bit integers.
  ASMJIT_INLINE void setSb(
    int8_t x0, int8_t x1, int8_t x2, int8_t x3, int8_t x4, int8_t x5, int8_t x6, int8_t x7)
  {
    sb[0] = x0; sb[1] = x1; sb[2] = x2; sb[3] = x3;
    sb[4] = x4; sb[5] = x5; sb[6] = x6; sb[7] = x7;
  }

  //! @brief Set all eight signed 8-bit integers.
  ASMJIT_INLINE void setSb(
    int8_t x0)
  {
    setUb(static_cast<uint8_t>(x0));
  }

  //! @brief Set all eight unsigned 8-bit integers.
  ASMJIT_INLINE void setUb(
    uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7)
  {
    ub[0] = x0; ub[1] = x1; ub[2] = x2; ub[3] = x3;
    ub[4] = x4; ub[5] = x5; ub[6] = x6; ub[7] = x7;
  }

  //! @brief Set all eight unsigned 8-bit integers.
  ASMJIT_INLINE void setUb(
    uint8_t x0)
  {
    if (kArchHost64Bit) {
      uint64_t t = static_cast<uint64_t>(x0) * ASMJIT_UINT64_C(0x0101010101010101);
      uq[0] = t;
    }
    else {
      uint32_t t = static_cast<uint32_t>(x0) * static_cast<uint32_t>(0x01010101U);
      ud[0] = t;
      ud[1] = t;
    }
  }

  //! @brief Set all four signed 16-bit integers.
  ASMJIT_INLINE void setSw(
    int16_t x0, int16_t x1, int16_t x2, int16_t x3)
  {
    sw[0] = x0; sw[1] = x1; sw[2] = x2; sw[3] = x3;
  }

  //! @brief Set all four signed 16-bit integers.
  ASMJIT_INLINE void setSw(
    int16_t x0)
  {
    setUw(static_cast<uint16_t>(x0));
  }

  //! @brief Set all four unsigned 16-bit integers.
  ASMJIT_INLINE void setUw(
    uint16_t x0, uint16_t x1, uint16_t x2, uint16_t x3)
  {
    uw[0] = x0; uw[1] = x1; uw[2] = x2; uw[3] = x3;
  }

  //! @brief Set all four unsigned 16-bit integers.
  ASMJIT_INLINE void setUw(
    uint16_t x0)
  {
    if (kArchHost64Bit) {
      uint64_t t = static_cast<uint64_t>(x0) * ASMJIT_UINT64_C(0x0001000100010001);
      uq[0] = t;
    }
    else {
      uint32_t t = static_cast<uint32_t>(x0) * static_cast<uint32_t>(0x00010001U);
      ud[0] = t;
      ud[1] = t;
    }
  }

  //! @brief Set all two signed 32-bit integers.
  ASMJIT_INLINE void setSd(
    int32_t x0, int32_t x1)
  {
    sd[0] = x0; sd[1] = x1;
  }

  //! @brief Set all two signed 32-bit integers.
  ASMJIT_INLINE void setSd(
    int32_t x0)
  {
    sd[0] = x0; sd[1] = x0;
  }

  //! @brief Set all two unsigned 32-bit integers.
  ASMJIT_INLINE void setUd(
    uint32_t x0, uint32_t x1)
  {
    ud[0] = x0; ud[1] = x1;
  }

  //! @brief Set all two unsigned 32-bit integers.
  ASMJIT_INLINE void setUd(
    uint32_t x0)
  {
    ud[0] = x0; ud[1] = x0;
  }

  //! @brief Set signed 64-bit integer.
  ASMJIT_INLINE void setSq(
    int64_t x0)
  {
    sq[0] = x0;
  }

  //! @brief Set unsigned 64-bit integer.
  ASMJIT_INLINE void setUq(
    uint64_t x0)
  {
    uq[0] = x0;
  }

  //! @brief Set all two SP-FP values.
  ASMJIT_INLINE void setSf(
    float x0, float x1)
  {
    sf[0] = x0; sf[1] = x1;
  }

  //! @brief Set all two SP-FP values.
  ASMJIT_INLINE void setSf(
    float x0)
  {
    sf[0] = x0; sf[1] = x0;
  }

  //! @brief Set all two SP-FP values.
  ASMJIT_INLINE void setDf(
    double x0)
  {
    df[0] = x0;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Array of eight signed 8-bit integers.
  int8_t sb[8];
  //! @brief Array of eight unsigned 8-bit integers.
  uint8_t ub[8];
  //! @brief Array of four signed 16-bit integers.
  int16_t sw[4];
  //! @brief Array of four unsigned 16-bit integers.
  uint16_t uw[4];
  //! @brief Array of two signed 32-bit integers.
  int32_t sd[2];
  //! @brief Array of two unsigned 32-bit integers.
  uint32_t ud[2];
  //! @brief Array of one signed 64-bit integer.
  int64_t sq[1];
  //! @brief Array of one unsigned 64-bit integer.
  uint64_t uq[1];

  //! @brief Array of two SP-FP values.
  float sf[2];
  //! @brief Array of one DP-FP value.
  double df[1];
};

// ============================================================================
// [asmjit::Vec128Data]
// ============================================================================

//! @brief 128-bit vector register data.
union Vec128Data {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Set all sixteen signed 8-bit integers.
  static ASMJIT_INLINE Vec128Data fromSb(
    int8_t x0 , int8_t x1 , int8_t x2 , int8_t x3 ,
    int8_t x4 , int8_t x5 , int8_t x6 , int8_t x7 ,
    int8_t x8 , int8_t x9 , int8_t x10, int8_t x11,
    int8_t x12, int8_t x13, int8_t x14, int8_t x15)
  {
    Vec128Data self;
    self.setSb(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
    return self;
  }

  //! @brief Set all sixteen signed 8-bit integers.
  static ASMJIT_INLINE Vec128Data fromSb(
    int8_t x0)
  {
    Vec128Data self;
    self.setSb(x0);
    return self;
  }

  //! @brief Set all sixteen unsigned 8-bit integers.
  static ASMJIT_INLINE Vec128Data fromUb(
    uint8_t x0 , uint8_t x1 , uint8_t x2 , uint8_t x3 ,
    uint8_t x4 , uint8_t x5 , uint8_t x6 , uint8_t x7 ,
    uint8_t x8 , uint8_t x9 , uint8_t x10, uint8_t x11,
    uint8_t x12, uint8_t x13, uint8_t x14, uint8_t x15)
  {
    Vec128Data self;
    self.setUb(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
    return self;
  }

  //! @brief Set all sixteen unsigned 8-bit integers.
  static ASMJIT_INLINE Vec128Data fromUb(
    uint8_t x0)
  {
    Vec128Data self;
    self.setUb(x0);
    return self;
  }

  //! @brief Set all eight signed 16-bit integers.
  static ASMJIT_INLINE Vec128Data fromSw(
    int16_t x0, int16_t x1, int16_t x2, int16_t x3, int16_t x4, int16_t x5, int16_t x6, int16_t x7)
  {
    Vec128Data self;
    self.setSw(x0, x1, x2, x3, x4, x5, x6, x7);
    return self;
  }

  //! @brief Set all eight signed 16-bit integers.
  static ASMJIT_INLINE Vec128Data fromSw(
    int16_t x0)
  {
    Vec128Data self;
    self.setSw(x0);
    return self;
  }

  //! @brief Set all eight unsigned 16-bit integers.
  static ASMJIT_INLINE Vec128Data fromUw(
    uint16_t x0, uint16_t x1, uint16_t x2, uint16_t x3, uint16_t x4, uint16_t x5, uint16_t x6, uint16_t x7)
  {
    Vec128Data self;
    self.setUw(x0, x1, x2, x3, x4, x5, x6, x7);
    return self;
  }

  //! @brief Set all eight unsigned 16-bit integers.
  static ASMJIT_INLINE Vec128Data fromUw(
    uint16_t x0)
  {
    Vec128Data self;
    self.setUw(x0);
    return self;
  }

  //! @brief Set all four signed 32-bit integers.
  static ASMJIT_INLINE Vec128Data fromSd(
    int32_t x0, int32_t x1, int32_t x2, int32_t x3)
  {
    Vec128Data self;
    self.setSd(x0, x1, x2, x3);
    return self;
  }

  //! @brief Set all four signed 32-bit integers.
  static ASMJIT_INLINE Vec128Data fromSd(
    int32_t x0)
  {
    Vec128Data self;
    self.setSd(x0);
    return self;
  }

  //! @brief Set all four unsigned 32-bit integers.
  static ASMJIT_INLINE Vec128Data fromUd(
    uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3)
  {
    Vec128Data self;
    self.setUd(x0, x1, x2, x3);
    return self;
  }

  //! @brief Set all four unsigned 32-bit integers.
  static ASMJIT_INLINE Vec128Data fromUd(
    uint32_t x0)
  {
    Vec128Data self;
    self.setUd(x0);
    return self;
  }

  //! @brief Set all two signed 64-bit integers.
  static ASMJIT_INLINE Vec128Data fromSq(
    int64_t x0, int64_t x1)
  {
    Vec128Data self;
    self.setSq(x0, x1);
    return self;
  }

  //! @brief Set all two signed 64-bit integers.
  static ASMJIT_INLINE Vec128Data fromSq(
    int64_t x0)
  {
    Vec128Data self;
    self.setSq(x0);
    return self;
  }

  //! @brief Set all two unsigned 64-bit integers.
  static ASMJIT_INLINE Vec128Data fromUq(
    uint64_t x0, uint64_t x1)
  {
    Vec128Data self;
    self.setUq(x0, x1);
    return self;
  }

  //! @brief Set all two unsigned 64-bit integers.
  static ASMJIT_INLINE Vec128Data fromUq(
    uint64_t x0)
  {
    Vec128Data self;
    self.setUq(x0);
    return self;
  }

  //! @brief Set all four SP-FP floats.
  static ASMJIT_INLINE Vec128Data fromSf(
    float x0, float x1, float x2, float x3)
  {
    Vec128Data self;
    self.setSf(x0, x1, x2, x3);
    return self;
  }

  //! @brief Set all four SP-FP floats.
  static ASMJIT_INLINE Vec128Data fromSf(
    float x0)
  {
    Vec128Data self;
    self.setSf(x0);
    return self;
  }

  //! @brief Set all two DP-FP floats.
  static ASMJIT_INLINE Vec128Data fromDf(
    double x0, double x1)
  {
    Vec128Data self;
    self.setDf(x0, x1);
    return self;
  }

  //! @brief Set all two DP-FP floats.
  static ASMJIT_INLINE Vec128Data fromDf(
    double x0)
  {
    Vec128Data self;
    self.setDf(x0);
    return self;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Set all sixteen signed 8-bit integers.
  ASMJIT_INLINE void setSb(
    int8_t x0 , int8_t x1 , int8_t x2 , int8_t x3 ,
    int8_t x4 , int8_t x5 , int8_t x6 , int8_t x7 ,
    int8_t x8 , int8_t x9 , int8_t x10, int8_t x11,
    int8_t x12, int8_t x13, int8_t x14, int8_t x15)
  {
    sb[0 ] = x0 ; sb[1 ] = x1 ; sb[2 ] = x2 ; sb[3 ] = x3 ;
    sb[4 ] = x4 ; sb[5 ] = x5 ; sb[6 ] = x6 ; sb[7 ] = x7 ;
    sb[8 ] = x8 ; sb[9 ] = x9 ; sb[10] = x10; sb[11] = x11;
    sb[12] = x12; sb[13] = x13; sb[14] = x14; sb[15] = x15;
  }

  //! @brief Set all sixteen signed 8-bit integers.
  ASMJIT_INLINE void setSb(
    int8_t x0)
  {
    setUb(static_cast<uint8_t>(x0));
  }

  //! @brief Set all sixteen unsigned 8-bit integers.
  ASMJIT_INLINE void setUb(
    uint8_t x0 , uint8_t x1 , uint8_t x2 , uint8_t x3 ,
    uint8_t x4 , uint8_t x5 , uint8_t x6 , uint8_t x7 ,
    uint8_t x8 , uint8_t x9 , uint8_t x10, uint8_t x11,
    uint8_t x12, uint8_t x13, uint8_t x14, uint8_t x15)
  {
    ub[0 ] = x0 ; ub[1 ] = x1 ; ub[2 ] = x2 ; ub[3 ] = x3 ;
    ub[4 ] = x4 ; ub[5 ] = x5 ; ub[6 ] = x6 ; ub[7 ] = x7 ;
    ub[8 ] = x8 ; ub[9 ] = x9 ; ub[10] = x10; ub[11] = x11;
    ub[12] = x12; ub[13] = x13; ub[14] = x14; ub[15] = x15;
  }

  //! @brief Set all sixteen unsigned 8-bit integers.
  ASMJIT_INLINE void setUb(
    uint8_t x0)
  {
    if (kArchHost64Bit) {
      uint64_t t = static_cast<uint64_t>(x0) * ASMJIT_UINT64_C(0x0101010101010101);
      uq[0] = t;
      uq[1] = t;
    }
    else {
      uint32_t t = static_cast<uint32_t>(x0) * static_cast<uint32_t>(0x01010101U);
      ud[0] = t;
      ud[1] = t;
      ud[2] = t;
      ud[3] = t;
    }
  }

  //! @brief Set all eight signed 16-bit integers.
  ASMJIT_INLINE void setSw(
    int16_t x0, int16_t x1, int16_t x2, int16_t x3, int16_t x4, int16_t x5, int16_t x6, int16_t x7)
  {
    sw[0] = x0; sw[1] = x1; sw[2] = x2; sw[3] = x3;
    sw[4] = x4; sw[5] = x5; sw[6] = x6; sw[7] = x7;
  }

  //! @brief Set all eight signed 16-bit integers.
  ASMJIT_INLINE void setSw(
    int16_t x0)
  {
    setUw(static_cast<uint16_t>(x0));
  }

  //! @brief Set all eight unsigned 16-bit integers.
  ASMJIT_INLINE void setUw(
    uint16_t x0, uint16_t x1, uint16_t x2, uint16_t x3, uint16_t x4, uint16_t x5, uint16_t x6, uint16_t x7)
  {
    uw[0] = x0; uw[1] = x1; uw[2] = x2; uw[3] = x3;
    uw[4] = x4; uw[5] = x5; uw[6] = x6; uw[7] = x7;
  }

  //! @brief Set all eight unsigned 16-bit integers.
  ASMJIT_INLINE void setUw(
    uint16_t x0)
  {
    if (kArchHost64Bit) {
      uint64_t t = static_cast<uint64_t>(x0) * ASMJIT_UINT64_C(0x0001000100010001);
      uq[0] = t;
      uq[1] = t;
    }
    else {
      uint32_t t = static_cast<uint32_t>(x0) * static_cast<uint32_t>(0x00010001U);
      ud[0] = t;
      ud[1] = t;
      ud[2] = t;
      ud[3] = t;
    }
  }

  //! @brief Set all four signed 32-bit integers.
  ASMJIT_INLINE void setSd(
    int32_t x0, int32_t x1, int32_t x2, int32_t x3)
  {
    sd[0] = x0; sd[1] = x1; sd[2] = x2; sd[3] = x3;
  }

  //! @brief Set all four signed 32-bit integers.
  ASMJIT_INLINE void setSd(
    int32_t x0)
  {
    setUd(static_cast<uint32_t>(x0));
  }

  //! @brief Set all four unsigned 32-bit integers.
  ASMJIT_INLINE void setUd(
    uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3)
  {
    ud[0] = x0; ud[1] = x1; ud[2] = x2; ud[3] = x3;
  }

  //! @brief Set all four unsigned 32-bit integers.
  ASMJIT_INLINE void setUd(
    uint32_t x0)
  {
    if (kArchHost64Bit) {
      uint64_t t = (static_cast<uint64_t>(x0) << 32) + x0;
      uq[0] = t;
      uq[1] = t;
    }
    else {
      ud[0] = x0;
      ud[1] = x0;
      ud[2] = x0;
      ud[3] = x0;
    }
  }

  //! @brief Set all two signed 64-bit integers.
  ASMJIT_INLINE void setSq(
    int64_t x0, int64_t x1)
  {
    sq[0] = x0; sq[1] = x1;
  }

  //! @brief Set all two signed 64-bit integers.
  ASMJIT_INLINE void setSq(
    int64_t x0)
  {
    sq[0] = x0; sq[1] = x0;
  }

  //! @brief Set all two unsigned 64-bit integers.
  ASMJIT_INLINE void setUq(
    uint64_t x0, uint64_t x1)
  {
    uq[0] = x0; uq[1] = x1;
  }

  //! @brief Set all two unsigned 64-bit integers.
  ASMJIT_INLINE void setUq(
    uint64_t x0)
  {
    uq[0] = x0; uq[1] = x0;
  }

  //! @brief Set all four SP-FP floats.
  ASMJIT_INLINE void setSf(
    float x0, float x1, float x2, float x3)
  {
    sf[0] = x0; sf[1] = x1; sf[2] = x2; sf[3] = x3;
  }

  //! @brief Set all four SP-FP floats.
  ASMJIT_INLINE void setSf(
    float x0)
  {
    sf[0] = x0; sf[1] = x0; sf[2] = x0; sf[3] = x0;
  }

  //! @brief Set all two DP-FP floats.
  ASMJIT_INLINE void setDf(
    double x0, double x1)
  {
    df[0] = x0; df[1] = x1;
  }

  //! @brief Set all two DP-FP floats.
  ASMJIT_INLINE void setDf(
    double x0)
  {
    df[0] = x0; df[1] = x0;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Array of sixteen signed 8-bit integers.
  int8_t sb[16];
  //! @brief Array of sixteen unsigned 8-bit integers.
  uint8_t ub[16];
  //! @brief Array of eight signed 16-bit integers.
  int16_t sw[8];
  //! @brief Array of eight unsigned 16-bit integers.
  uint16_t uw[8];
  //! @brief Array of four signed 32-bit integers.
  int32_t sd[4];
  //! @brief Array of four unsigned 32-bit integers.
  uint32_t ud[4];
  //! @brief Array of two signed 64-bit integers.
  int64_t sq[2];
  //! @brief Array of two unsigned 64-bit integers.
  uint64_t uq[2];

  //! @brief Array of four 32-bit single precision floating points.
  float sf[4];
  //! @brief Array of two 64-bit double precision floating points.
  double df[2];
};

// ============================================================================
// [asmjit::Vec256Data]
// ============================================================================

//! @brief 256-bit vector register data.
union Vec256Data {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Set all thirty two signed 8-bit integers.
  static ASMJIT_INLINE Vec256Data fromSb(
    int8_t x0 , int8_t x1 , int8_t x2 , int8_t x3 ,
    int8_t x4 , int8_t x5 , int8_t x6 , int8_t x7 ,
    int8_t x8 , int8_t x9 , int8_t x10, int8_t x11,
    int8_t x12, int8_t x13, int8_t x14, int8_t x15,
    int8_t x16, int8_t x17, int8_t x18, int8_t x19,
    int8_t x20, int8_t x21, int8_t x22, int8_t x23,
    int8_t x24, int8_t x25, int8_t x26, int8_t x27,
    int8_t x28, int8_t x29, int8_t x30, int8_t x31)
  {
    Vec256Data self;
    self.setSb(
      x0,  x1 , x2 , x3 , x4 , x5 , x6 , x7 , x8 , x9 , x10, x11, x12, x13, x14, x15,
      x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31);
    return self;
  }

  //! @brief Set all thirty two signed 8-bit integers.
  static ASMJIT_INLINE Vec256Data fromSb(
    int8_t x0)
  {
    Vec256Data self;
    self.setSb(x0);
    return self;
  }

  //! @brief Set all thirty two unsigned 8-bit integers.
  static ASMJIT_INLINE Vec256Data fromUb(
    uint8_t x0 , uint8_t x1 , uint8_t x2 , uint8_t x3 ,
    uint8_t x4 , uint8_t x5 , uint8_t x6 , uint8_t x7 ,
    uint8_t x8 , uint8_t x9 , uint8_t x10, uint8_t x11,
    uint8_t x12, uint8_t x13, uint8_t x14, uint8_t x15,
    uint8_t x16, uint8_t x17, uint8_t x18, uint8_t x19,
    uint8_t x20, uint8_t x21, uint8_t x22, uint8_t x23,
    uint8_t x24, uint8_t x25, uint8_t x26, uint8_t x27,
    uint8_t x28, uint8_t x29, uint8_t x30, uint8_t x31)
  {
    Vec256Data self;
    self.setUb(
      x0,  x1 , x2 , x3 , x4 , x5 , x6 , x7 , x8 , x9 , x10, x11, x12, x13, x14, x15,
      x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31);
    return self;
  }

  //! @brief Set all thirty two unsigned 8-bit integers.
  static ASMJIT_INLINE Vec256Data fromUb(
    uint8_t x0)
  {
    Vec256Data self;
    self.setUb(x0);
    return self;
  }

  //! @brief Set all sixteen signed 16-bit integers.
  static ASMJIT_INLINE Vec256Data fromSw(
    int16_t x0, int16_t x1, int16_t x2 , int16_t x3 , int16_t x4 , int16_t x5 , int16_t x6 , int16_t x7 ,
    int16_t x8, int16_t x9, int16_t x10, int16_t x11, int16_t x12, int16_t x13, int16_t x14, int16_t x15)
  {
    Vec256Data self;
    self.setSw(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
    return self;
  }

  //! @brief Set all sixteen signed 16-bit integers.
  static ASMJIT_INLINE Vec256Data fromSw(
    int16_t x0)
  {
    Vec256Data self;
    self.setSw(x0);
    return self;
  }

  //! @brief Set all sixteen unsigned 16-bit integers.
  static ASMJIT_INLINE Vec256Data fromUw(
    uint16_t x0, uint16_t x1, uint16_t x2 , uint16_t x3 , uint16_t x4 , uint16_t x5 , uint16_t x6 , uint16_t x7 ,
    uint16_t x8, uint16_t x9, uint16_t x10, uint16_t x11, uint16_t x12, uint16_t x13, uint16_t x14, uint16_t x15)
  {
    Vec256Data self;
    self.setUw(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
    return self;
  }

  //! @brief Set all sixteen unsigned 16-bit integers.
  static ASMJIT_INLINE Vec256Data fromUw(
    uint16_t x0)
  {
    Vec256Data self;
    self.setUw(x0);
    return self;
  }

  //! @brief Set all eight signed 32-bit integers.
  static ASMJIT_INLINE Vec256Data fromSd(
    int32_t x0, int32_t x1, int32_t x2, int32_t x3,
    int32_t x4, int32_t x5, int32_t x6, int32_t x7)
  {
    Vec256Data self;
    self.setSd(x0, x1, x2, x3, x4, x5, x6, x7);
    return self;
  }

  //! @brief Set all eight signed 32-bit integers.
  static ASMJIT_INLINE Vec256Data fromSd(
    int32_t x0)
  {
    Vec256Data self;
    self.setSd(x0);
    return self;
  }

  //! @brief Set all eight unsigned 32-bit integers.
  static ASMJIT_INLINE Vec256Data fromUd(
    uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
    uint32_t x4, uint32_t x5, uint32_t x6, uint32_t x7)
  {
    Vec256Data self;
    self.setUd(x0, x1, x2, x3, x4, x5, x6, x7);
    return self;
  }

  //! @brief Set all eight unsigned 32-bit integers.
  static ASMJIT_INLINE Vec256Data fromUd(
    uint32_t x0)
  {
    Vec256Data self;
    self.setUd(x0);
    return self;
  }

  //! @brief Set all four signed 64-bit integers.
  static ASMJIT_INLINE Vec256Data fromSq(
    int64_t x0, int64_t x1, int64_t x2, int64_t x3)
  {
    Vec256Data self;
    self.setSq(x0, x1, x2, x3);
    return self;
  }

  //! @brief Set all four signed 64-bit integers.
  static ASMJIT_INLINE Vec256Data fromSq(
    int64_t x0)
  {
    Vec256Data self;
    self.setSq(x0);
    return self;
  }

  //! @brief Set all four unsigned 64-bit integers.
  static ASMJIT_INLINE Vec256Data fromUq(
    uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3)
  {
    Vec256Data self;
    self.setUq(x0, x1, x2, x3);
    return self;
  }

  //! @brief Set all four unsigned 64-bit integers.
  static ASMJIT_INLINE Vec256Data fromUq(
    uint64_t x0)
  {
    Vec256Data self;
    self.setUq(x0);
    return self;
  }

  //! @brief Set all eight SP-FP floats.
  static ASMJIT_INLINE Vec256Data fromSf(
    float x0, float x1, float x2, float x3,
    float x4, float x5, float x6, float x7)
  {
    Vec256Data self;
    self.setSf(x0, x1, x2, x3, x4, x5, x6, x7);
    return self;
  }

  //! @brief Set all eight SP-FP floats.
  static ASMJIT_INLINE Vec256Data fromSf(
    float x0)
  {
    Vec256Data self;
    self.setSf(x0);
    return self;
  }

  //! @brief Set all four DP-FP floats.
  static ASMJIT_INLINE Vec256Data fromDf(
    double x0, double x1, double x2, double x3)
  {
    Vec256Data self;
    self.setDf(x0, x1, x2, x3);
    return self;
  }

  //! @brief Set all four DP-FP floats.
  static ASMJIT_INLINE Vec256Data fromDf(
    double x0)
  {
    Vec256Data self;
    self.setDf(x0);
    return self;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Set all thirty two signed 8-bit integers.
  ASMJIT_INLINE void setSb(
    int8_t x0 , int8_t x1 , int8_t x2 , int8_t x3 ,
    int8_t x4 , int8_t x5 , int8_t x6 , int8_t x7 ,
    int8_t x8 , int8_t x9 , int8_t x10, int8_t x11,
    int8_t x12, int8_t x13, int8_t x14, int8_t x15,
    int8_t x16, int8_t x17, int8_t x18, int8_t x19,
    int8_t x20, int8_t x21, int8_t x22, int8_t x23,
    int8_t x24, int8_t x25, int8_t x26, int8_t x27,
    int8_t x28, int8_t x29, int8_t x30, int8_t x31)
  {
    sb[0 ] = x0 ; sb[1 ] = x1 ; sb[2 ] = x2 ; sb[3 ] = x3 ;
    sb[4 ] = x4 ; sb[5 ] = x5 ; sb[6 ] = x6 ; sb[7 ] = x7 ;
    sb[8 ] = x8 ; sb[9 ] = x9 ; sb[10] = x10; sb[11] = x11;
    sb[12] = x12; sb[13] = x13; sb[14] = x14; sb[15] = x15;
    sb[16] = x16; sb[17] = x17; sb[18] = x18; sb[19] = x19;
    sb[20] = x20; sb[21] = x21; sb[22] = x22; sb[23] = x23;
    sb[24] = x24; sb[25] = x25; sb[26] = x26; sb[27] = x27;
    sb[28] = x28; sb[29] = x29; sb[30] = x30; sb[31] = x31;
  }

  //! @brief Set all thirty two signed 8-bit integers.
  ASMJIT_INLINE void setSb(
    int8_t x0)
  {
    setUb(static_cast<uint8_t>(x0));
  }

  //! @brief Set all thirty two unsigned 8-bit integers.
  ASMJIT_INLINE void setUb(
    uint8_t x0 , uint8_t x1 , uint8_t x2 , uint8_t x3 ,
    uint8_t x4 , uint8_t x5 , uint8_t x6 , uint8_t x7 ,
    uint8_t x8 , uint8_t x9 , uint8_t x10, uint8_t x11,
    uint8_t x12, uint8_t x13, uint8_t x14, uint8_t x15,
    uint8_t x16, uint8_t x17, uint8_t x18, uint8_t x19,
    uint8_t x20, uint8_t x21, uint8_t x22, uint8_t x23,
    uint8_t x24, uint8_t x25, uint8_t x26, uint8_t x27,
    uint8_t x28, uint8_t x29, uint8_t x30, uint8_t x31)
  {
    ub[0 ] = x0 ; ub[1 ] = x1 ; ub[2 ] = x2 ; ub[3 ] = x3 ;
    ub[4 ] = x4 ; ub[5 ] = x5 ; ub[6 ] = x6 ; ub[7 ] = x7 ;
    ub[8 ] = x8 ; ub[9 ] = x9 ; ub[10] = x10; ub[11] = x11;
    ub[12] = x12; ub[13] = x13; ub[14] = x14; ub[15] = x15;
    ub[16] = x16; ub[17] = x17; ub[18] = x18; ub[19] = x19;
    ub[20] = x20; ub[21] = x21; ub[22] = x22; ub[23] = x23;
    ub[24] = x24; ub[25] = x25; ub[26] = x26; ub[27] = x27;
    ub[28] = x28; ub[29] = x29; ub[30] = x30; ub[31] = x31;
  }

  //! @brief Set all thirty two unsigned 8-bit integers.
  ASMJIT_INLINE void setUb(
    uint8_t x0)
  {
    if (kArchHost64Bit) {
      uint64_t t = static_cast<uint64_t>(x0)* ASMJIT_UINT64_C(0x0101010101010101);
      uq[0] = t;
      uq[1] = t;
      uq[2] = t;
      uq[3] = t;
    }
    else {
      uint32_t t = static_cast<uint32_t>(x0)* static_cast<uint32_t>(0x01010101U);
      ud[0] = t;
      ud[1] = t;
      ud[2] = t;
      ud[3] = t;
      ud[4] = t;
      ud[5] = t;
      ud[6] = t;
      ud[7] = t;
    }
  }

  //! @brief Set all sixteen signed 16-bit integers.
  ASMJIT_INLINE void setSw(
    int16_t x0, int16_t x1, int16_t x2, int16_t x3, int16_t x4, int16_t x5, int16_t x6, int16_t x7,
    int16_t x8, int16_t x9, int16_t x10, int16_t x11, int16_t x12, int16_t x13, int16_t x14, int16_t x15)
  {
    sw[0 ] = x0 ; sw[1 ] = x1 ; sw[2 ] = x2 ; sw[3 ] = x3 ;
    sw[4 ] = x4 ; sw[5 ] = x5 ; sw[6 ] = x6 ; sw[7 ] = x7 ;
    sw[8 ] = x8 ; sw[9 ] = x9 ; sw[10] = x10; sw[11] = x11;
    sw[12] = x12; sw[13] = x13; sw[14] = x14; sw[15] = x15;
  }

  //! @brief Set all sixteen signed 16-bit integers.
  ASMJIT_INLINE void setSw(
    int16_t x0)
  {
    setUw(static_cast<uint16_t>(x0));
  }

  //! @brief Set all sixteen unsigned 16-bit integers.
  ASMJIT_INLINE void setUw(
    uint16_t x0, uint16_t x1, uint16_t x2 , uint16_t x3 , uint16_t x4 , uint16_t x5 , uint16_t x6 , uint16_t x7 ,
    uint16_t x8, uint16_t x9, uint16_t x10, uint16_t x11, uint16_t x12, uint16_t x13, uint16_t x14, uint16_t x15)
  {
    uw[0 ] = x0 ; uw[1 ] = x1 ; uw[2 ] = x2 ; uw[3 ] = x3 ;
    uw[4 ] = x4 ; uw[5 ] = x5 ; uw[6 ] = x6 ; uw[7 ] = x7 ;
    uw[8 ] = x8 ; uw[9 ] = x9 ; uw[10] = x10; uw[11] = x11;
    uw[12] = x12; uw[13] = x13; uw[14] = x14; uw[15] = x15;
  }

  //! @brief Set all eight unsigned 16-bit integers.
  ASMJIT_INLINE void setUw(
    uint16_t x0)
  {
    if (kArchHost64Bit) {
      uint64_t t = static_cast<uint64_t>(x0)* ASMJIT_UINT64_C(0x0001000100010001);
      uq[0] = t;
      uq[1] = t;
      uq[2] = t;
      uq[3] = t;
    }
    else {
      uint32_t t = static_cast<uint32_t>(x0)* static_cast<uint32_t>(0x00010001U);
      ud[0] = t;
      ud[1] = t;
      ud[2] = t;
      ud[3] = t;
      ud[4] = t;
      ud[5] = t;
      ud[6] = t;
      ud[7] = t;
    }
  }

  //! @brief Set all eight signed 32-bit integers.
  ASMJIT_INLINE void setSd(
    int32_t x0, int32_t x1, int32_t x2, int32_t x3,
    int32_t x4, int32_t x5, int32_t x6, int32_t x7)
  {
    sd[0] = x0; sd[1] = x1; sd[2] = x2; sd[3] = x3;
    sd[4] = x4; sd[5] = x5; sd[6] = x6; sd[7] = x7;
  }

  //! @brief Set all eight signed 32-bit integers.
  ASMJIT_INLINE void setSd(
    int32_t x0)
  {
    setUd(static_cast<uint32_t>(x0));
  }

  //! @brief Set all eight unsigned 32-bit integers.
  ASMJIT_INLINE void setUd(
    uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
    uint32_t x4, uint32_t x5, uint32_t x6, uint32_t x7)
  {
    ud[0] = x0; ud[1] = x1; ud[2] = x2; ud[3] = x3;
    ud[4] = x4; ud[5] = x5; ud[6] = x6; ud[7] = x7;
  }

  //! @brief Set all eight unsigned 32-bit integers.
  ASMJIT_INLINE void setUd(
    uint32_t x0)
  {
    if (kArchHost64Bit) {
      uint64_t t = (static_cast<uint64_t>(x0) << 32) + x0;
      uq[0] = t;
      uq[1] = t;
      uq[2] = t;
      uq[3] = t;
    }
    else {
      ud[0] = x0;
      ud[1] = x0;
      ud[2] = x0;
      ud[3] = x0;
      ud[4] = x0;
      ud[5] = x0;
      ud[6] = x0;
      ud[7] = x0;
    }
  }

  //! @brief Set all four signed 64-bit integers.
  ASMJIT_INLINE void setSq(
    int64_t x0, int64_t x1, int64_t x2, int64_t x3)
  {
    sq[0] = x0; sq[1] = x1; sq[2] = x2; sq[3] = x3;
  }

  //! @brief Set all four signed 64-bit integers.
  ASMJIT_INLINE void setSq(
    int64_t x0)
  {
    sq[0] = x0; sq[1] = x0; sq[2] = x0; sq[3] = x0;
  }

  //! @brief Set all four unsigned 64-bit integers.
  ASMJIT_INLINE void setUq(
    uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3)
  {
    uq[0] = x0; uq[1] = x1; uq[2] = x2; uq[3] = x3;
  }

  //! @brief Set all four unsigned 64-bit integers.
  ASMJIT_INLINE void setUq(
    uint64_t x0)
  {
    uq[0] = x0; uq[1] = x0; uq[2] = x0; uq[3] = x0;
  }

  //! @brief Set all eight SP-FP floats.
  ASMJIT_INLINE void setSf(
    float x0, float x1, float x2, float x3,
    float x4, float x5, float x6, float x7)
  {
    sf[0] = x0; sf[1] = x1; sf[2] = x2; sf[3] = x3;
    sf[4] = x4; sf[5] = x5; sf[6] = x6; sf[7] = x7;
  }

  //! @brief Set all eight SP-FP floats.
  ASMJIT_INLINE void setSf(
    float x0)
  {
    sf[0] = x0; sf[1] = x0; sf[2] = x0; sf[3] = x0;
    sf[4] = x0; sf[5] = x0; sf[6] = x0; sf[7] = x0;
  }

  //! @brief Set all four DP-FP floats.
  ASMJIT_INLINE void setDf(
    double x0, double x1, double x2, double x3)
  {
    df[0] = x0; df[1] = x1; df[2] = x2; df[3] = x3;
  }

  //! @brief Set all four DP-FP floats.
  ASMJIT_INLINE void setDf(
    double x0)
  {
    df[0] = x0; df[1] = x0; df[2] = x0; df[3] = x0;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Array of thirty two signed 8-bit integers.
  int8_t sb[32];
  //! @brief Array of thirty two unsigned 8-bit integers.
  uint8_t ub[32];
  //! @brief Array of sixteen signed 16-bit integers.
  int16_t sw[16];
  //! @brief Array of sixteen unsigned 16-bit integers.
  uint16_t uw[16];
  //! @brief Array of eight signed 32-bit integers.
  int32_t sd[8];
  //! @brief Array of eight unsigned 32-bit integers.
  uint32_t ud[8];
  //! @brief Array of four signed 64-bit integers.
  int64_t sq[4];
  //! @brief Array of four unsigned 64-bit integers.
  uint64_t uq[4];

  //! @brief Array of eight 32-bit single precision floating points.
  float sf[8];
  //! @brief Array of four 64-bit double precision floating points.
  double df[4];
};

//! @}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_VECTYPES_H
