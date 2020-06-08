//===- SbvGraph.h --Sparse bitVector graph ----------------------------------//
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

#ifndef _SBVGRAPH_H_
#define _SBVGRAPH_H_

#include <algorithm>
#include <unordered_map>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SparseBitVector.h>
#include "callgraph/GenericGraph.h"
#include "callgraph/GraphTraits.h"


class SbvGraphNode;

class SbvGraphEdge: public GenericEdge<SbvGraphNode>
{

public:
    SbvGraphEdge (SbvGraphNode* s, SbvGraphNode* d):GenericEdge<SbvGraphNode>(s, d)  
    {
    }

    ~SbvGraphEdge ()
    {
    }
};


// The node of a graph class where successor edges are represented by sparse bit vectors
class SbvGraphNode: public GenericNode<SbvGraphEdge> 
{
protected:
    llvm::SparseBitVector<> m_Successors;

public:

    SbvGraphNode(DWORD Id):GenericNode<SbvGraphEdge>(Id) { }

    using iterator = llvm::SparseBitVector<>::iterator;

    iterator SuccBegin() const 
    {
        return m_Successors.begin(); 
    }
    
    iterator SuccEnd() const 
    { 
        return m_Successors.end(); 
    }

    inline DWORD GetSuccNum() const 
    { 
        return m_Successors.count(); 
    }

    inline VOID SetSuccessor (DWORD Succ)
    {
        m_Successors.set(Succ);
    }

    // src's successors += dst's successors
    inline VOID MergeSuccessor (SbvGraphNode *Succ)
    {
        m_Successors |= Succ->m_Successors;
    }
};


// A graph class where successor edges are represented by sparse bit vectors
class SbvGraph: public GenericGraph<SbvGraphNode, SbvGraphEdge>  
{
public:
    SbvGraph() {}

    SbvGraphNode *GetOrAddSbvNode(DWORD Id) 
    {
        SbvGraphNode *SbvNode = GetGNode (Id);
        if (SbvNode == NULL)
        {
            SbvNode = new SbvGraphNode (Id);
            AddNode (Id, SbvNode);
        }
        
        return SbvNode;
    }

    VOID AddSbvEdge(DWORD Src, DWORD Dst) 
    {
        SbvGraphNode *SrcNd = GetOrAddSbvNode (Src);
        SbvGraphNode *DstNd = GetOrAddSbvNode (Dst);
        
        SbvGraphEdge *SbvEdge = new SbvGraphEdge (SrcNd, DstNd);
        AddEdge (SbvEdge);

        SrcNd->SetSuccessor(Dst);

        return;        
    }

    VOID MergeEdge(DWORD Src, DWORD Dst) 
    {   
        SbvGraphNode *DstNd = GetGNode (Dst);
        if (DstNd == NULL)
        {
            return;
        }

        SbvGraphNode *SrcNd = GetOrAddSbvNode (Src);

        SrcNd->MergeSuccessor (DstNd);
        return;
    }
};


template <> class AfGraphTraits<SbvGraph> 
{
public:
    typedef SbvGraphNode NodeType;
    typedef typename llvm::DenseMap<DWORD, NodeType*>::iterator NodeIterator;
    typedef SbvGraphNode::iterator ChildIterator;

    static inline ChildIterator child_begin(NodeType *Node) 
    { 
        return Node->SuccBegin (); 
    }
    
    static inline ChildIterator child_end(NodeType *Node) 
    { 
        return Node->SuccEnd ();
    }

    static inline NodeIterator node_begin(SbvGraph *Graph) 
    {
        return Graph->begin();
    }
    
    static inline NodeIterator node_end(SbvGraph *Graph) 
    {
        return Graph->end();
    }
};


#endif
