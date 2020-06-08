//===- CycleDetect.h -- Cycle Detect ---------------------------------------//
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

#ifndef _CYCLEDECTECT_H_
#define _CYCLEDECTECT_H_
#include "callgraph/SccDetect.h"
#include "callgraph/SbvGraph.h"
#include "analysis/points-to/ConstraintGraph.h"
#include "common/Stat.h"

class OfflineCycleDetector : public SccDetector<ConstraintGraph>
{
private:

    ConstraintGraph *m_CstGraph; 
    llvm::DenseMap<DWORD, DWORD> m_MergeMap; 
    llvm::SparseBitVector<> m_SccNodes;

public:
    OfflineCycleDetector (ConstraintGraph *CstGraph)
    {
        m_CstGraph = CstGraph;
    }
    
private:

    ConstraintNode *GetRepNode(DWORD Id) override 
    {
        return m_CstGraph->GetGNode(Id);
    }

    VOID ProcNodeOnCycle(const ConstraintNode *Node, const ConstraintNode *RepNode) override 
    {
        m_SccNodes.set(Node->GetId());
    }

    VOID ProcRepNodeOnCycle(const ConstraintNode *Node) override 
    {
        if (m_SccNodes.count() == 0)
        {
            return;
        }
        
        m_SccNodes.set(Node->GetId());

        //printf ("Cycle: \r\n");
        DWORD RepNode = m_SccNodes.find_first();
        for (auto itr = ++m_SccNodes.begin(), ite = m_SccNodes.end(); itr != ite; ++itr) 
        {
            DWORD cycleNode = *itr;
            m_MergeMap[cycleNode] = RepNode;

            //printf (" id = %d, rep=%d \r\n", cycleNode, RepNode);
        }
        //printf ("\r\n ");

        m_SccNodes.clear();
    }

    VOID RunMerge ()
    {
        for (auto const &MapNodes : m_MergeMap)
        {
            m_CstGraph->MergeNode (MapNodes.first, MapNodes.second);
        }

        Stat::IncStatNum ("MergeNodes", m_MergeMap.size());

        m_MergeMap.clear();
    }

public:

    VOID RunDectect() override 
    {
        RunOnGraph(m_CstGraph);

        RunMerge();

        return;
    }
};



class OnlineCycleDetector : public SccDetector<ConstraintGraph> 
{
private:

    ConstraintGraph *m_CstGraph;
    llvm::DenseMap<DWORD, DWORD> m_MergeMap;
    llvm::SparseBitVector<> m_SccNodes;
    llvm::DenseSet<DWORD> m_Candiates;
    llvm::DenseMap<DWORD, DWORD> m_VisitedEdge;

    DWORD m_CandiateNum;
    DWORD m_MergeNum;

    NodeType *GetRepNode(DWORD Idx) override 
    {
        DWORD TgtId = m_CstGraph->GetMergeTarget(Idx);
        
        return m_CstGraph->GetGNode(TgtId);
    }

    inline VOID CollapseNodes()
    {
        if (m_MergeNum == 0)
        {
            return;
        }
        
        for (auto const &MapNodes : m_MergeMap)
        {
            m_CstGraph->MergeNode (MapNodes.first, MapNodes.second);
        }

        Stat::IncStatNum ("MergeNodes", m_MergeNum);

        m_MergeMap.clear();
        m_MergeNum = 0;

        return;
    }
    
    inline VOID ProcNodeOnCycle(const NodeType *Node, const NodeType *RepNode) override 
    {
        DWORD NodeId = m_CstGraph->GetMergeTarget(Node->GetId());
        m_SccNodes.set(NodeId);
    }
    
    inline VOID ProcRepNodeOnCycle(const NodeType *Node) override 
    {
        if (m_SccNodes.count() == 0)
        {
            return;
        }

        DWORD NodeId = m_CstGraph->GetMergeTarget(Node->GetId());
        m_SccNodes.set(NodeId);

        DWORD RepNode = m_SccNodes.find_first();
        for (auto itr = ++m_SccNodes.begin(), ite = m_SccNodes.end(); itr != ite; ++itr) 
        {
            DWORD cycleNode = *itr;
            m_MergeMap[cycleNode] = RepNode;

            m_MergeNum++;
        }

        m_SccNodes.clear();
        return;
    }

public:
    OnlineCycleDetector(ConstraintGraph *CstGraph):m_CstGraph(CstGraph)
    {
        m_CandiateNum = 0;
        m_MergeNum    = 0;
    }

    inline VOID RunDectect() override 
    {
        if (m_CandiateNum == 0)
        {
            return;
        }
        
        for (auto Node : m_Candiates)
        {
            RunOnNode(Node);
        }

        CollapseNodes();
        m_Candiates.clear ();
        m_CandiateNum = 0;
    }

    inline VOID SetCandiate (DWORD Src, DWORD Dst)
    {       
        m_Candiates.insert (Dst);
        m_CandiateNum++;
    }
};


#endif 
