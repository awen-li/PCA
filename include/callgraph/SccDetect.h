//===- SccDetect.h -- SCC detection algorithm-----------------------------------//
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
#ifndef _SCCDETECT_H_
#define _SCCDETECT_H_

#include "common/BasicMacro.h"
#include <llvm/ADT/GraphTraits.h>
#include <llvm/ADT/SparseBitVector.h>
#include <llvm/ADT/DenseMap.h>
#include "callgraph/GraphTraits.h"


template<class GraphType> class SccDetector
{
public:
    typedef AfGraphTraits<GraphType> SccGraphTraits;
    typedef typename SccGraphTraits::NodeType NodeType;

private:
    std::stack<NodeType*> m_SccStack;
    llvm::DenseMap<NodeType *, DWORD> m_DfsNdMap;
    llvm::DenseSet<NodeType *> m_InComponent;
    DWORD m_TimeStamp;

public:
    SccDetector() : m_TimeStamp(0) 
    {
        Reset ();
    }

    virtual VOID RunDectect() = 0;

private:

    VOID Reset ()
    {
        while(!m_SccStack.empty())
        {
            m_SccStack.pop();
        }
        m_DfsNdMap.clear();
        m_InComponent.clear();

        return;
    }

    inline VOID Dectect(NodeType *Node) 
    {
        DWORD CurTimeStamp = m_TimeStamp++;
        assert(!m_DfsNdMap.count(Node) && "Revisit the same node again?");
        
        m_DfsNdMap[Node] = CurTimeStamp;

        for (auto childItr = SccGraphTraits::child_begin(Node),
                  childIte = SccGraphTraits::child_end(Node); childItr != childIte; ++childItr)
        {
            NodeType *SuccRep = GetRepNode(*childItr);
            if (!m_DfsNdMap.count(SuccRep))
            {
                Dectect(SuccRep);
            }

            if (!m_InComponent.count(SuccRep) && (m_DfsNdMap[Node] > m_DfsNdMap[SuccRep]))
            {
                m_DfsNdMap[Node] = m_DfsNdMap[SuccRep];
            }
        }

        if (CurTimeStamp != m_DfsNdMap[Node]) 
        {
            m_SccStack.push(Node);
            return;
        }

        m_InComponent.insert(Node);
        while (!m_SccStack.empty()) 
        {
            NodeType *CycleNode = m_SccStack.top();
            if (m_DfsNdMap[CycleNode] < CurTimeStamp)
            {
                break;
            }

            ProcNodeOnCycle(CycleNode, Node);
            m_InComponent.insert(CycleNode);
            m_SccStack.pop();
        }

        ProcRepNodeOnCycle(Node);
    }

protected:
    /* Nodes may get merged during the analysis. This function returns the merge target */
    virtual NodeType *GetRepNode(DWORD Id) = 0;
  
    virtual VOID ProcNodeOnCycle(const NodeType *Node, const NodeType *RepNode) = 0;  
    virtual VOID ProcRepNodeOnCycle(const NodeType *Node) = 0;

    inline VOID RunOnGraph(GraphType *Graph) 
    {
        for (auto itr = SccGraphTraits::node_begin(Graph),
                  ite = SccGraphTraits::node_end(Graph); itr != ite; itr++)
        {
            NodeType *repNode = GetRepNode(itr->first);
            if (!m_DfsNdMap.count(repNode))
            {
                Dectect(repNode);
            }
        }

        assert(m_SccStack.empty() && "sccStack not empty after cycle detection!");
        Reset ();
    }

    // Running the cycle detection algorithm on a given graph node. This function
    // is used when walking through the entire graph is not the desirable behavior.
    VOID RunOnNode(DWORD node) 
    {
        assert(m_SccStack.empty() && "sccStack is not empty before cycle detection!");

        NodeType *repNode = GetRepNode(node);
        if (!m_DfsNdMap.count(repNode))
        {
            Dectect(repNode);
        }

        assert(m_SccStack.empty() && "sccStack not empty after cycle detection!");
    }
};

#endif 
