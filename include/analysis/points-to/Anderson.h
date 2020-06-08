//===- Anderson.h -- anderson points-to analysis algorithm ---------------------//
//
// Copyright (C) <2019-2024>  <Wen Li>

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


#ifndef _ANDERSON_H_
#define _ANDERSON_H_
#include "analysis/points-to/ConstraintGraph.h"
#include "analysis/ExternalLib.h"


using namespace llvm;

#define GLOBAL_CONST_STR ".str"

class Anderson
{    
public:

    Anderson(ModuleManage &ModMange)
    {
        m_ModMange = ModMange;
        
        m_CstGraph = new ConstraintGraph ();
        assert (m_CstGraph != NULL);
        
        m_WorkList = new BitQueue ();
        assert (m_WorkList != NULL);
        
        m_ExtLib   = new ExternalLib ();
        assert (m_ExtLib != NULL);
        
        m_DebugLib = new DebugLib ();
        assert (m_DebugLib != NULL);
        

        /* Init node */
        m_CstGraph->AddCstNode (ConstraintNode::E_VALUE,  NULL);
        m_CstGraph->AddCstNode (ConstraintNode::E_OBJECT, NULL);
        m_CstGraph->AddCstNode (ConstraintNode::E_VALUE,  NULL);
        m_CstGraph->AddCstNode (ConstraintNode::E_OBJECT, NULL);

    }

    ~ Anderson() 
    {
        if (m_CstGraph != NULL)
        {
            delete m_CstGraph;
        }

        if (m_WorkList != NULL)
        {
            delete m_WorkList;
        }

        if (m_ExtLib != NULL)
        {
            delete m_ExtLib;
        }

        if (m_DebugLib != NULL)
        {
            delete m_DebugLib;
        }
    }

    DWORD RunPtsAnalysis ();
    
    VOID GetPtsTo (Value *Src, std::vector<Value*>& Dst);

    inline llvm::SparseBitVector<>::iterator PtsBegin(llvm::Value* Val)
    {
        DWORD Id = GetValueNode (Val);
        if (Id == 0)
        {
            return NULL;
        }

        ConstraintNode *CstNode = m_CstGraph->GetGNode (Id);
        assert (CstNode != NULL);
        
        PtsSet * Pst = CstNode->GetPtsSet ();
        return Pst->begin ();
    }

    inline llvm::SparseBitVector<>::iterator PtsEnd(llvm::Value* Val)
    {
        DWORD Id = GetValueNode (Val);
        if (Id == 0)
        {
            return NULL;
        }

        ConstraintNode *CstNode = m_CstGraph->GetGNode (Id);
        assert (CstNode != NULL);
        
        PtsSet * Pst = CstNode->GetPtsSet ();
        return Pst->end ();
    }

    inline bool IsDebugFunction (std::string FuncName)
    {
        return m_DebugLib->IsDebugFunction (FuncName);
    }

private:
    ModuleManage m_ModMange;
        
    BitQueue *m_WorkList;
    ConstraintGraph *m_CstGraph;
    ExternalLib *m_ExtLib;
    DebugLib *m_DebugLib;
            
    llvm::DenseMap<llvm::Value*, DWORD> m_ValueNodes;
    llvm::DenseMap<llvm::Value*, DWORD> m_ObjectNodes;
    llvm::DenseMap<llvm::Function*, DWORD> m_ReturnNodes;
    llvm::DenseMap<llvm::Function*, DWORD> m_VarargNodes; 

    std::vector<Constraint> m_Constraints;

    std::set<std::string> m_UnsolvedFunc;

    std::set<llvm::Value*> m_FuncPointer;

private:

    VOID InitCstGraph();
    VOID ClearMem();

    DWORD GetConstValueNode(llvm::Constant *Const);
    DWORD GetConstObjNode(llvm::Constant *Const);
    
    VOID AddGlbInitialCst(DWORD Id, Constant *Const);
    VOID AddCallSiteCst(ImmutableCallSite Cs);
    BOOL AddExtLibCst(ImmutableCallSite Cs, const Function *Func);
    VOID AddFuncArgCst(ImmutableCallSite Cs, Function *Func);
    
    VOID CollectCstOfGlobal();
    VOID CollectCstOfInst(Instruction *Inst);

	VOID CollectConstraints();
	DWORD SolveConstraints ();

    
    VOID CollectAlloca(Instruction *Inst);   
    VOID CollectCall(Instruction *Inst);
    VOID CollectRet(Instruction *Inst); 
    VOID CollectLoad(Instruction *Inst);    
    VOID CollectStore(Instruction *Inst);   
    VOID CollectGetElementPtr(Instruction *Inst);
    VOID CollectPHI(Instruction *Inst); 
    VOID CollectBitCast(Instruction *Inst);
    VOID CollectSelect(Instruction *Inst);
    VOID CollectVAArg(Instruction *Inst);    
    VOID CollectIntToPtr(Instruction *Inst);
    VOID CollectInstExtractValue(Instruction *Inst);

    VOID AddFuncPointer(Instruction *CallInst);

    VOID UpdatePointsTo ();

    inline VOID SetUnsolvedFunc (llvm::Function *Func)
    {
        std::string str(Func->getName().data());
        if (m_UnsolvedFunc.insert (str).second)
        {
            DEBUG("\"%s\", \r\n", str.c_str());
        }
        
        return;
    }

    inline DWORD CreateValueNode (llvm::Value *Val=NULL)
    {  
        DWORD Id = m_CstGraph->AddCstNode (ConstraintNode::E_VALUE, Val);

        if (Val != NULL)
        {
            m_ValueNodes[Val] = Id;
        }
        
        return Id;
    }

    inline DWORD CreateObjNode (llvm::Value *Val)
    {  
        DWORD Id = m_CstGraph->AddCstNode (ConstraintNode::E_OBJECT, Val);

        m_ObjectNodes[Val] = Id;
        
        return Id;
    }

    inline DWORD CreateObjNode (llvm::Value *Val, DWORD Id)
    {  
        m_ObjectNodes[Val] = Id;
        
        return Id;
    }

    inline DWORD GetUniversalPtrNode() 
    { 
       return UniversalPtr; 
    }
    
    inline DWORD GetUniversalObjNode() 
    { 
        return UniversalObj; 
    }
    
    inline DWORD GetNullPtrNode() 
    { 
        return NullPtr; 
    }
    
    inline DWORD GetNullObjNode() 
    { 
        return NullObject; 
    }
    
    inline DWORD GetObjNode (llvm::Value *Val)
    {
        if (Constant *Const = dyn_cast<Constant>(Val))
        {
            if (!isa<GlobalValue>(Const))
            {
                return GetConstObjNode(Const);
            }
        }

        auto itr = m_ObjectNodes.find(Val);
        if (itr == m_ObjectNodes.end())
        {
            return 0;
        }
        else
        {
            return itr->second;
        }
    }

    inline DWORD GetValueNode (llvm::Value *Val) 
    {
        if (Constant *Const = dyn_cast<Constant>(Val))
        {
            if (!isa<GlobalValue>(Const))
            {
                return GetConstValueNode(Const);
            }
        }

        auto itr = m_ValueNodes.find(Val);
        if (itr == m_ValueNodes.end())
        {
            return 0;
        }
        else
        {
            return itr->second;
        }
    }
    
    inline DWORD CreateRetNode (llvm::Function *Func)
    {  
        DWORD Id = m_CstGraph->AddCstNode (ConstraintNode::E_VALUE, Func);

        m_ReturnNodes[Func] = Id;
        
        return Id;
    }

    inline DWORD GetRetNode (llvm::Function *Func)
    {
        auto itr = m_ReturnNodes.find(Func);
        if (itr == m_ReturnNodes.end())
        {
            return 0;
        }
        else
        {
            return itr->second;
        }
    }

    inline DWORD CreateVarArgNode (llvm::Function *Func)
    {  
        DWORD Id = m_CstGraph->AddCstNode (ConstraintNode::E_OBJECT, Func);

        m_VarargNodes[Func] = Id;
        
        return Id;
    }

    inline DWORD GetVarArgNode (llvm::Function *Func)
    {     
        auto itr = m_VarargNodes.find(Func);
        if (itr == m_VarargNodes.end())
        {
            return 0;
        }
        else
        {
            return itr->second;
        }
    }

    inline VOID AddCosntraint (Constraint::ConstraintType Ty, DWORD D, DWORD S)
    {
        std::string TypeAry[] = {"E_COPY", "E_LOAD", "E_STORE", "E_ADDR_OF"};
        
        m_Constraints.push_back(Constraint(Ty, D, S));

        //errs()<<"type: "<<TypeAry[Ty]<<" Src: "<<S<<" Dst: "<<D<<"\r\n";

        return;
    }

    inline bool IsGlobalStr (llvm::Value* GValue)
    {
        const char *ValueName = GValue->getName ().data();
        if (strncmp (GLOBAL_CONST_STR, ValueName, sizeof(GLOBAL_CONST_STR)-1) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    inline bool IsFuncPointer(Value* FuncPoiner)
    {
        auto Itr = m_FuncPointer.find (FuncPoiner);
        if (Itr != m_FuncPointer.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

};

#endif 
