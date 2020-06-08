//===- CallGraph.cpp -- Call graph used internally  -------------------------//
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

#include "llvmadpt/LlvmAdpt.h"
#include "callgraph/CallGraph.h"
#include "analysis/Dependence.h"
#include <llvm/Support/DOTGraphTraits.h>	 
#include <llvm/IR/InstIterator.h>	       

using namespace llvm;

T_CallSiteToIdMap CallGraph::m_CsToIdMap;
T_IdToCallSiteMap CallGraph::m_IdToCSMap;
DWORD CallGraph::m_CallSiteNum = 1;

VOID CallGraph::BuildCallGraph() 
{
    T_FunVector::iterator ItF, End;

    // nodes
    for (ItF = m_ModMange.func_begin(), End = m_ModMange.func_end(); ItF != End; ++ItF) 
    {     
        AddCgNode(*ItF);      
    }

    // edges
    for (ItF = m_ModMange.func_begin(), End = m_ModMange.func_end(); ItF != End; ++ItF) 
    {
        Function *Func = *ItF;

        if (llvmAdpt::IsDebugFunction (Func->getName ()))
        {
            continue;
        }

        for (inst_iterator ItI = inst_begin(*Func), Ed = inst_end(*Func); ItI != Ed; ++ItI) 
        {
            Instruction *Inst = &*ItI;
            
            if (llvmAdpt::IsCallSite(Inst) == false ||
                llvmAdpt::IsInstrinsicDbgInst(Inst) == true) 
            {
                continue;
            }

            if (Function* Callee = llvmAdpt::GetCallee (Inst))
            {
                AddDirectCgEdge(Inst);
            }
            else
            {
                ImmutableCallSite Cs(Inst);
                if (!Cs.isIndirectCall())
                {
                    continue;
                }

                std::vector<llvm::Function*> CalleeAry;
                llvmAdpt::GetIndirectCallee(Inst, CalleeAry);
                if (CalleeAry.size() == 0)
                {
                    m_FailCallsite.push_back(Inst);
                    continue;
                }

                for (auto it = CalleeAry.begin(), end = CalleeAry.end(); it != end; it++)
                {
                    Callee = llvmAdpt::GetFuncDef(*it);
                    if (Callee != NULL && (Callee->arg_size()+1) == Inst->getNumOperands ())
                    {
                        AddIndirectCgEdge(Inst, Callee);
                    }
                    else
                    {
                        m_FailCallsite.push_back(Inst);
                    } 
                }
            }
        }
    }


    DumpInfo ();
    return;
}


VOID CallGraph::LinkGraph()
{
    std::queue<CallGraphNode*> NodeQueue;

    CallGraphNode* Node = GetEntryNode();
    if (Node == NULL)
    {
        return;
    }
    NodeQueue.push(Node);

    while (NodeQueue.size())
    {
        Node = NodeQueue.front();
        NodeQueue.pop();
        
        Node->SetReachable(AF_TRUE);
        
        for (auto Egit = Node->OutEdgeBegin (), End = Node->OutEdgeEnd (); Egit != End; Egit++)
        {
            CallGraphEdge *CgEdge = *Egit;

            CallGraphNode* DstNode = CgEdge->GetDstNode ();
            if (!DstNode->IsReachable ())
            {
                NodeQueue.push(DstNode);
            }   
        }
    }

    return;
}

VOID CallGraph::DFSNode (DgGraph *Dg, CallGraphNode *Node)
{
    m_DfsNode.insert(Node);
    m_NodeStack.push_back(Node->GetFunction ());

    DWORD StackSize = m_NodeStack.size();

    if ((Node->IsLeaf () == AF_TRUE) || (StackSize >= m_PathDepth))
    {
        m_PathCount++;
        if (!(m_PathCount%1000))
        {
            printf ("1-UpdatePtsByFs [%-6u]\r", m_PathCount);
        }
        
        Dg->UpdatePtsByFs (m_NodeStack);
        #if 0
        printf ("[%u] path length = %u \r\n", m_PathCount, m_NodeStack.size());
        for (auto it = m_NodeStack.begin(); it != m_NodeStack.end(); it++)
        {
            Function *Func = *it;
            printf ("%s -> ", Func->getName().data());
        }
        printf (" end\r\n");
        #endif
    }
    else
    {
        for (auto Egit = Node->OutEdgeBegin (), End = Node->OutEdgeEnd (); Egit != End; Egit++)
        {
            CallGraphEdge *CgEdge = *Egit;
            if (CgEdge->GetAttr () & EA_CFG_DMY)
            {
                continue;
            }

            CallGraphNode* DstNode = CgEdge->GetDstNode ();
            
            Function *Func = DstNode->GetFunction ();
            if (Func->getInstructionCount() == 0)
            {
                continue;
            }
                
            if (m_DfsNode.find(DstNode) == m_DfsNode.end())
            {
                DFSNode (Dg, DstNode);
            }
        }
    }

    m_NodeStack.pop_back();
    m_DfsNode.erase(Node);
}


VOID CallGraph::UpdatePtsByFs(DgGraph *Dg)
{
    CallGraphNode* Node = GetEntryNode();
    if (Node == NULL)
    {
        return;
    }

    DEBUG ("Path depth to %u \r\n", m_PathDepth);
    
    DFSNode (Dg, Node);

    m_DfsNode.clear();

    printf ("1-UpdatePtsByFs [%-6u] paths\r\n", m_PathCount);
    return;
}


VOID CallGraph::DumpInfo ()
{
    FILE *f = fopen ("IndeirectFail.txt", "w");

    for (auto it = m_FailCallsite.begin(), end = m_FailCallsite.end(); it != end; it++)
    {
        Instruction *Inst = *it;
        Function *Func = Inst->getParent ()->getParent ();
        
        fprintf(f, "%s:%p \r\n", Func->getName().data(), Inst);
    }

    m_FailCallsite.clear();
    fclose(f);   
}


VOID CallGraph::AddCgNode(llvm::Function* Func) 
{
    DWORD Id = m_NodeNum;
    CallGraphNode* CgNode = new CallGraphNode(Id, Func);
    
    AddNode(Id, CgNode);
    
    m_FuncToCgNodeMap[Func] = CgNode;
}

CallGraphEdge* CallGraph::GetCgEdge(CallGraphNode* SrcNode, CallGraphNode* DstNode)
{
    CallGraphEdge::T_CgEdgeSet::iterator It, End;
    
    for (It = SrcNode->OutEdgeBegin(), End = SrcNode->OutEdgeEnd(); It != End; ++It) 
    {
        CallGraphEdge* edge = (*It);
        if (edge->GetDstID() == DstNode->GetId())
        {
            return edge;
        }
    }
            
    return NULL;
}

VOID CallGraph::AddDirectCgEdge(llvm::Instruction* CallInst) 
{  
    Function *Func = llvmAdpt::GetCallee(CallInst);
    if (Func == NULL)
    {
        errs()<<"In function " <<CallInst->getParent()->getParent()->getName()<<" get callee fail!!\n";
        return;
    }

    Function *CallerFunc = CallInst->getParent()->getParent();
    if (CallerFunc == Func)
    {
        return;
    }

    CallGraphNode* Caller = GetCgNode(CallerFunc);
    CallGraphNode* Callee = GetCgNode(Func);

    CallGraphEdge* CallEdge = GetCgEdge(Caller, Callee);
    if(CallEdge == NULL) 
    {
        CallEdge = new CallGraphEdge(Caller, Callee);
        AddEdge(CallEdge);
    }

    CallEdge->AddDirectCallSite(CallInst);
    AddCgEdgeSetMap(CallInst, CallEdge);

    if (Callee->GetFunction ()->getInstructionCount () != 0)
    {
        Caller->SetLeafFlag (AF_FALSE);
        Callee->AddInCallSite (CallInst);
    }

    Caller->AddOutCallSite (CallInst);

    return;
}

VOID CallGraph::AddIndirectCgEdge(llvm::Instruction* CallInst, llvm::Function* Calleefun)
{
    Function *CallerFunc = CallInst->getParent()->getParent();
    if (CallerFunc == Calleefun)
    {
        return;
    }
    
    CallGraphNode* Caller = GetCgNode(CallInst->getParent()->getParent());
    CallGraphNode* Callee = GetCgNode(Calleefun);

    CallGraphEdge* CallEdge = GetCgEdge(Caller, Callee);
    if(CallEdge == NULL)
    {
        CallEdge = new CallGraphEdge(Caller, Callee);
        AddEdge(CallEdge);   
    }

    CallEdge->AddInDirectCallSite(CallInst);
    AddCgEdgeSetMap(CallInst, CallEdge);

    if (Callee->GetFunction ()->getInstructionCount () != 0)
    {
        Caller->SetLeafFlag (AF_FALSE);
        Callee->AddInCallSite (CallInst);
    }

    Caller->AddOutCallSite (CallInst);

    return;
}

VOID CallGraph::DumpCgWeight()
{
    CallGraphNode *CgNode = GetEntryNode ();

    std::vector<CallGraphNode*> Queue;
    Queue.push_back (CgNode);

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
            CallGraphNode *Node = Queue.back();
            Queue.pop_back();
            
            Bit->SetBit (Node->GetId (), 1);

            for (auto It = Node->OutEdgeBegin (), End = Node->OutEdgeEnd (); It != End; It++)
            {
                CallGraphEdge* Edge = *It;
                
                CallGraphNode* DstNode = Edge->GetDstNode ();
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

    FILE *F = fopen ("CG_heavyweight", "w");
    assert (F != NULL);

    for (auto It = Depth2Width.begin(), End = Depth2Width.end(); It != End; It++)
    {
        fprintf (F, "%d\t%d\r\n", It->first, It->second);
    }

    fclose (F);
    delete Bit;

    return;
}



