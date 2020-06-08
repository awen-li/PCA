//===- Anderson.cpp -- Anderson algorithms for pointer analysis  ------------//
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
#include "analysis/points-to/Anderson.h"
#include "analysis/CycleDetect.h"

using namespace llvm;
using namespace std;

DWORD Anderson::GetConstValueNode(llvm::Constant *Const) 
{
    assert(llvmAdpt::IsValuePtrType (Const) && "Not a constant pointer!");

    if (isa<ConstantPointerNull>(Const) || isa<UndefValue>(Const))
    {
        return GetNullPtrNode();
    }
    else if (GlobalValue *GVal = dyn_cast<GlobalValue>(Const))
    {
        return GetValueNode(GVal);
    }
    else if (ConstantExpr *ConstExp = dyn_cast<ConstantExpr>(Const)) 
    {
        switch (ConstExp->getOpcode())
        {
            /* Pointer to any field within a struct is treated as a pointer to the first field */
            case Instruction::GetElementPtr:
            {
                return GetValueNode(Const->getOperand(0));
            }
            case Instruction::IntToPtr:
            case Instruction::PtrToInt:
            {
                return GetUniversalPtrNode();
            }
            case Instruction::BitCast:
            {
                return GetConstValueNode(ConstExp->getOperand(0));
            }
            default:
            {
                errs() << "Constant Expr not yet handled: " << *ConstExp << "\n";
                llvm_unreachable(0);
            }
        }
    }

    llvm_unreachable("Unknown constant pointer!");
    return 0;
}


DWORD Anderson::GetConstObjNode(llvm::Constant *Const) 
{
    assert(llvmAdpt::IsValuePtrType (Const) && "Not a constant pointer!");

    if (isa<ConstantPointerNull>(Const))
    {
        return GetNullObjNode();
    }
    else if (GlobalValue *Gv = dyn_cast<GlobalValue>(Const))
    {
        return GetObjNode(Gv);
    }
    else if (ConstantExpr *ConstExp = dyn_cast<ConstantExpr>(Const)) 
    {
        switch (ConstExp->getOpcode()) 
        {
            /* Pointer to any field within a struct is treated as a pointer to the first field */
            case Instruction::GetElementPtr:
            {
                return GetConstObjNode(ConstExp->getOperand(0));
            }
            case Instruction::IntToPtr:
            case Instruction::PtrToInt:
            {
                return GetUniversalObjNode();
            }
            case Instruction::BitCast:
            {
                return GetConstObjNode(ConstExp->getOperand(0));
            }
            default:
            {
                errs() << "Constant Expr not yet handled: " << *ConstExp << "\n";
                llvm_unreachable(0);
            }
        }
    }

    llvm_unreachable("Unknown constant pointer!");
    return 0;
}


VOID Anderson::AddGlbInitialCst(DWORD ObjId, Constant *Const) 
{
    if (Const->getType()->isSingleValueType()) 
    {
        if (llvmAdpt::IsValuePtrType (Const)) 
        {
            DWORD RshId = GetConstObjNode(Const);
            assert(RshId != 0);
            AddCosntraint(Constraint::E_ADDR_OF, ObjId, RshId);
        }
    } 
    else if (Const->isNullValue()) 
    {
        AddCosntraint(Constraint::E_COPY, ObjId, GetNullObjNode ());
    } 
    else if (!isa<UndefValue>(Const))
    {
        /* field-insensitive analysis, all objects in the array/struct are pointed-to by the 1st-field pointer*/
        assert(isa<ConstantArray>(Const) || isa<ConstantDataSequential>(Const) || isa<ConstantStruct>(Const));

        for (unsigned Index = 0, OpNum = Const->getNumOperands(); Index != OpNum; ++Index)
        {
            AddGlbInitialCst(ObjId, cast<Constant>(Const->getOperand(Index)));
        }
    }
    else
    {
        llvm_unreachable("Unknown constant pointer!");
    }
}

VOID Anderson::CollectCstOfGlobal()
{
    DWORD ValId, ObjId;
    GlobalVariable *Global;
    Function *Func;

    DWORD GlobStrNum = 0, Total = 0;
    for (auto It = m_ModMange.global_begin (); 
              It != m_ModMange.global_end (); It++) 
    {
        Global = *It;

        ValId = CreateValueNode(Global);
        if (IsGlobalStr(Global))
        {
            ObjId = CreateObjNode(Global, GetUniversalObjNode());
            GlobStrNum++;
        }
        else
        {
            ObjId = CreateObjNode(Global);
        }  

        Total++;
        AddCosntraint(Constraint::E_ADDR_OF, ValId, ObjId);
    }

    DEBUG ("GlobStrNum = [%u/%u] \r\n", GlobStrNum, Total);

    for (auto It = m_ModMange.func_begin (); 
              It != m_ModMange.func_end (); It++)
    {
        Func  = *It;
        
        ValId = CreateValueNode(Func);
        ObjId = CreateObjNode(Func);
        AddCosntraint(Constraint::E_ADDR_OF, ValId, ObjId);

        if (Func->isDeclaration() || Func->isIntrinsic())
        {
            continue;
        }

        /* return a pointer */
        if (llvmAdpt::IsFunctionPtrType (Func)) 
        {
            ValId = CreateRetNode(Func);
        }

        /* var arg function */
        if (llvmAdpt::IsFunctionVarArg (Func))
        {
            ValId = CreateVarArgNode(Func);
        }

        /* Add nodes for all formal arguments.*/
        for (Function::arg_iterator itr = Func->arg_begin(); itr != Func->arg_end(); ++itr) 
        {
            if (llvmAdpt::IsValuePtrType (&*itr))
            {
                ValId = CreateValueNode(&*itr);
            }
        }
    }

    // Init globals here since an initializer may refer to a global var/func below
    for (auto It = m_ModMange.global_begin (); 
              It != m_ModMange.global_end (); It++) 
    {
        Global = *It;

        DWORD ObjId = GetObjNode(Global);
        assert(ObjId != 0);

        if (Global->hasDefinitiveInitializer()) 
        {
            AddGlbInitialCst(ObjId, Global->getInitializer());
        } 
        else 
        {
            /* If it doesn't have an initializer (i.e. it's defined in another
               translation unit), it points to the universal set. */
            AddCosntraint(Constraint::E_COPY, ObjId, GetUniversalObjNode());
        }
    }
}

BOOL Anderson::AddExtLibCst(ImmutableCallSite Cs, const Function *Func)
{
    m_ExtLib->CacheExtType (Func);
    
    if (!m_ExtLib->IsDealWithPts ())
    {
        return AF_TRUE;
    }

    if (m_ExtLib->IsMalloc () ||
        (m_ExtLib->IsRealloc () && !isa<ConstantPointerNull>(Cs.getArgument(0))))
    {
        Instruction *Inst = (Instruction *)Cs.getInstruction(); 
        DWORD ValId = GetValueNode (Inst);
        if (ValId != 0)
        {
            DWORD ObjId = CreateObjNode(Inst);
            AddCosntraint(Constraint::E_ADDR_OF, ValId, ObjId);
            //errs()<<"Malloc: "<<ObjId<<"->"<<ValId<<"\r\n";
        }

        return AF_TRUE;
    }

    if (m_ExtLib->IsRetArg0 ()) 
    {
        DWORD RetId = GetValueNode((Instruction *)Cs.getInstruction());
        if (RetId != 0) 
        {
            DWORD Arg0 = GetValueNode((Value *)Cs.getArgument(0));
            assert(Arg0 != 0);
            AddCosntraint(Constraint::E_COPY, RetId, Arg0);
        }

        return AF_TRUE;
   }

    if (m_ExtLib->IsMemcpy ()) 
    {
        DWORD Arg0 = GetValueNode((Value *)Cs.getArgument(0));
        assert(Arg0 != 0);
        
        DWORD Arg1 = GetValueNode((Value *)Cs.getArgument(1));
        assert(Arg1 != 0);

        DWORD TmpId = CreateValueNode();
        AddCosntraint(Constraint::E_LOAD, TmpId, Arg1);
        AddCosntraint(Constraint::E_STORE, Arg0, TmpId);
        
        DWORD RetId = CreateValueNode((Instruction *)Cs.getInstruction());
        AddCosntraint(Constraint::E_COPY, RetId, Arg0);

        return AF_TRUE;
    }

    if (m_ExtLib->IsCast ())
    {
        if (!isa<ConstantPointerNull>(Cs.getArgument(1))) 
        {
          DWORD Arg0 = GetValueNode((Value *)Cs.getArgument(0));
          assert(Arg0 != 0);
          
          DWORD Arg1 = GetValueNode((Value *)Cs.getArgument(1));
          assert(Arg1 != 0);

          AddCosntraint(Constraint::E_STORE, Arg0, Arg1);
        }

        return AF_TRUE;
    }
    
    return AF_FALSE;
}

VOID Anderson::AddFuncArgCst(ImmutableCallSite Cs, Function *Func)
{
    Function::arg_iterator fItr = Func->arg_begin();
    ImmutableCallSite::arg_iterator aItr = Cs.arg_begin();
    
    for (; fItr != Func->arg_end() && aItr != Cs.arg_end(); ++fItr, ++aItr) 
    {
        Argument *Formal = &*fItr;
        Value *Actual = *aItr;

        if (!Formal->getType()->isPointerTy()) 
        {
            continue;
        }
        
        DWORD FormalId = GetValueNode(Formal);
        assert(FormalId != 0);
        if (Actual->getType()->isPointerTy()) 
        {
            DWORD ActualId = GetValueNode(Actual);
            if (ActualId == 0)
            {
                ActualId = CreateValueNode (Actual);
                assert(ActualId != 0);
            }

            AddCosntraint(Constraint::E_COPY, FormalId, ActualId);
        } 
        else
        {
            AddCosntraint(Constraint::E_COPY, FormalId, GetUniversalPtrNode ());
        }
    } 

    // Copy all pointers passed through the varargs section to the varargs node
    if (Func->getFunctionType()->isVarArg()) 
    {
        for (; aItr != Cs.arg_end(); aItr++) 
        {
            Value *Actual = *aItr;
            if (!Actual->getType()->isPointerTy()) 
            {
                continue;
            }
            
            DWORD ActualId = GetValueNode(Actual);
            assert(ActualId != 0);
            
            DWORD ValId = GetVarArgNode(Func);
            assert(ValId != 0);
            AddCosntraint(Constraint::E_COPY, ValId, ActualId);
        }
    }
}


/*
 - ValueNode(callsite) = ReturnNode(call target)
 - ValueNode(formal arg) = ValueNode(actual arg)
*/
VOID Anderson::AddCallSiteCst(ImmutableCallSite Cs) 
{
    Function *Func = (Function*)Cs.getCalledFunction();
    if (Func != NULL)
    {
        if (Func->isDeclaration())
        {
            Function *DefFunc = llvmAdpt::GetFuncDef (Func);
            if (DefFunc != Func)
            {
                Func = DefFunc;
            }
        }

        if (IsDebugFunction(Func->getName()))
        {
            return;
        }
        
        /* External library call */
        if (Func->isDeclaration() || Func->isIntrinsic())
        {
            if (AddExtLibCst(Cs, Func))
            {
                return;
            }

            SetUnsolvedFunc (Func);
          
            if (Cs.getType()->isPointerTy()) 
            {
                DWORD RetId = GetValueNode((Value*)Cs.getInstruction());
                assert(RetId != 0);

                AddCosntraint(Constraint::E_COPY, RetId, GetUniversalPtrNode());
            }
            
            for (ImmutableCallSite::arg_iterator itr = Cs.arg_begin(), ite = Cs.arg_end(); itr != ite; ++itr) 
            {
                Value *ArgVal = *itr;
                if (llvmAdpt::IsValuePtrType (ArgVal)) 
                {
                    DWORD ArdId = GetValueNode(ArgVal);
                    if (ArdId == 0)
                    {
                        ArdId = CreateValueNode (ArgVal);
                        assert(ArdId != 0);
                    }
                    
                    AddCosntraint(Constraint::E_COPY, ArdId, GetUniversalPtrNode());
                }
            }
        } 
        else
        {
            const Function *Caller = Cs.getInstruction()->getParent ()->getParent ();
            if (Caller == Func)
            {
                return;
            }
            
            /*  Non-external function call */
            if (Cs.getType ()->isPointerTy()) 
            {
                DWORD RetId = GetValueNode((Value*)Cs.getInstruction());
                assert(RetId != 0);

                DWORD FuncRetId = GetRetNode(Func);
                assert(FuncRetId != 0);
                AddCosntraint(Constraint::E_COPY, RetId, FuncRetId);
            }
            
            /* The argument constraints */
            AddFuncArgCst(Cs, Func);
        }
    }
    else
    {
        /* For Indirect call, We do the simplest thing here: just assume the returned value can be anything */
        if (Cs.getType ()->isPointerTy()) 
        {
            DWORD RetId = GetValueNode((Value *)Cs.getInstruction());
            assert(RetId != 0);
            
            AddCosntraint(Constraint::E_COPY, RetId, GetUniversalPtrNode());
        }
    }
}


VOID Anderson::CollectAlloca(Instruction *Inst)
{
    DWORD ValId = GetValueNode(Inst);
    assert(ValId != 0);
            
    DWORD ObjId = CreateObjNode(Inst);
    assert(ObjId != 0);
    
    AddCosntraint(Constraint::E_ADDR_OF, ValId, ObjId);

    return;
}

VOID Anderson::CollectCall(Instruction *Inst)
{
    if(llvmAdpt::IsInstrinsicDbgInst(Inst) == true)
    {
        return;
    }
    
    ImmutableCallSite Cs(Inst);

    //errs()<<"------------->"<<*Inst<<"\r\n";
    AddCallSiteCst(Cs);

    if (Cs.isIndirectCall ())
    {
        AddFuncPointer (Inst);
    }

    return;
}

VOID Anderson::CollectRet(Instruction *Inst)
{
    if (!Inst->getNumOperands() ||  !llvmAdpt::IsValuePtrType (Inst->getOperand(0))) 
    {
        return;
    }

    DWORD RetId = GetRetNode(Inst->getParent()->getParent());
    assert(RetId != 0);
                
    DWORD ValId = GetValueNode(Inst->getOperand(0));
    if (ValId == 0)
    {
        ValId = CreateValueNode (Inst->getOperand(0));
        assert(ValId != 0);
    }
       
    AddCosntraint(Constraint::E_COPY, RetId, ValId);

    return;
}

VOID Anderson::CollectLoad(Instruction *Inst)
{
    if (!llvmAdpt::IsValuePtrType (Inst)) 
    {
        return;
    }

    DWORD OpId = GetValueNode(Inst->getOperand(0));
    assert(OpId != 0);

    DWORD ValId = GetValueNode(Inst);
    assert(ValId != 0);

    AddCosntraint(Constraint::E_LOAD, ValId, OpId);

    return;
}

VOID Anderson::CollectStore(Instruction *Inst)
{
    if (!llvmAdpt::IsValuePtrType (Inst->getOperand(0)))
    {
        return;
    }

    DWORD Src = GetValueNode(Inst->getOperand(0));
    if (Src == 0)
    {
        Src = CreateValueNode (Inst->getOperand(0));
        assert(Src != 0);
                    
    }
                
    DWORD Dst = GetValueNode(Inst->getOperand(1));
    assert(Dst != 0);
                
    AddCosntraint(Constraint::E_STORE, Dst, Src);

    return;
}


VOID Anderson::CollectGetElementPtr(Instruction *Inst)
{
    assert(llvmAdpt::IsValuePtrType (Inst));

    llvm::GetElementPtrInst *GepInst = cast<llvm::GetElementPtrInst>(Inst);
    if(llvm::isa<llvm::VectorType>(GepInst->getType()))
    {         
        DWORD Dst = GetValueNode(Inst);
        assert(Dst != 0);

        AddCosntraint(Constraint::E_COPY, Dst, GetNullPtrNode());
    }
    else
    {
        //llvm::errs()<<" OpNum = " <<Inst->getNumOperands ()<<"\r\n"\
        //            <<"\t Op0 = "<<*(Inst->getOperand(0))\
        //            <<", Op0-Type "<<*(Inst->getOperand(0)->getType ())<<"\r\n";
        //for (int OpIndex = 1; OpIndex < Inst->getNumOperands (); OpIndex++)
        //{
        //    llvm::errs()<<"\t Op"<<OpIndex<<" = "<<*(Inst->getOperand(OpIndex))\
        //                <<", Op"<<OpIndex<<"-Type "<<*(Inst->getOperand(OpIndex)->getType ())\
        //                <<"\r\n";
        //}
            
        

        /* P1 = getelementptr P2, ... --> <Copy/P1/P2> */
        DWORD Src = GetValueNode(Inst->getOperand(0));
        assert(Src != 0);
                
        DWORD Dst = GetValueNode(Inst);
        assert(Dst != 0);

        AddCosntraint(Constraint::E_COPY, Dst, Src);
    } 

    return;
}

VOID Anderson::CollectPHI(Instruction *Inst)
{
    if (!llvmAdpt::IsValuePtrType (Inst)) 
    {
        return;
    }

    PHINode *PhiInst = cast<PHINode>(Inst);
    DWORD Dst = GetValueNode(Inst);
    assert(Dst != 0);
                
    for (DWORD Index = 0, e = PhiInst->getNumIncomingValues(); Index != e; ++Index) 
    {
        DWORD Src = GetValueNode(PhiInst->getIncomingValue(Index));
        //assert(Src != 0);
                    
        AddCosntraint(Constraint::E_COPY, Dst, Src);
    }

    return;
}

VOID Anderson::CollectBitCast(Instruction *Inst)
{
    if (!llvmAdpt::IsValuePtrType (Inst)) 
    {
        return;
    }

    DWORD Src = GetValueNode(Inst->getOperand(0));
    assert(Src != 0);
                
    DWORD Dst = GetValueNode(Inst);
    assert(Dst != 0);

    //llvm::errs()<<*Inst<<" <- copy "<<*(Inst->getOperand(0))<<"\r\n";
    AddCosntraint(Constraint::E_COPY, Dst, Src);

    return;
}


VOID Anderson::CollectSelect(Instruction *Inst)
{
    if (!llvmAdpt::IsValuePtrType (Inst))
    {
        return;
    }

    DWORD Src1 = GetValueNode(Inst->getOperand(1));
    assert(Src1 != 0);
                  
    DWORD Src2 = GetValueNode(Inst->getOperand(2));
    assert(Src2 != 0);
                  
    DWORD Dst = GetValueNode(Inst);
    assert(Dst != 0);
                  
    AddCosntraint(Constraint::E_COPY, Dst, Src1);
    AddCosntraint(Constraint::E_COPY, Dst, Src2);

    return;
}

VOID Anderson::CollectVAArg(Instruction *Inst)
{
    if (llvmAdpt::IsValuePtrType (Inst)) 
    {
        return;
    }

    DWORD Dst = GetValueNode(Inst);
    assert(Dst != 0);
                
    DWORD Src = GetValueNode(Inst->getParent()->getParent());
    assert(Src != 0);
                
    AddCosntraint(Constraint::E_COPY, Dst, Src);

    return;
}

VOID Anderson::CollectInstExtractValue(Instruction *Inst)
{
    /* %4 = inttoptr i32 %3 to i8* */
    if (!llvmAdpt::IsValuePtrType (Inst)) 
    {
        return;
    }

    DWORD Dst = GetValueNode(Inst);
    assert(Dst != 0);
                
    AddCosntraint(Constraint::E_COPY, Dst, GetNullPtrNode());

    return;
}

VOID Anderson::CollectIntToPtr(Instruction *Inst)
{
    /* %4 = inttoptr i32 %3 to i8* */
    if (!llvmAdpt::IsValuePtrType (Inst)) 
    {
        return;
    }

    DWORD Dst = GetValueNode(Inst);
    assert(Dst != 0);
                
    DWORD ObjId = CreateObjNode(Inst->getOperand (0));
    assert(ObjId != 0);
    
    AddCosntraint(Constraint::E_ADDR_OF, Dst, ObjId);

    return;
}


VOID Anderson::CollectCstOfInst(Instruction *Inst) 
{       
    switch (Inst->getOpcode()) 
    {
        case Instruction::Alloca: 
        {
            CollectAlloca (Inst);
            break;
        }
        case Instruction::Call:
        case Instruction::Invoke: 
        {
            CollectCall (Inst);
            break;
        }
        case Instruction::Ret: 
        {
            CollectRet (Inst);            
             break;
        }
        case Instruction::Load: 
        {
            CollectLoad (Inst);          
            break;
        }
        case Instruction::Store: 
        {
            CollectStore (Inst);            
            break;
        }
        case Instruction::GetElementPtr: 
        {
            CollectGetElementPtr (Inst);
            break;
        }
        case Instruction::PHI: 
        {
            CollectPHI (Inst);                
            break;
        }
        case Instruction::BitCast:
        {
            CollectBitCast (Inst);      
            break;
        }
        case Instruction::Select: 
        {
            CollectSelect (Inst);            
            break;
        }
        case Instruction::VAArg: 
        {
            CollectVAArg (Inst);              
            break;
        }
        case Instruction::ExtractValue:
        {
            /* %asmresult67 = extractvalue { i64, i64* } %43, 1, */
            CollectInstExtractValue (Inst);
            break;
        }
        case Instruction::IntToPtr:
        {
            CollectIntToPtr(Inst);
            break;
        }
        default: 
        {
            if (llvmAdpt::IsValuePtrType (Inst)) 
            {
                errs ()<<"OpCode = "<<Inst->getOpcode()<<"---> ";
                errs() << *Inst << "pointer-related inst not handled!\n";
            }
            break;
        }
    }
}


VOID Anderson::CollectConstraints() 
{
    Function *Func;
    Instruction *Inst;
    
    /* 1. the universal set points to itself */
    AddCosntraint(Constraint::E_ADDR_OF, UniversalPtr, UniversalObj);
    AddCosntraint(Constraint::E_STORE, UniversalObj, UniversalObj);

    /* 2. the null pointer points to the null object */
    AddCosntraint(Constraint::E_ADDR_OF, NullPtr, NullObject);

    /* 3. add any constraints on global variables and their initializers. */
    CollectCstOfGlobal ();

    /* 4. collect function internal instruction objects */
    for (auto It = m_ModMange.func_begin (), End = m_ModMange.func_end (); 
              It != End; It++)
    {
        Func  = *It;
       
        if (Func->isDeclaration() || Func->isIntrinsic() || IsDebugFunction (Func->getName()))
        {
            continue;
        }

        /* First, create a value node for each instruction with pointer type */
        for (inst_iterator itr = inst_begin(*Func), ite = inst_end(*Func); itr != ite; ++itr) 
        {
            Inst = &*itr.getInstructionIterator();
            if (llvmAdpt::IsValuePtrType (Inst))
            {
                CreateValueNode(Inst);
            }
        }

        //errs()<<"Collect Func: "<<Func->getName()<<"\r\n";
        /* Sec, collect constraint for each relevant instruction */
        for (inst_iterator itr = inst_begin(*Func), ite = inst_end(*Func); itr != ite; ++itr) 
        {
            Inst = &*itr.getInstructionIterator();
            CollectCstOfInst(Inst);
        }
    }

    m_ObjectNodes.clear();
    m_ReturnNodes.clear();
    m_VarargNodes.clear();
    m_UnsolvedFunc.clear();
    return;
}

VOID Anderson::InitCstGraph() 
{
    for (Constraint &Cst : m_Constraints) 
    {
        DWORD SrcTgt = m_CstGraph->GetMergeTarget(Cst.GetSrc());
        DWORD DstTgt = m_CstGraph->GetMergeTarget(Cst.GetDst());
        
        switch (Cst.GetType()) 
        {
            case Constraint::E_ADDR_OF: 
            {
                /* simple constraint relation */
                ConstraintNode *CstNode = m_CstGraph->GetGNode (DstTgt);
                CstNode->SetPointsTo(Cst.GetSrc());

                m_CstGraph->AddAddrCstEdge (SrcTgt, DstTgt);
                //errs()<<"add Addr edge: ("<<SrcTgt<<","<<DstTgt<<")\r\n";

                /* add to the worklist */
                m_WorkList->InQueue (DstTgt);
                break;
            }
            case Constraint::E_LOAD:
            {
                m_CstGraph->AddLoadCstEdge (SrcTgt, DstTgt);
                //errs()<<"add Load edge: ("<<SrcTgt<<","<<DstTgt<<")\r\n";
                break;
            }
            case Constraint::E_STORE:
            {
                m_CstGraph->AddStoreCstEdge (SrcTgt, DstTgt);
                //errs()<<"add Store edge: ("<<SrcTgt<<","<<DstTgt<<")\r\n";
                break;
            }
            case Constraint::E_COPY: 
            {
                m_CstGraph->AddCopyCstEdge (SrcTgt, DstTgt);
                //errs()<<"add Copy edge: ("<<SrcTgt<<","<<DstTgt<<")\r\n";
                break;
            }
            default:
            {
                assert (0 && "No support type!!!");
            }
        }
    }

    m_Constraints.clear();
}


DWORD Anderson::SolveConstraints() 
{
	DWORD PrintNum = 0;
    OfflineCycleDetector OffCycleDt (m_CstGraph);
    OnlineCycleDetector  OnCycleDt (m_CstGraph);
    
    /* 1. init constraints graph */
    InitCstGraph ();
    
    /* 2. offline cycle detect */
    OffCycleDt.RunDectect ();

    /* 3. constraint solve */
    //printf("m_WorkList: %-8d, (V,E)=(%-8d, %-8d)\r\n", m_WorkList->Size (), m_CstGraph->GetNodeNum (), m_CstGraph->GetEdgeNum ());  
    while (!m_WorkList->IsEmpty ())
    {
		PrintNum++;
		if (!(PrintNum%10000))
		{
			printf("m_WorkList: %-8d, (V,E)=(%-8d, %-8d)\r", m_WorkList->Size (), m_CstGraph->GetNodeNum (), m_CstGraph->GetEdgeNum ());
		}
        
        DWORD NodeId = m_WorkList->OutQueue ();
        NodeId = m_CstGraph->GetMergeTarget (NodeId);

        ConstraintNode *CstNode = m_CstGraph->GetGNode(NodeId);
        if (CstNode == NULL)
        {
            continue;
        }

        /* check the Pts-To set */
        PtsSet *NodePtsSet = CstNode->GetPtsSet();
        //for (auto PtsTo : *NodePtsSet)
        for (auto PtsTo = NodePtsSet->begin (), End = NodePtsSet->end (); PtsTo != End; PtsTo++)
        {
            DWORD PtId = m_CstGraph->GetMergeTarget(*PtsTo);

            /* out going load edges */
            //for (auto OIt : CstNode->OutLoadEdge ())
            for (auto OIt = CstNode->LoadEgBegin (), Oend = CstNode->LoadEgEnd (); OIt != Oend; OIt++)
            {
                ConstraintEdge *CstEdge = *OIt;
                
                if (m_CstGraph->ProcessLoad (PtId, CstEdge))
                {
                    m_WorkList->InQueue (PtId);
                }
            }
            

            /* In coming stote edges */
            //for (auto IIt : CstNode->InStoreEdge ())
            for (auto IIt = CstNode->StoreEgBegin (), Iend = CstNode->StoreEgEnd (); IIt != Iend; IIt++)
            {
                ConstraintEdge *CstEdge = *IIt;

                if (m_CstGraph->ProcessStore (PtId, CstEdge))
                {
                    m_WorkList->InQueue (CstEdge->GetSrcID ());
                }          
            }       
        }

        /* now, propagate the points-to set */
        //for (auto OIt : CstNode->OutCpoyEdge ())
        for (auto OIt = CstNode->CopyEgBegin (), Oend = CstNode->CopyEgEnd (); OIt != Oend; OIt++)
        {
            ConstraintEdge *CstEdge = *OIt;

            if (m_CstGraph->ProcessCopy(NodeId, CstEdge))
            {
                m_WorkList->InQueue (CstEdge->GetDstID ());
            }
            else
            {
                OnCycleDt.SetCandiate(NodeId, CstEdge->GetDstID ());
            }
        }

        /* run the online scc detect */
        OnCycleDt.RunDectect ();    
    }


    printf("m_WorkList: %-8d, (V,E)=(%-8d, %-8d)\r\n", m_WorkList->Size (), m_CstGraph->GetNodeNum(), m_CstGraph->GetEdgeNum ());

    return AF_SUCCESS;    
}


DWORD Anderson::RunPtsAnalysis ()
{
    printf("---> start points-to analysis...\r\n");
    /* 1. compute constraints */
    CollectConstraints();
    
    /* 2. solve constraints */
    SolveConstraints();

    //UpdatePointsTo ();
    
    //m_CstGraph->StatPtsSize ();
    
    ClearMem();
    Stat::GetStatNum ("MergeNodes");
    printf("---> finish points-to analysis...\r\n");
    
    return AF_SUCCESS;
}

VOID Anderson::GetPtsTo (Value *Src, std::vector<Value*>& Dst)
{
    DWORD ValId = GetValueNode (Src);
    
    DWORD TargetId = m_CstGraph->GetMergeTarget2 (ValId);
    if (TargetId != 0)
    {
        ConstraintNode *CstNode = m_CstGraph->GetGNode (TargetId);
        assert (CstNode != NULL);

        PtsSet * Pst = CstNode->GetPtsSet ();
        for (auto It = Pst->begin (), End = Pst->end (); It != End; It++)
        {
            ConstraintNode *PtsNode = m_CstGraph->GetGNode (*It);

            Dst.push_back(PtsNode->GetValue ());
        }
    }

    return;
}

VOID Anderson::ClearMem()
{
    if (m_WorkList != NULL)
    {
        delete m_WorkList;
        m_WorkList = NULL;
    }

    if (m_ExtLib != NULL)
    {
        delete m_ExtLib;
        m_ExtLib = NULL;
    }

    m_CstGraph->ClearMem ();

    m_FuncPointer.clear();

    return;
}


VOID Anderson::AddFuncPointer(Instruction *CallInst)
{
    DWORD Index = 0;
    DWORD OpNum = CallInst->getNumOperands ();

    Value *FuncPoiner = CallInst->getOperand (OpNum-1);
    assert (FuncPoiner != NULL);
        
    m_FuncPointer.insert(FuncPoiner);

    return;
}

VOID Anderson::UpdatePointsTo ()
{
    for (auto It = m_CstGraph->begin (), End = m_CstGraph->end(); It != End; It++)
    {
        ConstraintNode *Node = It->second;

        Value* Val = Node->GetValue ();
        PtsSet* Pts = Node->GetPtsSet ();
        if (Pts->GetSize () == 0)
        {
            continue;
        }

        PtsSet Set;
        if (IsFuncPointer (Val))
        {      
            for (auto ts = Pts->begin (), tsend = Pts->end (); ts != tsend; ts++)
            {
                DWORD Id = *ts;
                ConstraintNode *PtNode = m_CstGraph->GetGNode (Id);
                
                Value *PtVal = PtNode->GetValue ();
                if (PtVal == NULL || llvm::isa<llvm::Function>(PtVal))
                {
                    continue;
                }
                
                Set.Insert (Id);          
            } 
        }
        
        if (Set.GetSize ())
        {
            Pts->Sub (Set);
        }       
    }

    return;
}


