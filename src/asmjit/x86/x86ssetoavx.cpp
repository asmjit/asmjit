// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_DISABLE_BUILDER)

// [Dependencies]
#include "../core/zonestack.h"

#include "../x86/x86instdb.h"
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
  6, 6, 0, 7, 7, 0, 0, 0, 8, 8, 0, 0, 0, 9, 9, 9, 0, 10, 0, 0, 8, 8, 0, 11, 12,
  13, 13, 13, 14, 15, 0, 16, 0, 17, 18, 0, 0, 0, 0, 0, 0, 19, 19, 19, 19, 19, 19,
  0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21,
  21, 0, 22, 22, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25,
  0, 26, 26, 26, 26, 0, 27, 27, 27, 27, 0, 0, 0, 28, 28, 0, 29, 29, 0, 30, 28,
  26, 31, 31, 26, 31, 31, 32, 32, 32, 32, 0, 25, 25, 0, 0, 0, 28, 0, 0, 33, 30,
  30, 33, 0, 0, 34, 34, 0, 35, 0, 36, 36, 36, 36, 0, 0, 0, 0, 0, 0, 0, 37, 37, 0,
  0, 38, 38, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 0,
  42, 0, 43, 44, 42, 45, 46, 46, 46, 46, 28, 28, 46, 46, 46, 46, 28, 28, 0, 0,
  0, 47, 47, 47, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 48, 49, 50, 51, 52, 53, 53, 0, 0, 52, 52, 52, 52, 54, 54, 55, 55, 56, 56,
  56, 57, 57, 57, 58, 58, 58, 59, 10, 60, 60, 60, 60, 60, 60, 61, 61, 61, 61, 61,
  61, 62, 62, 0, 63, 63, 63, 62, 64, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0,
  0, 0, 65, 66, 67, 67, 67, 0, 66, 66, 66, 66, 66, 66, 68, 68, 69, 69, 69, 69, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 0, 3, 70, 70, 70, 70, 70, 70, 70, 70, 0, 0, 0, 0, 0,
  0, 71, 0, 72, 73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 74, 74, 75, 75, 0, 76,
  77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 78, 78, 0, 0, 0, 79, 79, 22, 22, 0, 0, 0, 0, 0, 0, 0, 0, 80, 80, 80, 80, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 81, 81, 0, 82, 82, 82, 82, 0, 0, 0, 0, 1, 1, 1,
  1, 1, 1, 2, 2, 2, 2, 3, 3, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 4, 4, 5, 5, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 7, 7, 8, 8, 0, 0, 9, 9, 9, 10,
  0, 0, 0, 0, 8, 8, 0, 0, 0, 0, 0, 0, 11, 12, 0, 13, 13, 13, 14, 0, 15, 0, 0, 0,
  16, 0, 0, 0, 17, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 19, 19, 19, 19, 19, 19, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 21,
  22, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 24, 0, 25, 0, 0, 26, 26, 26, 26, 27,
  27, 27, 27, 28, 28, 29, 29, 30, 0, 0, 28, 0, 0, 0, 0, 26, 31, 31, 26, 31, 31,
  32, 32, 32, 32, 25, 25, 28, 33, 30, 30, 33, 34, 34, 35, 36, 36, 36, 36, 37,
  37, 0, 0, 38, 38, 0, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
  0, 41, 0, 0, 0, 42, 43, 0, 44, 42, 0, 0, 0, 0, 0, 0, 45, 0, 0, 0, 46, 46, 46,
  46, 28, 28, 46, 46, 46, 46, 28, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 47, 47, 47, 47, 0, 0, 0, 0, 0, 0, 0, 48, 0, 49, 0,
  0, 0, 0, 0, 0, 50, 0, 0, 51, 0, 52, 0, 53, 53, 0, 52, 52, 52, 52, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 54, 54, 0, 0, 55, 55, 0, 56, 56, 56, 0, 57,
  57, 57, 0, 58, 58, 58, 0, 59, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 60, 60, 60, 60, 60, 60, 0, 0, 0, 0, 0, 0, 0, 0, 61, 61, 61, 61, 61,
  61, 62, 62, 63, 63, 63, 0, 62, 0, 64, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 66, 0, 67, 67, 67, 66, 66, 66, 66, 66, 66, 0, 0, 0, 68, 68,
  0, 0, 0, 0, 69, 69, 69, 69, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 3, 0,
  0, 0, 0, 0, 0, 0, 0, 70, 70, 70, 70, 70, 70, 70, 70, 71, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 72, 73, 0, 0, 0, 0, 0, 0, 0, 0, 74, 74, 75, 75, 0, 0, 0,
  0, 0, 0, 0, 0, 76, 77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 78, 78, 79, 79, 22, 22, 0, 80, 80, 80, 80, 0, 0, 81, 81, 82, 82, 82, 82,
  83, 83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 83, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0
};
// ----------------------------------------------------------------------------
// ${SseToAvxIndex:End}

// ${SseToAvxTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
static const SseToAvxData sseToAvxData[] = {
  { SseToAvxData::kModeNone     , 0    }, // #0 [ref=976x]
  { SseToAvxData::kModeExtend   , 729  }, // #1 [ref=6x]
  { SseToAvxData::kModeExtend   , 728  }, // #2 [ref=8x]
  { SseToAvxData::kModeMove     , 728  }, // #3 [ref=3x]
  { SseToAvxData::kModeExtend   , 727  }, // #4 [ref=11x]
  { SseToAvxData::kModeBlend    , 727  }, // #5 [ref=2x]
  { SseToAvxData::kModeExtend   , 675  }, // #6 [ref=2x]
  { SseToAvxData::kModeExtend   , 674  }, // #7 [ref=2x]
  { SseToAvxData::kModeMove     , 671  }, // #8 [ref=4x]
  { SseToAvxData::kModeMove     , 670  }, // #9 [ref=3x]
  { SseToAvxData::kModeMove     , 669  }, // #10 [ref=2x]
  { SseToAvxData::kModeMove     , 676  }, // #11 [ref=1x]
  { SseToAvxData::kModeExtend   , 676  }, // #12 [ref=1x]
  { SseToAvxData::kModeExtend   , 677  }, // #13 [ref=3x]
  { SseToAvxData::kModeMove     , 677  }, // #14 [ref=1x]
  { SseToAvxData::kModeMove     , 678  }, // #15 [ref=1x]
  { SseToAvxData::kModeMove     , 680  }, // #16 [ref=1x]
  { SseToAvxData::kModeMove     , 682  }, // #17 [ref=1x]
  { SseToAvxData::kModeMove     , 683  }, // #18 [ref=1x]
  { SseToAvxData::kModeExtend   , 685  }, // #19 [ref=7x]
  { SseToAvxData::kModeMove     , 699  }, // #20 [ref=1x]
  { SseToAvxData::kModeExtend   , 712  }, // #21 [ref=2x]
  { SseToAvxData::kModeExtend   , 711  }, // #22 [ref=4x]
  { SseToAvxData::kModeExtend   , 716  }, // #23 [ref=1x]
  { SseToAvxData::kModeMove     , 620  }, // #24 [ref=1x]
  { SseToAvxData::kModeMove     , 601  }, // #25 [ref=3x]
  { SseToAvxData::kModeExtend   , 602  }, // #26 [ref=6x]
  { SseToAvxData::kModeExtend   , 601  }, // #27 [ref=4x]
  { SseToAvxData::kModeMove     , 598  }, // #28 [ref=8x]
  { SseToAvxData::kModeMove     , 597  }, // #29 [ref=2x]
  { SseToAvxData::kModeMove     , 596  }, // #30 [ref=3x]
  { SseToAvxData::kModeMoveIfMem, 602  }, // #31 [ref=4x]
  { SseToAvxData::kModeMove     , 602  }, // #32 [ref=4x]
  { SseToAvxData::kModeMoveIfMem, 596  }, // #33 [ref=2x]
  { SseToAvxData::kModeMove     , 594  }, // #34 [ref=2x]
  { SseToAvxData::kModeExtend   , 593  }, // #35 [ref=1x]
  { SseToAvxData::kModeExtend   , 592  }, // #36 [ref=4x]
  { SseToAvxData::kModeExtend   , 585  }, // #37 [ref=2x]
  { SseToAvxData::kModeMove     , 585  }, // #38 [ref=2x]
  { SseToAvxData::kModeMove     , 586  }, // #39 [ref=1x]
  { SseToAvxData::kModeExtend   , 586  }, // #40 [ref=14x]
  { SseToAvxData::kModeExtend   , 587  }, // #41 [ref=1x]
  { SseToAvxData::kModeExtend   , 589  }, // #42 [ref=2x]
  { SseToAvxData::kModeExtend   , 588  }, // #43 [ref=1x]
  { SseToAvxData::kModeBlend    , 589  }, // #44 [ref=1x]
  { SseToAvxData::kModeExtend   , 595  }, // #45 [ref=1x]
  { SseToAvxData::kModeExtend   , 598  }, // #46 [ref=8x]
  { SseToAvxData::kModeMove     , 643  }, // #47 [ref=4x]
  { SseToAvxData::kModeExtend   , 629  }, // #48 [ref=1x]
  { SseToAvxData::kModeExtend   , 630  }, // #49 [ref=1x]
  { SseToAvxData::kModeExtend   , 636  }, // #50 [ref=1x]
  { SseToAvxData::kModeMove     , 638  }, // #51 [ref=1x]
  { SseToAvxData::kModeExtend   , 639  }, // #52 [ref=5x]
  { SseToAvxData::kModeExtend   , 640  }, // #53 [ref=2x]
  { SseToAvxData::kModeExtend   , 655  }, // #54 [ref=2x]
  { SseToAvxData::kModeExtend   , 657  }, // #55 [ref=2x]
  { SseToAvxData::kModeExtend   , 658  }, // #56 [ref=3x]
  { SseToAvxData::kModeExtend   , 659  }, // #57 [ref=3x]
  { SseToAvxData::kModeExtend   , 660  }, // #58 [ref=3x]
  { SseToAvxData::kModeExtend   , 661  }, // #59 [ref=1x]
  { SseToAvxData::kModeMove     , 679  }, // #60 [ref=6x]
  { SseToAvxData::kModeMove     , 687  }, // #61 [ref=6x]
  { SseToAvxData::kModeExtend   , 687  }, // #62 [ref=3x]
  { SseToAvxData::kModeExtend   , 686  }, // #63 [ref=3x]
  { SseToAvxData::kModeExtend   , 688  }, // #64 [ref=1x]
  { SseToAvxData::kModeExtend   , 693  }, // #65 [ref=1x]
  { SseToAvxData::kModeExtend   , 717  }, // #66 [ref=7x]
  { SseToAvxData::kModeMove     , 718  }, // #67 [ref=3x]
  { SseToAvxData::kModeExtend   , 720  }, // #68 [ref=2x]
  { SseToAvxData::kModeExtend   , 724  }, // #69 [ref=4x]
  { SseToAvxData::kModeExtend   , 736  }, // #70 [ref=8x]
  { SseToAvxData::kModeExtend   , 730  }, // #71 [ref=1x]
  { SseToAvxData::kModeMove     , 743  }, // #72 [ref=1x]
  { SseToAvxData::kModeExtend   , 743  }, // #73 [ref=1x]
  { SseToAvxData::kModeMove     , 738  }, // #74 [ref=2x]
  { SseToAvxData::kModeExtend   , 738  }, // #75 [ref=2x]
  { SseToAvxData::kModeMove     , 745  }, // #76 [ref=1x]
  { SseToAvxData::kModeExtend   , 745  }, // #77 [ref=1x]
  { SseToAvxData::kModeExtend   , 714  }, // #78 [ref=2x]
  { SseToAvxData::kModeMove     , 711  }, // #79 [ref=2x]
  { SseToAvxData::kModeExtend   , 704  }, // #80 [ref=4x]
  { SseToAvxData::kModeMove     , 695  }, // #81 [ref=2x]
  { SseToAvxData::kModeExtend   , 694  }, // #82 [ref=4x]
  { SseToAvxData::kModeExtend   , -16  }  // #83 [ref=2x]
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

// [Guard]
#endif // ASMJIT_BUILD_X86 && !ASMJIT_DISABLE_BUILDER
