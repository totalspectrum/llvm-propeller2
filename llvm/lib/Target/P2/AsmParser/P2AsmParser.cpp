//===-- P2AsmParser.cpp - Parse P2 assembly to MCInst instructions ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "P2.h"
#include "MCTargetDesc/P2MCTargetDesc.h"
#include "MCTargetDesc/P2BaseInfo.h"
#include "P2RegisterInfo.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"

#include <set>

using namespace llvm;

#define DEBUG_TYPE "p2-asm-parser"

namespace {
    class P2AssemblerOptions {
    public:
        P2AssemblerOptions():
            reorder(true), macro(true) {
        }

        bool isReorder() {return reorder;}
        void setReorder() {reorder = true;}
        void setNoreorder() {reorder = false;}

        bool isMacro() {return macro;}
        void setMacro() {macro = true;}
        void setNomacro() {macro = false;}

    private:
        bool reorder;
        bool macro;
    };
}

namespace {
    class P2AsmParser : public MCTargetAsmParser {
        MCAsmParser &Parser;
        P2AssemblerOptions Options;

        std::set<StringRef> cond_strings {
            "_ret_",
            "if_nc_and_nz",
            "if_nc_and_z",
            "if_nc",
            "if_c_and_nz",
            "if_nz",
            "if_c_ne_z",
            "if_nc_or_nz",
            "if_c_and_z",
            "if_c_eq_z",
            "if_z",
            "if_nc_or_z",
            "if_c",
            "if_c_or_nz",
            "if_c_or_z",
            ""
        };

    #define GET_ASSEMBLER_HEADER
    #include "P2GenAsmMatcher.inc"

        bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode, OperandVector &Operands, MCStreamer &Out,
                                        uint64_t &ErrorInfo, bool MatchingInlineAsm) override;
        bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;
        bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name, SMLoc NameLoc, OperandVector &Operands) override;
        bool ParseDirective(AsmToken DirectiveID) override;
        OperandMatchResultTy tryParseRegister(unsigned &reg_no, SMLoc &start, SMLoc &end) override;

        bool parseOperand(OperandVector &Operands, StringRef Mnemonic);
        int parseRegister(StringRef Mnemonic);
        bool tryParseRegisterOperand(OperandVector &Operands, StringRef Mnemonic);
        int matchRegisterName(StringRef Symbol);
        int matchRegisterByNumber(unsigned RegNum, StringRef Mnemonic);
        unsigned getReg(int RC,int RegNo);

    public:
        P2AsmParser(const MCSubtargetInfo &sti, MCAsmParser &parser, const MCInstrInfo &MII, const MCTargetOptions &Options)
                    : MCTargetAsmParser(Options, sti, MII), Parser(parser) {
            // Initialize the set of available features.
            setAvailableFeatures(ComputeAvailableFeatures(getSTI().getFeatureBits()));
        }

        MCAsmParser &getParser() const { return Parser; }
        MCAsmLexer &getLexer() const { return Parser.getLexer(); }
    };
}

namespace {

    /// P2Operand - Instances of this class represent a parsed P2 machine
    /// instruction.
    class P2Operand : public MCParsedAsmOperand {

        enum KindTy {
            k_Immediate,
            k_Memory,
            k_Register,
            k_Token
        } Kind;

    public:
        P2Operand(KindTy K) : MCParsedAsmOperand(), Kind(K) {}

        struct Token {
            const char *Data;
            unsigned Length;
        };
        struct PhysRegOp {
            unsigned RegNum; /// Register Number
        };
        struct ImmOp {
            const MCExpr *Val;
        };
        struct MemOp {
            unsigned Base;
            const MCExpr *Off;
        };

        union {
            struct Token Tok;
            struct PhysRegOp Reg;
            struct ImmOp Imm;
            struct MemOp Mem;
        };

        SMLoc StartLoc, EndLoc;

    public:
        void addRegOperands(MCInst &Inst, unsigned N) const {
            assert(N == 1 && "Invalid number of operands!");
            Inst.addOperand(MCOperand::createReg(getReg()));
        }

        void addExpr(MCInst &Inst, const MCExpr *Expr) const{
            // Add as immediate when possible.  Null MCExpr = 0.
            if (Expr == 0)
                Inst.addOperand(MCOperand::createImm(0));
            else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
                Inst.addOperand(MCOperand::createImm(CE->getValue()));
            else
                Inst.addOperand(MCOperand::createExpr(Expr));
        }

        void addImmOperands(MCInst &Inst, unsigned N) const {
            assert(N == 1 && "Invalid number of operands!");
            const MCExpr *Expr = getImm();
            addExpr(Inst,Expr);
        }

        void addMemOperands(MCInst &Inst, unsigned N) const {
            assert(N == 2 && "Invalid number of operands!");

            Inst.addOperand(MCOperand::createReg(getMemBase()));

            const MCExpr *Expr = getMemOff();
            addExpr(Inst,Expr);
        }

        bool isReg() const override { return Kind == k_Register; }
        bool isImm() const override { return Kind == k_Immediate; }
        bool isToken() const override { return Kind == k_Token; }
        bool isMem() const override { return Kind == k_Memory; }

        StringRef getToken() const {
            assert(Kind == k_Token && "Invalid access!");
            return StringRef(Tok.Data, Tok.Length);
        }

        unsigned getReg() const override {
            assert((Kind == k_Register) && "Invalid access!");
            return Reg.RegNum;
        }

        const MCExpr *getImm() const {
            assert((Kind == k_Immediate) && "Invalid access!");
            return Imm.Val;
        }

        unsigned getMemBase() const {
            assert((Kind == k_Memory) && "Invalid access!");
            return Mem.Base;
        }

        const MCExpr *getMemOff() const {
            assert((Kind == k_Memory) && "Invalid access!");
            return Mem.Off;
        }

        static std::unique_ptr<P2Operand> CreateToken(StringRef Str, SMLoc S) {
            auto Op = std::make_unique<P2Operand>(k_Token);
            Op->Tok.Data = Str.data();
            Op->Tok.Length = Str.size();
            Op->StartLoc = S;
            Op->EndLoc = S;
            return Op;
        }

        /// Internal constructor for register kinds
        static std::unique_ptr<P2Operand> CreateReg(unsigned RegNum, SMLoc S, SMLoc E) {
            auto Op = std::make_unique<P2Operand>(k_Register);
            Op->Reg.RegNum = RegNum;
            Op->StartLoc = S;
            Op->EndLoc = E;
            return Op;
        }

        static std::unique_ptr<P2Operand> CreateImm(const MCExpr *Val, SMLoc S, SMLoc E) {
            auto Op = std::make_unique<P2Operand>(k_Immediate);
            Op->Imm.Val = Val;
            Op->StartLoc = S;
            Op->EndLoc = E;
            return Op;
        }

        static std::unique_ptr<P2Operand> CreateMem(unsigned Base, const MCExpr *Off, SMLoc S, SMLoc E) {
            auto Op = std::make_unique<P2Operand>(k_Memory);
            Op->Mem.Base = Base;
            Op->Mem.Off = Off;
            Op->StartLoc = S;
            Op->EndLoc = E;
            return Op;
        }

        /// getStartLoc - Get the location of the first token of this operand.
        SMLoc getStartLoc() const override { return StartLoc; }
        /// getEndLoc - Get the location of the last token of this operand.
        SMLoc getEndLoc() const override { return EndLoc; }

        void print(raw_ostream &OS) const override {
            switch (Kind) {
            case k_Immediate:
                OS << "Imm<";
                OS << *Imm.Val;
                OS << ">";
                break;
            case k_Memory:
                OS << "Mem<";
                OS << Mem.Base;
                OS << ", ";
                OS << *Mem.Off;
                OS << ">";
                break;
            case k_Register:
                OS << "Register<" << Reg.RegNum << ">";
                break;
            case k_Token:
                OS << Tok.Data;
                break;
            }
        }
    };
}

void printP2Operands(OperandVector &Operands) {
    for (size_t i = 0; i < Operands.size(); i++) {
        P2Operand* op = static_cast<P2Operand*>(&*Operands[i]);
        assert(op != nullptr);
        LLVM_DEBUG(dbgs() << " " << *op);
    }
    LLVM_DEBUG(dbgs() << "\n");
}

/*
implement virtual functions
*/
bool P2AsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode, OperandVector &Operands,
                                            MCStreamer &Out, uint64_t &ErrorInfo, bool MatchingInlineAsm) {

    LLVM_DEBUG(errs() << "match and emit instruction\n");
    printP2Operands(Operands);
    MCInst Inst;
    unsigned MatchResult = MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);

    switch (MatchResult) {
        default:
            break;
        case Match_Success: {
            Inst.setLoc(IDLoc);
            Inst.setFlags(Inst.getFlags() | P2::ALWAYS); // don't try to parse conditional instructions yet
            Inst.dump();
            Out.emitInstruction(Inst, getSTI());
            return false;
        }

        case Match_MissingFeature:
            Error(IDLoc, "instruction requires a CPU feature not currently enabled");
            return true;
        case Match_InvalidOperand: {
            SMLoc ErrorLoc = IDLoc;
            if (ErrorInfo != ~0U) {
                if (ErrorInfo >= Operands.size())
                    return Error(IDLoc, "too few operands for instruction");

                ErrorLoc = ((P2Operand &)*Operands[ErrorInfo]).getStartLoc();
                if (ErrorLoc == SMLoc()) ErrorLoc = IDLoc;
            }

            return Error(ErrorLoc, "invalid operand for instruction");
        }
        case Match_MnemonicFail:
            return Error(IDLoc, "invalid instruction");
    }

    return true;
}

OperandMatchResultTy P2AsmParser::tryParseRegister(unsigned &reg_no, SMLoc &start, SMLoc &end) {
    start = Parser.getTok().getLoc();
    reg_no = parseRegister("");
    end = Parser.getTok().getLoc();

    if (reg_no == (unsigned)-1)
        return MatchOperand_NoMatch;

    return MatchOperand_Success;
}

bool P2AsmParser::ParseRegister(unsigned &reg_no, SMLoc &start, SMLoc &end) {
    start = Parser.getTok().getLoc();
    reg_no = parseRegister("");
    end = Parser.getTok().getLoc();
    return (reg_no == (unsigned)-1);
}

bool P2AsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name, SMLoc NameLoc, OperandVector &Operands) {

    LLVM_DEBUG(errs() << "=== Parse Instruction ===\n");
    size_t start = 0, next = Name.find('\t');
    StringRef inst = Name.slice(start, next);
    StringRef condition;
    if (cond_strings.find(inst) != cond_strings.end()) {
        // we have a condition, save it, and we'll put it as the last operand
        // also read the next operand which is the actual instruction
        start = next;
        next = Name.find(' ');
        condition = inst;
        inst = Name.slice(start, next);
    }

    LLVM_DEBUG(errs() << "inst: " << inst << "\n");

    Operands.push_back(P2Operand::CreateToken(inst, NameLoc));

    // Read the operands.
    if (getLexer().isNot(AsmToken::EndOfStatement)) {
        // Read the first operand.
        if (parseOperand(Operands, Name)) {
            SMLoc Loc = getLexer().getLoc();
            Parser.eatToEndOfStatement();
            return Error(Loc, "unexpected token in argument list");
        }

        while (getLexer().is(AsmToken::Comma) ) {
            Parser.Lex();  // Eat the comma.

            // Parse and remember the operand.
            if (parseOperand(Operands, Name)) {
                SMLoc Loc = getLexer().getLoc();
                Parser.eatToEndOfStatement();
                return Error(Loc, "unexpected token in argument list");
            }
        }
    }

    if (getLexer().isNot(AsmToken::EndOfStatement)) {
        SMLoc Loc = getLexer().getLoc();
        Parser.eatToEndOfStatement();
        LLVM_DEBUG(errs() << "haven't found the end of the statement\n");
        return Error(Loc, "unexpected token in argument list");
    }

    Parser.Lex(); // Consume the EndOfStatement

    LLVM_DEBUG(errs() << "=== End Parse Instruction ===\n");
    return false;
}

bool P2AsmParser::ParseDirective(llvm::AsmToken DirectiveID) {
    llvm_unreachable("can't parse directives yet!");
    return true;
}

/*
support functions
*/
unsigned P2AsmParser::getReg(int RC,int RegNo) {
    return *(getContext().getRegisterInfo()->getRegClass(RC).begin() + RegNo);
}

int P2AsmParser::parseRegister(StringRef Mnemonic) {
    const AsmToken &Tok = Parser.getTok();
    int RegNum = -1;

    if (Tok.is(AsmToken::Identifier)) {
        std::string lowerCase = Tok.getString().lower();
        RegNum = matchRegisterName(lowerCase);
    } else if (Tok.is(AsmToken::Integer)) {
        RegNum = matchRegisterByNumber(static_cast<unsigned>(Tok.getIntVal()), Mnemonic.lower());
    } else {
        return RegNum;  //error
    }

    return RegNum;
}

int P2AsmParser::matchRegisterByNumber(unsigned RegNum, StringRef Mnemonic) {
    if (RegNum > 15)
        return -1;

    return getReg(P2::P2GPRRegClassID, RegNum);
}

int P2AsmParser::matchRegisterName(StringRef Name) {

    int CC = StringSwitch<unsigned>(Name)
            .Case("r0",     P2::R0)
            .Case("r1",     P2::R1)
            .Case("r2",     P2::R2)
            .Case("r3",     P2::R3)
            .Case("r4",     P2::R4)
            .Case("r5",     P2::R5)
            .Case("r6",     P2::R6)
            .Case("r7",     P2::R7)
            .Case("r8",     P2::R8)
            .Case("r9",     P2::R9)
            .Case("r10",    P2::R10)
            .Case("r11",    P2::R11)
            .Case("r12",    P2::R12)
            .Case("r13",    P2::R13)
            .Case("r14",    P2::R14)
            .Case("r15",    P2::R15)
            .Case("sp",     P2::SP)
            .Case("ptra",   P2::PTRA)
            .Case("ptrb",   P2::PTRB)
            .Case("dira",   P2::DIRA)
            .Case("dirb",   P2::DIRB)
            .Default(-1);

    return CC;
}

bool P2AsmParser::tryParseRegisterOperand(OperandVector &Operands, StringRef Mnemonic) {

    SMLoc S = Parser.getTok().getLoc();
    SMLoc E = Parser.getTok().getEndLoc();
    int RegNo = parseRegister(Mnemonic);

    if (RegNo == -1) {
        return true;
    }

    //Operands.push_back(P2Operand::CreateReg(RegNo, S, Parser.getTok().getLoc()));
    Operands.push_back(P2Operand::CreateReg(RegNo, S, E));
    Parser.Lex(); // Eat register token.
    return false;
}

bool P2AsmParser::parseOperand(OperandVector &Operands, StringRef Mnemonic) {

    LLVM_DEBUG(dbgs() << "Generic Operand Parser\n");

    AsmToken::TokenKind tok_kind = getLexer().getKind();

    LLVM_DEBUG(errs() << tok_kind << "\n");

    switch (tok_kind) {
        default:
            Error(Parser.getTok().getLoc(), "unexpected token in operand");
            return true;
        case AsmToken::Dollar: {
            // parse register
            SMLoc S = Parser.getTok().getLoc();
            Parser.Lex(); // Eat dollar token.

            // parse register operand
            if (!tryParseRegisterOperand(Operands, Mnemonic)) {
                return false; // success
            }

            // maybe it is a symbol reference
            StringRef Identifier;
            if (Parser.parseIdentifier(Identifier)) {
                return true; // fail
            }

            SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
            MCSymbol *Sym = getContext().getOrCreateSymbol("$" + Identifier);

            // Otherwise create a symbol ref.
            const MCExpr *Res = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None,getContext());
            Operands.push_back(P2Operand::CreateImm(Res, S, E));
            return false;
        }

        case AsmToken::Hash: {
            // is an immediate expression, so first create the token for the #
            SMLoc S = Parser.getTok().getLoc();
            Parser.Lex(); // eat the pound sign
            Operands.push_back(P2Operand::CreateToken("#", S));

            const MCExpr *IdVal;
            S = Parser.getTok().getLoc();
            if (getParser().parseExpression(IdVal))
                return true;

            SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
            Operands.push_back(P2Operand::CreateImm(IdVal, S, E));
            return false;
        }
    }
    return true;
}

extern "C" void LLVMInitializeP2AsmParser() {
    RegisterMCAsmParser<P2AsmParser> X(TheP2Target);
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "P2GenAsmMatcher.inc"