
#include "DynFlow.h"

DynInst::DynInst ()
{
    m_InstNo  = 0;
    m_Lang    = "";    
    m_DefVal  = "";  
    m_Callee  = "";
    m_Caller  = "";
}

DynInst::~DynInst ()
{

}

DWORD DynInst::DynInstSetValue (string &Type, string &Value)
{
    if (!Type.compare("caller"))
    {
         m_Caller = Value;
    }
    else if (!Type.compare("callee"))
    {
         m_Callee = Value;          
    }
    else if (!Type.compare("define"))
    {
         m_DefVal = Value; 
    }
    else if (!Type.compare("use"))
    {
         m_UseVal.push_back (Value); 
    }
    else if (!Type.compare("lang"))
    {
         m_Lang = Value; 
    }
    else
    {
        cout<<"unsupported type message!!\n";
        return AF_FAIL;
    }
    
    return AF_SUCCESS;
}

string& DynInst::DynInstGetCaller ()
{
    return m_Caller;
}


DynCgNode::DynCgNode ()
{
    m_CallDepth = 0;
    m_FuncName  = "";

}

DynCgNode::~DynCgNode ()
{

}


DynCg::DynCg ()
{
    m_RefNum = 0;   

}

DynCg::~DynCg ()
{

}

DWORD DynCg::AddInst (DynInst &Inst)
{
    string CallName = Inst.DynInstGetCaller ();

    map<string, DynCgNode>::iterator It = m_FNameToCgNodeMap.find (CallName);
    if (It != m_FNameToCgNodeMap.end())
    {
    }
    else
    {
        DynCgNode &DynNode = It->second;
    }

    return AF_SUCCESS;
}



