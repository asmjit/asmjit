// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_DISABLE_BUILDER)

#include "../core/zonestack.h"

#include "../x86/x86instdb_p.h"
#include "../x86/x86operand.h"
#include "../x86/x86ssetoavx_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// ${SseToAvxIndex:Begin}
// ------------------- Automatically generated, do not edit -------------------
static const uint8_t sseToAvxIndex[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 2, 2, 2, 2, 3, 3, 0, 0, 2, 2, 2,
  2, 0, 0, 0, 0, 0, 0, 0, 4, 4, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 6, 6, 0, 7, 7, 0, 0, 0, 8, 8, 0, 0, 0, 9, 9, 10, 0, 8, 0, 0, 11, 11, 0,
  12, 13, 14, 14, 14, 15, 16, 0, 17, 0, 18, 19, 0, 0, 0, 0, 0, 0, 20, 20, 20, 20,
  21, 21, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 23, 23, 23, 23, 23, 0, 24, 24, 0, 0, 0, 0, 0, 25, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 28, 28, 28, 28, 0, 29, 29,
  29, 29, 0, 0, 0, 30, 30, 0, 31, 31, 0, 0, 0, 32, 33, 34, 35, 35, 34, 35, 35, 36,
  36, 36, 36, 0, 37, 37, 0, 0, 0, 33, 0, 0, 38, 32, 32, 38, 0, 0, 39, 39, 0,
  40, 0, 28, 28, 28, 28, 0, 0, 0, 0, 0, 0, 0, 40, 40, 0, 0, 41, 41, 39, 42, 42,
  42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 0, 44, 0, 45, 46, 44, 47,
  48, 48, 48, 48, 49, 49, 48, 48, 48, 48, 49, 49, 0, 0, 0, 50, 50, 50, 50, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51, 52, 53, 54, 55,
  56, 56, 0, 0, 55, 55, 55, 55, 57, 57, 58, 58, 59, 59, 59, 60, 60, 60, 61, 61,
  61, 62, 15, 63, 63, 63, 63, 63, 63, 64, 64, 64, 64, 64, 64, 65, 65, 0, 66, 66,
  66, 65, 67, 0, 0, 0, 0, 0, 0, 0, 68, 0, 0, 0, 0, 0, 0, 0, 69, 70, 71, 71, 71,
  0, 70, 70, 70, 70, 70, 70, 2, 2, 72, 72, 72, 72, 73, 73, 73, 73, 73, 73, 73,
  73, 73, 0, 74, 75, 75, 75, 75, 75, 75, 75, 75, 0, 0, 0, 0, 0, 0, 76, 0, 77, 78,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 79, 79, 80, 80, 0, 81, 82, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 83, 0,
  0, 0, 0, 0, 84, 84, 85, 85, 0, 0, 0, 0, 0, 0, 0, 0, 0, 86, 86, 86, 86, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 87, 87, 0, 88, 88, 88, 88, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 2, 2, 2, 2, 3, 3, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 4, 4, 5, 5, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 7, 7, 8, 8, 0, 0, 9, 9, 0, 0, 10, 8,
  0, 0, 0, 0, 11, 11, 0, 0, 0, 0, 0, 0, 12, 13, 0, 14, 14, 14, 15, 0, 16, 0, 0,
  0, 17, 0, 0, 0, 18, 0, 19, 0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 20, 20, 0, 21, 21,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23,
  23, 23, 23, 23, 24, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 26, 0, 27, 0, 0,
  28, 28, 28, 28, 0, 0, 0, 29, 29, 29, 29, 0, 0, 0, 30, 30, 31, 31, 32, 0, 0, 33,
  0, 0, 0, 0, 34, 35, 35, 34, 35, 35, 36, 36, 36, 36, 37, 37, 33, 38, 32, 32,
  38, 39, 39, 40, 0, 0, 0, 0, 0, 0, 28, 28, 28, 28, 0, 0, 40, 40, 0, 0, 41, 41,
  0, 39, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 0, 43, 0, 0, 0,
  44, 45, 0, 46, 44, 0, 0, 0, 0, 0, 0, 47, 0, 0, 0, 48, 48, 48, 48, 49, 49, 48,
  48, 48, 48, 49, 49, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 50, 0, 0, 0, 0, 0, 0, 0, 51, 0, 52, 0, 0, 0,
  0, 0, 0, 53, 0, 0, 54, 0, 55, 0, 56, 56, 0, 55, 55, 55, 55, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 57, 0, 0, 58, 58, 0, 59, 59, 59, 0, 60, 60,
  60, 0, 61, 61, 61, 0, 62, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 63, 63, 63, 63, 63, 63, 0, 0, 0, 0, 0, 0, 0, 0, 64, 64, 64, 64, 64, 64,
  65, 65, 66, 66, 66, 0, 65, 0, 67, 0, 0, 0, 0, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 70, 0, 71, 71, 71, 70, 70, 70, 70, 70, 70, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 72, 72, 72, 72, 0, 0, 0, 73, 73, 73, 73, 73, 73, 73, 73, 73, 0, 0, 74,
  0, 0, 0, 0, 0, 0, 0, 0, 75, 75, 75, 75, 75, 75, 75, 75, 76, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 77, 78, 0, 0, 0, 0, 0, 0, 0, 0, 79, 79, 80, 80, 0, 0,
  0, 0, 0, 0, 0, 0, 81, 82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 83, 83, 84, 84, 85, 85, 0, 86, 86, 86, 86, 0, 0, 87, 87, 88, 88, 88, 88,
  89, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 89, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0
};
// ----------------------------------------------------------------------------
// ${SseToAvxIndex:End}

// ${SseToAvxTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
static const SseToAvxData sseToAvxData[] = {
  { SseToAvxData::kModeNone     , 0    }, // #0 [ref=1015x]
  { SseToAvxData::kModeExtend   , 749  }, // #1 [ref=6x]
  { SseToAvxData::kModeExtend   , 748  }, // #2 [ref=10x]
  { SseToAvxData::kModeMove     , 748  }, // #3 [ref=2x]
  { SseToAvxData::kModeExtend   , 747  }, // #4 [ref=2x]
  { SseToAvxData::kModeBlend    , 747  }, // #5 [ref=2x]
  { SseToAvxData::kModeExtend   , 693  }, // #6 [ref=2x]
  { SseToAvxData::kModeExtend   , 692  }, // #7 [ref=2x]
  { SseToAvxData::kModeMove     , 689  }, // #8 [ref=3x]
  { SseToAvxData::kModeMove     , 688  }, // #9 [ref=2x]
  { SseToAvxData::kModeMove     , 690  }, // #10 [ref=1x]
  { SseToAvxData::kModeMove     , 691  }, // #11 [ref=2x]
  { SseToAvxData::kModeMove     , 696  }, // #12 [ref=1x]
  { SseToAvxData::kModeExtend   , 696  }, // #13 [ref=1x]
  { SseToAvxData::kModeExtend   , 697  }, // #14 [ref=3x]
  { SseToAvxData::kModeMove     , 697  }, // #15 [ref=2x]
  { SseToAvxData::kModeMove     , 698  }, // #16 [ref=1x]
  { SseToAvxData::kModeMove     , 700  }, // #17 [ref=1x]
  { SseToAvxData::kModeMove     , 702  }, // #18 [ref=1x]
  { SseToAvxData::kModeMove     , 703  }, // #19 [ref=1x]
  { SseToAvxData::kModeExtend   , 705  }, // #20 [ref=4x]
  { SseToAvxData::kModeExtend   , 706  }, // #21 [ref=2x]
  { SseToAvxData::kModeMove     , 718  }, // #22 [ref=1x]
  { SseToAvxData::kModeExtend   , 730  }, // #23 [ref=5x]
  { SseToAvxData::kModeExtend   , 729  }, // #24 [ref=2x]
  { SseToAvxData::kModeExtend   , 734  }, // #25 [ref=1x]
  { SseToAvxData::kModeMove     , 635  }, // #26 [ref=1x]
  { SseToAvxData::kModeMove     , 613  }, // #27 [ref=1x]
  { SseToAvxData::kModeExtend   , 614  }, // #28 [ref=8x]
  { SseToAvxData::kModeExtend   , 616  }, // #29 [ref=4x]
  { SseToAvxData::kModeMove     , 616  }, // #30 [ref=2x]
  { SseToAvxData::kModeMove     , 615  }, // #31 [ref=2x]
  { SseToAvxData::kModeMove     , 612  }, // #32 [ref=3x]
  { SseToAvxData::kModeMove     , 614  }, // #33 [ref=2x]
  { SseToAvxData::kModeExtend   , 618  }, // #34 [ref=2x]
  { SseToAvxData::kModeMoveIfMem, 618  }, // #35 [ref=4x]
  { SseToAvxData::kModeMove     , 618  }, // #36 [ref=4x]
  { SseToAvxData::kModeMove     , 617  }, // #37 [ref=2x]
  { SseToAvxData::kModeMoveIfMem, 612  }, // #38 [ref=2x]
  { SseToAvxData::kModeMove     , 610  }, // #39 [ref=3x]
  { SseToAvxData::kModeExtend   , 609  }, // #40 [ref=3x]
  { SseToAvxData::kModeMove     , 609  }, // #41 [ref=2x]
  { SseToAvxData::kModeExtend   , 610  }, // #42 [ref=14x]
  { SseToAvxData::kModeExtend   , 611  }, // #43 [ref=1x]
  { SseToAvxData::kModeExtend   , 613  }, // #44 [ref=2x]
  { SseToAvxData::kModeExtend   , 612  }, // #45 [ref=1x]
  { SseToAvxData::kModeBlend    , 613  }, // #46 [ref=1x]
  { SseToAvxData::kModeExtend   , 619  }, // #47 [ref=1x]
  { SseToAvxData::kModeExtend   , 622  }, // #48 [ref=8x]
  { SseToAvxData::kModeMove     , 622  }, // #49 [ref=4x]
  { SseToAvxData::kModeMove     , 671  }, // #50 [ref=4x]
  { SseToAvxData::kModeExtend   , 657  }, // #51 [ref=1x]
  { SseToAvxData::kModeExtend   , 658  }, // #52 [ref=1x]
  { SseToAvxData::kModeExtend   , 664  }, // #53 [ref=1x]
  { SseToAvxData::kModeMove     , 666  }, // #54 [ref=1x]
  { SseToAvxData::kModeExtend   , 667  }, // #55 [ref=5x]
  { SseToAvxData::kModeExtend   , 668  }, // #56 [ref=2x]
  { SseToAvxData::kModeExtend   , 683  }, // #57 [ref=2x]
  { SseToAvxData::kModeExtend   , 685  }, // #58 [ref=2x]
  { SseToAvxData::kModeExtend   , 686  }, // #59 [ref=3x]
  { SseToAvxData::kModeExtend   , 687  }, // #60 [ref=3x]
  { SseToAvxData::kModeExtend   , 688  }, // #61 [ref=3x]
  { SseToAvxData::kModeExtend   , 689  }, // #62 [ref=1x]
  { SseToAvxData::kModeMove     , 707  }, // #63 [ref=6x]
  { SseToAvxData::kModeMove     , 715  }, // #64 [ref=6x]
  { SseToAvxData::kModeExtend   , 715  }, // #65 [ref=3x]
  { SseToAvxData::kModeExtend   , 714  }, // #66 [ref=3x]
  { SseToAvxData::kModeExtend   , 716  }, // #67 [ref=1x]
  { SseToAvxData::kModeExtend   , 713  }, // #68 [ref=1x]
  { SseToAvxData::kModeExtend   , 721  }, // #69 [ref=1x]
  { SseToAvxData::kModeExtend   , 745  }, // #70 [ref=7x]
  { SseToAvxData::kModeMove     , 746  }, // #71 [ref=3x]
  { SseToAvxData::kModeExtend   , 752  }, // #72 [ref=4x]
  { SseToAvxData::kModeExtend   , 755  }, // #73 [ref=9x]
  { SseToAvxData::kModeMove     , 756  }, // #74 [ref=1x]
  { SseToAvxData::kModeExtend   , 764  }, // #75 [ref=8x]
  { SseToAvxData::kModeExtend   , 758  }, // #76 [ref=1x]
  { SseToAvxData::kModeMove     , 771  }, // #77 [ref=1x]
  { SseToAvxData::kModeExtend   , 771  }, // #78 [ref=1x]
  { SseToAvxData::kModeMove     , 765  }, // #79 [ref=2x]
  { SseToAvxData::kModeExtend   , 765  }, // #80 [ref=2x]
  { SseToAvxData::kModeMove     , 772  }, // #81 [ref=1x]
  { SseToAvxData::kModeExtend   , 772  }, // #82 [ref=1x]
  { SseToAvxData::kModeExtend   , 741  }, // #83 [ref=2x]
  { SseToAvxData::kModeMove     , 736  }, // #84 [ref=2x]
  { SseToAvxData::kModeExtend   , 736  }, // #85 [ref=2x]
  { SseToAvxData::kModeExtend   , 728  }, // #86 [ref=4x]
  { SseToAvxData::kModeMove     , 719  }, // #87 [ref=2x]
  { SseToAvxData::kModeExtend   , 718  }, // #88 [ref=4x]
  { SseToAvxData::kModeExtend   , -17  }  // #89 [ref=2x]
};
// ----------------------------------------------------------------------------
// ${SseToAvxTable:End}

// ============================================================================
// [asmjit::x86::SseToAvxPass - Construction / Destruction]
// ============================================================================

SseToAvxPass::SseToAvxPass() noexcept
  : Pass("SseToAvxPass"),
    _translated(false) {}

// ============================================================================
// [asmjit::x86::SseToAvxPass - Run]
// ============================================================================

Error SseToAvxPass::run(Zone* zone, Logger* logger) noexcept {
  ASMJIT_UNUSED(logger);

  ZoneAllocator allocator(zone);
  ZoneStack<InstNode*> insts;
  ASMJIT_PROPAGATE(insts.init(&allocator));

  // Probe loop. Ad all nodes that can be converted to `insts` list and fail if
  // they are not consistent or there is node that cannot be translated to AVX
  // (conversion between MMX<->XMM or use of SHA or SSE4A extensions).
  BaseNode* node = cb()->firstNode();

  while (node) {
    if (node->type() == BaseNode::kNodeInst) {
      InstNode* inst = node->as<InstNode>();
      uint32_t instId = inst->id();

      // Skip invalid and abstract instructions.
      if (!Inst::isDefinedId(instId))
        continue;

      // Skip non-SSE instructions.
      const InstDB::InstInfo& instInfo = InstDB::infoById(instId);
      if (!instInfo.isVec() || instInfo.isVex() || instInfo.isEvex())
        continue;

      uint32_t opCount = inst->opCount();
      uint32_t regMask = probeRegs(inst->operands(), opCount);

      // Skip instructions that don't use XMM registers.
      if (!(regMask & kProbeXmm))
        continue;

      if (!(regMask & kProbeMmx)) {
        const SseToAvxData& sseToAvx = sseToAvxData[sseToAvxIndex[instId]];
        switch (sseToAvx.mode()) {
          case SseToAvxData::kModeNone:
            // Cannot translate.
            return kErrorOk;

          case SseToAvxData::kModeMove:
            break;

          case SseToAvxData::kModeMoveIfMem:
          case SseToAvxData::kModeExtend:
            // Cannot translate if the instruction is malformed.
            if (ASMJIT_UNLIKELY(opCount < 1 || opCount > 3))
              return kErrorOk;
            break;

          case SseToAvxData::kModeBlend:
            // Cannot translate if the instruction is malformed.
            if (ASMJIT_UNLIKELY(opCount < 2 || opCount > 3))
              return kErrorOk;
            break;
        }
      }
      else {
        // Conversion between MMX and XMM register cannot be translated to AVX.
        return kErrorOk;
      }

      ASMJIT_PROPAGATE(insts.append(inst));
    }

    node = node->next();
  }

  // Second loop - patch all nodes we added to `insts` to use AVX instead of SSE.
  // At this moment we know that patching should not cause any performance issues
  // and all instructions added to `insts` are patchable.
  while (!insts.empty()) {
    InstNode* inst = insts.popFirst();
    uint32_t instId = inst->id();
    uint32_t opCount = inst->opCount();

    const SseToAvxData& sseToAvx = sseToAvxData[sseToAvxIndex[instId]];
    uint32_t mode = sseToAvx.mode();

    if (mode == SseToAvxData::kModeMoveIfMem) {
      // Only extends if the instruction doesn't move from/to memory.
      if (!inst->hasMemOp())
        mode = SseToAvxData::kModeExtend;
    }
    else if (mode == SseToAvxData::kModeBlend) {
      // Translate [xmmA, xmmB/m128, <xmm0>] to [xmmA, xmmA, xmmB/m128, xmm0].
      if (opCount == 2)
        inst->_opArray[opCount++] = xmm0;
      mode = SseToAvxData::kModeExtend;
    }

    if (mode == SseToAvxData::kModeExtend) {
      for (uint32_t i = opCount; i > 0; i--)
        inst->setOp(i, inst->opType(i - 1));
      inst->setOpCount(opCount + 1);
    }

    instId = uint32_t(int32_t(instId) + sseToAvx.delta());
    ASMJIT_ASSERT(Inst::isDefinedId(instId));
    inst->setId(instId);
  }

  _translated = true;
  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_BUILD_X86 && !ASMJIT_DISABLE_BUILDER
