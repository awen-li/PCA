//===- llvmadpt/LlvmAdpt.h -- packaging for llvm IR functions----------------------------//
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

#ifndef _LLVMADPT_H_
#define _LLVMADPT_H_

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>	 
#include <llvm/IR/CallSite.h>
#include "llvm/IR/Intrinsics.h"
#include <llvm/Support/Debug.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/InstIterator.h>
#include "llvmadpt/ModuleSet.h"
#include "llvmadpt/ModuleSet.h"
#include "analysis/points-to/PointsTo.h"


namespace llvmAdpt 
{

inline bool IsCallSite(const llvm::Instruction* inst) 
{
    return (llvm::isa<llvm::CallInst>(inst) || llvm::isa<llvm::InvokeInst>(inst));
}

inline bool IsReturn(const llvm::Instruction* inst) 
{
    return llvm::isa<llvm::ReturnInst>(inst);
}

inline const llvm::Function* GetLLVMFunction(const llvm::Value* val) 
{
    const llvm::Function *fun = llvm::dyn_cast<llvm::Function>(val->stripPointerCasts());
    return fun;
}

inline llvm::CallSite GetLLVMCallSite(const llvm::Instruction* inst) 
{
    assert(llvm::isa<llvm::CallInst>(inst)|| llvm::isa<llvm::InvokeInst>(inst));
    
    llvm::CallSite cs(const_cast<llvm::Instruction*>(inst));
    return cs;
}

inline llvm::Function* GetFuncDef(llvm::Function* Func)
{
    llvm::Function* DeclFunc;
    
	if(Func == NULL)
    {
        return NULL;
	}
    
    ModuleManage ModMng;
    if (Func->isDeclaration() && (DeclFunc = ModMng.GetDefinition(Func)))
    {
        Func = DeclFunc;
    }

    return Func;
}

inline DWORD GetBasicBlockNum(llvm::Function* Func)
{
    DWORD BBNum = 0;
    
    for (auto it = Func->begin (), end = Func->end(); it != end; it++)
    {
        BBNum++;
    }

    return BBNum;
}


inline llvm::Function* GetCallee(const llvm::CallSite Cs) 
{
    llvm::Function *Callee = llvm::dyn_cast<llvm::Function>(Cs.getCalledValue()->stripPointerCasts());
    
    return GetFuncDef(Callee);
}

inline llvm::Function* GetCallee(const llvm::Instruction *Inst) 
{
    if (!llvm::isa<llvm::CallInst>(Inst) && !llvm::isa<llvm::InvokeInst>(Inst))
    {
        return NULL;
    }
    
    llvm::CallSite Cs(const_cast<llvm::Instruction*>(Inst));
    
    return GetCallee(Cs);
}

inline BOOL IsIRFile(const std::string &BcFilePath) 
{
    
    return AF_FALSE;
}


inline bool IsInstrinsicDbgInst(const llvm::Instruction* inst) 
{
    return llvm::isa<llvm::DbgInfoIntrinsic>(inst);
}

inline bool IsBranchInst(const llvm::Instruction* inst) 
{
    return (llvm::isa<llvm::BranchInst>(inst) && (inst->getNumOperands () > 1));
}

inline bool IsCmpInst(const llvm::Instruction* inst) 
{
    return (llvm::isa<llvm::ICmpInst>(inst));
}


inline bool IsStoreInst(const llvm::Instruction* inst) 
{
    return (llvm::isa<llvm::StoreInst>(inst));
}

inline bool IsGepInst(const llvm::Instruction* inst) 
{
    return (llvm::isa<llvm::GetElementPtrInst>(inst));
}


inline bool IsValuePtrType(llvm::Value* Val) 
{
    return llvm::isa<llvm::PointerType>(Val->getType());
}

inline bool IsFunctionPtrType (llvm::Function *Func)
{
    return llvm::isa<llvm::PointerType>(Func->getFunctionType()->getReturnType());
}

inline bool IsFunctionVoidType (llvm::Function *Func)
{
    return Func->getReturnType()->isVoidTy();
}


inline bool IsFunctionVarArg (llvm::Function *Func)
{
    return Func->getFunctionType()->isVarArg();
}

inline VOID GetPtsTo (llvm::Value* Src, std::vector<llvm::Value*>& Dst)
{
    ModuleManage ModMng;
    PointsTo PtsTo (ModMng, T_ANDRESEN);

    return PtsTo.GetPtsTo (Src, Dst);
}

inline bool IsDebugFunction (std::string FuncName)
{
    ModuleManage ModMng;
    PointsTo PtsTo (ModMng, T_ANDRESEN);

    return PtsTo.IsDebugFunction(FuncName);
}

inline std::string GetSourceLoc(llvm::Instruction* inst) 
{
    std::string str;
    raw_string_ostream rawstr(str);

    MDNode *MdNode = inst->getMetadata("dbg");
    if (MdNode == NULL)
    {
        return "";
    }
    
    llvm::DILocation* Loc = llvm::cast<llvm::DILocation>(MdNode);
    unsigned Line = Loc->getLine();
    StringRef File = Loc->getFilename();
    rawstr << "line: " << Line << " file: " << File;

    return rawstr.str();
}

inline llvm::Value* GetElemBaseValue (llvm::Value *Val)
{
    if (!llvm::isa<llvm::LoadInst>(Val))
    {
        return Val;
    }

    llvm::Instruction *Inst = (llvm::Instruction *)Val;

    return Inst->getOperand (0);
}


llvm::Function* GetIndirectCallee(const llvm::Instruction *Inst);

VOID GetIndirectCallee(const llvm::Instruction *Inst, std::vector<llvm::Function*>& FuncAry);


}

class DefUse 
{
private:
    std::set<llvm::Value*> m_DefVal;
    llvm::Value* m_DDef;
    std::map<llvm::Value*, DWORD> m_PDefFlg;
    
    std::set<llvm::Value*> m_UseVal;
    std::map<llvm::Value*, DWORD> m_PUseFlg;
    
    llvm::Instruction* m_Inst;

public:
    DefUse (llvm::Instruction *Inst)
    {
        m_Inst = Inst;
        m_DDef = NULL;
    }

    ~DefUse ()
    {
    }

    inline DWORD GetSize ()
    {
        DWORD Size = 8;

        Size += m_DefVal.size() * (8+24);
        Size += m_PDefFlg.size() * (12+8);

        Size += m_UseVal.size() * (8+24);
        Size += m_PUseFlg.size() * (12+8);

        printf ("m_DefVal[%lu]  m_PDefFlg[%lu]  m_UseVal[%lu]  m_PUseFlg[%lu]  ", 
                m_DefVal.size(), m_PDefFlg.size(), m_UseVal.size(), m_PUseFlg.size());
        return Size;
    }

    inline VOID SetInst(llvm::Instruction* Inst)
    {
        m_Inst = Inst;
    }

    inline VOID SetDef(llvm::Value* Def)
    {
        m_DefVal.insert(Def);

        assert (m_DDef == NULL);
        m_DDef = Def;
    }

    inline VOID SetDef(llvm::Value* Def, DWORD IsPDef)
    {
        m_DefVal.insert(Def);

        if (IsPDef == AF_TRUE)
        {
            m_PDefFlg[Def] = AF_TRUE;
        }
        else
        {
            assert (m_DDef == NULL);
            m_DDef = Def;
        }

        return;
    }

    inline llvm::Value* GetDDef()
    {
        return m_DDef;
    }

    inline BOOL IsPDef (llvm::Value* Def)
    {
        std::map<llvm::Value*, DWORD>::iterator It = m_PDefFlg.find (Def);
        if (It == m_PDefFlg.end())
        {
            return AF_FALSE;
        }

        return AF_TRUE;
    }
    
    inline VOID SetUse(llvm::Value* Use)
    {
        m_UseVal.insert(Use);
    }

    inline VOID SetUse(llvm::Value* Use, DWORD IsPUse)
    {
        m_UseVal.insert(Use);

        if (IsPUse == AF_TRUE)
        {
            m_PUseFlg[Use] = AF_TRUE;
        }

        return;
    }

    inline BOOL IsPUse (llvm::Value* Use)
    {
        std::map<llvm::Value*, DWORD>::iterator It = m_PUseFlg.find (Use);
        if (It == m_PUseFlg.end())
        {
            return AF_FALSE;
        }

        return AF_TRUE;
    }

    inline llvm::Instruction* GetInst()
    {
        return m_Inst;
    }

    inline BOOL HasDefUse()
    {
        if (m_UseVal.size() != 0 || m_DefVal.size() != 0)
        {
            return AF_TRUE;
        }
        
        return AF_FALSE;
    }

    inline BOOL HasDef()
    {
        return (m_DefVal.size() != 0);
    }

    inline BOOL IsLoadInst ()
    {
        return (BOOL)(m_Inst->getOpcode() == llvm::Instruction::Load);
    }

    inline BOOL IsAllocaInst ()
    {
        return (BOOL)(m_Inst->getOpcode() == llvm::Instruction::Alloca);
    }

    inline BOOL IsStoreInst ()
    {
        return (BOOL)(m_Inst->getOpcode() == llvm::Instruction::Store);
    }

    inline BOOL IsCmpInst ()
    {
        return (BOOL)(m_Inst->getOpcode() == llvm::Instruction::ICmp);
    }

    inline BOOL IsRetInst ()
    {
        return (BOOL)(m_Inst->getOpcode() == llvm::Instruction::Ret);
    }

    inline BOOL IsCallInst ()
    {
        return (BOOL)(m_Inst->getOpcode() == llvm::Instruction::Call ||
                      m_Inst->getOpcode() == llvm::Instruction::Invoke);
    }

    std::set<llvm::Value*>::iterator UseBegin ()
    {
        return m_UseVal.begin();
    }

    std::set<llvm::Value*>::iterator UseEnd ()
    {
        return m_UseVal.end();
    }

    std::set<llvm::Value*>::iterator DefBegin ()
    {
        return m_DefVal.begin();
    }

    std::set<llvm::Value*>::iterator DefEnd ()
    {
        return m_DefVal.end();
    }
    
};


class Element
{
public:
    llvm::Value *Base;
    llvm::Value *Offset;

    llvm::Value *DefVal;
    llvm::Instruction *DefInst;

    typedef struct 
    {
        bool operator()(Element lhs, Element rhs) 
        {
            if (lhs.Base != rhs.Base)
            {
                return lhs.Base < rhs.Base;
            }
            else
            {
                return lhs.Offset < rhs.Offset;
            }
        }
    } EqualElem; 
};

typedef std::set<Element, typename Element::EqualElem> T_ElemSet;

class InstAnalyzer
{
public:
typedef std::set<llvm::Value*> T_ValueSet;
typedef std::set<llvm::Instruction*> T_InstSet;
typedef std::vector<llvm::Instruction*> T_InstVector;

typedef llvm::DenseMap<llvm::Value*, T_InstSet> T_Value2InstSet;
typedef llvm::DenseMap<llvm::Instruction*, DefUse*> T_Inst2DefUse;
typedef llvm::DenseMap<llvm::Value*, llvm::Instruction*> T_Value2Inst;

typedef llvm::DenseMap<llvm::Value*, Element*> T_Value2Elem;
typedef llvm::DenseMap<Element, llvm::Instruction*> T_Elem2DefInst;

    
private:
    llvm::Function *m_Function;
         
    llvm::Instruction* m_RetInst;
    
    T_ValueSet *m_ValueSet;
    
    DWORD m_DelPtsNum;
    DWORD m_AddPtsNum;
  
    T_Inst2DefUse m_InstToDuMap;

    T_Value2Inst  m_FparaToInst;

    /* handle array/structure field */
    T_Value2Elem m_ElemPtrMap;    
    T_ElemSet  m_ElemSet;

    /* handle global variables */
    static T_Value2InstSet m_DefGlobal2Inst;
    T_Value2InstSet m_UseGlobal2Inst;

    /* handle output para */
    T_InstVector *m_CallsiteSet;
    T_ElemSet m_PDefActSet;
    
private:

    VOID  GetDefUseInfo();
    DWORD GetInstNum();

    VOID ProcInst (llvm::Instruction* Inst, DefUse* Du);

    inline bool IsActualPara (llvm::Value *Val)
    {
        for (auto It = m_CallsiteSet->begin(), end = m_CallsiteSet->end(); It != end; It++)
        {
            llvm::ImmutableCallSite Cs(*It);
            ImmutableCallSite::arg_iterator aItr = Cs.arg_begin();

            for (; aItr != Cs.arg_end(); ++aItr) 
            {
                llvm::Value *Act = *aItr;
                if (Val == Act)
                {
                    return true;
                }
            }
        }
        
        return false;
    }

    inline bool IsInValueSet (llvm::Value *Val)
    {
        if (m_ValueSet == NULL)
        {
            return true;
        }
        
        auto it = m_ValueSet->find (Val);
        if (it == m_ValueSet->end())
        {
            return false;
        }

        return true;
    }

    inline VOID CollectDefOfInst (llvm::Instruction* Inst)
    {
        if (m_ValueSet == NULL)
        {
            return;
        }
        
        m_ValueSet->insert (Inst);

        if (Inst->getOpcode() == llvm::Instruction::Store)
        {
            m_ValueSet->insert (Inst->getOperand(1));
        }

        return;
    }

    inline bool IsGlobalValue (llvm::Value *Val)
    {
        llvm::GlobalValue *GVal = llvm::dyn_cast<llvm::GlobalValue>(Val);

        return (GVal != NULL);
    }


    inline bool SetPUse (DefUse* Du, llvm::Value *Val)
    {       
        std::vector<llvm::Value*> PtsTo;
        llvmAdpt::GetPtsTo (Val, PtsTo);
        if (PtsTo.size() == 0)
        {
            return false;
        }

        DWORD PtsNum = 0;
        DWORD PassNum = 0;
        for (auto it = PtsTo.begin(), end = PtsTo.end(); it != end; it++)
        {
            llvm::Value *PtsVal = *it;
            if (PtsVal == NULL || PtsVal == Val)
            {
                continue;
            }

            if (!IsInValueSet (PtsVal) && !IsGlobalValue(PtsVal))
            {
                PassNum++;
                continue;                
            }

            PtsNum++;
            Du->SetUse (PtsVal, AF_TRUE);
        }

        m_AddPtsNum += PtsNum;
        m_DelPtsNum += PassNum;

        //if (PtsNum+PassNum != 0)
        //{
        //    printf ("SetPUse, [%-4u/%-4u]\r\n", PtsNum, PassNum);
        //}
        
        return (PtsNum != 0);
    }

    inline VOID ActualParaProcess (llvm::Instruction *Inst,
                                        llvm::Value *Def,
                                        llvm::Value *ActPara)
    {
        Element Em;
        
        auto it = m_ElemPtrMap.find (Def);
        if (it == m_ElemPtrMap.end())
        {
            Em.Base    = ActPara;
            Em.Offset  = NULL;
            Em.DefVal  = Def;
            Em.DefInst = Inst;

            //DEBUG("=>Add %s <%s, %p> map to %p\r\n", Def->getName ().data(),\
            //      Em.Base->getName ().data(), Em.Offset, Inst);
        }
        else
        {
            Element *E = it->second;
            
            Em.Base    = ActPara;
            Em.Offset  = E->Offset;
            Em.DefVal  = Def;
            Em.DefInst = Inst;

            //DEBUG("=>Update %s <%s, %p> map to %p\r\n", Def->getName ().data(),\
            //      Em.Base->getName ().data(), Em.Offset, Inst);
        } 

        m_PDefActSet.insert (Em);
    }

    inline bool SetPDef (DefUse* Du, llvm::Value *DefVal)
    {
        std::vector<llvm::Value*> PtsTo;
        llvmAdpt::GetPtsTo (DefVal, PtsTo);
        if (PtsTo.size() == 0)
        {
            return false;
        }

        DWORD PtsNum = 0;
        DWORD PassNum = 0;
        for (auto it = PtsTo.begin(), end = PtsTo.end(); it != end; it++)
        {
            llvm::Value *PtsVal = *it;
            if (PtsVal == NULL || PtsVal == DefVal)
            {
                continue;
            }

            if (!IsInValueSet (PtsVal) && !IsGlobalValue(PtsVal))
            {
                PassNum++;
                continue;                
            }

            PtsNum++;
            Du->SetDef (PtsVal, AF_TRUE);

            if (IsActualPara (PtsVal))
            {
                ActualParaProcess (Du->GetInst (), DefVal, PtsVal);
            }
        }

        m_AddPtsNum += PtsNum;
        m_DelPtsNum += PassNum;
        
        //if (PtsNum+PassNum != 0)
        //{
        //    printf ("SetPDef, [%-4u/%-4u]\r\n", PtsNum, PassNum);
        //}
        
        return (PtsNum != 0);
    }

    inline BOOL IsArgument(llvm::Value *Val)
    {
        llvm::Function::arg_iterator fItr = m_Function->arg_begin();
        
        for (auto End = m_Function->arg_end(); fItr != End; ++fItr) 
        {
            llvm::Argument *Formal = &*fItr;
            
            if (Val == Formal)
            {
                return AF_TRUE;
            }
        }

        return AF_FALSE;
    }
    
    inline VOID ProcAlloca(llvm::Instruction *Inst, DefUse* Du)
    {
        //auto allocate = llvm::dyn_cast<AllocaInst>(Inst);
        //if (llvm::isa<PointerType>(allocate->getAllocatedType()))
        //{
        //    m_Pointers.push_back(Inst);
        //}
        
        return;
    }
    
    inline VOID ProcCall(llvm::Instruction *Inst, DefUse* Du)
    {   
        llvm::Function *Callee = llvmAdpt::GetCallee (Inst);
        if (Callee == NULL)
        {
            llvm::ImmutableCallSite Cs(Inst);
            if (!Cs.isIndirectCall())
            {
                return;
            }
            
            /* indirect call, get pointsto info */
            Callee = llvmAdpt::GetIndirectCallee(Inst);
            if (Callee == NULL)
            {
                //errs()<<"Inst:"<<*Inst<<"-->ProcCall fail!!!\r\n";
                return;
            }
        }


        /* def */
        if (!Callee->getReturnType()->isVoidTy())
        {
            Du->SetDef(Inst);
        }

        DWORD Index = 0;
        DWORD OpNum = Inst->getNumOperands ()-1;
        while (Index < OpNum)
        {
            Du->SetUse(Inst->getOperand (Index));
            Index++;
        }
    
        return;
    }
    
    inline VOID ProcRet(llvm::Instruction *Inst, DefUse* Du)
    {
        m_RetInst = Inst;
        
        /* ret i32 %10 */
        if (Inst->getNumOperands() == 0) 
        {
            return;
        }
    
        Du->SetUse (Inst->getOperand(0));
    
        return;
    }
    
    inline VOID ProcLoad(llvm::Instruction *Inst, DefUse* Du)
    {
        llvm::Value *Use = Inst->getOperand(0);
        
        Du->SetUse (Use);
        Du->SetDef (Inst);

        if (IsGlobalValue(Use))
        {
            m_UseGlobal2Inst[Use].insert (Inst);
            //llvm::errs()<<Use->getName () <<" use map to inst: "<<*Inst<<"\r\n";
        }
    
        return;
    }
    
    inline VOID ProcStore(llvm::Instruction *Inst, DefUse* Du)
    {
        llvm::Value *Def = Inst->getOperand(1);

        Du->SetDef (Def);
        Du->SetUse (Inst->getOperand(0));
        
        if (IsGlobalValue(Def))
        {
            m_DefGlobal2Inst[Def].insert (Inst);
            //llvm::errs()<<Def->getName () <<" def map to inst: "<<*Inst<<"\r\n";
        }

        auto It = m_ElemPtrMap.find (Def);
        if (It  != m_ElemPtrMap.end())
        {
            Element *Em = It->second;
            Em->DefVal = Def;
        }
        
        return;
    }


    inline llvm::Value* GetElemUse (llvm::Instruction *Inst)
    {
        Element Em;
        llvm::Value *Op0 = Inst->getOperand(0);
        
        Em.Base = llvmAdpt::GetElemBaseValue(Op0);
        Em.Offset = Inst->getOperand (Inst->getNumOperands ()-1);
        assert (Em.Offset != NULL);
        
        /* try to find, if this value is defined */
        auto It = m_ElemSet.find (Em);
        if (It == m_ElemSet.end())
        {
            auto EIt = m_ElemSet.insert (Em);
            m_ElemPtrMap[Inst] = (Element*)(&(*EIt.first));

            //DEBUG("Add AryStrt: <%s, %p>\r\n", Em.Base->getName ().data(), Em.Offset);
            return Op0;
        }
        else
        {
            /* add use as the specific field defined before */
            return (It->DefVal);
        }
    }
    
    
    inline VOID ProcGetElementPtr(llvm::Instruction *Inst, DefUse* Du)
    { 
        llvm::Value *Use = GetElemUse (Inst);
        Du->SetUse (Use);

        llvm::Value *Op0 = Inst->getOperand(0);
        if (Use != Op0)
        {
            Du->SetUse (Op0);
        }
 
        Du->SetDef (Inst);

        return;
    }
    
    inline VOID ProcPHI(llvm::Instruction *Inst, DefUse* Du)
    {     
        Du->SetDef (Inst);
        DWORD Index = 0;
        DWORD OpNum = Inst->getNumOperands ();
        while (Index < OpNum)
        {
            Du->SetUse(Inst->getOperand (Index));
            Index++;
        }
        
        return;
    }
    
    inline VOID ProcBitCast(llvm::Instruction *Inst, DefUse* Du)
    {
        Du->SetUse (Inst->getOperand(0));
        Du->SetDef (Inst);
         
        return;
    }
    
    
    inline VOID ProcSelect(llvm::Instruction *Inst, DefUse* Du)
    {
        Du->SetUse (Inst->getOperand(1));
        Du->SetUse (Inst->getOperand(2));
        Du->SetDef (Inst);

        return;
    }
    
    inline VOID ProcVAArg(llvm::Instruction *Inst, DefUse* Du)
    {
        Du->SetUse (Inst->getParent()->getParent());
        Du->SetDef (Inst);
    
        return;
    }

    inline VOID ProcCmp (llvm::Instruction* Inst, DefUse* Du)
    {
        /* %cmp = icmp slt i32 %2, %3 */
        Du->SetDef (Inst);
        Du->SetUse (Inst->getOperand(0));
        Du->SetUse (Inst->getOperand(1));

        return;
    }
    
    VOID ProcBr (llvm::Instruction* Inst, DefUse* Du)
    {
        /* br i1 %cmp, label %while.body, label %while.end */
        Du->SetUse (Inst->getOperand(0));

        return;
    }

    VOID ProcSBOp (llvm::Instruction* Inst, DefUse* Du)
    {
        /* %add = add nsw i32 %3, %4 */
        Du->SetDef (Inst);
        Du->SetUse (Inst->getOperand(0));
        Du->SetUse (Inst->getOperand(1));

        return;
    }
    
    VOID ProcShOp (llvm::Instruction* Inst, DefUse* Du)
    {
        /* %shl = shl i32 %6, 3  */
        Du->SetDef (Inst);
        Du->SetUse (Inst->getOperand(0));

        return;
    }

    inline VOID PrintDu(DefUse *Du);
        
public:
   
    InstAnalyzer (llvm::Function *Func, T_InstVector *CallsiteSet)
    {
        m_Function = Func;
        m_RetInst  = NULL;
        m_ValueSet = NULL;

        m_DelPtsNum = 0;
        m_AddPtsNum = 0;

        m_CallsiteSet = CallsiteSet;

        GetDefUseInfo();
    }
    
    ~InstAnalyzer ()
    {

    }

    inline VOID ClearMem ()
    {
        m_InstToDuMap.clear();
        m_ElemPtrMap.clear();    
        m_ElemSet.clear();
    }

    inline VOID SetFormalPara (llvm::Instruction *Inst)
    {
        llvm::Function::arg_iterator FIt;

        if (m_FparaToInst.size() == m_Function->arg_size () ||
            !llvmAdpt::IsStoreInst(Inst))
        {
            return;
        }

        llvm::Value *FpVal = Inst->getOperand (0);
        for (llvm::Function::arg_iterator fItr = m_Function->arg_begin(); 
             fItr != m_Function->arg_end(); ++fItr) 
        {
            llvm::Argument *Formal = &*fItr;
            if (FpVal == Formal)
            {
                m_FparaToInst[Formal] = Inst;
                break;
            }
        }
        
        return;
    }

    inline llvm::Instruction* GetRetInst ()
    {
        return m_RetInst;
    }

    inline llvm::Instruction* GetFpInst (llvm::Value *FormPara)
    {
        auto it = m_FparaToInst.find (FormPara);
        if (it != m_FparaToInst.end())
        {
            return it->second;
        }
        
        return NULL;
    }

    inline DefUse* GetDuByInst (llvm::Instruction* Inst)
    {
        auto it = m_InstToDuMap.find (Inst);
        if (it == m_InstToDuMap.end())
        {
            return NULL;
        }

        return it->second;
    }

    inline T_ElemSet *GetActDefNode ()
    {
        return &m_PDefActSet;
    }

    inline T_Value2InstSet* GetGlobUse ()
    {
        return &m_UseGlobal2Inst;
    }

    inline T_InstSet* GetGlobDefInst (llvm::Value *Glob)
    {
        auto It = m_DefGlobal2Inst.find (Glob);
        if (It != m_DefGlobal2Inst.end())
        {
            return &(It->second);
        }
        
        return NULL;
    }


    VOID  GetPDefUseInfo(std::set<llvm::Value*> *ValueSet);
};


class InstGraphViz
{
private:
    llvm::Instruction* m_Inst;
    
public:
    InstGraphViz (llvm::Instruction* Inst)
    {
        m_Inst = Inst;
    }

    ~InstGraphViz ()
    {
    }
    
    inline std::string GetGraphViz ()
    {
        DWORD ValId;
        llvm::Instruction* Inst = m_Inst;
    
        switch (Inst->getOpcode()) 
        {
            case Instruction::Alloca: 
            {
                return GvAlloca (Inst);
            }
            case Instruction::Call:
            case Instruction::Invoke: 
            {
                return GvCall (Inst);
            }
            case Instruction::Ret: 
            {
                return GvRet (Inst);            
            }
            case Instruction::Load: 
            {
                return GvLoad (Inst);          
            }
            case Instruction::Store:
            {
                return GvStore (Inst);
            }
            case Instruction::GetElementPtr: 
            {
                return GvGetElementPtr (Inst);
            }
            case Instruction::PHI: 
            {
                return GvPHI (Inst);                
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
                return GvCast (Inst);
            }
            case Instruction::Select: 
            {
                return GvSelect (Inst);
            }
            case Instruction::VAArg: 
            {
                return GvVAArg (Inst);
            }
            case Instruction::ICmp:
            {
                return GvCmp (Inst);
            }
            case Instruction::LShr:
            case Instruction::AShr:
            case Instruction::Shl:
            {
                return GvShOp (Inst);
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
                return GvSBOp (Inst);
            }
            case Instruction::Br:
            {
                return GvBr (Inst);
            }
            default: 
            {
                //errs() <<*Inst<< Inst->getOpcodeName()<<"\r\n";
                return "";
            }
        }
    }
    
private:


    inline string GetValueName (llvm::Value* Val) 
    {
        std::string str;
        raw_string_ostream RawStr(str);
        
        if (Val->hasName ())
        {
            RawStr << Val->getName ();
        }
        else
        {
            RawStr << Val;
        }

        return RawStr.str();
    }

    inline std::string GvAlloca(llvm::Instruction *Inst)
    { 
        std::string str;
        raw_string_ostream RawStr(str);

        /* variable = alloc() */
        RawStr << GetValueName(Inst) << "= alloca()";
        
        return RawStr.str();
    }
    
    inline std::string GvCall(llvm::Instruction *Inst)
    {  
        std::string str;
        raw_string_ostream RawStr(str);
   
        llvm::Function *Callee = llvmAdpt::GetCallee (Inst);
        if (Callee == NULL)
        {
            llvm::CallSite Cs(const_cast<llvm::Instruction*>(Inst));
            RawStr << Inst->getOpcodeName() <<" "<<Cs.getCalledValue() << "(";
        }
        else
        {
            if (!Callee->getReturnType()->isVoidTy())
            {
                RawStr << GetValueName(Inst) << " = " << Inst->getOpcodeName()<<" ";
            }
            RawStr << Callee->getName () << "(";
        }

        DWORD Index = 0;
        DWORD OpNum = Inst->getNumOperands ()-1;
        while (Index < OpNum)
        {
            RawStr << GetValueName(Inst->getOperand (Index));
            if (Index != OpNum-1)
            {
                RawStr <<", ";
            }
            Index++;
        }

        RawStr <<")";
    
        return RawStr.str();
    }
    
    inline std::string GvRet(llvm::Instruction *Inst)
    {
        std::string str;
        raw_string_ostream RawStr(str);
        
        /* ret i32 %10 */
        if (Inst->getNumOperands() == 0) 
        {
            RawStr <<Inst->getOpcodeName();
        }
        else
        {
            RawStr <<Inst->getOpcodeName()<<" "<<GetValueName(Inst->getOperand(0));
        }

        return RawStr.str();
    }
    
    inline std::string GvLoad(llvm::Instruction *Inst)
    {
        std::string str;
        raw_string_ostream RawStr(str);

        RawStr << GetValueName(Inst) <<" = "<<Inst->getOpcodeName()<<" "<< GetValueName(Inst->getOperand(0));
    
        return RawStr.str();
    }
    
    inline std::string GvStore(llvm::Instruction *Inst)
    {
        std::string str;
        raw_string_ostream RawStr(str);

        RawStr <<Inst->getOpcodeName()<<" "<< GetValueName(Inst->getOperand(0))\
                                      <<" "<< GetValueName(Inst->getOperand(1));
    
        return RawStr.str();
    }
    
    
    inline std::string GvGetElementPtr(llvm::Instruction *Inst)
    {
        std::string str;
        raw_string_ostream RawStr(str);

        RawStr << GetValueName(Inst) <<" = "<<Inst->getOpcodeName()<<" ";

        DWORD Index = 0;
        while (Index < Inst->getNumOperands ())
        {
            RawStr<<GetValueName(Inst->getOperand(Index))<<" ";
            Index++;
        }
 
        return RawStr.str();
    }
    
    inline std::string GvPHI(llvm::Instruction *Inst)
    {
        std::string str;
        raw_string_ostream RawStr(str);
        
        RawStr << GetValueName(Inst) <<" = "<<Inst->getOpcodeName()<<"(";

        DWORD Index = 0;
        DWORD OpNum = Inst->getNumOperands ();
        while (Index < OpNum)
        {
            RawStr <<GetValueName(Inst->getOperand (Index));
            if (Index != OpNum-1)
            {
                RawStr <<", ";
            }
            Index++;
        }
        RawStr<<")";

        return RawStr.str();
    }
    
    inline std::string GvCast(llvm::Instruction *Inst)
    {         
        std::string str;
        raw_string_ostream RawStr(str);
    
        RawStr << GetValueName(Inst) << " = " << Inst->getOpcodeName() << " "<<GetValueName(Inst->getOperand(0));
    
        return RawStr.str();
    }
    
    
    inline std::string GvSelect(llvm::Instruction *Inst)
    {
        std::string str;
        raw_string_ostream RawStr(str);
    
        RawStr << GetValueName(Inst) <<" = "<<Inst->getOpcodeName()<<"(" <<GetValueName(Inst->getOperand(0))\
                                     <<", "<<GetValueName(Inst->getOperand(2))<<")";
    
        return RawStr.str();
    }
    
    inline std::string GvVAArg(llvm::Instruction *Inst)
    { 
        std::string str;
        raw_string_ostream RawStr(str);
    
        RawStr << GetValueName(Inst) <<" = "<<Inst->getOpcodeName()<<" "<<Inst->getParent()->getParent();
    
        return RawStr.str();
    }

    inline std::string GvCmp (llvm::Instruction* Inst)
    {
        std::string str;
        raw_string_ostream RawStr(str);
    
        RawStr <<GetValueName(Inst)<<" = "<<Inst->getOpcodeName()<<" ("<<GetValueName(Inst->getOperand(0))
                                                   <<", "<<GetValueName(Inst->getOperand(1))<<")";
 
        return RawStr.str();
    }
    
    inline std::string GvBr (llvm::Instruction* Inst)
    {
        std::string str;
        raw_string_ostream RawStr(str);
    
        RawStr <<Inst->getOpcodeName()<<" "<<Inst->getOperand(0);
    
        return RawStr.str();
    }

    inline std::string GvSBOp (llvm::Instruction* Inst)
    {
        /* %add = add nsw i32 %3, %4 */
        std::string str;
        raw_string_ostream RawStr(str);
    
        RawStr <<GetValueName(Inst)<<" = "<<Inst->getOpcodeName()<<"(" <<GetValueName(Inst->getOperand(0))\
                                                   <<", "<<GetValueName(Inst->getOperand(1))<<")";
    
        return RawStr.str();
    }
    
    inline std::string GvShOp (llvm::Instruction* Inst)
    {
        std::string str;
        raw_string_ostream RawStr(str);
    
        RawStr <<GetValueName(Inst) <<" = "<<Inst->getOpcodeName()<<" "<<GetValueName(Inst->getOperand(0));
    
        return RawStr.str();
    }
   
};





#endif  
