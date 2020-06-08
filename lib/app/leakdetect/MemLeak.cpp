//===- MemLeak.cpp -- Memory leak detector ------------------------------//
//
//                     OAF: Optimiazed Analysis Framework
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
#include "app/leakdetect/MemLeak.h"

bool MemLeak::IsNormalCheck(llvm::Function *CallFunc)
{
    if (llvmAdpt::GetBasicBlockNum(CallFunc) >= 6)
    {
        return false;
    }

    CallGraph *Cg = m_DgGraph->GetCallGraph ();
    T_CallciteSet* OutCsSet = Cg->GetCgNode (CallFunc)->GetOutCallSite ();
    assert (OutCsSet != NULL);

    DWORD MallocNum = 0;
    for (auto It = OutCsSet->begin(), End = OutCsSet->end(); It != End; It++)
    {
        llvm::Instruction* CallInst = *It;
        llvm::Function *Callee = llvmAdpt::GetCallee(CallInst);
        if (Callee != NULL && IsSourceFunc (Callee))
        {
            MallocNum++;
        }
    }

    return (MallocNum == 1);
}


bool MemLeak::IsSrcInAWrapper(DgNode *Root, ComQueue<DgNode *>* SrcQueue) 
{
    DgNode *Node;
    DgNode *RetNode;
    bool IsReachExit = false;

    DgNodeSet Visited;
    
    ComQueue<DgNode *> Queue;
    Queue.InQueue(Root);

    llvm::Function *CurFunc = Root->GetFunction ();
    if (!IsNormalCheck(CurFunc))
    {
        return false;
    }

    while (!Queue.IsEmpty ())
    {
        Node  = Queue.OutQueue();
        if (Visited.find (Node) != Visited.end())
        {
            continue;
        }
        else
        {
            Visited.insert (Node);
        }

        for (auto it = Node->OutEdgeBegin(), end = Node->OutEdgeEnd(); it != end; ++it) 
        {
            DgEdge* Edge = (*it);

            /* follow the data flow */
            if (!(Edge->GetAttr () & EA_DD))
            {
                 continue;
            }
     
            DgNode *DstNode = Edge->GetDstNode ();
            llvm::Instruction *Inst = DstNode->GetInst();
            
            /* reach ret */
            if (llvm::isa<llvm::ReturnInst>(Inst))
            {
                if (Inst->getNumOperands () == 0)
                {
                    return false;
                }
                IsReachExit = true;
                RetNode = DstNode;
                break;
            }
            else
            {
                /* internal */
                if (Inst->getParent ()->getParent () == CurFunc)
                {
                    Queue.InQueue(DstNode);
                }
            }
        }
    }

    if (!IsReachExit)
    {
        return false;
    }

    /* add callsite node */
    for (auto cit = RetNode->OutEdgeBegin(), cend = RetNode->OutEdgeEnd(); cit != cend; ++cit)
    {
        DgEdge* Edge = (*cit);
        if (!(Edge->GetAttr () & EA_RET) || !(Edge->GetAttr () & EA_DD))
        {
             continue;
        }

        DgNode *Dst = Edge->GetDstNode ();        
        SrcQueue->InQueue(Dst);
    }

    return true;
}


VOID MemLeak::CollectSources()
{
    CallGraph *Cg = m_DgGraph->GetCallGraph ();
    T_CallSiteToIdMap::iterator It, End;
    T_CallSitePair CsPair;
    DgNodeSet Visited;

    for (auto It = Cg->CItoIdBegin (), End = Cg->CItoIdEnd (); It != End; It++)
    {
        CsPair = It->first;
        
        llvm::CallSite Cs = CsPair.first;
        llvm::Function* Func = (llvm::Function*)CsPair.second;

        if (!IsSourceFunc (Func))
        {
            continue;
        }

        llvm::Instruction *Inst = Cs.getInstruction();
        
        DgNode *Node = m_DgGraph->GetDgNode (Cs.getInstruction());
        if (Node == NULL)
        {
            continue;
        }

        if (Visited.find (Node) != Visited.end())
        {
            continue;
        }
        else
        {
            Visited.insert (Node);
        }

        DEBUG("Source like function: %s -> %s\r\n", Func->getName().data(), llvmAdpt::GetSourceLoc(Inst).c_str());

        ComQueue<DgNode *> Queue;
        Queue.InQueue(Node);
        do
        {
            Node = Queue.OutQueue ();
            if (!IsSrcInAWrapper (Node, &Queue) && m_SrcSet.insert (Node).second)
            {
                DEBUG("==> Add Source: %d -> %s\r\n", \
                      Node->GetId (), llvmAdpt::GetSourceLoc(Node->GetInst ()).c_str());
            }
        }while (!Queue.IsEmpty ());     
    }
}


VOID MemLeak::CollectSinks()
{
    CallGraph *Cg = m_DgGraph->GetCallGraph ();
    T_CallSiteToIdMap::iterator It, End;
    T_CallSitePair CsPair;

    for (It = Cg->CItoIdBegin (), End = Cg->CItoIdEnd (); It != End; It++)
    {
        CsPair = It->first;
        
        llvm::CallSite Cs = CsPair.first;
        llvm::Function* Func = (llvm::Function*)CsPair.second;

        if (!IsSinkFunc (Func))
        {
            continue;
        }

        DgNode *Node = m_DgGraph->GetDgNode (Cs.getInstruction());
        if (Node == NULL)
        {
            continue;
        }

        
        DEBUG("==> Add Sink: %d -> %s\r\n", Node->GetId (), llvmAdpt::GetSourceLoc(Node->GetInst ()).c_str());
        m_SinkSet.insert (Node);
    }
}
    


VOID MemLeak::ReportBug(ProgramSlice *PgSlice, DgNode *Source)
{ 
    string BugInfo = llvmAdpt::GetSourceLoc(Source->GetInst ());
    
    switch (PgSlice->Reachability())
    {
        case REACH_NONE:
        {  
            printf ("%s memory allocation at : %s\r\n", ErrMsg("\t NeverFree :").c_str(), BugInfo.c_str());
            InsertCheckResult (BugInfo, REACH_NONE);
            break;
        }
        case REACH_PARITAL:
        {
            printf ("%s memory allocation at : %s\r\n", WarnMsg("\t Partial Free :").c_str(), BugInfo.c_str());
            InsertCheckResult (BugInfo, REACH_PARITAL);
            break;
        }
        case REACH_ALL:
        {
            DEBUG("%s memory allocation at : %s\r\n", LightMsg("\t REACH_ALL :").c_str(), BugInfo.c_str());
            InsertCheckResult (BugInfo, REACH_ALL);
            return;
        }
        default:
        {
            assert (0);
        }
    }
    
    return;
}


DWORD MemLeak::RunDetector ()
{
    /* 1. collect source and sinks */
    CollectSources();
    CollectSinks();

    /* 2. start analysis by each source */
    auto end = m_SrcSet.end();
    for (auto it = m_SrcSet.begin(); it != end; ++it) 
    {
        DgNode *Source = *it;

        ProgramSlice PgSlice (Source, m_DgGraph);
        PgSlice.RunSlicing(m_SinkSet);
    

        ReportBug(&PgSlice, Source);
    }
    
    return AF_SUCCESS;
}



