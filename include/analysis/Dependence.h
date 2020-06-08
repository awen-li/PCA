//===- Dependence.h -- dependence graph ---------------------------------------//
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


#ifndef _DEPENDENCE_H_
#define _DEPENDENCE_H_
#include <llvm/Analysis/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/ADT/GraphTraits.h>
#include <llvm/ADT/STLExtras.h>	
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SparseBitVector.h>
#include "llvm/IR/Instructions.h"
#include "common/BasicMacro.h"
#include "callgraph/GenericGraph.h"
#include "callgraph/CallGraph.h"
#include "graphviz/GraphViz.h"


class DgNode;

class DgEdge : public GenericEdge<DgNode> 
{
private:


public:
    /// Constructor
    DgEdge(DgNode* s, DgNode* d, DWORD Attr=EA_CFG):GenericEdge<DgNode>(s, d, Attr)                       
    {
    }

    ~DgEdge() 
    {
    }    
};

class DgNode : public GenericNode<DgEdge> 
{
public:
    typedef std::set<llvm::Instruction*> T_InstSet;  
    typedef std::vector<llvm::Value*> T_ValueSet;
    typedef std::set<DWORD> T_DefIdSet;

private:
    llvm::Instruction* m_Inst;
    DefUse *m_DefUse;
        
    T_DefIdSet m_InSet;  
    T_DefIdSet m_OutSet;
    T_DefIdSet m_Gen;

    T_DefIdSet* m_KillSet;

public:
    
    DgNode(DWORD Id,  llvm::Instruction* Inst) : GenericNode<DgEdge>(Id)
    {
        m_Inst    = Inst;
        m_DefUse  = NULL;
        m_KillSet = NULL;
    }

    inline VOID ClearMem ()
    {       
        m_InSet.clear();
        m_OutSet.clear();
        m_Gen.clear();
        m_KillSet = NULL;
    }

    inline VOID SetKillSet (set<DWORD>* KillSet)
    {
        m_KillSet = KillSet;
    }

    inline T_DefIdSet* GetKillSet ()
    {
        return m_KillSet;
    }

    inline VOID SetGen (DWORD DefValId)
    {
        m_Gen.insert (DefValId);
    }

    inline T_DefIdSet::iterator GenBegin()
    {
        return m_Gen.begin();
    }

    inline T_DefIdSet::iterator GenEnd()
    {
        return m_Gen.end();
    }

    inline VOID SetIn (DWORD DefValId)
    {       
		m_InSet.insert (DefValId);
    }

    inline T_DefIdSet::iterator InBegin()
    {
        return m_InSet.begin();
    }
    
    inline T_DefIdSet::iterator InEnd()
    {
        return m_InSet.end();
    }
    
    inline bool SetOut (DWORD DefValId)
    {
        return m_OutSet.insert (DefValId).second;
    }
    
    inline DWORD GetOutNum ()
    {
        return m_OutSet.size();
    }
    
    inline T_DefIdSet::iterator OutBegin()
    {
        return m_OutSet.begin();
    }
    
    inline T_DefIdSet::iterator OutEnd()
    {    
        return m_OutSet.end();
    }
        
    inline llvm::Instruction* GetInst() const 
    {
        return m_Inst;
    }

    inline llvm::BasicBlock* GetBasicBlock() const 
    {
        return m_Inst->getParent ();
    }

    inline llvm::Function* GetFunction() const 
    {
        return m_Inst->getParent ()->getParent ();
    }

    inline VOID SetDefUse (DefUse *Du)
    {
        m_DefUse = Du;

        return;
    }

    inline DefUse* GetDefUse ()
    {
        return m_DefUse;
    }

    llvm::Value* GetDDef()
    {
        if (m_DefUse == NULL)
        {
            return (llvm::Value*)NULL;
        }
        
        return m_DefUse->GetDDef ();
    }

    DWORD GetSetNum()
    {
        return (m_InSet.size() + m_OutSet.size() + m_Gen.size());
    }
};


typedef struct SegGraph
{
    DgNode *Head; 
    DgNode* Tail;  
}SegGraph; 

struct T_DefVal
{
    llvm::Value* m_Value;
    llvm::Instruction* m_Inst;

    T_DefVal ()
    {
        m_Value = NULL;
        m_Inst  = NULL;
    }

    T_DefVal (llvm::Value* Value, llvm::Instruction* Inst)
    {
        m_Inst = Inst;
        m_Value = Value;
    }

    typedef struct 
    {
        bool operator()(const T_DefVal& lhs, const T_DefVal& rhs) const 
        {
            if (lhs.m_Value != rhs.m_Value)
            {
                return lhs.m_Value < rhs.m_Value;
            }
            else 
            {
                return lhs.m_Inst < rhs.m_Inst;
            }
        }
    } EqualVal;
};


class DgGraph;

class FuncDg
{
public:

typedef std::set<T_DefVal, typename T_DefVal::EqualVal> T_DefValSet;
typedef llvm::DenseMap<DWORD, T_DefVal*> T_Id2DefVal;
typedef llvm::DenseMap<T_DefVal*, DWORD> T_DefVal2Id;
typedef llvm::DenseMap<llvm::Value*, std::set<DWORD>> T_Val2KillSet;
typedef std::vector<llvm::Instruction*> T_InstVector;



private:
    llvm::Function *m_Function;
    DgNode *m_EntryNode;
    DgNode *m_ExitNode;
    
    InstAnalyzer *m_InstAlz;
    DgGraph   *m_Dg;

    std::vector<DgNode*> m_FuncDgNode;
    T_Val2KillSet m_ValToKillSet;


    T_DefValSet m_DefValSet;
    T_Id2DefVal m_Id2DefVal;
    DWORD m_DefValId;

    /* dump dot switch */
    DWORD m_IsDumpCfgDef;


private:
    VOID AddNode (DgNode*);

    inline VOID ClearMem()
    {
        m_ValToKillSet.clear();
        
        std::vector<DgNode*> Empty;
        m_FuncDgNode.clear();
        m_FuncDgNode.swap(Empty);

        m_DefValSet.clear();
        m_Id2DefVal.clear();

        m_InstAlz->ClearMem();
    }

    inline VOID InitNodeKillSet ()
    {
        DgNode *CurNode;
        
        for (auto Fit = m_FuncDgNode.begin(), Fend = m_FuncDgNode.end(); Fit != Fend; Fit++)
        {
            CurNode = *Fit;
            
            set<DWORD>* KillSet = GetKillSet(CurNode);
            CurNode->SetKillSet (KillSet);
        }
    }


public:

    /* control flow graph */
    VOID BuildCfg ();
    VOID BuildBCfg (llvm::BasicBlock *Block, SegGraph* Sg);
      
    
    /* data dependence */
    VOID ReachingDefs ();
    VOID BuildDdg ();

    FuncDg (DgGraph *Dg, llvm::Function *Function, T_InstVector* CallsiteSet)
    {
        m_EntryNode = NULL;
        m_ExitNode  = NULL;

        m_DefValId  = 0;

        m_Function = Function;
        m_InstAlz  = new InstAnalyzer(Function, CallsiteSet);
        assert (m_InstAlz != NULL);

        m_Dg = Dg;
    }

    ~FuncDg ()
    {
        if (m_InstAlz != NULL)
        {
            delete m_InstAlz;
            m_InstAlz = NULL;
        }
    }

    inline T_DefVal* GetDefValue(DWORD DefValId)
    {
        T_DefVal* DefVal = m_Id2DefVal[DefValId];
        if (DefVal == NULL)
        {
            return NULL;
        }

        return DefVal;
    }

    inline llvm::Function* GetFunction()
    {
        assert (m_Function != NULL);
        return m_Function;
    }

    inline VOID SetFunction (llvm::Function *Func)
    {
        assert (Func != NULL);
        m_Function = Func;
    }
    
    inline DgNode* GetHead ()
    {
        return m_EntryNode;
    }
    
    inline DgNode* GetTail ()
    {
        return m_ExitNode;
    }

    inline std::vector<DgNode*>::iterator FdnBegin()
    {
        return m_FuncDgNode.begin();
    }

    inline std::vector<DgNode*>::iterator FdnEnd()
    {
        return m_FuncDgNode.end();
    }

    inline InstAnalyzer *GetInstAlz ()
    {
        return m_InstAlz;
    }

    inline llvm::Instruction* GetRetInst ()
    {
        return m_InstAlz->GetRetInst();
    }
    
    inline set<DWORD>* GetKillSet(DgNode *CurNode)
    {
        std::set<DWORD> *KillSet;
            
        DefUse *Du = CurNode->GetDefUse ();
        if (Du == NULL || !Du->HasDefUse ())
        {
            return NULL;
        }

        auto It = m_ValToKillSet.find (Du->GetDDef ());
        if (It == m_ValToKillSet.end())
        {
            return NULL;
        }
            
        KillSet = &(It->second);
        assert (KillSet->size() != 0);
        
        return KillSet;
    }  

    inline T_ElemSet *GetActDefNode ()
    {
        return m_InstAlz->GetActDefNode ();
    }

    VOID UpdateGlobalDd ();
};

class DgGraph : public GenericGraph<DgNode, DgEdge>
{

public:
    typedef llvm::DenseMap<llvm::Instruction*, DgNode *> T_InstToDgNodeMap;
    typedef llvm::DenseMap<llvm::Function*, FuncDg*> T_FuncToFdgMap;
    typedef llvm::DenseMap<DWORD, DWORD> T_IdToLevelMap;
    

private:
    CallGraph *m_CallGraph;
      
    T_InstToDgNodeMap m_InstToNode;
    T_FuncToFdgMap    m_FuncToFdg;
    DWORD m_NodeNum;

    std::set<llvm::Function*> m_UpdatePtsFunc;

    std::set<llvm::Value*> m_ValueSet;
    
    DWORD GetInstNum (); 

    VOID BuildCfg ();
    VOID BuildIntraCfg ();
    VOID BuildInterCfg ();

    VOID BuildDdg ();
    VOID BuildIntraDdg ();
    VOID BuildInterDdg ();
    VOID RelateFpRet(FuncDg* Fdg, DgNode *CsNode, Function *Callee);
    VOID RelateActPara(FuncDg* Fdg, DgNode *CsNode);
    
    llvm::Value* GetFpByAp(llvm::ImmutableCallSite &Cs, llvm::Function* Func, llvm::Value* Actual);

    inline FuncDg* GetFuncDg (llvm::Function *Func)
    {
        T_FuncToFdgMap::iterator it = m_FuncToFdg.find(Func);
        if (it == m_FuncToFdg.end())
        {
            return NULL;
        }

        return it->second;
    }

    VOID UpdatePtsByFs();
    VOID UpdateGlobalDd();

    VOID DumpCfgWeight();

public:
    DgGraph(ModuleManage &ModMng)
    {
        m_NodeNum = 0;
        
        m_CallGraph = new CallGraph (ModMng);
        //m_CallGraph->PrintCg ();
        
        BuildDgGraph();
    }
    
    ~DgGraph() 
    {
        if (m_CallGraph)
        {
            delete m_CallGraph;
        }
    }


    inline FuncDg* GetFuncDg(DgNode* Node) 
    {
        llvm::Instruction *Inst = Node->GetInst ();
        llvm::Function *Func = Inst->getParent ()->getParent ();
        
        return m_FuncToFdg[Func];
    }

    inline DgNode* GetDgNode(DWORD Id) 
    {
        return GetGNode(Id);
    }
    
    inline DgNode* GetDgNode(llvm::Instruction* Inst) 
    {
        return m_InstToNode[Inst];
    }
    
    inline DgNode* AddDgNode (llvm::Instruction * Inst)
    {
        DgNode* Node = new DgNode (++m_NodeNum, Inst);
    
        AddNode(m_NodeNum, Node);
        
        m_InstToNode[Inst] = Node;
    
        return Node;
    }
    
    inline VOID AddDdgEdge (DgNode *Src, DgNode *Dst, llvm::Value *Val)
    {
        DgEdge *Edge = new DgEdge(Src, Dst, EA_DD);
        if (!AddEdge(Edge, Val))
        {
            delete Edge;
        }
    
        return;
    }

    inline VOID AddDdgCallEdge (DgNode *Src, DgNode *Dst, llvm::Value *Val)
    {
        DgEdge *Edge = new DgEdge(Src, Dst, EA_DD|EA_CALL);
        if (!AddEdge(Edge, Val))
        {
            delete Edge;
        }
    
        return;
    }

    inline VOID AddDdgRetEdge (DgNode *Src, DgNode *Dst, llvm::Value *Val)
    {
        DgEdge *Edge = new DgEdge(Src, Dst, EA_DD|EA_RET);
        if (!AddEdge(Edge, Val))
        {
            delete Edge;
        }
    
        return;
    }

    inline VOID AddCfgEdge (DgNode *Src, DgNode *Dst)
    {
        DgEdge *Edge = new DgEdge(Src, Dst, EA_CFG);
        if (!AddEdge(Edge))
        {
            delete Edge;
        }
    
        return;
    }

    inline VOID AddCfgCallEdge (DgNode *Src, DgNode *Dst)
    {
        DgEdge *Edge = new DgEdge(Src, Dst, EA_CFG|EA_CALL);
        if (!AddEdge(Edge))
        {
            delete Edge;
        }
    
        return;
    }

    inline VOID AddCfgRetEdge (DgNode *Src, DgNode *Dst)
    {
        DgEdge *Edge = new DgEdge(Src, Dst, EA_CFG|EA_RET);
        if (!AddEdge(Edge))
        {
            delete Edge;
        }
    
        return;
    }

    VOID BuildDgGraph();

    VOID UpdatePtsByFs(std::vector<Function*> &NodeStack);

    inline CallGraph *GetCallGraph () 
    {
        return m_CallGraph;
    }

    inline set<DWORD>* GetKillSet (DgNode *Node)
    {
        llvm::Function* CurFunc = Node->GetInst ()->getParent ()->getParent ();
        FuncDg* FDg = GetFuncDg (CurFunc);
        if (FDg == NULL)
        {
            return NULL;
        }

        return FDg->GetKillSet (Node);
    }
    
};


class CfgViz: public GraphViz <DgNode, DgEdge, DgGraph>
{

public:
    CfgViz(string GraphName, DgGraph   * Graph):GraphViz<DgNode, DgEdge, DgGraph>(GraphName, Graph)
    {
    }

    ~ CfgViz ()
    {
    }

    inline string GetNodeLabel(DgNode *Node) 
    {
        string str;
        raw_string_ostream RawStr(str);

        RawStr << "N" << Node->GetId () <<"\\n";
        
        InstGraphViz InstGv (Node->GetInst());
        RawStr << InstGv.GetGraphViz ();

        return RawStr.str();
    }

    inline BOOL IsEdgeType (DgEdge *Edge)
    {
        if (Edge->GetAttr ()  & EA_CFG)
        {
            return AF_TRUE;
        }
        else
        {
            return AF_FALSE;
        }
    }
};

class DfgViz: public GraphViz <DgNode, DgEdge, DgGraph>
{

public:
    DfgViz(string GraphName, DgGraph   * Graph):GraphViz<DgNode, DgEdge, DgGraph>(GraphName, Graph)
    {
    }

    ~DfgViz ()
    {
    }

    inline string GetNodeLabel(DgNode *Node) 
    {
        string str;
        raw_string_ostream RawStr(str);

        RawStr << "N" << Node->GetId () <<"\\n";
        
        InstGraphViz InstGv (Node->GetInst());
        RawStr << InstGv.GetGraphViz ();

        return RawStr.str();
    }

    inline BOOL IsEdgeType (DgEdge *Edge)
    {
        return AF_TRUE;
    }

    inline string GetEdgeAttributes(DgEdge *Edge) 
    {
        string str;
        raw_string_ostream RawStr(str);

        if (Edge->GetAttr () & EA_CFG)
        {
            RawStr <<  "color=black";
        }
        else
        {
            RawStr <<  "color=red";
        }
    
        return RawStr.str();
    }
};


#endif 
