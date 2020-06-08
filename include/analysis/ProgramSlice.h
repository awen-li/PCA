//===- ProgramSlice.h -- for program slice ---------------------------------------//
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
#ifndef _PROGRAMSLICE_H_
#define _PROGRAMSLICE_H_
#include "common/WorkList.h"
#include "analysis/Dependence.h"

using namespace std;


typedef enum
{
    REACH_NONE    = 0,
    REACH_PARITAL = 1,
    REACH_ALL     = 2,  
}REACH_TYPE;

class ProgramSlice 
{
typedef set<DgNode*> DgNodeSet;
typedef set<DgEdge*> DgEdgeSet;
typedef set<llvm::BasicBlock*> BlockSet;
typedef set<llvm::Function*> FunctionSet;
typedef map<llvm::Function*, DWORD> FuncToMaxId;


    
private:    
    DgNode* m_Root;
    DgGraph *m_Dg;
    
    DgNodeSet m_ForwardSlice;
    FunctionSet m_FdFuncSet;

    DgNodeSet m_BackwardSlice;
    FunctionSet m_BdFuncSet;
    FunctionSet m_PathFuncSet;

    FuncToMaxId m_FuncToMaxId;
    
    
    DgNodeSet m_BrNodes;
    DgEdgeSet m_BrEdges;
    
    DgNodeSet m_Sinks;
    
    REACH_TYPE m_ReachType;

    
    ComQueue<DgNode *> m_Queue;

    bool m_ReachOut;
    DWORD m_CrossFuncNum;

public:

    ProgramSlice(DgNode* Root, DgGraph* Dg)
    {
        m_Dg   = Dg;
        m_Root = Root;

        m_ReachType = REACH_NONE;
        m_ReachOut  = false;

        m_CrossFuncNum = 0;
    }

    ~ProgramSlice() 
    {
        
    }


    VOID RunSlicing(DgNodeSet &SinksSet);
    DWORD inline Reachability()
    {
        return m_ReachType;
    }

private:

    bool IsForward (DgNode *Node);
    VOID ForwardTraverse(DgNodeSet &SinksSet);
    VOID ForwardBfs(DgNodeSet &Sinks);
    VOID ForwardDfs (DgNode *Node, DgNodeSet *Visited, DgNodeSet &SinksSet);
    
    VOID BackwardTraverse(DgNode *Node);
    VOID BackwardDfs (DgNode *Node, DgNodeSet *Visited);

    bool IsExit (DgNode *DstNode);
    DgNode* GetPredom (DgNode *Node);
    bool IsDeBranch (DgNodeSet *Path);
    DWORD VisitEdgeType (DgNode *Node, DgNodeSet *Path);
    bool IsRetContext (DgNodeSet *Path, DgNode *DstNode);
    bool IsPathValid (DgNode *SinkNode, DgNodeSet *Path);
    
    VOID CfgPathDfs (DgNode *Node, DgNodeSet *Path);
    VOID ComputePathReachable();

};



#endif 
