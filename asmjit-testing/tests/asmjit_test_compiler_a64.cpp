// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>
#if !defined(ASMJIT_NO_COMPILER) && !defined(ASMJIT_NO_AARCH64)

#include <asmjit/a64.h>

#include <asmjit-testing/tests/asmjit_test_compiler.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;

// a64::Compiler - A64TestCase
// ===========================

class A64TestCase : public TestCase {
public:
  A64TestCase(const char* name = nullptr)
    : TestCase(name, Arch::kAArch64) {}

  void compile(BaseCompiler& cc) override {
    compile(static_cast<a64::Compiler&>(cc));
  }

  virtual void compile(a64::Compiler& cc) = 0;
};

// a64::Compiler - A64Test_GpArgs
// ==============================

class A64Test_GpArgs : public A64TestCase {
public:
  uint32_t _arg_count;
  bool _preserve_fp;

  A64Test_GpArgs(uint32_t arg_count, bool preserve_fp)
    : _arg_count(arg_count),
      _preserve_fp(preserve_fp) {
    _name.assign_format("GpArgs {NumArgs=%u PreserveFP=%c}", arg_count, preserve_fp ? 'Y' : 'N');
  }

  static void add(TestApp& app) {
    for (uint32_t i = 0; i <= 16; i++) {
      app.add(new A64Test_GpArgs(i, true));
      app.add(new A64Test_GpArgs(i, false));
    }
  }

  void compile(a64::Compiler& cc) override {
    uint32_t arg_count = _arg_count;
    FuncSignature signature;

    signature.set_ret_t<int>();
    for (uint32_t i = 0; i < arg_count; i++) {
      signature.add_arg_t<int>();
    }

    FuncNode* func_node = cc.add_func(signature);
    if (_preserve_fp)
      func_node->frame().set_preserved_fp();

    a64::Gp sum;

    if (arg_count) {
      for (uint32_t i = 0; i < arg_count; i++) {
        a64::Gp i_reg = cc.new_gp32("i%u", i);
        func_node->set_arg(i, i_reg);

        if (i == 0)
          sum = i_reg;
        else
          cc.add(sum, sum, i_reg);
      }
    }
    else {
      sum = cc.new_gp32("i");
      cc.mov(sum, 0);
    }

    cc.ret(sum);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using U = unsigned int;

    using Func0 = U (*)();
    using Func1 = U (*)(U);
    using Func2 = U (*)(U, U);
    using Func3 = U (*)(U, U, U);
    using Func4 = U (*)(U, U, U, U);
    using Func5 = U (*)(U, U, U, U, U);
    using Func6 = U (*)(U, U, U, U, U, U);
    using Func7 = U (*)(U, U, U, U, U, U, U);
    using Func8 = U (*)(U, U, U, U, U, U, U, U);
    using Func9 = U (*)(U, U, U, U, U, U, U, U, U);
    using Func10 = U (*)(U, U, U, U, U, U, U, U, U, U);
    using Func11 = U (*)(U, U, U, U, U, U, U, U, U, U, U);
    using Func12 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U);
    using Func13 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U);
    using Func14 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    using Func15 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    using Func16 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);

    unsigned int result_ret = 0;
    unsigned int expect_ret = 0;

    switch (_arg_count) {
      case 0:
        result_ret = ptr_as_func<Func0>(_func)();
        expect_ret = 0;
        break;
      case 1:
        result_ret = ptr_as_func<Func1>(_func)(1);
        expect_ret = 1;
        break;
      case 2:
        result_ret = ptr_as_func<Func2>(_func)(1, 2);
        expect_ret = 1 + 2;
        break;
      case 3:
        result_ret = ptr_as_func<Func3>(_func)(1, 2, 3);
        expect_ret = 1 + 2 + 3;
        break;
      case 4:
        result_ret = ptr_as_func<Func4>(_func)(1, 2, 3, 4);
        expect_ret = 1 + 2 + 3 + 4;
        break;
      case 5:
        result_ret = ptr_as_func<Func5>(_func)(1, 2, 3, 4, 5);
        expect_ret = 1 + 2 + 3 + 4 + 5;
        break;
      case 6:
        result_ret = ptr_as_func<Func6>(_func)(1, 2, 3, 4, 5, 6);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6;
        break;
      case 7:
        result_ret = ptr_as_func<Func7>(_func)(1, 2, 3, 4, 5, 6, 7);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7;
        break;
      case 8:
        result_ret = ptr_as_func<Func8>(_func)(1, 2, 3, 4, 5, 6, 7, 8);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8;
        break;
      case 9:
        result_ret = ptr_as_func<Func9>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
        break;
      case 10:
        result_ret = ptr_as_func<Func10>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;
        break;
      case 11:
        result_ret = ptr_as_func<Func11>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
        break;
      case 12:
        result_ret = ptr_as_func<Func12>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12;
        break;
      case 13:
        result_ret = ptr_as_func<Func13>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13;
        break;
      case 14:
        result_ret = ptr_as_func<Func14>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14;
        break;
      case 15:
        result_ret = ptr_as_func<Func15>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15;
        break;
      case 16:
        result_ret = ptr_as_func<Func16>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15 + 16;
        break;
    }

    result.assign_format("ret={%u, %u}", result_ret >> 28, result_ret & 0x0FFFFFFFu);
    expect.assign_format("ret={%u, %u}", expect_ret >> 28, expect_ret & 0x0FFFFFFFu);

    return result == expect;
  }
};

// a64::Compiler - A64Test_Simd1
// =============================

class A64Test_Simd1 : public A64TestCase {
public:
  A64Test_Simd1()
    : A64TestCase("Simd1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Simd1());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*, const void*, const void*>());

    a64::Gp dst = cc.new_gp_ptr("dst");
    a64::Gp src1 = cc.new_gp_ptr("src1");
    a64::Gp src2 = cc.new_gp_ptr("src2");

    func_node->set_arg(0, dst);
    func_node->set_arg(1, src1);
    func_node->set_arg(2, src2);

    a64::Vec v1 = cc.new_vec_q("vec1");
    a64::Vec v2 = cc.new_vec_q("vec2");
    a64::Vec v3 = cc.new_vec_q("vec3");

    cc.ldr(v2, a64::ptr(src1));
    cc.ldr(v3, a64::ptr(src2));
    cc.add(v1.b16(), v2.b16(), v3.b16());
    cc.str(v1, a64::ptr(dst));

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, const void*, const void*);

    uint32_t dst[4];
    uint32_t a_src[4] = { 0 , 1 , 2 , 255 };
    uint32_t b_src[4] = { 99, 17, 33, 1   };

    // NOTE: It's a byte-add, so uint8_t(255+1) == 0.
    uint32_t ref[4] = { 99, 18, 35, 0 };

    ptr_as_func<Func>(_func)(dst, a_src, b_src);

    result.assign_format("ret={%u, %u, %u, %u}", dst[0], dst[1], dst[2], dst[3]);
    expect.assign_format("ret={%u, %u, %u, %u}", ref[0], ref[1], ref[2], ref[3]);

    return result == expect;
  }
};

// a64::Compiler - A64Test_SimdPreserveDst
// =======================================

// Regression coverage for https://github.com/asmjit/asmjit/issues/533.
class A64Test_SimdPreserveDst : public A64TestCase {
public:
  enum Op : uint32_t {
    kSri,
    kSriScalar,
    kAddhn2,
    kBfcvtn2,
    kTbx,
    kBicImm,
    kOrrImm,
    kBicShift,
    kOrrShift,
    kBicReg,
    kOrrReg,
    kOpCount
  };

  Op _op {};

  A64Test_SimdPreserveDst(Op op) : _op(op) {
    static const char* const names[] = {
      "SRI", "SRI scalar", "ADDHN2", "BFCVTN2", "TBX",
      "BIC immediate", "ORR immediate", "BIC shifted immediate", "ORR shifted immediate",
      "BIC register", "ORR register"
    };
    _name.assign_format("SimdPreserveDst {%s}", names[op]);
  }

  static void add(TestApp& app) {
    for (uint32_t i = 0; i < kOpCount; i++) {
      if (i == kBfcvtn2 && Arch::kHost == Arch::kAArch64 && !CpuInfo::host().features().arm().has_bf16()) {
        continue;
      }
      app.add(new A64Test_SimdPreserveDst(Op(i)));
    }
  }

  void compile(a64::Compiler& cc) override {
    FuncSignature called_signature = FuncSignature::build<Type::Vec128, Type::Vec128, Type::Vec128, Type::Vec128>();

    FuncNode* main_fn = cc.add_func(FuncSignature::build<void, void*, const void*>());
    FuncNode* called_fn = cc.new_func(called_signature);

    {
      a64::Gp dst = cc.new_gp_ptr("dst");
      a64::Gp src = cc.new_gp_ptr("src");
      a64::Vec v[3] = {cc.new_vec_q("v0"), cc.new_vec_q("v1"), cc.new_vec_q("v2")};

      main_fn->set_arg(0, dst);
      main_fn->set_arg(1, src);

      cc.ldr(v[0], a64::ptr(src, 0 * 16));
      cc.ldr(v[1], a64::ptr(src, 1 * 16));
      cc.ldr(v[2], a64::ptr(src, 2 * 16));

      InvokeNode* invoke_node;
      cc.invoke(Out(invoke_node), called_fn->label(), called_signature);

      invoke_node->set_arg(0, v[0]);
      invoke_node->set_arg(1, v[1]);
      invoke_node->set_arg(2, v[2]);
      invoke_node->set_ret(0, v[0]);

      cc.str(v[0], a64::ptr(dst));
      cc.end_func();
    }

    {
      cc.add_func(called_fn);

      a64::Vec a = cc.new_vec_q("a");
      a64::Vec b = cc.new_vec_q("b");
      a64::Vec dst = cc.new_vec_q("dst");

      called_fn->set_arg(0, a);
      called_fn->set_arg(1, b);
      called_fn->set_arg(2, dst);

      // The incoming destination is v2, whereas the result must be returned in v0.
      // Write-only metadata would allow the allocator to discard the initial v2.
      switch (_op) {
        case kSri      : cc.sri(dst.d2(), a.d2(), 61); break;
        case kSriScalar: cc.sri(dst.d(), a.d(), 61); break;
        case kAddhn2   : cc.addhn2(dst.b16(), a.h8(), b.h8()); break;
        case kBfcvtn2  : cc.bfcvtn2(dst.h8(), a.s4()); break;
        case kTbx      : cc.tbx(dst.b16(), a.b16(), b.b16()); break;
        case kBicImm   : cc.bic(dst.s4(), 0x12); break;
        case kOrrImm   : cc.orr(dst.s4(), 0x12); break;
        case kBicShift : cc.bic(dst.s4(), 0x12, a64::lsl(8)); break;
        case kOrrShift : cc.orr(dst.s4(), 0x12, a64::lsl(8)); break;
        case kBicReg   : cc.bic(dst.b16(), a.b16(), b.b16()); break;
        case kOrrReg   : cc.orr(dst.b16(), a.b16(), b.b16()); break;
        default: break;
      }

      cc.ret(dst);
      cc.end_func();
    }
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, const void*);

    // The first vector also contains exactly representable BF16 values (1, 2, 3, 4).
    const uint32_t src[12] = {
      0x3F800000u, 0x40000000u, 0x40400000u, 0x40800000u,
      0xFF100F00u, 0x80110E01u, 0xFE1F0D02u, 0x7F200C03u,
      0xA5B6C7D8u, 0x12345678u, 0x9ABCDEF0u, 0xFEDCBA98u
    };

    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(src);
    uint8_t dst[16] = {};
    uint8_t ref[16];

    memcpy(ref, src + 8, 16);

    switch (_op) {
      case kSri:
      case kSriScalar: {
        for (uint32_t i = 0; i < 2; i++) {
          uint64_t a;
          uint64_t d;
          memcpy(&a, bytes + i * 8, 8);
          memcpy(&d, ref + i * 8, 8);
          d = (d & ~uint64_t(7)) | (a >> 61);
          if (_op == kSriScalar && i == 1) {
            d = 0;
          }
          memcpy(ref + i * 8, &d, 8);
        }
        break;
      }

      case kAddhn2: {
        for (uint32_t i = 0; i < 8; i++) {
          uint16_t a;
          uint16_t b;
          memcpy(&a, bytes + i * 2, 2);
          memcpy(&b, bytes + 16 + i * 2, 2);
          ref[8 + i] = uint8_t((uint32_t(a) + b) >> 8);
        }
        break;
      }

      case kBfcvtn2: {
        for (uint32_t i = 0; i < 4; i++) {
          uint16_t value = uint16_t(src[i] >> 16);
          memcpy(ref + 8 + i * 2, &value, 2);
        }
        break;
      }

      case kTbx: {
        for (uint32_t i = 0; i < 16; i++) {
          uint32_t index = bytes[16 + i];
          if (index < 16) {
            ref[i] = bytes[index];
          }
        }
        break;
      }

      case kBicImm:
      case kOrrImm:
      case kBicShift:
      case kOrrShift: {
        for (uint32_t i = 0; i < 4; i++) {
          uint32_t mask = (_op == kBicShift || _op == kOrrShift) ? 0x1200u : 0x12u;
          uint32_t value = src[8 + i];
          value = (_op == kBicImm || _op == kBicShift) ? value & ~mask : value | mask;
          memcpy(ref + i * 4, &value, 4);
        }
        break;
      }

      case kBicReg:
      case kOrrReg: {
        for (uint32_t i = 0; i < 16; i++) {
          ref[i] = _op == kBicReg ? uint8_t(bytes[i] & ~bytes[16 + i]) : uint8_t(bytes[i] | bytes[16 + i]);
        }
        break;
      }

      default: {
        break;
      }
    }

    ptr_as_func<Func>(_func)(dst, src);
    result.assign_hex(dst, 16);
    expect.assign_hex(ref, 16);
    return result == expect;
  }
};

// a64::Compiler - A64Test_ManyRegs
// ================================

class A64Test_ManyRegs : public A64TestCase {
public:
  uint32_t _reg_count;

  A64Test_ManyRegs(uint32_t n)
    : A64TestCase(),
      _reg_count(n) {
    _name.assign_format("GpRegs {NumRegs=%u}", n);
  }

  static void add(TestApp& app) {
    for (uint32_t i = 2; i < 64; i++)
      app.add(new A64Test_ManyRegs(i));
  }

  void compile(a64::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    a64::Gp* regs = static_cast<a64::Gp*>(malloc(_reg_count * sizeof(a64::Gp)));

    for (uint32_t i = 0; i < _reg_count; i++) {
      regs[i] = cc.new_gp32("reg%u", i);
      cc.mov(regs[i], i + 1);
    }

    a64::Gp sum = cc.new_gp32("sum");
    cc.mov(sum, 0);

    for (uint32_t i = 0; i < _reg_count; i++) {
      cc.add(sum, sum, regs[i]);
    }

    cc.ret(sum);
    cc.end_func();

    free(regs);
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assign_format("ret={%d}", func());
    expect.assign_format("ret={%d}", calc_sum());

    return result == expect;
  }

  uint32_t calc_sum() const {
    return (_reg_count | 1) * ((_reg_count + 1) / 2);
  }
};

// a64::Compiler - A64Test_Adr
// ===========================

class A64Test_Adr : public A64TestCase {
public:
  A64Test_Adr()
    : A64TestCase("Adr") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Adr());
  }

  void compile(a64::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    a64::Gp addr = cc.new_gp_ptr("addr");
    a64::Gp val = cc.new_gp_ptr("val");

    Label L_Table = cc.new_label();

    cc.adr(addr, L_Table);
    cc.ldrsw(val, a64::ptr(addr, 8));
    cc.ret(val);
    cc.end_func();

    cc.bind(L_Table);
    cc.embed_int32(1);
    cc.embed_int32(2);
    cc.embed_int32(3);
    cc.embed_int32(4);
    cc.embed_int32(5);
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assign_format("ret={%d}", func());
    expect.assign_format("ret={%d}", 3);

    return result == expect;
  }
};

// a64::Compiler - A64Test_Branch1
// ===============================

class A64Test_Branch1 : public A64TestCase {
public:
  A64Test_Branch1()
    : A64TestCase("Branch1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Branch1());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*, size_t>());

    a64::Gp p = cc.new_gp_ptr("p");
    a64::Gp count = cc.new_gp_ptr("count");
    a64::Gp i = cc.new_gp_ptr("i");
    Label L = cc.new_label();

    func_node->set_arg(0, p);
    func_node->set_arg(1, count);

    cc.mov(i, 0);

    cc.bind(L);
    cc.strb(i.w(), a64::ptr(p, i));
    cc.add(i, i, 1);
    cc.cmp(i, count);
    cc.b_ne(L);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void* p, size_t n);
    Func func = ptr_as_func<Func>(_func);

    uint8_t array[16];
    func(array, 16);

    expect.assign("ret={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}");

    result.assign("ret={");
    for (size_t i = 0; i < 16; i++) {
      if (i)
        result.append(", ");
      result.append_format("%d", int(array[i]));
    }
    result.append("}");

    return result == expect;
  }
};

// a64::Compiler - A64Test_Invoke1
// ===============================

class A64Test_Invoke1 : public A64TestCase {
public:
  A64Test_Invoke1()
    : A64TestCase("Invoke1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke1());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<uint32_t, uint32_t, uint32_t>());

    a64::Gp x = cc.new_gp32("x");
    a64::Gp y = cc.new_gp32("y");
    a64::Gp r = cc.new_gp32("r");
    a64::Gp fn = cc.new_gp_ptr("fn");

    func_node->set_arg(0, x);
    func_node->set_arg(1, y);

    cc.mov(fn, (uint64_t)called_fn);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), fn, FuncSignature::build<uint32_t, uint32_t, uint32_t>(), pauth_c_func());
    invoke_node->set_arg(0, x);
    invoke_node->set_arg(1, y);
    invoke_node->set_ret(0, r);

    cc.ret(r);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(uint32_t, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t x = 49;
    uint32_t y = 7;

    result.assign_format("ret={%u}", func(x, y));
    expect.assign_format("ret={%u}", x - y);

    return result == expect;
  }

  static uint32_t called_fn(uint32_t x, uint32_t y) {
    return x - y;
  }
};

// a64::Compiler - A64Test_Invoke2
// ===============================

class A64Test_Invoke2 : public A64TestCase {
public:
  A64Test_Invoke2()
    : A64TestCase("Invoke2") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke2());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<double, double, double>());

    a64::Vec x = cc.new_vec_d("x");
    a64::Vec y = cc.new_vec_d("y");
    a64::Vec r = cc.new_vec_d("r");
    a64::Gp fn = cc.new_gp_ptr("fn");

    func_node->set_arg(0, x);
    func_node->set_arg(1, y);
    cc.mov(fn, (uint64_t)called_fn);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), fn, FuncSignature::build<double, double, double>(), pauth_c_func());
    invoke_node->set_arg(0, x);
    invoke_node->set_arg(1, y);
    invoke_node->set_ret(0, r);

    cc.ret(r);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double x = 49;
    double y = 7;

    result.assign_format("ret={%f}", func(x, y));
    expect.assign_format("ret={%f}", called_fn(x, y));

    return result == expect;
  }

  static double called_fn(double x, double y) {
    return x - y;
  }
};

// a64::Compiler - A64Test_Invoke3
// ===============================

class A64Test_Invoke3 : public A64TestCase {
public:
  A64Test_Invoke3()
    : A64TestCase("Invoke3") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke3());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<double, double, double>());

    a64::Vec x = cc.new_vec_d("x");
    a64::Vec y = cc.new_vec_d("y");
    a64::Vec r = cc.new_vec_d("r");
    a64::Gp fn = cc.new_gp_ptr("fn");

    func_node->set_arg(0, x);
    func_node->set_arg(1, y);
    cc.mov(fn, (uint64_t)called_fn);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), fn, FuncSignature::build<double, double, double>(), pauth_c_func());
    invoke_node->set_arg(0, y);
    invoke_node->set_arg(1, x);
    invoke_node->set_ret(0, r);

    cc.ret(r);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double x = 49;
    double y = 7;

    result.assign_format("ret={%f}", func(x, y));
    expect.assign_format("ret={%f}", called_fn(y, x));

    return result == expect;
  }

  static double called_fn(double x, double y) {
    return x - y;
  }
};

// a64::Compiler - A64Test_Invoke4
// ===============================

class A64Test_Invoke4 : public A64TestCase {
public:
  A64Test_Invoke4()
    : A64TestCase("Invoke4") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke4());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* called_fn = cc.new_func(FuncSignature::build<uint32_t, uint32_t, uint32_t>());
    FuncNode* main_fn = cc.add_func(FuncSignature::build<uint32_t, uint32_t, uint32_t>());

    {
      a64::Gp v1 = cc.new_gp32("v1");
      a64::Gp v2 = cc.new_gp32("v2");

      InvokeNode* invoke_node;
      cc.invoke(Out(invoke_node), called_fn->label(), FuncSignature::build<uint32_t, uint32_t, uint32_t>());

      main_fn->set_arg(0, v1);
      main_fn->set_arg(1, v2);

      invoke_node->set_ret(0, v1);
      invoke_node->set_arg(0, v1);
      invoke_node->set_arg(1, v2);

      cc.ret(v1);
      cc.end_func();
    }

    {
      a64::Gp v1 = cc.new_gp32("v1");
      a64::Gp v2 = cc.new_gp32("v2");

      cc.add_func(called_fn);

      called_fn->set_arg(0, v1);
      called_fn->set_arg(1, v2);

      cc.add(v1, v1, v2);
      cc.ret(v1);
      cc.end_func();
    }
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(uint32_t, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    result.assign_format("ret={%u}", func(10, 32));
    expect.assign_format("ret={%u}", 42);

    return result == expect;
  }
};

// a64::Compiler - A64Test_Invoke5
// ===============================

class A64Test_Invoke5 : public A64TestCase {
public:
  A64Test_Invoke5()
    : A64TestCase("Invoke5") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke5());
  }

  void compile(a64::Compiler& cc) override {
    static constexpr uint32_t kRegCount = 2;

    cc.add_func(FuncSignature::build<uint32_t>());

    a64::Gp fn = cc.new_gp_ptr("fn");
    a64::Gp out = cc.new_gp32("out");
    a64::Gp gpr[kRegCount];

    Label L_Done = cc.new_label();
    Label L_Failed = cc.new_label();

    cc.mov(fn, (uint64_t)called_fn);

    for (uint32_t i = 0; i < kRegCount; i++) {
      gpr[i] = cc.new_gp32("reg%u", i);
      cc.mov(gpr[i], i + 1000u);
    }

    cc.virt_reg_by_reg(gpr[kRegCount - 1])->set_home_id_hint(a64::Gp::kIdLr);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), fn, FuncSignature::build<uint32_t>(), pauth_c_func());
    invoke_node->set_ret(0, out);

    for (uint32_t i = 0; i < kRegCount; i++) {
      cc.cmp(gpr[i], i + 1000u);
      cc.b_ne(L_Failed);
    }

    cc.b(L_Done);

    cc.bind(L_Failed);
    cc.mov(out, 0xFFFFFFFFu);

    cc.bind(L_Done);
    cc.ret(out);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assign_format("ret={%u}", func());
    expect.assign_format("ret={%u}", called_fn());

    return result == expect;
  }

  static uint32_t called_fn() {
    return 42;
  }
};

// a64::Compiler - A64Test_JumpTable
// =================================

class A64Test_JumpTable : public A64TestCase {
public:
  bool _annotated;

  A64Test_JumpTable(bool annotated)
    : A64TestCase("JumpTable"),
      _annotated(annotated) {
    _name.assign_format("JumpTable {%s}", annotated ? "Annotated" : "Not Annotated");
  }

  enum Operator {
    kOperatorAdd = 0,
    kOperatorSub = 1,
    kOperatorMul = 2,
    kOperatorDiv = 3
  };

  static void add(TestApp& app) {
    app.add(new A64Test_JumpTable(false));
    app.add(new A64Test_JumpTable(true));
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<float, float, float, uint32_t>());

    a64::Vec a = cc.new_vec_s("a");
    a64::Vec b = cc.new_vec_s("b");
    a64::Gp op = cc.new_gp32("op");

    a64::Gp target = cc.new_gp_ptr("target");
    a64::Gp offset = cc.new_gp_ptr("offset");

    Label L_End = cc.new_label();

    Label L_Table = cc.new_label();
    Label L_Add = cc.new_label();
    Label L_Sub = cc.new_label();
    Label L_Mul = cc.new_label();
    Label L_Div = cc.new_label();

    func_node->set_arg(0, a);
    func_node->set_arg(1, b);
    func_node->set_arg(2, op);

    cc.adr(target, L_Table);
    cc.ldrsw(offset, a64::ptr(target, op, a64::sxtw(2)));
    cc.add(target, target, offset);

    // JumpAnnotation allows to annotate all possible jump targets of
    // instructions where it cannot be deduced from operands.
    if (_annotated) {
      JumpAnnotation* annotation = cc.new_jump_annotation();
      annotation->add_label(L_Add);
      annotation->add_label(L_Sub);
      annotation->add_label(L_Mul);
      annotation->add_label(L_Div);
      cc.br(target, annotation);
    }
    else {
      cc.br(target);
    }

    cc.bind(L_Add);
    cc.fadd(a, a, b);
    cc.b(L_End);

    cc.bind(L_Sub);
    cc.fsub(a, a, b);
    cc.b(L_End);

    cc.bind(L_Mul);
    cc.fmul(a, a, b);
    cc.b(L_End);

    cc.bind(L_Div);
    cc.fdiv(a, a, b);

    cc.bind(L_End);
    cc.ret(a);
    cc.end_func();

    cc.bind(L_Table);
    cc.embed_label_delta(L_Add, L_Table, 4);
    cc.embed_label_delta(L_Sub, L_Table, 4);
    cc.embed_label_delta(L_Mul, L_Table, 4);
    cc.embed_label_delta(L_Div, L_Table, 4);
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = float (*)(float, float, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    float dst[4];
    float ref[4];

    dst[0] = func(33.0f, 14.0f, kOperatorAdd);
    dst[1] = func(33.0f, 14.0f, kOperatorSub);
    dst[2] = func(10.0f, 6.0f, kOperatorMul);
    dst[3] = func(80.0f, 8.0f, kOperatorDiv);

    ref[0] = 47.0f;
    ref[1] = 19.0f;
    ref[2] = 60.0f;
    ref[3] = 10.0f;

    result.assign_format("ret={%f, %f, %f, %f}", double(dst[0]), double(dst[1]), double(dst[2]), double(dst[3]));
    expect.assign_format("ret={%f, %f, %f, %f}", double(ref[0]), double(ref[1]), double(ref[2]), double(ref[3]));

    return result == expect;
  }
};

// a64::Compiler - A64Test_FuncTailCall
// ==========================================

class A64Test_FuncTailCall : public A64TestCase {
public:
  A64Test_FuncTailCall() : A64TestCase("FuncTailCall") {}

  static void add(TestApp& app) {
    app.add(new A64Test_FuncTailCall());
  }

  void compile(a64::Compiler& cc) override {
    a64::Gp v0 = cc.new_gp32("v0");
    a64::Gp v1 = cc.new_gp32("v1");
    a64::Gp fn = cc.gpz(16);

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int>());
    func_node->set_arg(0, v0);
    func_node->set_arg(1, v1);

    // Just do something.
    cc.lsl(v0, v0, 1);
    cc.lsl(v1, v1, 1);

    cc.mov(fn, (uint64_t)called_fn);

    // Call a function.
    InvokeNode* invoke_node;
    cc.invoke_tail(Out(invoke_node), fn, FuncSignature::build<int, int, int>(), pauth_c_func());
    // Swap arguments around.
    invoke_node->set_arg(0, v1);
    invoke_node->set_arg(1, v0);
    invoke_node->set_ret(0, v0);

    // Tail call must not get here.
    cc.udf(Imm(0));

    cc.ret(v0);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(5, 10);
    int expect_ret = called_fn(20, 10);

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }
  static int called_fn(int a, int b) { return (a - b); }
};

// a64::Compiler - A64Test_FuncTailCallPermute
// ===========================================

// Tail call that reverses 16 arguments - 8 are passed in registers and 8 on the stack, so the outgoing stack arguments
// overlap the incoming ones and register and stack arguments are exchanged (parallel move).
class A64Test_FuncTailCallPermute : public A64TestCase {
public:
  static inline constexpr uint32_t kArgCount = 16;

  A64Test_FuncTailCallPermute(bool preserve_fp)
    : _preserve_fp(preserve_fp) {
    _name.assign_format("FuncTailCallPermute {PreserveFP=%c}", preserve_fp ? 'Y' : 'N');
  }

  static void add(TestApp& app) {
    app.add(new A64Test_FuncTailCallPermute(false));
    app.add(new A64Test_FuncTailCallPermute(true));
  }

  void compile(a64::Compiler& cc) override {
    FuncSignature signature(CallConvId::kCDecl);
    signature.set_ret_t<int>();
    for (uint32_t i = 0; i < kArgCount; i++) {
      signature.add_arg_t<int>();
    }

    FuncNode* func_node = cc.add_func(signature);

    if (_preserve_fp) {
      func_node->frame().set_preserved_fp();
    }

    a64::Gp args[kArgCount];
    for (uint32_t i = 0; i < kArgCount; i++) {
      args[i] = cc.new_gp32("a%u", i);
      func_node->set_arg(i, args[i]);
    }

    a64::Gp fn = cc.gpz(16);
    cc.mov(fn, (uint64_t)called_fn);

    InvokeNode* invoke_node;
    cc.invoke_tail(Out(invoke_node), fn, signature, pauth_c_func());

    for (uint32_t i = 0; i < kArgCount; i++) {
      invoke_node->set_arg(i, args[kArgCount - 1 - i]);
    }

    cc.udf(Imm(0));
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    int expect_ret = called_fn(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }

  // Position-weighted sum - any misplaced argument changes the result.
  static int called_fn(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
                       int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15) {
    int args[kArgCount] = { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15 };
    int sum = 0;
    for (uint32_t i = 0; i < kArgCount; i++) {
      sum += args[i] * int(i + 1) * int(i + 1);
    }
    return sum;
  }

  bool _preserve_fp;
};

// a64::Compiler - A64Test_ConsecutiveAllocation
// =============================================

class A64Test_ConsecutiveOutAllocation : public A64TestCase {
public:
  A64Test_ConsecutiveOutAllocation()
    : A64TestCase("ConsecutiveOutAllocation") {}

  static void add(TestApp& app) {
    app.add(new A64Test_ConsecutiveOutAllocation());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*, const void*>());

    a64::Gp src = cc.new_gp_ptr();
    a64::Gp dst = cc.new_gp_ptr();

    func_node->set_arg(0, dst);
    func_node->set_arg(1, src);

    a64::Vec v[] = {
      cc.new_vec_q(),
      cc.new_vec_q(),
      cc.new_vec_q(),
      cc.new_vec_q()
    };

    cc.ld2(v[0].b16(), v[1].b16(), a64::ptr(src));
    cc.add(src, src, 32);
    cc.ld2(v[2].b16(), v[3].b16(), a64::ptr(src));

    for (int i = 0; i < 4; ++i) {
      cc.str(v[i], a64::ptr(dst, i * 16));
    }

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, const void*);
    Func func = ptr_as_func<Func>(_func);

    uint8_t src_buffer[64];
    uint8_t dst_buffer[64];

    for (uint32_t i = 0; i < 64u; i++) {
      src_buffer[i] = static_cast<uint8_t>(i);
    }

    func(dst_buffer, src_buffer);

    result.assign("ret={");
    expect.assign("ret={");

    for (uint32_t i = 0; i < 64u; i++) {
      if (i != 0) {
        result.append(", ");
        expect.append(", ");
      }
      result.append_format("%u", dst_buffer[i]);
      // Each LD2 deinterleaves 32 bytes into an even-byte vector followed by an odd-byte vector.
      uint32_t src_index = (i / 32u) * 32u + (i % 16u) * 2u + (i / 16u) % 2u;
      expect.append_format("%u", src_buffer[src_index]);
    }

    return result == expect;
  }
};

// a64::Compiler - A64Test_LoadLane
// ================================

// Regression coverage for https://github.com/asmjit/asmjit/issues/533.
class A64Test_LoadLane : public A64TestCase {
public:
  uint32_t _reg_count {};
  uint32_t _element_size {};
  uint32_t _lane_index {};

  A64Test_LoadLane(uint32_t reg_count, uint32_t element_size, uint32_t lane_index)
    : _reg_count(reg_count),
      _element_size(element_size),
      _lane_index(lane_index) {
    _name.assign_format("LoadLane {Count=%u ElementSize=%u Index=%u}", reg_count, element_size, lane_index);
  }

  static void add(TestApp& app) {
    for (uint32_t reg_count = 1; reg_count <= 4; reg_count++) {
      for (uint32_t element_size = 1; element_size <= 8; element_size *= 2) {
        app.add(new A64Test_LoadLane(reg_count, element_size, 0));
        app.add(new A64Test_LoadLane(reg_count, element_size, 16 / element_size - 1));
      }
    }
  }

  void compile(a64::Compiler& cc) override {
    FuncSignature signature = FuncSignature::build<void, void*, const void*>();
    for (uint32_t i = 0; i <= _reg_count; i++) {
      signature.add_arg(TypeId::kUInt8x16);
    }

    FuncNode* called_fn = cc.new_func(signature);
    FuncNode* main_fn = cc.add_func(FuncSignature::build<void, void*, const void*, const void*>());

    {
      a64::Gp dst = cc.new_gp_ptr("dst");
      a64::Gp initial = cc.new_gp_ptr("initial");
      a64::Gp src = cc.new_gp_ptr("src");

      main_fn->set_arg(0, dst);
      main_fn->set_arg(1, initial);
      main_fn->set_arg(2, src);

      a64::Vec v[5];
      for (uint32_t i = 0; i <= _reg_count; i++) {
        v[i] = cc.new_vec_q("initial%u", i);
        cc.ldr(v[i], a64::ptr(initial, int32_t(i * 16)));
      }

      InvokeNode* invoke_node;
      cc.invoke(Out(invoke_node), called_fn->label(), signature);

      invoke_node->set_arg(0, dst);
      invoke_node->set_arg(1, src);

      for (uint32_t i = 0; i <= _reg_count; i++) {
        invoke_node->set_arg(i + 2, v[i]);
      }

      cc.end_func();
    }

    {
      cc.add_func(called_fn);

      a64::Gp dst = cc.new_gp_ptr("dst");
      a64::Gp src = cc.new_gp_ptr("src");

      called_fn->set_arg(0, dst);
      called_fn->set_arg(1, src);

      // The unused first SIMD argument occupies v0. If lane destinations are treated
      // as write-only, the allocator loses their incoming values in v1 and higher.
      a64::Vec v[4];
      a64::Vec lane[4];

      for (uint32_t i = 0; i < _reg_count; i++) {
        v[i] = cc.new_vec_q("v%u", i);
        called_fn->set_arg(i + 3, v[i]);

        switch (_element_size) {
          case 1: lane[i] = v[i].b(_lane_index); break;
          case 2: lane[i] = v[i].h(_lane_index); break;
          case 4: lane[i] = v[i].s(_lane_index); break;
          case 8: lane[i] = v[i].d(_lane_index); break;
        }
      }

      switch (_reg_count) {
        case 1: cc.ld1(lane[0], a64::ptr(src)); break;
        case 2: cc.ld2(lane[0], lane[1], a64::ptr(src)); break;
        case 3: cc.ld3(lane[0], lane[1], lane[2], a64::ptr(src)); break;
        case 4: cc.ld4(lane[0], lane[1], lane[2], lane[3], a64::ptr(src)); break;
      }

      for (uint32_t i = 0; i < _reg_count; i++) {
        cc.str(v[i], a64::ptr(dst, int32_t(i * 16)));
      }

      cc.end_func();
    }
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, const void*, const void*);

    uint8_t initial[80];
    uint8_t src[32];
    uint8_t dst[64] = {};
    uint8_t ref[64];

    for (uint32_t i = 0; i < 80; i++) {
      initial[i] = uint8_t(17 + i * 3);
    }

    for (uint32_t i = 0; i < 32; i++) {
      src[i] = uint8_t(255 - i * 5);
    }

    memcpy(ref, initial + 16, _reg_count * 16);
    for (uint32_t i = 0; i < _reg_count; i++) {
      memcpy(ref + i * 16 + _lane_index * _element_size, src + i * _element_size, _element_size);
    }

    ptr_as_func<Func>(_func)(dst, initial, src);

    result.assign_hex(dst, _reg_count * 16);
    expect.assign_hex(ref, _reg_count * 16);
    return result == expect;
  }
};

// a64::Compiler - Export
// ======================

void compiler_add_a64_tests(TestApp& app) {
  app.add_t<A64Test_GpArgs>();
  app.add_t<A64Test_ManyRegs>();
  app.add_t<A64Test_Simd1>();
  app.add_t<A64Test_SimdPreserveDst>();
  app.add_t<A64Test_Adr>();
  app.add_t<A64Test_Branch1>();
  app.add_t<A64Test_Invoke1>();
  app.add_t<A64Test_Invoke2>();
  app.add_t<A64Test_Invoke3>();
  app.add_t<A64Test_Invoke4>();
  app.add_t<A64Test_Invoke5>();
  app.add_t<A64Test_JumpTable>();
  app.add_t<A64Test_FuncTailCall>();
  app.add_t<A64Test_FuncTailCallPermute>();
  app.add_t<A64Test_ConsecutiveOutAllocation>();
  app.add_t<A64Test_LoadLane>();
}

#endif // !ASMJIT_NO_COMPILER && !ASMJIT_NO_AARCH64
