//===- Dependence.cpp -- dependance analysis:cfg & dfg----//
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
#include <llvm/IR/InstIterator.h>
#include "analysis/Dependence.h"
#include "common/WorkList.h"
#include "common/Stat.h"


using namespace llvm;
using namespace std;

VOID FuncDg::BuildBCfg (BasicBlock *Block, SegGraph* Sg)
{
    DgNode *Node;
    DefUse *Du;
    Value* Def;
    Instruction *Inst;
    
    for (BasicBlock::iterator curIt = Block->begin(), End = Block->end(); curIt != End; ++curIt) 
    {
        Inst = &*curIt;
        
        if (llvmAdpt::IsInstrinsicDbgInst(Inst))
        {
            continue;
        }
        
        Node = m_Dg->AddDgNode(&*curIt);        
        if (Sg->Head == NULL)
        {
            Sg->Head = Node;
            Sg->Tail = Node;
        }
        else
        {
            assert (Node != NULL);
            m_Dg->AddCfgEdge (Sg->Tail, Node);
            Sg->Tail = Node;
        }

        m_FuncDgNode.push_back(Node);

        Du = m_InstAlz->GetDuByInst (Inst);
        if (Du == NULL)
        {
            continue;
        }

        /* init def and use */
        Node->SetDefUse (Du);

        /* init killset */
        for (auto Dit = Du->DefBegin(), Dend = Du->DefEnd(); Dit != Dend; Dit++)
        {
            Def = *Dit;

            /* insert to set and allot an id */
            auto It = m_DefValSet.insert (T_DefVal(Def, Inst));
            assert (It.second == true);

            m_DefValId++;
            T_DefVal *DefVal = (T_DefVal *)(&(*It.first));
            m_Id2DefVal[m_DefValId] = DefVal;

            /* init gen set */
            Node->SetGen (m_DefValId);

            /* init definite define to killset */
            if (!Du->IsPDef (Def))
            {
                m_ValToKillSet[Def].insert(m_DefValId);
            }
        }
    }

    return;
}


VOID FuncDg::BuildCfg ()
{
    SegGraph BSegG;
    map<BasicBlock*, SegGraph> m_Bb2BSg;

    /* construct basicblock's graph */
    for (Function::iterator Bit = m_Function->begin(), Bend = m_Function->end(); Bit != Bend; ++Bit) 
    {        
        BSegG.Head = NULL;
        BSegG.Tail = NULL;
        BuildBCfg(&*Bit, &BSegG);

        if (BSegG.Head != NULL)
        {
            m_Bb2BSg[&*Bit] = BSegG;
        }
	}

    /* function with no body */
    if (m_Bb2BSg.size() == 0)
    {
        return;
    }

    BasicBlock* Entry = &m_Function->getEntryBlock();
    m_EntryNode = (m_Bb2BSg.find(Entry)->second).Head;

    for(auto IT = m_Bb2BSg.begin(), End = m_Bb2BSg.end(); IT != End; IT++)
    {
        SegGraph *CurSg = &IT->second;
        if (succ_empty (IT->first))
        {     
            m_ExitNode = CurSg->Tail;
            continue;
        }

        for (BasicBlock* sucBB : successors(IT->first)) 
        {
            auto It = m_Bb2BSg.find(sucBB);
            if (It == m_Bb2BSg.end())
            {
                continue;
            }
                
            SegGraph *sucSg = &(It->second);
            m_Dg->AddCfgEdge (CurSg->Tail, sucSg->Head);         
        }
    }

    /* relate to one function */
    assert(m_EntryNode != NULL && m_ExitNode != NULL);
     
}

VOID FuncDg::ReachingDefs ()
{
    DgNode *CurNode;
    DgNode *PreNode;
    std::vector<T_DefVal> *KillSet;
    BOOL Change;

    InitNodeKillSet ();

    auto Head = m_FuncDgNode.begin();
    do
    {
        Change = AF_FALSE;

        for (auto Fit = Head, Fend = m_FuncDgNode.end(); Fit != Fend; Fit++)
        {
            CurNode = *Fit;

            /* calculate inset */
            if (Fit != Head)
            {
                for (auto Eit = CurNode->InEdgeBegin(), Eend = CurNode->InEdgeEnd (); Eit != Eend; Eit++)
                {
                    DgEdge *Edge = *Eit;
                    if (!(Edge->GetAttr () & EA_CFG))
                    {
                        continue;
                    }
                    
                    PreNode = Edge->GetSrcNode ();
                    for (auto Oit = PreNode->OutBegin (), Oend = PreNode->OutEnd (); Oit != Oend; Oit++)
                    {
                        CurNode->SetIn (*Oit);
                    }
                }
            }

            /* add gen to out set */
            for (auto Git = CurNode->GenBegin (), Gend = CurNode->GenEnd (); Git != Gend; Git++)
            {
                Change |= CurNode->SetOut (*Git);
            }

            /* add IN - KILL to the out */
            set<DWORD> *KillSet = CurNode->GetKillSet ();
            for (auto Iit = CurNode->InBegin (), Iend = CurNode->InEnd (); Iit != Iend; Iit++)
            {
                if (KillSet != NULL && KillSet->find (*Iit) != KillSet->end())
                {
                    continue;
                }

                Change |= CurNode->SetOut (*Iit);
            }

        }       
    }while (Change == AF_TRUE);

}


VOID FuncDg::UpdateGlobalDd ()
{
    InstAnalyzer::T_InstSet* UseInstSet;
    InstAnalyzer::T_InstSet* DefInstSet;
    InstAnalyzer::T_Value2InstSet* Value2Inst = m_InstAlz->GetGlobUse ();

    for (auto Itu = Value2Inst->begin(), Endu = Value2Inst->end(); Itu != Endu; Itu++)
    {
        llvm::Value *ValUse = Itu->first;
        DefInstSet = m_InstAlz->GetGlobDefInst (ValUse);
        if (DefInstSet == NULL)
        {
            continue;
        }

        UseInstSet = &(Itu->second);
        for (auto Iit = UseInstSet->begin(), Iend = UseInstSet->end(); Iit != Iend; Iit++)
        {
            DgNode *UseNode = m_Dg->GetDgNode (*Iit);
            for (auto Jit = DefInstSet->begin(), Jend = DefInstSet->end(); Jit != Jend; Jit++)
            {
                DgNode *DefNode = m_Dg->GetDgNode (*Jit);

                m_Dg->AddDdgEdge (DefNode, UseNode, ValUse);           
            }           
        }     
    }
}


VOID FuncDg::BuildDdg ()
{
    DgNode *CurNode = NULL;
    DgNode *DefNode;
    Value  *UseVal;
    DgNode::T_InstSet *InstSet;

    /* calculate reachable defs */
    ReachingDefs (); 

    /* calculate data dependence */
    for (auto it = m_FuncDgNode.begin(), End = m_FuncDgNode.end(); it != End; it++)
    {
        CurNode = *it;

        DefUse *Du = CurNode->GetDefUse ();
        if (Du == NULL)
        {
            continue;
        }

        for (auto uit = Du->UseBegin (), Uend = Du->UseEnd (); uit != Uend; uit++)
        {
            UseVal = *uit;

            /* visit all in defval items */
            for (auto Init = CurNode->InBegin (), Iend = CurNode->InEnd (); Init != Iend; Init++)
            {
                T_DefVal *DefVal = m_Id2DefVal[*Init];
                if (DefVal == NULL)
                {
                    continue;
                }
                
                if (DefVal->m_Value != UseVal)
                {
                    continue;
                }
        
                /* add du edge */
                DefNode = m_Dg->GetDgNode(DefVal->m_Inst);
                assert (DefNode != NULL);

                m_Dg->AddDdgEdge (DefNode, CurNode, UseVal);
            }
        }

        CurNode->ClearMem();
    }

    ClearMem ();
}


VOID DgGraph::BuildCfg ()
{
    BuildIntraCfg ();

    BuildInterCfg ();

    if (llaf::GetParaValue (PARA_CFG_DUMP) == "1")
    {
        CfgViz cfgViz (string("CFG"), this);
        cfgViz.WiteGraph ();
    }

    if (llaf::GetParaValue (PARA_CFG_WEIGHT) == "1")
    {
        DumpCfgWeight ();
        exit (0);
    }
}

VOID DgGraph::BuildIntraCfg ()
{
    FuncDg *Fdg;
    Function *Func;
    DWORD FuncId = 0;
    DWORD FuncNum = m_CallGraph->GetNodeNum ();
    CallGraphNode *CgNode;

    /* intra-procedure cfg */
    for (auto GIt = m_CallGraph->begin (), End = m_CallGraph->end (); GIt != End; GIt++)
    {
        CgNode = GIt->second;
        Func = CgNode->GetFunction ();
        ++FuncId;
        
        if (!CgNode->IsReachable() || Func->getInstructionCount() == 0)
        {
            continue;
        }        

        if (!(FuncId%200))
        {
            printf("IntraCfg:[%-8d/%-8d] - (V,E):(%-8d, %-8d) => process function:%-64s\r", 
                   FuncId, FuncNum, m_NodeNum, m_EdgeNum, Func->getName().data());
        }

        Fdg = new FuncDg (this, Func, CgNode->GetInCallSite ());
        assert (Fdg != NULL);

        m_FuncToFdg[Func] = Fdg;

        Fdg->BuildCfg ();    
    }  

    printf("IntraCfg:[%-8d/%-8d] - (V,E):(%-8d, %-8d)\n", FuncId, FuncNum, m_NodeNum, m_EdgeNum);

    return;
}

VOID DgGraph::BuildInterCfg ()
{
    DgNode *dgDstNode = NULL;
    DWORD Index = 0;
    DWORD CsNum = m_CallGraph->GetCallsiteNum();
    
    for (auto SetIt = m_CallGraph->CItoEdgeBegin(), SetEnd = m_CallGraph->CItoEdgeEnd (); SetIt != SetEnd; SetIt++)
    {
        ++Index;
        if (!(Index%5000))
        {
            printf ("InterCfg:[%-8d/%-8d] \r", Index, CsNum);
        }
        CallGraphEdgeSet &CgEdgeSet = SetIt->second;

        DgNode *CallSiteNode = GetDgNode (SetIt->first);
        if (CallSiteNode == NULL)
        {
            continue;
        }

        /* remove the outgoing edge first */
        DgEdge *dgEdge = *(CallSiteNode->OutEdgeBegin ());
        dgDstNode = dgEdge->GetDstNode ();
        //CallSiteNode->RmOutgoingEdge (dgEdge);
        //dgDstNode->RmIncomingEdge (dgEdge);
            
        for (auto EgIt = CgEdgeSet.begin(), EgEnd = CgEdgeSet.end(); EgIt != EgEnd; EgIt++)
        {
            CallGraphEdge *CgEdge = *EgIt;
                
            Function *Callee = CgEdge->GetDstNode()->GetFunction ();
            FuncDg *Fg = GetFuncDg (Callee);
            if (Fg == NULL)
            {
                continue;
            }
                
            /* add new edge: srcnode->callee head, callee tail->dstnode */
            AddCfgCallEdge (CallSiteNode, Fg->GetHead ());
            AddCfgRetEdge (Fg->GetTail (), dgDstNode);
        }

        if (dgDstNode->GetIncomingEdgeNum () > 1)
        {
            dgEdge->SetAttr (EA_CFG|EA_CFG_DMY);  
        }
    }

    printf ("InterCfg:[%-8d/%-8d]\r\n", Index, CsNum);
}

VOID DgGraph::BuildDdg ()
{
    /* 1. intra data depence */
    BuildIntraDdg ();

    /* 2. inter data depence() */
    BuildInterDdg ();

    /* 3. update global dd */
    UpdateGlobalDd();
}

VOID DgGraph::BuildIntraDdg ()
{
    FuncDg *Fdg;
    Function *Func;
    DWORD FuncId = 0;
    DWORD FuncNum = m_CallGraph->GetNodeNum ();
    CallGraphNode *CgNode;

    for (auto GIt = m_CallGraph->begin (), End = m_CallGraph->end (); GIt != End; GIt++)
    {
        CgNode = GIt->second;
        Func = CgNode->GetFunction ();
        ++FuncId;
        
        if (!CgNode->IsReachable() || Func->getInstructionCount() == 0)
        {
            continue;
        } 
        

        Fdg = GetFuncDg (Func);
        assert (Fdg != NULL);
    
        Fdg->BuildDdg ();

        printf("IntraDdg:[%-8d/%-8d] - (V,E):(%-8d, %-8d) => process function:%-32s\r", 
                   FuncId, FuncNum, m_NodeNum, m_EdgeNum, Func->getName().data());
    }  

    printf("IntraDdg:[%-8d/%-8d] - (V,E):(%-8d, %-8d) => process function:%-64s\r\n", 
                   FuncId, FuncNum, m_NodeNum, m_EdgeNum, Func->getName().data());  

    return;
}

VOID DgGraph::RelateActPara(FuncDg* CalleeFdg, DgNode *CsNode)
{
    T_ElemSet *PDefElemSet = CalleeFdg->GetActDefNode ();
    if (PDefElemSet->size() == 0)
    {
        return;
    }

    /* BFS the graph node */
    std::set<DgNode*> Visited;
    ComQueue<DgNode *> Queue;
    DgNode *CurNode  = CsNode;
    for (auto it = CurNode->OutEdgeBegin (), end = CurNode->OutEdgeEnd ();
            it != end; it++)
    {
        DgEdge *Edge = *it;
        if (!(Edge->GetAttr () & EA_CFG) || (Edge->GetAttr () & EA_CALL))
        {
            continue;
        }

        if (Edge->GetAttr () & EA_RET)
        {
            return;
        }
            
        Queue.InQueue (Edge->GetDstNode ());
    }

    Element Em;
    while (!Queue.IsEmpty ())
    {
        CurNode = Queue.OutQueue ();
        if (Visited.find (CurNode) != Visited.end())
        {
            continue;
        }
        else
        {
            Visited.insert (CurNode);
        }

        /* if  GetElementPtr */
        Instruction *Inst = CurNode->GetInst ();
        if (llvmAdpt::IsGepInst (Inst))
        {
            Em.Base   = llvmAdpt::GetElemBaseValue(Inst->getOperand (0));
            Em.Offset = Inst->getOperand (Inst->getNumOperands ()-1);
            auto It = PDefElemSet->find (Em);
            if (It != PDefElemSet->end())
            {
                AddDdgEdge (GetDgNode(It->DefInst), CurNode, It->DefVal);
            }
        }
        else
        {
            DefUse *Du = CurNode->GetDefUse ();
            if (Du != NULL)
            {
                for (auto it = Du->UseBegin (), end = Du->UseEnd (); it != end; it++)
                {
                    llvm::Value *Val = *it;
                    if (Du->IsPUse (Val))
                    {
                        continue;
                    }

                    Em.Base   = Val;
                    Em.Offset = NULL;
                    auto DfIt = PDefElemSet->find (Em);
                    if (DfIt == PDefElemSet->end())
                    {
                        for (auto dit = PDefElemSet->begin(), dend = PDefElemSet->end(); 
                             dit != dend; dit++)
                        {
                            if (dit->Base == Val)
                            {
                                AddDdgEdge (GetDgNode(dit->DefInst), CurNode, Val);
                            }
                        }
                    }
                    else
                    {
                        AddDdgEdge (GetDgNode(DfIt->DefInst), CurNode, Val);
                    }
                }
            }
        }
        

        /* iterate all children node */
        for (auto it = CurNode->OutEdgeBegin (), end = CurNode->OutEdgeEnd ();
            it != end; it++)
        {
            DgEdge *Edge = *it;
            if (!(Edge->GetAttr () & EA_CFG) || (Edge->GetAttr () & EA_CALL))
            {
                continue;
            }

            if (Edge->GetAttr () & EA_RET)
            {
                return;
            }
            
            Queue.InQueue (Edge->GetDstNode ());
        }
    }


    return;
}


llvm::Value* DgGraph::GetFpByAp(llvm::ImmutableCallSite &Cs, llvm::Function* Func, llvm::Value* Actual)
{    
    Function::arg_iterator fItr = Func->arg_begin();
    ImmutableCallSite::arg_iterator aItr = Cs.arg_begin();
        
    for (; fItr != Func->arg_end() && aItr != Cs.arg_end(); ++fItr, ++aItr) 
    {
        Argument *Formal = &*fItr;
        if (Actual == *aItr)
        {
            return Formal;
        }
    }

    return NULL;
}


VOID DgGraph::RelateFpRet(FuncDg* Fdg, DgNode *CsNode, Function *Callee)
{
    InstAnalyzer *InstAlz = Fdg->GetInstAlz ();
    llvm::ImmutableCallSite Cs(CsNode->GetInst ());

    DWORD DdEdgeNum = 0;
    for (auto it = CsNode->InEdgeBegin (), end = CsNode->InEdgeEnd (); it != end; it++)
    {
        DgEdge *dgEdge = *it;
        if (!(dgEdge->GetAttr () & EA_DD))
        {
            continue;
        }

        llvm::Value *ActualPara = dgEdge->GetEdgeValue ();
        assert (ActualPara != NULL);

        llvm::Value *FormalPara = GetFpByAp(Cs, Callee, ActualPara);
        if (FormalPara == NULL)
        {
            continue;
        }

        llvm::Instruction *Inst = InstAlz->GetFpInst (FormalPara);
        if (Inst == NULL)
        {
            continue;
        }

        DgNode *UseNode = GetDgNode (Inst);
        DgNode *DefNode = dgEdge->GetSrcNode ();
        if (DefNode->GetFunction () == CsNode->GetFunction ())
        {
            AddDdgCallEdge (DefNode, UseNode, ActualPara);
        }
        else
        {
            AddDdgEdge (DefNode, UseNode, ActualPara);
        }
        DdEdgeNum++;
    }

    #if 0
    if (DdEdgeNum == 0)
    {
        for (llvm::Function::arg_iterator fItr = Callee->arg_begin(); 
             fItr != Callee->arg_end(); ++fItr) 
        {
            llvm::Argument *Formal = &*fItr;
            llvm::Instruction *Inst = InstAlz->GetFpInst (Formal);
            if (Inst == NULL)
            {
                continue;
            }
            DgNode *UseNode = GetDgNode (Inst);

            AddDdgCallEdge (CsNode, UseNode, Formal);
        }
    }
    #endif

    /* related return value to the callsite */
    if (llvmAdpt::IsFunctionVoidType(Callee) || CsNode->GetDDef() == NULL)
    {
        return;
    }

    DgNode *RetNode = GetDgNode (Fdg->GetRetInst());
    if (RetNode == NULL)
    {
        return;
    }

    for (auto it = RetNode->InEdgeBegin (), end = RetNode->InEdgeEnd (); it != end; it++)
    {
        DgEdge *dgEdge = *it;
        if (!(dgEdge->GetAttr () & EA_DD))
        {
            continue;
        }

        AddDdgRetEdge (RetNode, CsNode, CsNode->GetDDef());
        break;
    }

    return;
}


VOID DgGraph::BuildInterDdg ()
{
    DWORD Index = 0;
    DWORD CsNum = m_CallGraph->GetCallsiteNum();
    T_CallSitePair CsPair;

    for (auto It = m_CallGraph->CItoIdBegin (), End = m_CallGraph->CItoIdEnd (); It != End; It++)
    {
        ++Index;
        if (!(Index%5000))
        {
            printf ("InterDdg[ %-8d/%-8d ]\r", Index, CsNum);
        }
        CsPair = It->first;
        
        llvm::Instruction *Inst = CsPair.first.getInstruction ();
        CallGraphEdgeSet *CgEdgeSet = m_CallGraph->GetCgEdgeSet (Inst);
        assert (CgEdgeSet != NULL);

        DgNode *CallSiteNode = GetDgNode (Inst);
        if (CallSiteNode == NULL)
        {
            continue;
        }

        
        for (auto EgIt = CgEdgeSet->begin(), EgEnd = CgEdgeSet->end(); EgIt != EgEnd; EgIt++)
        {
            CallGraphEdge *CgEdge = *EgIt;
            
            Function *Callee = CgEdge->GetDstNode()->GetFunction ();
            FuncDg *CalleeFdg = GetFuncDg (Callee);
            if (CalleeFdg == NULL)
            {
                continue;
            }

            RelateActPara (CalleeFdg, CallSiteNode);

            RelateFpRet(CalleeFdg, CallSiteNode, Callee);
        }
    }

    printf ("InterDdg[ %-8d/%-8d ]\r\n", Index, CsNum);
    return;
}


VOID DgGraph::BuildDgGraph()
{ 
    /* 1. build cfg */
    BuildCfg ();

     /* 2. Update pts by control flow */
    m_CallGraph->UpdatePtsByFs(this);
    
    /* 3. Update pts for left function */
    UpdatePtsByFs();

    /* 4. buid ddg */
    BuildDdg ();

    if (llaf::GetParaValue (PARA_DDG_DUMP) == "1")
    {
        DfgViz dfgViz(string("DDG"), this);
        dfgViz.WiteGraph ();
    }   
    
    return;
}

VOID DgGraph::UpdatePtsByFs()
{
    FuncDg *Fdg;
    Function *Func;
    DWORD FuncId = 0;
    DWORD FuncNum = m_CallGraph->GetNodeNum ();
    CallGraphNode *CgNode;

    for (auto GIt = m_CallGraph->begin (), End = m_CallGraph->end (); GIt != End; GIt++)
    {
        CgNode = GIt->second;
        Func = CgNode->GetFunction ();
        ++FuncId;
        
        if (!CgNode->IsReachable() || Func->getInstructionCount() == 0)
        {
            continue;
        }  

        if (m_UpdatePtsFunc.find (Func) != m_UpdatePtsFunc.end())
        {
            continue;
        }
        
        printf("2-UpdatePts:[%-8d/%-8d] %-64s\r", FuncId, FuncNum, Func->getName ().data()); 

        Fdg = GetFuncDg (Func);
        assert (Fdg != NULL);
    
        InstAnalyzer *InstAnly = Fdg->GetInstAlz();
        InstAnly->GetPDefUseInfo (NULL);

    }  

    printf("2-UpdatePts:[%-8d/%-8d]\r\n", FuncId, FuncNum);   

    return;
}


VOID DgGraph::UpdatePtsByFs(std::vector<Function*> &NodeStack)
{
    for (auto it = NodeStack.begin(), end = NodeStack.end(); it != end; it++)
    {
        Function *CurFunc = *it;

        FuncDg *Fdg = GetFuncDg (CurFunc);
        assert (Fdg != NULL);

        InstAnalyzer *InstAnly = Fdg->GetInstAlz();
        
        InstAnly->GetPDefUseInfo (&m_ValueSet);

        m_UpdatePtsFunc.insert(CurFunc);
    }

    m_ValueSet.clear();
    
    return;
}


VOID DgGraph::UpdateGlobalDd()
{
    FuncDg *Fdg;
    Function *Func;
    DWORD FuncId = 0;
    DWORD FuncNum = m_CallGraph->GetNodeNum ();
    CallGraphNode *CgNode;

    for (auto GIt = m_CallGraph->begin (), End = m_CallGraph->end (); GIt != End; GIt++)
    {
        CgNode = GIt->second;
        Func = CgNode->GetFunction ();
        ++FuncId;
        
        if (!CgNode->IsReachable() || Func->getInstructionCount() == 0)
        {
            continue;
        }  

        if (!(FuncId%100))
        {
            printf("3-UpdateGlobalDd:[%-8d/%-8d] %-64s\r", FuncId, FuncNum, Func->getName ().data());
        }

        Fdg = GetFuncDg (Func);
        assert (Fdg != NULL);
    
        Fdg->UpdateGlobalDd ();
    }  

    printf("3-UpdateGlobalDd:[%-8d/%-8d]\r\n", FuncId, FuncNum);   

    return;
}


VOID DgGraph::DumpCfgWeight()
{
    m_CallGraph->DumpCgWeight ();
    
    CallGraphNode *CgNode = m_CallGraph->GetEntryNode ();
    llvm::Function *EntryFunc = CgNode->GetFunction ();
    
    FuncDg *Fdg = GetFuncDg (EntryFunc);
    assert (Fdg != NULL);

    DgNode *EntryNode = *(Fdg->FdnBegin());
    assert (EntryNode != NULL);

    std::vector<DgNode*> Queue;
    Queue.push_back (EntryNode);

    Bitmap *Bit = new Bitmap (1024*1024);
    assert (Bit != NULL);

    std::map<DWORD, DWORD> Depth2Width;
    DWORD Depth = 0;
    DWORD Width = 1;
    
    DWORD NxtWidth = 0;
    DWORD Index = 0;
    
    Depth2Width[Depth] = Width;
    while (Width)
    {
        Index = 0;
        NxtWidth = 0;

        while (Index < Width)
        {
            DgNode *Node = Queue.back();
            Queue.pop_back();
            
            Bit->SetBit (Node->GetId (), 1);

            for (auto It = Node->OutEdgeBegin (), End = Node->OutEdgeEnd (); It != End; It++)
            {
                DgEdge* Edge = *It;
                if (Edge->GetAttr () & EA_CFG_DMY)
                {
                    continue;
                }
                
                DgNode* DstNode = Edge->GetDstNode ();
                if (Bit->CheckBit (DstNode->GetId ()))
                {
                    continue;
                }
                
                Queue.push_back (Edge->GetDstNode ());
                NxtWidth++;
            }

            Index++;
        }

        Depth++;
        Width = NxtWidth;
        
        Depth2Width[Depth] = Width;
    }

    FILE *F = fopen ("CFG_heavyweight", "w");
    assert (F != NULL);

    for (auto It = Depth2Width.begin(), End = Depth2Width.end(); It != End; It++)
    {
        fprintf (F, "%d\t%d\r\n", It->first, It->second);
    }

    fclose (F);
    delete Bit;

    return;
}


