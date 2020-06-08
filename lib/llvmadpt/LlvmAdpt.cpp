//===- LlvmAdpt.cpp -- packaging for llvm IR functions--------------//
//
//                      
//
// Copyright (C) <2019-2024>  <Wen Li>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

#include "llvmadpt/LlvmAdpt.h"

InstAnalyzer::T_Value2InstSet InstAnalyzer::m_DefGlobal2Inst;

using namespace llvm;
using namespace std;

VOID llvmAdpt::GetIndirectCallee(const llvm::Instruction *Inst, std::vector<llvm::Function*>& FuncAry)
{
    if (!llvm::isa<llvm::CallInst>(Inst) && !llvm::isa<llvm::InvokeInst>(Inst))
    {
        return;
    }

    DWORD OpNum = Inst->getNumOperands ();
    assert (OpNum > 0);

    std::vector<llvm::Value*> PtsTo;
    llvmAdpt::GetPtsTo(Inst->getOperand (OpNum-1), PtsTo);
    if (PtsTo.size() == 0)
    {
        return;
    }

    Function* Callee = NULL;
    for (auto it = PtsTo.begin(); it != PtsTo.end(); it++)
    {
        llvm::Value *Val = *it;
        if (Val == NULL || !Val->hasName())
        {
            continue;
        }

        if (!llvm::isa<Function>(Val))
        {
            continue;
        }

        /* get the first one */
        FuncAry.push_back(cast<Function>(Val));
    }

    return;
}


llvm::Function* llvmAdpt::GetIndirectCallee(const llvm::Instruction *Inst) 
{
    if (!llvm::isa<llvm::CallInst>(Inst) && !llvm::isa<llvm::InvokeInst>(Inst))
    {
        return NULL;
    }

    DWORD OpNum = Inst->getNumOperands ();
    assert (OpNum > 0);

    std::vector<llvm::Value*> PtsTo;
    llvmAdpt::GetPtsTo(Inst->getOperand (OpNum-1), PtsTo);
    if (PtsTo.size() == 0)
    {
        return NULL;
    }

    Function* Callee = NULL;
    for (auto it = PtsTo.begin(); it != PtsTo.end(); it++)
    {
        llvm::Value *Val = *it;
        if (Val == NULL || !Val->hasName())
        {
            continue;
        }

        if (!llvm::isa<Function>(Val))
        {
            continue;
        }

        /* get the first one */
        Callee = cast<Function>(Val);
        break;
    }
        
    return llvmAdpt::GetFuncDef(Callee);
}


VOID InstAnalyzer::GetDefUseInfo()
{
    DefUse *Du;
    for (inst_iterator iIt = inst_begin(m_Function); iIt != inst_end(m_Function); ++iIt) 
    {
        Instruction *Inst = &(*iIt);

        if (llvmAdpt::IsInstrinsicDbgInst(Inst))
        {
            continue;
        }

        Du = new DefUse(Inst);
        assert (Du != NULL);

        SetFormalPara(Inst);
        ProcInst(Inst, Du);
        
        if (Du->HasDefUse())
        {
            m_InstToDuMap[Inst] = Du;
        }
        else
        {
            delete Du;
        }

    }

    return;
}

VOID  InstAnalyzer::GetPDefUseInfo(std::set<llvm::Value*> *ValueSet)
{
    DefUse *Du;
    
    m_ValueSet  = ValueSet;
        
    for (inst_iterator iIt = inst_begin(m_Function), iEnd = inst_end(m_Function); iIt != iEnd; ++iIt) 
    {
        Instruction *Inst = &(*iIt);

        if (llvmAdpt::IsInstrinsicDbgInst(Inst))
        {
            continue;
        }

        
        switch (Inst->getOpcode()) 
        {
            case Instruction::Load:
            {
                Du = GetDuByInst(Inst);
                assert (Du != NULL);

                llvm::Value *Puse = Inst->getOperand(0);
                SetPUse (Du, Puse);
                if (!IsActualPara (Puse))
                {
                    for (auto It = m_ElemSet.begin(); It != m_ElemSet.end(); It++)
                    {
                        if (It->Base != Puse)
                        {
                            continue;
                        }
                        Du->SetUse (It->DefVal, true);
                    }
                }
                
                break;
            }
            case Instruction::Store:
            {
                Du = GetDuByInst(Inst);
                assert (Du != NULL);

                SetPDef (Du, Inst->getOperand(1));
                continue;
            }
            case Instruction::Br:
            case Instruction::Ret:
            {
                continue;
            }
            default:
            {
                break;
            }
       }


       CollectDefOfInst(Inst);
    }


    return;
}



VOID InstAnalyzer::ProcInst (llvm::Instruction* Inst, DefUse* Du)
{
    DWORD ValId;
    
    switch (Inst->getOpcode()) 
    {
        case Instruction::Alloca: 
        {
            ProcAlloca (Inst, Du);
            break;
        }
        case Instruction::Call:
        case Instruction::Invoke: 
        {
            ProcCall (Inst, Du);
            break;
        }
        case Instruction::Ret: 
        {
            ProcRet (Inst, Du);            
            break;
        }
        case Instruction::Load: 
        {
            ProcLoad (Inst, Du);          
            break;
        }
        case Instruction::Store: 
        {
            ProcStore (Inst, Du);            
            break;
        }
        case Instruction::GetElementPtr: 
        {
            ProcGetElementPtr (Inst, Du);
            break;
        }
        case Instruction::PHI: 
        {
            ProcPHI (Inst, Du);                
            break;
        }
        case Instruction::Trunc:
        case Instruction::ZExt:
        case Instruction::SExt:
        case Instruction::FPToUI:
        case Instruction::FPToSI:              
        case Instruction::UIToFP:
        case Instruction::SIToFP:
        case Instruction::FPTrunc:
        case Instruction::FPExt:           
        case Instruction::PtrToInt:
        case Instruction::IntToPtr:
        case Instruction::BitCast:
        case Instruction::AddrSpaceCast:
        {
            ProcBitCast (Inst, Du);      
            break;
        }
        case Instruction::Select: 
        {
            ProcSelect (Inst, Du);            
            break;
        }
        case Instruction::VAArg: 
        {
            ProcVAArg (Inst, Du);              
            break;
        }
        case Instruction::ICmp:
        {
            ProcCmp (Inst, Du);
            break;
        }
        case Instruction::LShr:
        case Instruction::AShr:
        case Instruction::Shl:
        {
            ProcShOp (Inst, Du);
            break;
        }
        case Instruction::And:
        case Instruction::Or:
        case Instruction::Xor:
        case Instruction::Add:
        case Instruction::FAdd:
        case Instruction::Sub:
        case Instruction::FSub:
        case Instruction::Mul:
        case Instruction::FMul:
        case Instruction::SDiv:
        case Instruction::UDiv:
        case Instruction::URem:
        case Instruction::SRem:
        {
            ProcSBOp (Inst, Du);
            break;
        }
        case Instruction::Br:
        {
            ProcBr (Inst, Du);
            break;
        }
        default: 
        {
            break;
        }
    }
}


