// AsmJit - Machine code generation for C++
//
//  * Official AsmJit Home Page: https://asmjit.com
//  * Official Github Repository: https://github.com/asmjit/asmjit
//
// Copyright (c) 2008-2020 The AsmJit Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <asmjit/core.h>
#if defined(ASMJIT_BUILD_X86)

#include <asmjit/x86.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asmjit_test_assembler.h"
#include "cmdline.h"

using namespace asmjit;

#define TEST_INSTRUCTION(OPCODE, ...)                                           \
  do {                                                                          \
    tester.beforeInstruction();                                                 \
    tester.testInstruction(OPCODE, #__VA_ARGS__, tester.assembler.__VA_ARGS__); \
    tester.afterInstruction();                                                  \
  } while (0)

bool testX86Assembler(const TestSettings& settings) noexcept {
  using namespace x86;

  AssemblerTester<Assembler> tester(Environment::kArchX86, settings);
  tester.printHeader("X86");

  // Base Instructions.
  TEST_INSTRUCTION("8AE0"                          , mov(ah, al));
  TEST_INSTRUCTION("8AF0"                          , mov(dh, al));
  TEST_INSTRUCTION("8BC3"                          , mov(eax, ebx));
  TEST_INSTRUCTION("89D8"                          , mod_mr().mov(eax, ebx));
  TEST_INSTRUCTION("B800000000"                    , mov(eax, 0));
  TEST_INSTRUCTION("BB00000000"                    , mov(ebx, 0));
  TEST_INSTRUCTION("B8FFFFFFFF"                    , mov(eax, 0xFFFFFFFF));
  TEST_INSTRUCTION("8CE0"                          , mov(eax, fs));
  TEST_INSTRUCTION("8EE0"                          , mov(fs, eax));
  TEST_INSTRUCTION("8B10"                          , mov(edx, ptr(eax)));
  TEST_INSTRUCTION("8B10"                          , mov(edx, ptr(eax, 0)));
  TEST_INSTRUCTION("8B9080000000"                  , mov(edx, ptr(eax, 128)));
  TEST_INSTRUCTION("8B1408"                        , mov(edx, ptr(eax, ecx)));
  TEST_INSTRUCTION("8B940880000000"                , mov(edx, ptr(eax, ecx, 0, 128)));
  TEST_INSTRUCTION("8B1408"                        , mov(edx, ptr(eax, ecx)));
  TEST_INSTRUCTION("8B544820"                      , mov(edx, ptr(eax, ecx, 1, 32)));
  TEST_INSTRUCTION("8B548840"                      , mov(edx, ptr(eax, ecx, 2, 64)));
  TEST_INSTRUCTION("8B94C800010000"                , mov(edx, ptr(eax, ecx, 3, 128 + 128)));
  TEST_INSTRUCTION("8B1408"                        , mov(edx, ptr(eax, ecx)));
  TEST_INSTRUCTION("8B940880000000"                , mov(edx, ptr(eax, ecx, 0, 128)));
  TEST_INSTRUCTION("8B1408"                        , mov(edx, ptr(eax, ecx)));
  TEST_INSTRUCTION("8B544820"                      , mov(edx, ptr(eax, ecx, 1, 32)));
  TEST_INSTRUCTION("8B54C802"                      , mov(edx, ptr(eax, ecx, 3, 2)));
  TEST_INSTRUCTION("0F20C0"                        , mov(eax, cr0));
  TEST_INSTRUCTION("F00F20C0"                      , mov(eax, cr8));
  TEST_INSTRUCTION("A344332211"                    , mov(ptr(0x11223344), eax));
  TEST_INSTRUCTION("890544332211"                  , mod_mr().mov(ptr(0x11223344), eax));
  TEST_INSTRUCTION("891D44332211"                  , mov(ptr(0x11223344), ebx));
  TEST_INSTRUCTION("0FBE07"                        , movsx(eax, byte_ptr(edi)));
  TEST_INSTRUCTION("0FBF07"                        , movsx(eax, word_ptr(edi)));
  TEST_INSTRUCTION("0FB607"                        , movzx(eax, byte_ptr(edi)));
  TEST_INSTRUCTION("0FB6C6"                        , movzx(eax, dh));
  TEST_INSTRUCTION("0FB707"                        , movzx(eax, word_ptr(edi)));
  TEST_INSTRUCTION("03D9"                          , add(ebx, ecx));
  TEST_INSTRUCTION("83C001"                        , add(eax, 1));
  TEST_INSTRUCTION("0504030201"                    , add(eax, 0x01020304));
  TEST_INSTRUCTION("66050201"                      , add(ax, 0x0102));
  TEST_INSTRUCTION("6603849004030201"              , add(ax, ptr(eax, edx, 2, 0x01020304)));
  TEST_INSTRUCTION("F00118"                        , lock().add(ptr(eax), ebx));
  TEST_INSTRUCTION("F00FC138"                      , lock().xadd(ptr(eax), edi));
  TEST_INSTRUCTION("660FBA2001"                    , bt(word_ptr(eax), 1));
  TEST_INSTRUCTION("0FBA2001"                      , bt(dword_ptr(eax), 1));
  TEST_INSTRUCTION("FE00"                          , inc(byte_ptr(eax)));
  TEST_INSTRUCTION("66FF00"                        , inc(word_ptr(eax)));
  TEST_INSTRUCTION("FF00"                          , inc(dword_ptr(eax)));
  TEST_INSTRUCTION("F6D8"                          , neg(al));
  TEST_INSTRUCTION("F6DC"                          , neg(ah));
  TEST_INSTRUCTION("F7D8"                          , neg(eax));
  TEST_INSTRUCTION("F7D0"                          , not_(eax));
  TEST_INSTRUCTION("0F95C3"                        , setnz(bl));
  TEST_INSTRUCTION("0F94C7"                        , setz(bh));
  TEST_INSTRUCTION("F600FF"                        , test(byte_ptr(eax), 0xFF));
  TEST_INSTRUCTION("66F700FF00"                    , test(word_ptr(eax), 0xFF));
  TEST_INSTRUCTION("F700FF000000"                  , test(dword_ptr(eax), 0xFF));
  TEST_INSTRUCTION("A836"                          , test(al, 0x36));
  TEST_INSTRUCTION("F6C436"                        , test(ah, 0x36));
  TEST_INSTRUCTION("50"                            , push(eax));
  TEST_INSTRUCTION("51"                            , push(ecx));
  TEST_INSTRUCTION("52"                            , push(edx));
  TEST_INSTRUCTION("53"                            , push(ebx));
  TEST_INSTRUCTION("54"                            , push(esp));
  TEST_INSTRUCTION("55"                            , push(ebp));
  TEST_INSTRUCTION("56"                            , push(esi));
  TEST_INSTRUCTION("57"                            , push(edi));
  TEST_INSTRUCTION("0E"                            , push(cs));
  TEST_INSTRUCTION("16"                            , push(ss));
  TEST_INSTRUCTION("1E"                            , push(ds));
  TEST_INSTRUCTION("06"                            , push(es));
  TEST_INSTRUCTION("0FA0"                          , push(fs));
  TEST_INSTRUCTION("0FA8"                          , push(gs));
  TEST_INSTRUCTION("C8010002"                      , enter(1, 2));
  TEST_INSTRUCTION("C9"                            , leave());
  TEST_INSTRUCTION("FF10"                          , call(ptr(eax)));
  TEST_INSTRUCTION("FF10"                          , call(dword_ptr(eax)));
  TEST_INSTRUCTION("66C501"                        , lds(ax, ptr(ecx)));
  TEST_INSTRUCTION("C501"                          , lds(eax, ptr(ecx)));
  TEST_INSTRUCTION("66C401"                        , les(ax, ptr(ecx)));
  TEST_INSTRUCTION("C401"                          , les(eax, ptr(ecx)));
  TEST_INSTRUCTION("660FB401"                      , lfs(ax, ptr(ecx)));
  TEST_INSTRUCTION("0FB401"                        , lfs(eax, ptr(ecx)));
  TEST_INSTRUCTION("660FB501"                      , lgs(ax, ptr(ecx)));
  TEST_INSTRUCTION("0FB501"                        , lgs(eax, ptr(ecx)));
  TEST_INSTRUCTION("660FB201"                      , lss(ax, ptr(ecx)));
  TEST_INSTRUCTION("0FB201"                        , lss(eax, ptr(ecx)));

  // NOP.
  TEST_INSTRUCTION("90"                            , nop());
  TEST_INSTRUCTION("660F1F0400"                    , nop(word_ptr(eax, eax)));
  TEST_INSTRUCTION("660F1F0400"                    , nop(word_ptr(eax, eax), ax));
  TEST_INSTRUCTION("660F1F1C00"                    , nop(word_ptr(eax, eax), bx));
  TEST_INSTRUCTION("0F1F0400"                      , nop(dword_ptr(eax, eax)));
  TEST_INSTRUCTION("0F1F0400"                      , nop(dword_ptr(eax, eax), eax));
  TEST_INSTRUCTION("0F1F1C00"                      , nop(dword_ptr(eax, eax), ebx));

  // LEA.
  TEST_INSTRUCTION("67668D00"                      , lea(ax, ptr(bx, si)));
  TEST_INSTRUCTION("67668D01"                      , lea(ax, ptr(bx, di)));
  TEST_INSTRUCTION("67668D02"                      , lea(ax, ptr(bp, si)));
  TEST_INSTRUCTION("67668D03"                      , lea(ax, ptr(bp, di)));
  TEST_INSTRUCTION("67668D04"                      , lea(ax, ptr(si)));
  TEST_INSTRUCTION("67668D05"                      , lea(ax, ptr(di)));
  TEST_INSTRUCTION("67668D4600"                    , lea(ax, ptr(bp)));
  TEST_INSTRUCTION("67668D07"                      , lea(ax, ptr(bx)));
  TEST_INSTRUCTION("67668D4010"                    , lea(ax, ptr(bx, si, 0, 0x10)));
  TEST_INSTRUCTION("67668D4120"                    , lea(ax, ptr(bx, di, 0, 0x20)));
  TEST_INSTRUCTION("67668D4240"                    , lea(ax, ptr(bp, si, 0, 0x40)));
  TEST_INSTRUCTION("67668D4360"                    , lea(ax, ptr(bp, di, 0, 0x60)));
  TEST_INSTRUCTION("67668D848000"                  , lea(ax, ptr(si, 0x80)));
  TEST_INSTRUCTION("67668D85A000"                  , lea(ax, ptr(di, 0xA0)));
  TEST_INSTRUCTION("67668D86C000"                  , lea(ax, ptr(bp, 0xC0)));
  TEST_INSTRUCTION("67668D87FF01"                  , lea(ax, ptr(bx, 0x01FF)));
  TEST_INSTRUCTION("678D00"                        , lea(eax, ptr(bx, si)));
  TEST_INSTRUCTION("678D01"                        , lea(eax, ptr(bx, di)));
  TEST_INSTRUCTION("8D0433"                        , lea(eax, ptr(ebx, esi)));
  TEST_INSTRUCTION("8D043B"                        , lea(eax, ptr(ebx, edi)));
  TEST_INSTRUCTION("8D0500000000"                  , lea(eax, ptr(0)));

  // XACQUIRE|XRELEASE|RTM.
  TEST_INSTRUCTION("C6F811"                        , xabort(0x11));
  TEST_INSTRUCTION("F2F00108"                      , xacquire().lock().add(dword_ptr(eax), ecx));
  TEST_INSTRUCTION("F3F00108"                      , xrelease().lock().add(dword_ptr(eax), ecx));

  // BND.
  TEST_INSTRUCTION("660F1ACA"                      , bndmov(bnd1, bnd2));
  TEST_INSTRUCTION("F20F1ACF"                      , bndcu(bnd1, edi));
  TEST_INSTRUCTION("0F1A0408"                      , bndldx(bnd0, ptr(eax, ecx)));
  TEST_INSTRUCTION("0F1B0C08"                      , bndstx(ptr(eax, ecx), bnd1));

  // BMI+.
  TEST_INSTRUCTION("66F30FB8C2"                    , popcnt(ax, dx));
  TEST_INSTRUCTION("F30FB8C2"                      , popcnt(eax, edx));
  TEST_INSTRUCTION("66F30FBDC2"                    , lzcnt(ax, dx));
  TEST_INSTRUCTION("F30FBDC2"                      , lzcnt(eax, edx));
  TEST_INSTRUCTION("66F30FBCC2"                    , tzcnt(ax, dx));
  TEST_INSTRUCTION("F30FBCC2"                      , tzcnt(eax, edx));

  // CRC32.
  TEST_INSTRUCTION("F20F38F0C7"                    , crc32(eax, bh));
  TEST_INSTRUCTION("66F20F38F1C3"                  , crc32(eax, bx));
  TEST_INSTRUCTION("F20F38F1C1"                    , crc32(eax, ecx));
  TEST_INSTRUCTION("F20F38F006"                    , crc32(eax, byte_ptr(esi)));
  TEST_INSTRUCTION("66F20F38F106"                  , crc32(eax, word_ptr(esi)));
  TEST_INSTRUCTION("F20F38F106"                    , crc32(eax, dword_ptr(esi)));

  // FPU.
  TEST_INSTRUCTION("9B"                            , fwait());
  TEST_INSTRUCTION("D800"                          , fadd(dword_ptr(eax)));
  TEST_INSTRUCTION("DC00"                          , fadd(qword_ptr(eax)));

  // MMX & SSE.
  TEST_INSTRUCTION("0F6FC1"                        , movq(mm0, mm1));
  TEST_INSTRUCTION("0F6E00"                        , movd(mm0, ptr(eax)));
  TEST_INSTRUCTION("0F6F0418"                      , movq(mm0, ptr(eax, ebx)));
  TEST_INSTRUCTION("0F7E38"                        , movd(ptr(eax), mm7));
  TEST_INSTRUCTION("0F7F0418"                      , movq(ptr(eax, ebx), mm0));
  TEST_INSTRUCTION("F30F7EC1"                      , movq(xmm0, xmm1));
  TEST_INSTRUCTION("660F6E0418"                    , movd(xmm0, ptr(eax, ebx)));
  TEST_INSTRUCTION("F30F7E0418"                    , movq(xmm0, ptr(eax, ebx)));
  TEST_INSTRUCTION("660F7E0C18"                    , movd(ptr(eax, ebx), xmm1));
  TEST_INSTRUCTION("660FD60C18"                    , movq(ptr(eax, ebx), xmm1));
  TEST_INSTRUCTION("0F280498"                      , movaps(xmm0, ptr(eax, ebx, 2)));
  TEST_INSTRUCTION("660F280498"                    , movapd(xmm0, ptr(eax, ebx, 2)));
  TEST_INSTRUCTION("660F6F0498"                    , movdqa(xmm0, ptr(eax, ebx, 2)));
  TEST_INSTRUCTION("0F290C98"                      , movaps(ptr(eax, ebx, 2), xmm1));
  TEST_INSTRUCTION("660F290C98"                    , movapd(ptr(eax, ebx, 2), xmm1));
  TEST_INSTRUCTION("660F7F0C98"                    , movdqa(ptr(eax, ebx, 2), xmm1));
  TEST_INSTRUCTION("F30F2DC1"                      , cvtss2si(eax, xmm1));
  TEST_INSTRUCTION("F20F2DC1"                      , cvtsd2si(eax, xmm1));
  TEST_INSTRUCTION("F30F2AC2"                      , cvtsi2ss(xmm0, edx));
  TEST_INSTRUCTION("F20F2AC2"                      , cvtsi2sd(xmm0, edx));
  TEST_INSTRUCTION("660F3A41C100"                  , dppd(xmm0, xmm1, 0));
  TEST_INSTRUCTION("0FDBC1"                        , pand(mm0, mm1));
  TEST_INSTRUCTION("660FDBC1"                      , pand(xmm0, xmm1));
  TEST_INSTRUCTION("660FFDC1"                      , paddw(xmm0, xmm1));

  // AVX & AVX512.
  TEST_INSTRUCTION("C5F96E5A10"                    , vmovd(xmm3, dword_ptr(edx, 0x10)));
  TEST_INSTRUCTION("C5FA7E5A10"                    , vmovq(xmm3, qword_ptr(edx, 0x10)));
  TEST_INSTRUCTION("C5F97E5A10"                    , vmovd(dword_ptr(edx, 0x10), xmm3));
  TEST_INSTRUCTION("C5F9D65A10"                    , vmovq(qword_ptr(edx, 0x10), xmm3));
  TEST_INSTRUCTION("C5F96EEB"                      , vmovd(xmm5, ebx));
  TEST_INSTRUCTION("C5F97EEB"                      , vmovd(ebx, xmm5));
  TEST_INSTRUCTION("C5FA7EC1"                      , vmovq(xmm0, xmm1));
  TEST_INSTRUCTION("62F17D086EC0"                  , evex().vmovd(xmm0, eax));
  TEST_INSTRUCTION("62F17D087EC0"                  , evex().vmovd(eax, xmm0));
  TEST_INSTRUCTION("C5F5FDC7"                      , vpaddw(ymm0, ymm1, ymm7));
  TEST_INSTRUCTION("C4E37141C200"                  , vdppd(xmm0, xmm1, xmm2, 0));
  TEST_INSTRUCTION("62F1F5D95800"                  , k(k1).z().vaddpd(zmm0, zmm1, ptr(eax)._1to8()));
  TEST_INSTRUCTION("C5F058C2"                      , vaddps(xmm0, xmm1, xmm2));
  TEST_INSTRUCTION("62F1748858C2"                  , z().vaddps(xmm0, xmm1, xmm2));
  TEST_INSTRUCTION("62F1748958C2"                  , k(k1).z().vaddps(xmm0, xmm1, xmm2));
  TEST_INSTRUCTION("62F16C4FC25498040F"            , k(k7).vcmpps(k2, zmm2, zmmword_ptr(eax, ebx, 2, 256), 15));
  TEST_INSTRUCTION("62F16C5FC25498400F"            , k(k7).vcmpps(k2, zmm2, dword_ptr(eax, ebx, 2, 256)._1to16(), 15));
  TEST_INSTRUCTION("C5FA2DC1"                      , vcvtss2si(eax, xmm1));
  TEST_INSTRUCTION("C5FB2DC1"                      , vcvtsd2si(eax, xmm1));
  TEST_INSTRUCTION("C5F22AC2"                      , vcvtsi2ss(xmm0, xmm1, edx));
  TEST_INSTRUCTION("C5F32AC2"                      , vcvtsi2sd(xmm0, xmm1, edx));
  TEST_INSTRUCTION("C5FBE63B"                      , vcvtpd2dq(xmm7, xmmword_ptr(ebx)));
  TEST_INSTRUCTION("C5FFE63B"                      , vcvtpd2dq(xmm7, ymmword_ptr(ebx)));
  TEST_INSTRUCTION("C5F95A3B"                      , vcvtpd2ps(xmm7, xmmword_ptr(ebx)));
  TEST_INSTRUCTION("C5FD5A3B"                      , vcvtpd2ps(xmm7, ymmword_ptr(ebx)));
  TEST_INSTRUCTION("C5F95AC1"                      , vcvtpd2ps(xmm0, xmm1));
  TEST_INSTRUCTION("C5F95A03"                      , vcvtpd2ps(xmm0, xmmword_ptr(ebx)));
  TEST_INSTRUCTION("C5FD5AC1"                      , vcvtpd2ps(xmm0, ymm1));
  TEST_INSTRUCTION("C5FD5A03"                      , vcvtpd2ps(xmm0, ymmword_ptr(ebx)));
  TEST_INSTRUCTION("62F1FD485AC1"                  , vcvtpd2ps(ymm0, zmm1));
  TEST_INSTRUCTION("62F1FD485A03"                  , vcvtpd2ps(ymm0, zmmword_ptr(ebx)));
  TEST_INSTRUCTION("62F1FC08793B"                  , vcvtpd2udq(xmm7, xmmword_ptr(ebx)));
  TEST_INSTRUCTION("62F1FC28793B"                  , vcvtpd2udq(xmm7, ymmword_ptr(ebx)));
  TEST_INSTRUCTION("62F1FC085B3B"                  , vcvtqq2ps(xmm7, xmmword_ptr(ebx)));
  TEST_INSTRUCTION("62F1FC285B3B"                  , vcvtqq2ps(xmm7, ymmword_ptr(ebx)));
  TEST_INSTRUCTION("C5F9E63B"                      , vcvttpd2dq(xmm7, xmmword_ptr(ebx)));
  TEST_INSTRUCTION("C5FDE63B"                      , vcvttpd2dq(xmm7, ymmword_ptr(ebx)));
  TEST_INSTRUCTION("62F1FC08783B"                  , vcvttpd2udq(xmm7, xmmword_ptr(ebx)));
  TEST_INSTRUCTION("62F1FC28783B"                  , vcvttpd2udq(xmm7, ymmword_ptr(ebx)));
  TEST_INSTRUCTION("62F1FF087A3B"                  , vcvtuqq2ps(xmm7, xmmword_ptr(ebx)));
  TEST_INSTRUCTION("62F1FF287A3B"                  , vcvtuqq2ps(xmm7, ymmword_ptr(ebx)));
  TEST_INSTRUCTION("62F3FD08663F01"                , vfpclasspd(k7, xmmword_ptr(edi), 0x01));
  TEST_INSTRUCTION("62F3FD28663F01"                , vfpclasspd(k7, ymmword_ptr(edi), 0x01));
  TEST_INSTRUCTION("62F3FD48663F01"                , vfpclasspd(k7, zmmword_ptr(edi), 0x01));
  TEST_INSTRUCTION("62F37D08663F01"                , vfpclassps(k7, xmmword_ptr(edi), 0x01));
  TEST_INSTRUCTION("62F37D28663F01"                , vfpclassps(k7, ymmword_ptr(edi), 0x01));
  TEST_INSTRUCTION("62F37D48663F01"                , vfpclassps(k7, zmmword_ptr(edi), 0x01));
  TEST_INSTRUCTION("C4E2F990040500000000"          , vpgatherdq(xmm0, ptr(0, xmm0), xmm0));
  TEST_INSTRUCTION("C4E2FD91040500000000"          , vpgatherqq(ymm0, ptr(0, ymm0), ymm0));
  TEST_INSTRUCTION("C4E2E9920C00"                  , vgatherdpd(xmm1, ptr(eax, xmm0), xmm2));
  TEST_INSTRUCTION("62F36D083ECB00"                , vpcmpub(k1, xmm2, xmm3, 0x0));
  TEST_INSTRUCTION("62F26D48CF4C1101"              , vgf2p8mulb(zmm1, zmm2, zmmword_ptr(ecx, edx, 0, 64)));
  TEST_INSTRUCTION("62F3ED48CE4C11010F"            , vgf2p8affineqb(zmm1, zmm2, zmmword_ptr(ecx, edx, 0, 64), 15));
  TEST_INSTRUCTION("62F3ED48CF4C11010F"            , vgf2p8affineinvqb(zmm1, zmm2, zmmword_ptr(ecx, edx, 0, 64), 15));
  TEST_INSTRUCTION("62F2674868246D00F8FFFF"        , vp2intersectd(k4, k5, zmm3, zmmword_ptr(0xFFFFF800, ebp, 1)));

  // AVX512_VNNI vs AVX_VNNI.
  TEST_INSTRUCTION("62F2552850F4"                  , vpdpbusd(ymm6, ymm5, ymm4));
  TEST_INSTRUCTION("C4E25550F4"                    , vex().vpdpbusd(ymm6, ymm5, ymm4));

  tester.printSummary();
  return tester.didPass();
}

bool testX64Assembler(const TestSettings& settings) noexcept {
  using namespace x86;

  AssemblerTester<Assembler> tester(Environment::kArchX64, settings);
  tester.printHeader("X64");

  // Base Instructions.
  TEST_INSTRUCTION("B800000000"                    , mov(eax, 0));
  TEST_INSTRUCTION("BB00000000"                    , mov(ebx, 0));
  TEST_INSTRUCTION("48C7C300000000"                , mov(rbx, 0));
  TEST_INSTRUCTION("48BB8877665544332211"          , mov(rbx, 0x001122334455667788));
  TEST_INSTRUCTION("48BB0000000000000000"          , long_().mov(rbx, 0));
  TEST_INSTRUCTION("8AE0"                          , mov(ah, al));
  TEST_INSTRUCTION("8AF0"                          , mov(dh, al));
  TEST_INSTRUCTION("B8E8030000"                    , mov(eax, 1000));
  TEST_INSTRUCTION("0F20C0"                        , mov(rax, cr0));
  TEST_INSTRUCTION("440F20C0"                      , mov(rax, cr8));
  TEST_INSTRUCTION("488B0500000000"                , mov(rax, ptr(rip)));
  TEST_INSTRUCTION("4A8B0460"                      , mov(rax, ptr(rax, r12, 1)));
  TEST_INSTRUCTION("4A8B0468"                      , mov(rax, ptr(rax, r13, 1)));
  TEST_INSTRUCTION("4A8B846000010000"              , mov(rax, ptr(rax, r12, 1, 256)));
  TEST_INSTRUCTION("89042544332211"                , mov(ptr_abs(0x11223344), eax));
  TEST_INSTRUCTION("891C2544332211"                , mov(ptr_abs(0x11223344), ebx));
  TEST_INSTRUCTION("A38877665544332211"            , mov(ptr_abs(0x1122334455667788), eax));
  TEST_INSTRUCTION("A34433221100000000"            , movabs(ptr(0x0000000011223344), eax));
  TEST_INSTRUCTION("A38877665544332211"            , movabs(ptr(0x1122334455667788), eax));
  TEST_INSTRUCTION("48A1EFCDAB8967452301"          , movabs(rax, ptr(0x123456789ABCDEF)));
  TEST_INSTRUCTION("0FBE07"                        , movsx(eax, byte_ptr(rdi)));
  TEST_INSTRUCTION("480FBE07"                      , movsx(rax, byte_ptr(rdi)));
  TEST_INSTRUCTION("0FBF07"                        , movsx(eax, word_ptr(rdi)));
  TEST_INSTRUCTION("480FBF07"                      , movsx(rax, word_ptr(rdi)));
  TEST_INSTRUCTION("486307"                        , movsxd(rax, ptr(rdi)));
  TEST_INSTRUCTION("486307"                        , movsxd(rax, dword_ptr(rdi)));
  TEST_INSTRUCTION("6663C3"                        , movsxd(ax, bx));
  TEST_INSTRUCTION("63C3"                          , movsxd(eax, ebx));
  TEST_INSTRUCTION("4863C3"                        , movsxd(rax, ebx));
  TEST_INSTRUCTION("0FB6C6"                        , movzx(eax, dh));
  TEST_INSTRUCTION("0FB607"                        , movzx(eax, byte_ptr(rdi)));
  TEST_INSTRUCTION("480FB607"                      , movzx(rax, byte_ptr(rdi)));
  TEST_INSTRUCTION("440FB6FA"                      , movzx(r15d, dl));
  TEST_INSTRUCTION("440FB6FD"                      , movzx(r15d, bpl));
  TEST_INSTRUCTION("0FB707"                        , movzx(eax, word_ptr(rdi)));
  TEST_INSTRUCTION("480FB707"                      , movzx(rax, word_ptr(rdi)));
  TEST_INSTRUCTION("03D9"                          , add(ebx, ecx));
  TEST_INSTRUCTION("83C001"                        , add(eax, 1));
  TEST_INSTRUCTION("0504030201"                    , add(eax, 0x01020304));
  TEST_INSTRUCTION("66050201"                      , add(ax, 0x0102));
  TEST_INSTRUCTION("6603849004030201"              , add(ax, ptr(rax, rdx, 2, 0x01020304)));
  TEST_INSTRUCTION("F00118"                        , lock().add(ptr(rax), ebx));
  TEST_INSTRUCTION("F0480FC138"                    , lock().xadd(ptr(rax), rdi));
  TEST_INSTRUCTION("660FC8"                        , bswap(ax));
  TEST_INSTRUCTION("0FC8"                          , bswap(eax));
  TEST_INSTRUCTION("480FC8"                        , bswap(rax));
  TEST_INSTRUCTION("660FBA2001"                    , bt(word_ptr(rax), 1));
  TEST_INSTRUCTION("0FBA2001"                      , bt(dword_ptr(rax), 1));
  TEST_INSTRUCTION("480FBA2001"                    , bt(qword_ptr(rax), 1));
  TEST_INSTRUCTION("FE00"                          , inc(byte_ptr(rax)));
  TEST_INSTRUCTION("66FF00"                        , inc(word_ptr(rax)));
  TEST_INSTRUCTION("FF00"                          , inc(dword_ptr(rax)));
  TEST_INSTRUCTION("48FF00"                        , inc(qword_ptr(rax)));
  TEST_INSTRUCTION("411351FD"                      , adc(edx, dword_ptr(r9, -3)));
  TEST_INSTRUCTION("F6D8"                          , neg(al));
  TEST_INSTRUCTION("F6DC"                          , neg(ah));
  TEST_INSTRUCTION("40F6DE"                        , neg(sil));
  TEST_INSTRUCTION("F7D8"                          , neg(eax));
  TEST_INSTRUCTION("F7D0"                          , not_(eax));
  TEST_INSTRUCTION("0F95C3"                        , setnz(bl));
  TEST_INSTRUCTION("0F94C7"                        , setz(bh));
  TEST_INSTRUCTION("400F94C0"                      , rex().setz(al));
  TEST_INSTRUCTION("410F94C7"                      , setz(r15b));
  TEST_INSTRUCTION("F600FF"                        , test(byte_ptr(rax), 0xFF));
  TEST_INSTRUCTION("66F700FF00"                    , test(word_ptr(rax), 0xFF));
  TEST_INSTRUCTION("F700FF000000"                  , test(dword_ptr(rax), 0xFF));
  TEST_INSTRUCTION("48F700FF000000"                , test(qword_ptr(rax), 0xFF));
  TEST_INSTRUCTION("A836"                          , test(al, 0x36));
  TEST_INSTRUCTION("F6C436"                        , test(ah, 0x36));
  TEST_INSTRUCTION("50"                            , push(rax));
  TEST_INSTRUCTION("51"                            , push(rcx));
  TEST_INSTRUCTION("52"                            , push(rdx));
  TEST_INSTRUCTION("53"                            , push(rbx));
  TEST_INSTRUCTION("54"                            , push(rsp));
  TEST_INSTRUCTION("55"                            , push(rbp));
  TEST_INSTRUCTION("56"                            , push(rsi));
  TEST_INSTRUCTION("57"                            , push(rdi));
  TEST_INSTRUCTION("4150"                          , push(r8));
  TEST_INSTRUCTION("4151"                          , push(r9));
  TEST_INSTRUCTION("4152"                          , push(r10));
  TEST_INSTRUCTION("4153"                          , push(r11));
  TEST_INSTRUCTION("4154"                          , push(r12));
  TEST_INSTRUCTION("4155"                          , push(r13));
  TEST_INSTRUCTION("4156"                          , push(r14));
  TEST_INSTRUCTION("4157"                          , push(r15));
  TEST_INSTRUCTION("0FA0"                          , push(fs));
  TEST_INSTRUCTION("0FA8"                          , push(gs));
  TEST_INSTRUCTION("400FA0"                        , rex().push(fs));
  TEST_INSTRUCTION("400FA8"                        , rex().push(gs));
  TEST_INSTRUCTION("C8010002"                      , enter(1, 2));
  TEST_INSTRUCTION("40C8010002"                    , rex().enter(1, 2));
  TEST_INSTRUCTION("C9"                            , leave());
  TEST_INSTRUCTION("FF10"                          , call(ptr(rax)));
  TEST_INSTRUCTION("FF10"                          , call(qword_ptr(rax)));
  TEST_INSTRUCTION("660FB401"                      , lfs(ax, ptr(rcx)));
  TEST_INSTRUCTION("0FB401"                        , lfs(eax, ptr(rcx)));
  TEST_INSTRUCTION("480FB401"                      , lfs(rax, ptr(rcx)));
  TEST_INSTRUCTION("660FB501"                      , lgs(ax, ptr(rcx)));
  TEST_INSTRUCTION("0FB501"                        , lgs(eax, ptr(rcx)));
  TEST_INSTRUCTION("480FB501"                      , lgs(rax, ptr(rcx)));
  TEST_INSTRUCTION("660FB201"                      , lss(ax, ptr(rcx)));
  TEST_INSTRUCTION("0FB201"                        , lss(eax, ptr(rcx)));
  TEST_INSTRUCTION("480FB201"                      , lss(rax, ptr(rcx)));
  TEST_INSTRUCTION("40863424"                      , xchg(ptr(rsp), sil));
  TEST_INSTRUCTION("40863C24"                      , xchg(ptr(rsp), dil));

  // NOP.
  TEST_INSTRUCTION("90"                            , nop());
  TEST_INSTRUCTION("660F1F0400"                    , nop(word_ptr(rax, rax)));
  TEST_INSTRUCTION("660F1F0400"                    , nop(word_ptr(rax, rax), ax));
  TEST_INSTRUCTION("660F1F1C00"                    , nop(word_ptr(rax, rax), bx));
  TEST_INSTRUCTION("0F1F0400"                      , nop(dword_ptr(rax, rax)));
  TEST_INSTRUCTION("0F1F0400"                      , nop(dword_ptr(rax, rax), eax));
  TEST_INSTRUCTION("0F1F1C00"                      , nop(dword_ptr(rax, rax), ebx));
  TEST_INSTRUCTION("480F1F0400"                    , nop(qword_ptr(rax, rax)));
  TEST_INSTRUCTION("480F1F0400"                    , nop(qword_ptr(rax, rax), rax));
  TEST_INSTRUCTION("480F1F1C00"                    , nop(qword_ptr(rax, rax), rbx));

  // LEA.
  TEST_INSTRUCTION("8D042500000000"                , lea(eax, ptr(0)));
  TEST_INSTRUCTION("488D042500000000"              , lea(rax, ptr(0)));
  TEST_INSTRUCTION("488D0433"                      , lea(rax, ptr(rbx, rsi)));
  TEST_INSTRUCTION("488D043B"                      , lea(rax, ptr(rbx, rdi)));
  TEST_INSTRUCTION("488D840000400000"              , lea(rax, ptr(rax, rax, 0, 0x4000)));

  // CRC32.
  TEST_INSTRUCTION("F20F38F0C7"                    , crc32(eax, bh));
  TEST_INSTRUCTION("66F20F38F1C3"                  , crc32(eax, bx));
  TEST_INSTRUCTION("F20F38F1C1"                    , crc32(eax, ecx));
  TEST_INSTRUCTION("F20F38F006"                    , crc32(eax, byte_ptr(rsi)));
  TEST_INSTRUCTION("66F20F38F106"                  , crc32(eax, word_ptr(rsi)));
  TEST_INSTRUCTION("F20F38F106"                    , crc32(eax, dword_ptr(rsi)));
  TEST_INSTRUCTION("F2480F38F0C3"                  , crc32(rax, bl));
  TEST_INSTRUCTION("F2480F38F1C1"                  , crc32(rax, rcx));
  TEST_INSTRUCTION("F2480F38F006"                  , crc32(rax, byte_ptr(rsi)));
  TEST_INSTRUCTION("F2480F38F106"                  , crc32(rax, qword_ptr(rsi)));

  // XACQUIRE|XRELEASE|RTM.
  TEST_INSTRUCTION("C6F811"                        , xabort(0x11));
  TEST_INSTRUCTION("F2F0480108"                    , xacquire().lock().add(qword_ptr(rax), rcx));
  TEST_INSTRUCTION("F3F0480108"                    , xrelease().lock().add(qword_ptr(rax), rcx));

  // BND.
  TEST_INSTRUCTION("660F1ACA"                      , bndmov(bnd1, bnd2));
  TEST_INSTRUCTION("F20F1ACF"                      , bndcu(bnd1, rdi));
  TEST_INSTRUCTION("0F1A0408"                      , bndldx(bnd0, ptr(rax, rcx)));
  TEST_INSTRUCTION("0F1B0C08"                      , bndstx(ptr(rax, rcx), bnd1));

  // BMI+.
  TEST_INSTRUCTION("66F30FB8C2"                    , popcnt(ax, dx));
  TEST_INSTRUCTION("66F3450FB8C1"                  , popcnt(r8w, r9w));
  TEST_INSTRUCTION("F30FB8C2"                      , popcnt(eax, edx));
  TEST_INSTRUCTION("F3480FB8C2"                    , popcnt(rax, rdx));
  TEST_INSTRUCTION("66F30FBDC2"                    , lzcnt(ax, dx));
  TEST_INSTRUCTION("66F3450FBDC7"                  , lzcnt(r8w, r15w));
  TEST_INSTRUCTION("F30FBDC2"                      , lzcnt(eax, edx));
  TEST_INSTRUCTION("F3490FBDC2"                    , lzcnt(rax, r10));
  TEST_INSTRUCTION("66F30FBCC2"                    , tzcnt(ax, dx));
  TEST_INSTRUCTION("66F3450FBCC7"                  , tzcnt(r8w, r15w));
  TEST_INSTRUCTION("F30FBCC2"                      , tzcnt(eax, edx));
  TEST_INSTRUCTION("F34D0FBCFA"                    , tzcnt(r15, r10));

  // FPU.
  TEST_INSTRUCTION("9B"                            , fwait());
  TEST_INSTRUCTION("D800"                          , fadd(dword_ptr(rax)));
  TEST_INSTRUCTION("DC00"                          , fadd(qword_ptr(rax)));

  // MMX & SSE.
  TEST_INSTRUCTION("0F6FC1"                        , movq(mm0, mm1));
  TEST_INSTRUCTION("0F6E00"                        , movd(mm0, ptr(rax)));
  TEST_INSTRUCTION("0F6F0418"                      , movq(mm0, ptr(rax, rbx)));
  TEST_INSTRUCTION("0F7E38"                        , movd(ptr(rax), mm7));
  TEST_INSTRUCTION("0F7F0418"                      , movq(ptr(rax, rbx), mm0));
  TEST_INSTRUCTION("F30F7EC1"                      , movq(xmm0, xmm1));
  TEST_INSTRUCTION("660F6E0418"                    , movd(xmm0, ptr(rax, rbx)));
  TEST_INSTRUCTION("F30F7E0418"                    , movq(xmm0, ptr(rax, rbx)));
  TEST_INSTRUCTION("660F7E0C18"                    , movd(ptr(rax, rbx), xmm1));
  TEST_INSTRUCTION("660FD60C18"                    , movq(ptr(rax, rbx), xmm1));
  TEST_INSTRUCTION("0F280498"                      , movaps(xmm0, ptr(rax, rbx, 2)));
  TEST_INSTRUCTION("660F280498"                    , movapd(xmm0, ptr(rax, rbx, 2)));
  TEST_INSTRUCTION("660F6F0498"                    , movdqa(xmm0, ptr(rax, rbx, 2)));
  TEST_INSTRUCTION("0F290C98"                      , movaps(ptr(rax, rbx, 2), xmm1));
  TEST_INSTRUCTION("660F290C98"                    , movapd(ptr(rax, rbx, 2), xmm1));
  TEST_INSTRUCTION("660F7F0C98"                    , movdqa(ptr(rax, rbx, 2), xmm1));
  TEST_INSTRUCTION("F30F2DC1"                      , cvtss2si(eax, xmm1));
  TEST_INSTRUCTION("F3480F2DC1"                    , cvtss2si(rax, xmm1));
  TEST_INSTRUCTION("F20F2DC1"                      , cvtsd2si(eax, xmm1));
  TEST_INSTRUCTION("F2480F2DC1"                    , cvtsd2si(rax, xmm1));
  TEST_INSTRUCTION("F30F2AC2"                      , cvtsi2ss(xmm0, edx));
  TEST_INSTRUCTION("F3480F2AC2"                    , cvtsi2ss(xmm0, rdx));
  TEST_INSTRUCTION("F20F2AC2"                      , cvtsi2sd(xmm0, edx));
  TEST_INSTRUCTION("F2480F2AC2"                    , cvtsi2sd(xmm0, rdx));
  TEST_INSTRUCTION("66450F3A41D300"                , dppd(xmm10, xmm11, 0));
  TEST_INSTRUCTION("0FDBC1"                        , pand(mm0, mm1));
  TEST_INSTRUCTION("660FDBC1"                      , pand(xmm0, xmm1));
  TEST_INSTRUCTION("660FFDC1"                      , paddw(xmm0, xmm1));

  // AVX & AVX512.
  TEST_INSTRUCTION("C5F96E5A10"                    , vmovd(xmm3, dword_ptr(rdx, 0x10)));
  TEST_INSTRUCTION("C5FA7E5A10"                    , vmovq(xmm3, qword_ptr(rdx, 0x10)));
  TEST_INSTRUCTION("C5F97E5A10"                    , vmovd(dword_ptr(rdx, 0x10), xmm3));
  TEST_INSTRUCTION("C5F9D65A10"                    , vmovq(qword_ptr(rdx, 0x10), xmm3));
  TEST_INSTRUCTION("C5F96EEB"                      , vmovd(xmm5, ebx));
  TEST_INSTRUCTION("C4E1F96EEB"                    , vmovq(xmm5, rbx));
  TEST_INSTRUCTION("62617D086EFB"                  , vmovd(xmm31, ebx));
  TEST_INSTRUCTION("6261FD086EFB"                  , vmovq(xmm31, rbx));
  TEST_INSTRUCTION("C5F97EEB"                      , vmovd(ebx, xmm5));
  TEST_INSTRUCTION("C4E1F97EEB"                    , vmovq(rbx, xmm5));
  TEST_INSTRUCTION("62617D087EFB"                  , vmovd(ebx, xmm31));
  TEST_INSTRUCTION("6261FD087EFB"                  , vmovq(rbx, xmm31));
  TEST_INSTRUCTION("C5FA7EC1"                      , vmovq(xmm0, xmm1));
  TEST_INSTRUCTION("62F17D086EC0"                  , evex().vmovd(xmm0, eax));
  TEST_INSTRUCTION("62F1FD086EC0"                  , evex().vmovq(xmm0, rax));
  TEST_INSTRUCTION("62F17D087EC0"                  , evex().vmovd(eax, xmm0));
  TEST_INSTRUCTION("62F1FD087EC0"                  , evex().vmovq(rax, xmm0));
  TEST_INSTRUCTION("C44135FDC7"                    , vpaddw(ymm8, ymm9, ymm15));
  TEST_INSTRUCTION("C4432141D400"                  , vdppd(xmm10, xmm11, xmm12, 0));
  TEST_INSTRUCTION("6271B5D95808"                  , k(k1).z().vaddpd(zmm9, zmm9, ptr(rax)._1to8()));
  TEST_INSTRUCTION("C5F058C2"                      , vaddps(xmm0, xmm1, xmm2));
  TEST_INSTRUCTION("62F1748858C2"                  , z().vaddps(xmm0, xmm1, xmm2));
  TEST_INSTRUCTION("C5FA2DC1"                      , vcvtss2si(eax, xmm1));
  TEST_INSTRUCTION("C4E1FA2DC1"                    , vcvtss2si(rax, xmm1));
  TEST_INSTRUCTION("C5FB2DC1"                      , vcvtsd2si(eax, xmm1));
  TEST_INSTRUCTION("C4E1FB2DC1"                    , vcvtsd2si(rax, xmm1));
  TEST_INSTRUCTION("C5F22AC2"                      , vcvtsi2ss(xmm0, xmm1, edx));
  TEST_INSTRUCTION("C4E1F22AC2"                    , vcvtsi2ss(xmm0, xmm1, rdx));
  TEST_INSTRUCTION("C5F32AC2"                      , vcvtsi2sd(xmm0, xmm1, edx));
  TEST_INSTRUCTION("C4E1F32AC2"                    , vcvtsi2sd(xmm0, xmm1, rdx));
  TEST_INSTRUCTION("C57BE63B"                      , vcvtpd2dq(xmm15, xmmword_ptr(rbx)));
  TEST_INSTRUCTION("C57FE63B"                      , vcvtpd2dq(xmm15, ymmword_ptr(rbx)));
  TEST_INSTRUCTION("C5795A3B"                      , vcvtpd2ps(xmm15, xmmword_ptr(rbx)));
  TEST_INSTRUCTION("C57D5A3B"                      , vcvtpd2ps(xmm15, ymmword_ptr(rbx)));
  TEST_INSTRUCTION("6271FC08793B"                  , vcvtpd2udq(xmm15, xmmword_ptr(rbx)));
  TEST_INSTRUCTION("6271FC28793B"                  , vcvtpd2udq(xmm15, ymmword_ptr(rbx)));
  TEST_INSTRUCTION("6271FC085B3B"                  , vcvtqq2ps(xmm15, xmmword_ptr(rbx)));
  TEST_INSTRUCTION("6271FC285B3B"                  , vcvtqq2ps(xmm15, ymmword_ptr(rbx)));
  TEST_INSTRUCTION("C5F95AC1"                      , vcvtpd2ps(xmm0, xmm1));
  TEST_INSTRUCTION("C5F95A03"                      , vcvtpd2ps(xmm0, xmmword_ptr(rbx)));
  TEST_INSTRUCTION("C5FD5AC1"                      , vcvtpd2ps(xmm0, ymm1));
  TEST_INSTRUCTION("C5FD5A03"                      , vcvtpd2ps(xmm0, ymmword_ptr(rbx)));
  TEST_INSTRUCTION("62F1FD485AC1"                  , vcvtpd2ps(ymm0, zmm1));
  TEST_INSTRUCTION("62F1FD485A03"                  , vcvtpd2ps(ymm0, zmmword_ptr(rbx)));
  TEST_INSTRUCTION("C579E63B"                      , vcvttpd2dq(xmm15, xmmword_ptr(rbx)));
  TEST_INSTRUCTION("C57DE63B"                      , vcvttpd2dq(xmm15, ymmword_ptr(rbx)));
  TEST_INSTRUCTION("6271FC08783B"                  , vcvttpd2udq(xmm15, xmmword_ptr(rbx)));
  TEST_INSTRUCTION("6271FC28783B"                  , vcvttpd2udq(xmm15, ymmword_ptr(rbx)));
  TEST_INSTRUCTION("6271FF087A3B"                  , vcvtuqq2ps(xmm15, xmmword_ptr(rbx)));
  TEST_INSTRUCTION("6271FF287A3B"                  , vcvtuqq2ps(xmm15, ymmword_ptr(rbx)));
  TEST_INSTRUCTION("62F3FD08663F01"                , vfpclasspd(k7, xmmword_ptr(rdi), 0x01));
  TEST_INSTRUCTION("62F3FD28663701"                , vfpclasspd(k6, ymmword_ptr(rdi), 0x01));
  TEST_INSTRUCTION("62F3FD48662F01"                , vfpclasspd(k5, zmmword_ptr(rdi), 0x01));
  TEST_INSTRUCTION("62F37D08662701"                , vfpclassps(k4, xmmword_ptr(rdi), 0x01));
  TEST_INSTRUCTION("62F37D28661F01"                , vfpclassps(k3, ymmword_ptr(rdi), 0x01));
  TEST_INSTRUCTION("62F37D48661701"                , vfpclassps(k2, zmmword_ptr(rdi), 0x01));
  TEST_INSTRUCTION("6201951058F4"                  , rn_sae().vaddpd(zmm30, zmm29, zmm28));
  TEST_INSTRUCTION("6201953058F4"                  , rd_sae().vaddpd(zmm30, zmm29, zmm28));
  TEST_INSTRUCTION("6201955058F4"                  , ru_sae().vaddpd(zmm30, zmm29, zmm28));
  TEST_INSTRUCTION("6201957058F4"                  , rz_sae().vaddpd(zmm30, zmm29, zmm28));
  TEST_INSTRUCTION("62F16C4FC25498040F"            , k(k7).vcmpps(k2, zmm2, zmmword_ptr(rax, rbx, 2, 256), 15));
  TEST_INSTRUCTION("62F16C1FC25498400F"            , k(k7).vcmpps(k2, xmm2, dword_ptr(rax, rbx, 2, 256)._1to4(), 15));
  TEST_INSTRUCTION("62F16C3FC25498400F"            , k(k7).vcmpps(k2, ymm2, dword_ptr(rax, rbx, 2, 256)._1to8(), 15));
  TEST_INSTRUCTION("62F16C5FC25498400F"            , k(k7).vcmpps(k2, zmm2, dword_ptr(rax, rbx, 2, 256)._1to16(), 15));
  TEST_INSTRUCTION("62F1FD58C2C100"                , sae().vcmppd(k0, zmm0, zmm1, 0x00));
  TEST_INSTRUCTION("6201FD182EF5"                  , sae().vucomisd(xmm30, xmm29));
  TEST_INSTRUCTION("62017C182EF5"                  , sae().vucomiss(xmm30, xmm29));
  TEST_INSTRUCTION("C4E2FD91040500000000"          , vpgatherqq(ymm0, ptr(0, ymm0), ymm0));
  TEST_INSTRUCTION("C4E2E9920C00"                  , vgatherdpd(xmm1, ptr(rax, xmm0), xmm2));
  TEST_INSTRUCTION("C4E26990440D00"                , vpgatherdd(xmm0, ptr(rbp, xmm1), xmm2));
  TEST_INSTRUCTION("C4C26990040C"                  , vpgatherdd(xmm0, ptr(r12, xmm1), xmm2));
  TEST_INSTRUCTION("C4C26990440D00"                , vpgatherdd(xmm0, ptr(r13, xmm1), xmm2));
  TEST_INSTRUCTION("62F36D083ECB00"                , vpcmpub(k1, xmm2, xmm3, 0x0));
  TEST_INSTRUCTION("C5E9FE4C1140"                  , vpaddd(xmm1, xmm2, ptr(rcx, rdx, 0, 64)));
  TEST_INSTRUCTION("C5EDFE4C1140"                  , vpaddd(ymm1, ymm2, ptr(rcx, rdx, 0, 64)));
  TEST_INSTRUCTION("62F16D48FE4C1101"              , vpaddd(zmm1, zmm2, ptr(rcx, rdx, 0, 64)));
  TEST_INSTRUCTION("62E23D0850441104"              , vpdpbusd(xmm16, xmm8, ptr(rcx, rdx, 0, 64)));
  TEST_INSTRUCTION("62E23D2850441102"              , vpdpbusd(ymm16, ymm8, ptr(rcx, rdx, 0, 64)));
  TEST_INSTRUCTION("62E23D4850441101"              , vpdpbusd(zmm16, zmm8, ptr(rcx, rdx, 0, 64)));
  TEST_INSTRUCTION("62F26D48CF4C1101"              , vgf2p8mulb(zmm1, zmm2, zmmword_ptr(rcx, rdx, 0, 64)));
  TEST_INSTRUCTION("62F3ED48CE4C11010F"            , vgf2p8affineqb(zmm1, zmm2, zmmword_ptr(rcx, rdx, 0, 64), 15));
  TEST_INSTRUCTION("62F3ED48CF4C11010F"            , vgf2p8affineinvqb(zmm1, zmm2, zmmword_ptr(rcx, rdx, 0, 64), 15));
  TEST_INSTRUCTION("62F27D087AC6"                  , vpbroadcastb(xmm0, esi));
  TEST_INSTRUCTION("62F27D287AC6"                  , vpbroadcastb(ymm0, esi));
  TEST_INSTRUCTION("62F27D487AC6"                  , vpbroadcastb(zmm0, esi));
  TEST_INSTRUCTION("62F2CD088DF8"                  , vpermw(xmm7, xmm6, xmm0));
  TEST_INSTRUCTION("C4E3FD01FE01"                  , vpermpd(ymm7, ymm6, 1));
  TEST_INSTRUCTION("62F3FD4801FE01"                , vpermpd(zmm7, zmm6, 1));
  TEST_INSTRUCTION("62F2CD2816F8"                  , vpermpd(ymm7, ymm6, ymm0));
  TEST_INSTRUCTION("62F2CD4816F8"                  , vpermpd(zmm7, zmm6, zmm0));
  TEST_INSTRUCTION("C4E24D16F9"                    , vpermps(ymm7, ymm6, ymm1));
  TEST_INSTRUCTION("62F24D4816F9"                  , vpermps(zmm7, zmm6, zmm1));
  TEST_INSTRUCTION("6292472068F0"                  , vp2intersectd(k6, k7, ymm23, ymm24));
  TEST_INSTRUCTION("62B2472068B4F500000010"        , vp2intersectd(k6, k7, ymm23, ptr(rbp, r14, 3, 268435456)));
  TEST_INSTRUCTION("62F24730683500000000"          , vp2intersectd(k6, k7, ymm23, dword_ptr(rip)._1to8()));
  TEST_INSTRUCTION("62F2472068742DE0"              , vp2intersectd(k6, k7, ymm23, ymmword_ptr(rbp, rbp, 0, -1024)));
  TEST_INSTRUCTION("62F2472068717F"                , vp2intersectd(k6, k7, ymm23, ymmword_ptr(rcx, 4064)));

  // AVX512_VNNI vs AVX_VNNI.
  TEST_INSTRUCTION("62F2552850F4"                  , vpdpbusd(ymm6, ymm5, ymm4));
  TEST_INSTRUCTION("C4E25550F4"                    , vex().vpdpbusd(ymm6, ymm5, ymm4));

  tester.printSummary();
  return tester.didPass();
}

#undef TEST_INSTRUCTION

#endif
