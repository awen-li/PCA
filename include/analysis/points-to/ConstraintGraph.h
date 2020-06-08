//===- ConstraintGraph.h -- ConstraintGraph for anderson algorithm ------------//
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


#ifndef _CONSTRAINTGRAPH_H_
#define _CONSTRAINTGRAPH_H_

#include <llvm/IR/Value.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/GraphTraits.h>
#include <llvm/ADT/STLExtras.h>	
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SparseBitVector.h>
#include "callgraph/GenericGraph.h"
#include "callgraph/GraphTraits.h"
#include "analysis/points-to/Constraint.h"
#include "common/Bitmap.h"
#include "common/WorkList.h"


using namespace llvm;

class PtsSet 
{
private:
  llvm::SparseBitVector<> m_PtsToVec;

public:
  using iterator = llvm::SparseBitVector<>::iterator;

  inline bool IsPtsTo(DWORD Id) 
    { 
        return m_PtsToVec.test(Id); 
    }

    bool Insert(DWORD Id) 
    { 
        return m_PtsToVec.test_and_set(Id); 
    }

    SparseBitVector<>& Data ()
    {
        return m_PtsToVec;
    }

    bool Contains(PtsSet& Pts) 
    {
        return m_PtsToVec.contains(Pts.Data());
    }

    bool Intersect(PtsSet& Pts) 
    {
        return m_PtsToVec.intersects(Pts.Data());
    }

    bool Union(PtsSet& Pts) 
    { 
        return m_PtsToVec |= Pts.Data(); 
    }

    DWORD GetSize() const 
    {
        return m_PtsToVec.count(); 
    }
    
    bool IsEmpty()
    {
        return m_PtsToVec.empty();
    }

    bool operator==(PtsSet &Other) const 
    {
        return m_PtsToVec == Other.Data();
    }

    void Sub(PtsSet &Other) 
    {
        m_PtsToVec = m_PtsToVec - Other.Data();
    }

    iterator begin() 
    { 
        return m_PtsToVec.begin(); 
    }
    
    iterator end()
    { 
        return m_PtsToVec.end();
    }
};


enum 
{
    UniversalPtr = 0,
    UniversalObj = 1,
    NullPtr      = 2,
    NullObject   = 3,
    NumberSpecialNodes
};


class ConstraintNode;
class ConstraintEdge: public GenericEdge<ConstraintNode> 
{
public:
    enum CstEgType 
    {
        E_ADDR_OF=8, E_COPY, E_STORE, E_LOAD, E_NORMALGEP, E_VARGEP
    };

    ConstraintEdge(ConstraintNode* s, ConstraintNode* d, DWORD Type):\
                      GenericEdge<ConstraintNode>(s, d, Type)                       
    {
    }
    
    ~ConstraintEdge ()
    {
    }

    typedef GenericNode<ConstraintEdge>::T_GEdgeSet T_ConstraintEdgeSet;
};

class ConstraintNode: public GenericNode<ConstraintEdge> 
{
public:
typedef enum {E_VALUE, E_OBJECT} NodeTy;
typedef ConstraintEdge::T_ConstraintEdgeSet::iterator eg_iterator;


private:
    llvm::Value *m_Val;
    NodeTy m_Type;
    DWORD  m_MergeTarget;
    llvm::SparseBitVector<> m_Successors;

    PtsSet m_PtsSet;

    ConstraintEdge::T_ConstraintEdgeSet m_OutLoadEdgeSet;
    ConstraintEdge::T_ConstraintEdgeSet m_InStoreEdgeSet;
    ConstraintEdge::T_ConstraintEdgeSet m_OutCopyEdgeSet;

public:
    ConstraintNode (DWORD Id, NodeTy Type, llvm::Value *Val):GenericNode<ConstraintEdge>(Id)
    {
        m_Val         = Val;
        m_Type        = Type;
        m_MergeTarget = Id;
    }

    ~ConstraintNode ()
    {
    }

    using iterator = llvm::SparseBitVector<>::iterator;

    inline VOID ClearMem ()
    {
        m_Successors.clear();
        m_OutLoadEdgeSet.clear();
        m_InStoreEdgeSet.clear();
        m_OutCopyEdgeSet.clear();
    }
    
    iterator SuccBegin() const 
    {
        return m_Successors.begin(); 
    }
    
    iterator SuccEnd() const 
    { 
        return m_Successors.end(); 
    }

    inline VOID SetSuccessor (DWORD Succ)
    {
        m_Successors.set(Succ);
    }

    inline VOID MergeSuccessor (ConstraintNode *Succ)
    {
        m_Successors |= Succ->m_Successors;
    }

    inline VOID SetPointsTo(DWORD NodeId)
    {
        m_PtsSet.Insert (NodeId);
    }

    inline VOID SetMergeTarget (DWORD Did)
    {
        m_MergeTarget = Did;
    }

    inline DWORD GetMergeTarget ()
    {
        return m_MergeTarget;
    }

    inline PtsSet* GetPtsSet ()
    {
        return &m_PtsSet;
    }

    inline bool HasPtsSet ()
    {
        return !m_PtsSet.IsEmpty ();
    }

    inline Value* GetValue()
    {
        return m_Val;
    }

    inline bool IsNodeType(ConstraintNode::NodeTy Type)
    {
        return (m_Type == Type);
    }

    inline VOID AddOutLoadEdge(ConstraintEdge* CstEdge)
    {
        m_OutLoadEdgeSet.insert(CstEdge).second;
    }

    inline VOID RmOutLoadEdge(ConstraintEdge* CstEdge)
    {
        auto it = m_OutLoadEdgeSet.find(CstEdge);
        if (it == m_OutLoadEdgeSet.end())
        {
            return;
        }
        
        m_OutLoadEdgeSet.erase(CstEdge);
    }

    inline VOID AddInStoreEdge(ConstraintEdge* CstEdge)
    {
        m_InStoreEdgeSet.insert(CstEdge).second;
    }

    inline VOID RmInStoreEdge(ConstraintEdge* CstEdge)
    {
        auto it = m_InStoreEdgeSet.find(CstEdge);
        if (it == m_InStoreEdgeSet.end())
        {
            return;
        }
        
        m_InStoreEdgeSet.erase(CstEdge);
    }

    inline VOID AddOutCopyEdge(ConstraintEdge* CstEdge)
    {
        m_OutCopyEdgeSet.insert(CstEdge).second;
    }

    inline VOID RmOutCopyEdge(ConstraintEdge* CstEdge)
    {
        auto it = m_OutCopyEdgeSet.find(CstEdge);
        if (it == m_OutCopyEdgeSet.end())
        {
            return;
        }
        
        m_OutCopyEdgeSet.erase(CstEdge);
    }

    eg_iterator CopyEgBegin() { return m_OutCopyEdgeSet.begin(); }
    eg_iterator CopyEgEnd()      { return m_OutCopyEdgeSet.end(); }
    llvm::iterator_range<eg_iterator> OutCpoyEdge() 
    {
        return llvm::iterator_range<eg_iterator>(CopyEgBegin(), CopyEgEnd());
    }

    eg_iterator LoadEgBegin() const { return m_OutLoadEdgeSet.begin(); }
    eg_iterator LoadEgEnd()      const { return m_OutLoadEdgeSet.end(); }
    llvm::iterator_range<eg_iterator> OutLoadEdge() 
    {
        return llvm::iterator_range<eg_iterator>(LoadEgBegin(), LoadEgEnd());
    }

    eg_iterator StoreEgBegin() const { return m_InStoreEdgeSet.begin(); }
    eg_iterator StoreEgEnd()      const { return m_InStoreEdgeSet.end(); }
    llvm::iterator_range<eg_iterator> InStoreEdge() 
    {
        return llvm::iterator_range<eg_iterator>(StoreEgBegin(), StoreEgEnd());
    }   
};


class ConstraintGraph: public GenericGraph<ConstraintNode, ConstraintEdge> 
{
public:
    typedef ConstraintEdge::T_ConstraintEdgeSet::iterator iterator;
    
private:
    DWORD m_NodeNo;

    ConstraintEdge::T_ConstraintEdgeSet m_AddrEdgeSet;
    ConstraintEdge::T_ConstraintEdgeSet m_DirectEdgeSet;
    ConstraintEdge::T_ConstraintEdgeSet m_LoadEdgeSet;
    ConstraintEdge::T_ConstraintEdgeSet m_StoreEdgeSet;

public:
    ConstraintGraph ()
    {
        m_NodeNo = 0;
    }

    ~ConstraintGraph ()
    {
    }

    inline VOID ClearMem ()
    {
        m_AddrEdgeSet.clear();
        m_DirectEdgeSet.clear();
        m_LoadEdgeSet.clear();
        m_StoreEdgeSet.clear();

        for (auto it = begin (), e = end(); it != e; it++)
        {
            ConstraintNode* Node = it->second;

            Node->ClearMem ();
        }
    }

    inline DWORD AddCstNode (ConstraintNode::NodeTy Type, llvm::Value *Val)
    {
        ConstraintNode *CstNode = new ConstraintNode (m_NodeNo, Type, Val);
        assert (CstNode != NULL);
        
        AddNode (m_NodeNo, CstNode);

        return m_NodeNo++;
    }


    inline ConstraintEdge* AddCstEdge (DWORD Sid, DWORD Did, DWORD Type)
    {
        if (Sid == Did)
        {
            return NULL;
        }

        ConstraintNode *Src = GetGNode (Sid);
        ConstraintNode *Dst = GetGNode (Did);
        
        ConstraintEdge *CstEdge = new ConstraintEdge (Src, Dst, Type);
        if (AddCstEdgeByType(CstEdge))
        {
            AddEdge (CstEdge);
            Src->SetSuccessor (Did);
        }
        else
        {
            delete CstEdge;
            CstEdge = NULL;
        }

        return CstEdge;
    }

    inline bool AddCstEdgeByType (ConstraintEdge *CstEdge)
    {
        bool Flag = false;
        ConstraintNode *Node;
        
        switch (CstEdge->GetAttr ())
        {
            case ConstraintEdge::E_ADDR_OF:
            {
                Flag = m_AddrEdgeSet.insert(CstEdge).second;
                break;
            }
            case ConstraintEdge::E_COPY:
            {
                Flag = m_DirectEdgeSet.insert(CstEdge).second;
                if (Flag)
                {
                    Node = CstEdge->GetSrcNode ();
                    Node->AddOutCopyEdge (CstEdge);
                }
                break;
            }
            case ConstraintEdge::E_LOAD:
            {
                Flag = m_LoadEdgeSet.insert(CstEdge).second;
                if (Flag)
                {
                    Node = CstEdge->GetSrcNode ();
                    Node->AddOutLoadEdge (CstEdge);
                }
                break;
            }
            case ConstraintEdge::E_STORE:
            {
                Flag = m_StoreEdgeSet.insert(CstEdge).second;
                if (Flag)
                {
                    Node = CstEdge->GetDstNode ();
                    Node->AddInStoreEdge (CstEdge);
                }
                break;
            }
            default:
            {
                assert(0);
                return false;
            }
        }

        return Flag;
    }

    inline VOID RmCstEdge (ConstraintEdge *CstEdge)
    {
        ConstraintNode *Node;
        switch (CstEdge->GetAttr ())
        {
            case ConstraintEdge::E_ADDR_OF:
            {
                m_AddrEdgeSet.erase(CstEdge);
                break;
            }
            case ConstraintEdge::E_COPY:
            {
                m_DirectEdgeSet.erase(CstEdge);
                
                Node = CstEdge->GetSrcNode ();
                Node->RmOutCopyEdge (CstEdge);
                
                break;
            }
            case ConstraintEdge::E_LOAD:
            {
                m_LoadEdgeSet.erase(CstEdge);

                Node = CstEdge->GetSrcNode ();
                Node->RmOutLoadEdge (CstEdge);
                
                break;
            }
            case ConstraintEdge::E_STORE:
            {
                m_StoreEdgeSet.erase(CstEdge);

                Node = CstEdge->GetDstNode ();
                Node->RmInStoreEdge (CstEdge);
                
                break;
            }
            default:
            {
                assert(0);
                return;
            }
        }

        //m_EdgeNum--;
        RmEdge(CstEdge);
    }

    
    inline bool AddAddrCstEdge (DWORD Sid, DWORD Did)
    {
        ConstraintEdge* CstEdge = AddCstEdge (Sid, Did, ConstraintEdge::E_ADDR_OF);
        if (CstEdge == NULL)
        {
            return false;
        }

        return true;
    }


    inline bool AddStoreCstEdge (DWORD Sid, DWORD Did)
    {
        ConstraintEdge* CstEdge = AddCstEdge (Sid, Did, ConstraintEdge::E_STORE);
        if (CstEdge == NULL)
        {
            return false;
        }

        return true;
    }

    inline bool AddLoadCstEdge (DWORD Sid, DWORD Did)
    {
        ConstraintEdge* CstEdge = AddCstEdge (Sid, Did, ConstraintEdge::E_LOAD);
        if (CstEdge == NULL)
        {
            return false;
        }

        return true;
    }

    inline bool AddCopyCstEdge (DWORD Sid, DWORD Did)
    {
        ConstraintEdge* CstEdge = AddCstEdge (Sid, Did, ConstraintEdge::E_COPY);
        if (CstEdge == NULL)
        {
            return false;
        }

        return true;
    }
    
    inline VOID MergeNode(DWORD Sid, DWORD Did)
    {
        if (Sid == Did)
        {
            return;
        }

        ConstraintNode *SrcNd = GetGNode (Sid);
        assert (SrcNd != NULL);
        
        /* Sid -> Did */
        SrcNd->SetMergeTarget(Did);

        /* Merge Edge */
        for (auto In = SrcNd->InEdgeBegin (), End = SrcNd->InEdgeEnd (); In != End; In++)
        {
            ConstraintEdge *CurEdge = *In;

            if (Did == CurEdge->GetSrcID ())
            {
                continue;
            }

            AddCstEdge (CurEdge->GetSrcID (), Did, CurEdge->GetAttr ());
            RmCstEdge (CurEdge);
        }

        for (auto Out = SrcNd->OutEdgeBegin (), End = SrcNd->OutEdgeEnd (); Out != End; Out++)
        {
            ConstraintEdge *CurEdge = *Out;

            if (Did == CurEdge->GetDstID ())
            {
                continue;
            }

            AddCstEdge (CurEdge->GetSrcID (), Did, CurEdge->GetAttr ());
            RmCstEdge (CurEdge);
        }
        
        /* union the pts set */
        UnionPts (GetGNode(Did), SrcNd);

        SrcNd->ClearMem ();

        return;
    }
    
    inline DWORD GetMergeTarget(DWORD Sid) 
    {
        DWORD TmpId;
        DWORD Did = GetNodeTarget (Sid);
        if (Did != Sid) 
        {
            std::vector<DWORD> path(1, Sid);

            while (Did != (TmpId = GetNodeTarget (Did))) 
            {
                path.push_back(Did);
                Did = TmpId;
            }
            
            for (auto Id : path)
            {
                SetNodeTarget (Id, Did);
            }
        }
        
        return Did;
    }

    inline DWORD GetMergeTarget2(DWORD Sid) 
    {
        DWORD TmpId;
        DWORD Did = GetNodeTarget (Sid);
        
        while (Did != (TmpId = GetNodeTarget (Did)))
        {
            Did = TmpId;
        }
        
        return Did;
    }

    iterator CeBbegin() { return m_DirectEdgeSet.begin(); }
    iterator CeEnd() { return m_DirectEdgeSet.end(); }

    iterator LeBegin() const { return m_LoadEdgeSet.begin(); }
    iterator LeEnd() const { return m_LoadEdgeSet.end(); }
    llvm::iterator_range<iterator> LoadEdge() const 
    {
        return llvm::iterator_range<iterator>(LeBegin(), LeEnd());
    }

    iterator SeBegin() const { return m_StoreEdgeSet.begin(); }
    iterator SeEnd() const { return m_StoreEdgeSet.end(); }
    llvm::iterator_range<iterator> StoreEdge() const 
    {
        return llvm::iterator_range<iterator>(SeBegin(), SeEnd());
    }

    /*!
    * Process address edges
    */
    inline bool ProcessAddr(const ConstraintEdge* AddrEdge) 
    {
        DWORD dst = AddrEdge->GetDstID();
        DWORD src = AddrEdge->GetSrcID();

        return AddPts(dst, src);
    }

    /*!
     * Process load edges
     *	src --load--> dst,
     *	node \in pts(src) ==>  node--copy-->dst
     */
    inline bool ProcessLoad(DWORD Node, const ConstraintEdge* LoadEdge) 
    {
        if (Node == UniversalObj)
        {
            return false;
        }

        DWORD Dst = LoadEdge->GetDstID();
        return AddCopyCstEdge(Node, Dst);
    }

    /*!
     * Process store edges
     *	src --store--> dst,
     *	node \in pts(dst) ==>  src--copy-->node
     */
    inline bool ProcessStore(DWORD Node, const ConstraintEdge* SotreEdge) 
    {
        if (Node == UniversalObj)
        {
            return false;
        }

        DWORD src = SotreEdge->GetSrcID();
        return AddCopyCstEdge(src, Node);
    }

    /*!
     * Process copy edges
     *	src --copy--> dst,
     *	union pts(dst) with pts(src)
     */
    inline bool ProcessCopy(DWORD Node, const ConstraintEdge* CopyEdge) 
    {
        return UnionPts(CopyEdge->GetDstNode (), GetGNode(Node));
    }

    inline bool UnionPts (ConstraintNode *Dst, ConstraintNode *Src)
    {
        bool IsChange = false;

        if (Src->HasPtsSet ())
        {
            IsChange =  Dst->GetPtsSet()->Union (*Src->GetPtsSet());
        }

        return IsChange;
    }

    inline bool AddPts (DWORD Did, DWORD Sid)
    {
        ConstraintNode *Dst = GetGNode (Did);
        
        return Dst->GetPtsSet()->Insert (Sid);
    }

    VOID StatPtsSize ()
    {

    #define STAT_NUM (1024)
        DWORD SizeStatic[STAT_NUM];

        memset (SizeStatic, 0, sizeof (SizeStatic));
        for (auto it = begin (); it != end(); it++)
        {
            ConstraintNode *Node = it->second;
            DWORD PtsSize = Node->GetPtsSet ()->GetSize ();
            if (PtsSize == 0)
            {
                continue;
            }
            
            if (PtsSize > STAT_NUM)
            {
                PtsSize = STAT_NUM;
            }

            SizeStatic[PtsSize-1]++;
        }

        printf("\r\n------------------------------------------------\r\n");
        printf("------ Points-to statistics \r\n");
        printf("------------------------------------------------\r\n"); 
        printf("%-8s    %-8s\r\n", "PtsSize", "Counts");
        for (DWORD Index = 0; Index < STAT_NUM; Index++)
        {
            if (SizeStatic[Index] == 0)
            {
                continue;
            }
            printf("%-8d    %-8d\r\n", Index+1, SizeStatic[Index]);         
        }
        printf("------------------------------------------------\r\n\r\n"); 

        PrintNode (1, (char*)"points-to.txt");
    }

    VOID PrintNode (DWORD Limit, char *FileName)
    {
        char* Type[2] = {(char*)"VALUE", (char*)"OBJ"};
        DWORD PtsSize;
        FILE *f = fopen (FileName, "w");
        assert (f != NULL);
        
        for (auto it = begin (); it != end(); it++)
        {
            ConstraintNode *Node = it->second;
            DWORD TypeIndex = (DWORD)Node->IsNodeType (ConstraintNode ::E_OBJECT);
            Value* Val = Node->GetValue ();
            PtsSet* Pts = Node->GetPtsSet ();
            
            PtsSize = Pts->GetSize ();
            if (Limit != 0 && PtsSize < Limit)
            {
                continue;
            }
            
            
            if (Val == NULL || !Val->hasName())
            {
                fprintf(f, "[%-4d]NodeId: %-6d %-8s:%-16p ", PtsSize, Node->GetId (), Type[TypeIndex], Val);
            }
            else
            {  
                fprintf(f, "[%-4d]NodeId: %-6d %-8s:%-16s ", PtsSize, Node->GetId (), Type[TypeIndex] ,Val->getName().data());             
            }

            fprintf(f, "PointsTo ----> ");
            for (auto ts = Pts->begin (); ts != Pts->end (); ts++)
            {
                DWORD Id = *ts;
                ConstraintNode *PtNode = GetGNode (Id);
                Value *PtVal = PtNode->GetValue (); 
                if (PtVal == NULL)
                {
                    fprintf(f, "%-d--%-16s, ", Id, "NULL");
                }
                else
                {
                    fprintf(f, "%-d--%-16s, ", Id, PtVal->getName().data());
                }
            } 

            fprintf(f, "\r\n\r\n");
        }

        fclose (f);
    }

private:

    inline DWORD GetNodeTarget (DWORD Sid)
    {
        DWORD NodeNum = GetNodeNum ();       
        assert(Sid < NodeNum);

        ConstraintNode *Src = GetGNode (Sid);
        
        return Src->GetMergeTarget();
    }

    inline VOID SetNodeTarget (DWORD Sid, DWORD Did)
    {
        ConstraintNode *Src = GetGNode (Sid);
        
        Src->SetMergeTarget(Did);

        return;
    }
};

template <> class AfGraphTraits<ConstraintGraph> 
{
public:
    typedef ConstraintNode NodeType;
    typedef typename llvm::DenseMap<DWORD, NodeType*>::iterator NodeIterator;
    typedef ConstraintNode::iterator ChildIterator;

    static inline ChildIterator child_begin(NodeType *Node) 
    { 
        return Node->SuccBegin (); 
    }
    
    static inline ChildIterator child_end(NodeType *Node) 
    { 
        return Node->SuccEnd ();
    }

    static inline NodeIterator node_begin(ConstraintGraph *Graph) 
    {
        return Graph->begin();
    }
    
    static inline NodeIterator node_end(ConstraintGraph *Graph) 
    {
        return Graph->end();
    }
};

#endif 
