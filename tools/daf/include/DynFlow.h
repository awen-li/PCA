//===- llaf/tool/daf/include/DynFlow.h -   dynamic online analysis server  -------*- C++ -*-===//
//
//                     The LLAF framework
//
// This file is distributed under the University of WSU Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the dynamic online analysis server class.
//
//===----------------------------------------------------------------------===//
#ifndef _DYNFLOW_H_
#define _DYNFLOW_H_ 
#include "common/BasicMacro.h"

using namespace std;

class DynInst
{
private:
    DWORD m_InstNo;
    string m_Lang;
    
    string m_DefVal;  
    vector<string> m_UseVal;
    
    string m_Callee;
    string m_Caller;
    
public:

    DynInst();
    ~DynInst ();

    DWORD DynInstSetValue (string &Type, string &Value);
    string& DynInstGetCaller ();
};

class DynCgNode
{
private:
    string m_FuncName;
    vector<DynInst*> m_InstVec;
    DWORD m_CallDepth;

public:
    DynCgNode ();
    ~DynCgNode ();
};


class DynCg
{
private:
    DWORD m_RefNum;
    
    vector<DynCgNode> m_DynNodeVec;
    vector<DynInst> m_InstVec;

    map<string, DynCgNode> m_FNameToCgNodeMap;

public:
    DynCg ();
    ~DynCg ();

    DWORD AddInst (DynInst &Inst);

    //DWORD IncreaseRef ();
    //DWORD ReleaseRef ();

};


#endif







