//===- P2.cpp ------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// generally, programs will be creates as follows:
//
//   ld.lld -Ttext=0 -o foo foo.o
//   objcopy -O binary --only-section=.text foo output.bin
//
// Note that the current P2 support is very preliminary so you can't
// link any useful program yet, though.
//
//===----------------------------------------------------------------------===//

#include "InputFiles.h"
#include "Symbols.h"
#include "Target.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/Object/ELF.h"
#include "llvm/Support/Endian.h"

#define DEBUG_TYPE "p2"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;

namespace lld {
    namespace elf {

        namespace {
            class P2 final : public TargetInfo {
            public:
                P2();
                RelExpr getRelExpr(RelType type, const Symbol &s, const uint8_t *loc) const override;
                void relocate(uint8_t *loc, const Relocation &rel, uint64_t val) const override;
            };
        } // namespace

        P2::P2() { noneRel = R_P2_NONE; }

        RelExpr P2::getRelExpr(RelType type, const Symbol &s, const uint8_t *loc) const {
            switch (type) {
                default:
                    return R_ABS;
                case R_P2_32:
                case R_P2_20:
                case R_P2_AUG20:
                    return R_ABS;
                case R_P2_PC20:
                    return R_PC;
            }
        }

        void P2::relocate(uint8_t *loc, const Relocation &rel, uint64_t val) const {
            switch (rel.type) {
                case R_P2_32: {
                    *loc = val;
                    break;
                }
                case R_P2_20: {
                    uint32_t inst = read32le(loc);
                    inst += val & 0xfffff; // val is the relocation value, which is just the section this relocation references
                                           // so, we just add the new value (val) to the instructions to offset the 20 bit operand
                    write32le(loc, inst);
                    break;
                }
                case R_P2_AUG20: {
                    // special relocation where we modify 2 instructions to perform an immediate load of a 20-bit (or greater) immediate.
                    // by invoking the augd or augs instruction
                    uint32_t inst = read32le(loc);
                    uint32_t aug = read32le(loc-4); // the previous instruction is expected to be an AUGS/D

                    inst += val & 0x1ff; // get the lower 9 bits into the current instruction
                    aug = (aug & ~0x7fffff) | (val >> 9); // get the upper 23 bits into the previous AUG instruction

                    write32le(loc-4, aug);
                    write32le(loc, inst);
                    break;
                }
                default:
                    error(getErrorLocation(loc) + "unrecognized relocation " + toString(rel.type));
            }
        }

        TargetInfo *getP2TargetInfo() {
            static P2 target;
            return &target;
        }
    } // namespace elf
} // namespace lld
