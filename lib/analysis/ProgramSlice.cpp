//===- ProgramSlice.cpp --   for program slice    -----------------------------//
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
#include "analysis/ProgramSlice.h"

using namespace llvm;

bool ProgramSlice::IsForward (DgNode *Node)
{
    DWORD NodeId = Node->GetId ();
    llvm::Function *Func = Node->GetFunction ();
    
    auto it = m_FuncToMaxId.find (Func);
    if (it == m_FuncToMaxId.end() || NodeId > it->second)
    {
        m_FuncToMaxId[Func] = NodeId;
        return true;
    }
    else
    {
        return false;
    }
}


VOID ProgramSlice::ForwardDfs (DgNode *Node, DgNodeSet *Visited, DgNodeSet &Sinks)
{
    Visited->insert(Node);

    DEBUG ("%d ", Node->GetId ());
    for (auto Egit = Node->OutEdgeBegin (), End = Node->OutEdgeEnd (); Egit != End; Egit++)
    {
        DgEdge *Edge = *Egit;
        if (!(Edge->GetAttr () & EA_DD))
        {
            continue;
        }

        DgNode *DstNode = Edge->GetDstNode ();
        if (Visited->find(DstNode) != Visited->end())
        {
            continue;
        }

        if (!IsForward (DstNode) && (Edge->GetAttr () & EA_RET))
        {
            DEBUG ("[%d][reach backward] ", DstNode->GetId ());
            continue;
        }

        m_ForwardSlice.insert (DstNode);     
        m_FdFuncSet.insert (DstNode->GetFunction ());
        
        if(Sinks.find (DstNode) != Sinks.end()) 
        {
            m_Sinks.insert (DstNode);
            m_ReachType = REACH_PARITAL;
            DEBUG ("<%d> ", DstNode->GetId ());
            break;
        }
                
        ForwardDfs (DstNode, Visited, Sinks);
    }
    

    //Visited->erase(Node);
}

/* forward slicing based on ddg */
VOID ProgramSlice::ForwardBfs(DgNodeSet &Sinks) 
{
    DgNodeSet Visited;
    DgNode *CurNode;

    DEBUG("ForwardTraverse: %d ---> ", m_Root->GetId ());
    
    m_Queue.InQueue (m_Root);
    do
    {
        CurNode = m_Queue.OutQueue ();
        DEBUG("%d ", CurNode->GetId ());

        if (Visited.find (CurNode) != Visited.end())
        {
            continue;
        }
        else
        {
            Visited.insert (CurNode);
        }

        /* forward */
        m_ForwardSlice.insert (CurNode);
        if(Sinks.find (CurNode) != Sinks.end()) 
        {
            m_Sinks.insert (CurNode);
            m_ReachType = REACH_PARITAL;
            break;
        }

        /* iterate all children node */
        for (auto it = CurNode->OutEdgeBegin (), end = CurNode->OutEdgeEnd ();
            it != end; it++)
        {
            DgEdge *Edge = *it;
            if (!(Edge->GetAttr () & EA_DD))
            {
                continue;
            }
            
            m_Queue.InQueue (Edge->GetDstNode ());
        }
      
    }while (!m_Queue.IsEmpty ());
    DEBUG("\r\n"); 

    return;
}


/* forward slicing based on ddg */
VOID ProgramSlice::ForwardTraverse(DgNodeSet &Sinks) 
{
    DgNodeSet Visited;

    DEBUG ("ForwardTraverse: %d ---> ", m_Root->GetId ());
    
    ForwardDfs (m_Root, &Visited, Sinks);

    DEBUG ("\r\n");

    return;
}


VOID ProgramSlice::BackwardDfs (DgNode *Node, DgNodeSet *Path)
{
    Path->insert(Node);
    
    for (auto Egit = Node->InEdgeBegin (), End = Node->InEdgeEnd (); Egit != End; Egit++)
    {
        DgEdge *Edge = *Egit;
        if (!(Edge->GetAttr () & EA_DD))
        {
            continue;
        }

        DgNode *SrcNode = Edge->GetSrcNode ();
        if (Node->GetFunction () != SrcNode->GetFunction ())
        {
            m_PathFuncSet.insert (SrcNode->GetFunction ());
        }
        
        if (SrcNode == m_Root || m_BackwardSlice.find (SrcNode) != m_BackwardSlice.end())
        {
            /* update function */
            for (auto It = m_PathFuncSet.begin (), End = m_PathFuncSet.end(); It != End; It++)
            {
                m_BdFuncSet.insert (*It);
            }

            /* update backward slice */
            for (auto It = Path->begin (), End = Path->end(); It != End; It++)
            {
                DgNode *N = *It;
                
                DEBUG("%d ", N->GetId ());
                m_BackwardSlice.insert (N);
            }
            m_BackwardSlice.insert (SrcNode);
            DEBUG("<%d> \r\n", SrcNode->GetId ());

            break;
        }

        if (Path->find(SrcNode) != Path->end())
        {
            continue;
        }

        if (m_ForwardSlice.find (SrcNode) == m_ForwardSlice.end())
        {
            continue;
        }


        if (m_FdFuncSet.find (SrcNode->GetFunction ()) == m_FdFuncSet.end())
        {
            continue;        
        }   
                
        BackwardDfs (SrcNode, Path);
    }

    //Path->erase(Node);
    //m_PathFuncSet.erase (Node->GetFunction ());

    return;
}


VOID ProgramSlice::BackwardTraverse(DgNode *Node) 
{
    DgNodeSet Path;

    DEBUG("Backforward slicing: "); 

    m_PathFuncSet.insert (Node->GetFunction ());
    BackwardDfs (Node, &Path);

    return;
}


DWORD ProgramSlice::VisitEdgeType (DgNode *Node, DgNodeSet *Path)
{
    if (!llvm::isa<llvm::CallInst>(Node->GetInst ()))
    {
        return EA_CFG;
    }

    DWORD EdgeType = 0;
    for (auto Egit = Node->OutEdgeBegin (), End = Node->OutEdgeEnd (); Egit != End; Egit++)
    {
        DgEdge *Edge = *Egit;
        if (!(Edge->GetAttr () & EA_CFG))
        {
            continue;
        }

        EdgeType |= Edge->GetAttr ();      
        DgNode *DstNode = Edge->GetDstNode ();
        if ((Edge->GetAttr () & EA_CALL) &&  m_BdFuncSet.find (DstNode->GetFunction ()) != m_BdFuncSet.end())
        {
            if (Path->find(DstNode) == Path->end())
            {
                return (EA_CALL);
        }   }
    }
    
    if (EdgeType & EA_CFG_DMY)
    {
        return (EA_CFG_DMY);
    }
    else
    {
        return EA_CFG;
    }
}

bool ProgramSlice::IsExit (DgNode *DstNode)
{
    llvm::Instruction *Inst = DstNode->GetInst ();
    if (!llvmAdpt::IsCallSite (Inst))
    {
        return false;
    }

    llvm::Function *Func = llvmAdpt::GetCallee (Inst);
    if (Func == NULL)
    {
        return false;
    }

    return (strcmp (Func->getName ().data(), "exit") == 0);              
}

DgNode* ProgramSlice::GetPredom (DgNode *Node)
{
    for (auto Egit = Node->InEdgeBegin (), End = Node->InEdgeEnd (); Egit != End; Egit++)
    {
        DgEdge *Edge = *Egit;
        if (!(Edge->GetAttr () & EA_DD))
        {
            continue;
        }

        return Edge->GetSrcNode ();
    }

    return NULL;
}


bool ProgramSlice::IsDeBranch (DgNodeSet *Path)
{
    DgNode *PreNode;
    DWORD BranchNum = 0;
    for (auto It = Path->begin(), End = Path->end(); It != End; It++)
    {
        DgNode *Node = *It;
        if (!llvmAdpt::IsBranchInst (Node->GetInst ()))
        {
            continue;
        }

        BranchNum++;
        DEBUG ("Branch: %d\r\n", Node->GetId ());

        if (m_ForwardSlice.find (Node) == m_ForwardSlice.end())
        {
            return false;
        }

        PreNode = GetPredom (Node);
        if (PreNode != NULL && llvmAdpt::IsCmpInst (PreNode->GetInst ()))
        {
            llvm::Instruction *Inst = PreNode->GetInst ();
            Constant *Const0 = dyn_cast<Constant>(Inst->getOperand(0));
            Constant *Const1 = dyn_cast<Constant>(Inst->getOperand(1));

            if (!(Const0 != NULL && isa<ConstantPointerNull>(Const0) && llvmAdpt::IsValuePtrType (Inst->getOperand(1))) &&
                !(Const1 != NULL && isa<ConstantPointerNull>(Const1)&& llvmAdpt::IsValuePtrType (Inst->getOperand(0))))
            {
                return false;
            }
        }
    }

    DEBUG ("BranchNum == %d\r\n", BranchNum);

    if (BranchNum == 0)
    {
        return false;
    }

    return true;
}


bool ProgramSlice::IsPathValid (DgNode *SinkNode, DgNodeSet *Path)
{
    DgNode *CurNode;
    ComQueue<DgNode *> Queue;

    DEBUG ("\r\n\tChecking Valid->backward slicing:");
    Queue.InQueue (SinkNode);
    do
    {
        CurNode = Queue.OutQueue ();
        DEBUG ("%d ", CurNode->GetId ());

        /* iterate all children node */
        for (auto it = CurNode->InEdgeBegin (), end = CurNode->InEdgeEnd ();
            it != end; it++)
        {
            DgEdge *Edge = *it;
            if (!(Edge->GetAttr () & EA_DD))
            {
                continue;
            }

            DgNode *SrcNode = Edge->GetSrcNode ();
            if (Path->find (SrcNode) == Path->end() &&
                !llvmAdpt::IsCallSite (SrcNode->GetInst ()))
            {
                continue;
            }

            if (m_BackwardSlice.find (SrcNode) == m_BackwardSlice.end())
            {
                continue;
            }

            if (SrcNode == m_Root)
            {
                DEBUG ("<%d> check ok!\r\n", SrcNode->GetId ());
                return true;
            }
            
            Queue.InQueue (SrcNode);
        }
      
    }while (!Queue.IsEmpty ());

    DEBUG (" -> slice abort, start check branch dependence \r\n");
    
    return IsDeBranch (Path);
}

bool ProgramSlice::IsRetContext (DgNodeSet *Path, DgNode *DstNode)
{

    for (auto it = DstNode->InEdgeBegin (), end = DstNode->InEdgeEnd (); it != end; it++)
    {
        DgEdge *InEdge = *it;
        if (!(InEdge->GetAttr () & EA_CFG) ||
            (InEdge->GetAttr () & EA_RET))
        {
            continue;
        }

        if (m_CrossFuncNum != 0 && Path->find (InEdge->GetSrcNode ()) == Path->end())
        {
            return false;
        }
    }

    return true;              
}


VOID ProgramSlice::CfgPathDfs (DgNode *Node, DgNodeSet *Path)
{
    if (IsExit (Node))
    {
        return;
    }
    
    Path->insert (Node);

    DWORD EdgeType = VisitEdgeType (Node, Path);
    DEBUG ("%d ", Node->GetId ());
    
    for (auto Egit = Node->OutEdgeBegin (), End = Node->OutEdgeEnd (); Egit != End; Egit++)
    {
        DgEdge *Edge = *Egit;
        if (!(Edge->GetAttr () & EA_CFG))
        {
            continue;
        }

        DgNode *DstNode = Edge->GetDstNode ();
        if (Path->find(DstNode) != Path->end())
        {
            continue;
        }

         if (!(Edge->GetAttr () & EdgeType))
         {
            continue;
         } 
        
        /* has no outgoing edge */
        if (DstNode->GetOutgoingEdgeNum () == 0)
        {
            m_ReachOut = true;//IsDeBranch (Path);
            DEBUG (" <%d->ReachOut>-[%s]", DstNode->GetId (), DstNode->GetFunction ()->getName().data()); 
            break;
        }

        if (Edge->GetAttr () & EA_RET)
        {
            /* reach out */
            if (m_FdFuncSet.find (DstNode->GetFunction ()) == m_FdFuncSet.end())
            {
                m_ReachOut = true;
                DEBUG (" <%d->ret-ReachOut>-[%s]", DstNode->GetId (), DstNode->GetFunction ()->getName().data());

                continue;
            }

            if (!IsRetContext (Path, DstNode))
            {
                DEBUG (" [%d]->[%d]context invalid. ", Node->GetId(), DstNode->GetId ());
                continue;
            }

            m_CrossFuncNum++;
        }

        /* reach the sink, check valid */
        if (m_Sinks.find (DstNode) != m_Sinks.end())
        {
            m_ReachOut = !IsPathValid (DstNode, Path);
            DEBUG ("\tReach sink <%d> -> IsPathValid==%d>\r\n", DstNode->GetId (), !m_ReachOut);
            continue;
        }
                
        CfgPathDfs (DstNode, Path);
        if (m_ReachOut)
        {
            break;
        }
    }
    
    Path->erase(Node);
}


VOID ProgramSlice::ComputePathReachable() 
{
    DgNodeSet Path;

    #ifdef DEBUG_MOD
    DEBUG("Backforward functions: ");
    for (auto it = m_BdFuncSet.begin(); it != m_BdFuncSet.end(); it++)
    {
        DEBUG("%s ", (*it)->getName().data());
    }
    DEBUG("\r\n");
    #endif

    m_ReachOut = false;
    m_CrossFuncNum = 0;
    
    DEBUG ("Path: <%d> -> ", m_Root->GetId ());
    CfgPathDfs (m_Root, &Path);
    DEBUG("\r\n");

    if (m_ReachOut == false)
    {
        /* be reached through all branchs */
        m_ReachType = REACH_ALL;
    }
    
    return;
}

VOID ProgramSlice::RunSlicing(DgNodeSet &SinksSet)
{
    DWORD SinkNo = 0;
    
    /* 1. forward traverse by data flow, determine partial reachability */
    ForwardTraverse(SinksSet);
    
    /* 2. backward traverse by control flow, identify branch pass */
    for (auto it = m_Sinks.begin(), end = m_Sinks.end(); it != end; ++it) 
    {
        DgNode *Sink = *it;

        BackwardTraverse (*it);
    }

    /* 3. compute all path reachability */
    DEBUG ("Sinks.size = %u \r\n", (DWORD)m_Sinks.size());
    if (m_Sinks.size() != 0)
    {
        ComputePathReachable ();
    }
    else
    {
        m_ReachType = REACH_NONE;
    }

    return;
}



