//===- CallGraph.h -- Call graph representation----------------------------//
//
//
// Copyright (C) <2019-2020>  <Wen Li>
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


#ifndef _CALLGRAPH_H_
#define _CALLGRAPH_H_

#include <llvm/IR/Module.h>	
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallSite.h>
#include <llvm/ADT/STLExtras.h>		 
#include <llvm/ADT/GraphTraits.h>
#include "llvmadpt/ModuleSet.h"
#include "llvmadpt/LlvmAdpt.h"
#include "callgraph/GenericGraph.h"

class DgGraph;
class CallGraphNode;
class ModuleManage;

class CallGraphEdge : public GenericEdge<CallGraphNode> 
{

public:
    typedef std::set<const llvm::Instruction*> CallInstSet;   
    typedef GenericNode<CallGraphEdge>::T_GEdgeSet T_CgEdgeSet;

private:
    CallInstSet m_DirectCallSet;
    CallInstSet m_IndirectCallSet;

public:
    /// Constructor
    CallGraphEdge(CallGraphNode* s, CallGraphNode* d):GenericEdge<CallGraphNode>(s, d)                       
    {
    }

    virtual ~CallGraphEdge() 
    {
    }

    void AddDirectCallSite(const llvm::Instruction* CallInst)
    {
        m_DirectCallSet.insert(CallInst);
    }

    void AddInDirectCallSite(const llvm::Instruction* CallInst) 
    {
        assert((llvm::isa<llvm::CallInst>(CallInst) || llvm::isa<llvm::InvokeInst>(CallInst)) && "not a call or inovke??");
        //assert((NULL == llvmAdpt::getCallee(CallInst) || NULL == llvm::dyn_cast<llvm::Function> (llvmAdpt::getForkedFun(CallInst))) && "not an indirect callsite??");
        m_IndirectCallSet.insert(CallInst);
    }

    CallInstSet& GetIndirectCallSet ()
    {
        return m_IndirectCallSet;
    }

};

typedef GenericNode<CallGraphEdge>::T_GEdgeSet CallGraphEdgeSet;

typedef std::vector<llvm::Instruction*> T_CallciteSet;

class CallGraphNode : public GenericNode<CallGraphEdge> 
{
private:
    llvm::Function* m_Function;
    BOOL m_IsLinked;
    BOOL m_IsLeaf;
    BOOL m_HasPointerArgs;

    T_CallciteSet m_InCallsiteSet;
    T_CallciteSet m_OutCallsiteSet;

private:
    inline BOOL HasPointerArgs(llvm::Function *Func)
    {
        llvm::Function::arg_iterator fItr = Func->arg_begin();
        
        for (auto End = Func->arg_end(); fItr != End; ++fItr) 
        {
            llvm::Argument *Formal = &*fItr;

            if (!Formal->getType()->isPointerTy()) 
            {
                continue;
            }
            
            return AF_TRUE;
        }

        return AF_FALSE;
    }
public:
    /// Constructor
    CallGraphNode(DWORD Id,    llvm::Function* Func) : \
                     GenericNode<CallGraphEdge>(Id), m_Function(Func) 
    {
        m_IsLinked = AF_FALSE;
        m_IsLeaf   = AF_TRUE;
        m_HasPointerArgs = HasPointerArgs (Func);
    }

    inline llvm::Function* GetFunction() 
    {
        return m_Function;
    }

    inline BOOL IsReachable()
    {
        return m_IsLinked;
    }

    inline VOID SetReachable(BOOL Rb)
    {
        m_IsLinked = Rb;
    }

    inline VOID SetLeafFlag (DWORD IsLeaf)
    {
        m_IsLeaf = IsLeaf;
        return;
    }

    inline BOOL IsLeaf ()
    {
        return m_IsLeaf;
    }

    inline BOOL HasPointerArgs ()
    {
        return m_HasPointerArgs;
    }

    inline VOID AddInCallSite (llvm::Instruction *Inst)
    {
        m_InCallsiteSet.push_back (Inst);
    }

    inline VOID AddOutCallSite (llvm::Instruction *Inst)
    {
        m_OutCallsiteSet.push_back (Inst);
    }

    inline T_CallciteSet* GetOutCallSite ()
    {
        return &m_OutCallsiteSet;
    }

    inline T_CallciteSet* GetInCallSite ()
    {
        return &m_InCallsiteSet;
    }
};

typedef llvm::DenseMap<llvm::Function*, CallGraphNode *> T_FunToCallGraphNodeMap;
typedef llvm::DenseMap<llvm::Instruction*, CallGraphEdgeSet> T_CallInstToCgEdgeMap;
typedef std::pair<llvm::CallSite, const llvm::Function*> T_CallSitePair;
typedef std::map<T_CallSitePair, DWORD> T_CallSiteToIdMap;
typedef std::map<DWORD, T_CallSitePair> T_IdToCallSiteMap;
    

class CallGraph : public GenericGraph<CallGraphNode, CallGraphEdge> 
{

private:
    ModuleManage m_ModMange;

    /// Call site information
    static T_CallSiteToIdMap m_CsToIdMap;	
    static T_IdToCallSiteMap m_IdToCSMap;
    static DWORD m_CallSiteNum;

    T_FunToCallGraphNodeMap m_FuncToCgNodeMap;
    T_CallInstToCgEdgeMap m_CallInstToCgEdgeMap;

    std::vector<llvm::Instruction*> m_FailCallsite;
    
    std::set<CallGraphNode*> m_DfsNode;
    std::vector<Function*> m_NodeStack;
    DWORD m_PathCount;
    DWORD m_PathDepth;

public:
    CallGraph(ModuleManage &ModMng)
    {
        m_PathCount = 0;
        m_PathDepth = 5;

        m_ModMange = ModMng; 
        
        BuildCallGraph();

        LinkGraph();
    }
    
    virtual ~CallGraph() 
    {
    }
    
    inline CallGraphNode* GetCgNode(DWORD Id) const 
    {
        return GetGNode(Id);
    }
    
    inline CallGraphNode* GetCgNode(llvm::Function* Func) const 
    {
        T_FunToCallGraphNodeMap::const_iterator it = m_FuncToCgNodeMap.find(Func);
        assert(it != m_FuncToCgNodeMap.end() && "call graph node not found!!");
        
        return it->second;
    }

    inline BOOL IsFuncReached(llvm::Function* Func)
    {
        CallGraphNode* CgNode = GetCgNode(Func);
        if (CgNode == NULL)
        {
            return AF_FALSE;
        }

        return CgNode->IsReachable ();
    }

    inline CallGraphEdgeSet* GetCgEdgeSet (llvm::Instruction *Inst) 
    {
        auto It = m_CallInstToCgEdgeMap.find (Inst);
        if (It == m_CallInstToCgEdgeMap.end())
        {
            return NULL;
        }

        return &(It->second);
    }

    inline T_CallInstToCgEdgeMap::iterator CItoEdgeBegin() 
    {
        return m_CallInstToCgEdgeMap.begin();
    }
    
    inline T_CallInstToCgEdgeMap::iterator CItoEdgeEnd() 
    {
        return m_CallInstToCgEdgeMap.end();
    }

    inline T_CallSiteToIdMap::iterator CItoIdBegin() 
    {
        return m_CsToIdMap.begin();
    }
    
    inline T_CallSiteToIdMap::iterator CItoIdEnd() 
    {
        return m_CsToIdMap.end();
    }

    inline DWORD GetCallsiteNum() 
    {
        return m_CsToIdMap.size();
    }

    inline CallGraphNode* GetEntryNode() 
    {
        llvm::Function* Entry = m_ModMange.GetEntryFunction();
        if (Entry == NULL)
        {
            return NULL;
        }

        return GetCgNode(Entry);
    }


    VOID UpdatePtsByFs(DgGraph *Dg);
    VOID DumpCgWeight();

private:

    VOID AddCgNode(llvm::Function* Func);
    VOID BuildCallGraph();
    VOID LinkGraph();

    VOID DFSNode (DgGraph *Dg, CallGraphNode *Node);

    inline VOID AddCallSiteMap(llvm::CallSite Cs, llvm::Function* Callee) 
    {
        std::pair<llvm::CallSite, const llvm::Function*> newCS(std::make_pair(Cs, Callee));
        T_CallSiteToIdMap::const_iterator it = m_CsToIdMap.find(newCS);

        if(it == m_CsToIdMap.end()) 
        {
            DWORD CsId = m_CallSiteNum++;
            m_CsToIdMap.insert(std::make_pair(newCS, CsId));
            m_IdToCSMap.insert(std::make_pair(CsId, newCS));
        }
    }

    inline VOID AddCgEdgeSetMap(llvm::Instruction* Inst, CallGraphEdge* Edge)
    {
        bool Insert = m_CallInstToCgEdgeMap[Inst].insert(Edge).second;
        if (Insert)
        {
            llvm::CallSite Cs = llvmAdpt::GetLLVMCallSite(Inst);
            AddCallSiteMap(Cs, Edge->GetDstNode()->GetFunction());
        }
    }

    VOID AddDirectCgEdge(llvm::Instruction* CallInst);
    VOID AddIndirectCgEdge(llvm::Instruction* CallInst,     llvm::Function* Callee);
    
    CallGraphEdge* GetCgEdge(CallGraphNode* SrcNode, CallGraphNode* DstNode);

    VOID DumpInfo ();
};


#endif /* CALLGRAPH_H_ */
