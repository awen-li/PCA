//===- GraphViz.h -- Graphviz for dependence graph ---------------------------------//
//
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
#ifndef _DGVIZ_H_
#define _DGVIZ_H_
#include <fstream>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FileSystem.h>
#include "analysis/Dependence.h"


using namespace std;

template<class NodeTy,class EdgeTy, class GraphType> class GraphViz 
{
protected:
    FILE  *m_File;
    GraphType *m_Graph;
    string m_GraphName;

protected:
    inline VOID WriteHeader (string GraphName) 
    {
        fprintf(m_File, "digraph \"%s\"{\n", GraphName.c_str());
        fprintf(m_File, "\tlabel=\"%s\";\n", GraphName.c_str()); 

        return;
    }

    virtual inline string GetNodeLabel(NodeTy *Node) 
    {
        string str;
        raw_string_ostream RawStr(str);
        
        RawStr << "N" << Node->GetId ();
        //errs () << "NodeId: " << Node->GetId () << *(Node->GetInst ()) << "\r\n";

        return RawStr.str();
    }

    
    virtual inline string GetNodeAttributes(NodeTy *Node) 
    {
        string str;
        raw_string_ostream RawStr(str);
        RawStr <<  "color=black";
    
        return RawStr.str();
    }

    virtual inline string GetEdgeLabel(EdgeTy *Edge) 
    {
        string str;
        raw_string_ostream RawStr(str);
        llvm::Value *Val = Edge->GetEdgeValue ();
        
        RawStr <<Edge->GetAttr ();

        if (Val != NULL && Val->hasName ())
        {
            RawStr <<","<<Val->getName ().data();
        }

        return RawStr.str();
    }

    virtual inline string GetEdgeAttributes(EdgeTy *Edge) 
    {
        string str;
        raw_string_ostream RawStr(str);
        RawStr <<  "color=black";
    
        return RawStr.str();
    }
 
    inline VOID WriteNodes(NodeTy *Node) 
    {
        /* NodeID [color=grey,label="{NodeID: 0}"]; */
        string str;
        raw_string_ostream RawStr(str);

        RawStr << "N" << to_string (Node->GetId ()) << " [" << GetNodeAttributes (Node) << ",label=\"{"\
               << GetNodeLabel (Node) << "}\"];";

        fprintf(m_File, "\t%s\n", RawStr.str().c_str());
        return;        
    }
 

    inline VOID WriteEdge(EdgeTy *Edge) 
    {
        DWORD SrcId = Edge->GetSrcID ();
        DWORD DstId = Edge->GetDstID ();
        
        /* NodeId -> NodeId[style=solid,color=black]; */
        string str;
        raw_string_ostream RawStr(str);

        RawStr <<"\tN" << to_string (SrcId) <<" -> " <<"N" << to_string (DstId)\
               <<"[" << GetEdgeAttributes (Edge) <<",label=\"{"\
                     << GetEdgeLabel (Edge) << "}\"];";
               

        fprintf(m_File, "%s\n", RawStr.str().c_str());
        return; 
     
    }

    virtual inline BOOL IsEdgeType (EdgeTy *Edge)
    {
        return AF_TRUE;
    }

public:
    GraphViz(string GraphName, GraphType   * Graph) 
    {
        m_GraphName = GraphName;
        
        GraphName = GraphName + ".dot";
        m_File  =fopen (GraphName.c_str(), "w");
        assert (m_File != NULL);

        m_Graph = Graph;
    }

    ~GraphViz()
    {
        fclose (m_File);
    }

    VOID WiteGraph () 
    {
        WriteHeader(m_GraphName);

        for (auto It = m_Graph->begin (), End = m_Graph->end (); It != End; It++)
        {
            NodeTy *Node = It->second;
            WriteNodes (Node);

            for (auto ItEdge = Node->OutEdgeBegin (), ItEnd = Node->OutEdgeEnd (); ItEdge != ItEnd; ItEdge++)
            {
                EdgeTy *Edge = *ItEdge;
                if (!IsEdgeType(Edge))
                {
                    continue;
                }
                
                WriteEdge (Edge);
            }
        }

        fprintf(m_File, "}\n");
    }   
};



#endif 
