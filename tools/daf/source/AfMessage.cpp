
#include "AfMessage.h"

vector<string> AfMessage::m_MsgType = {"caller", "callee", "define", "use", "lang"};

BOOL AfMessage::IsTypeValid(CHAR* Type)
{
    string MsgTy;
    vector<string>:: iterator It, End;

    for (It = m_MsgType.begin(), End = m_MsgType.end(); It != End; It++)
    {
        MsgTy = *It;
        if (!MsgTy.compare (Type))
        {
            return AF_TRUE;
        }
    }

    return AF_FALSE;
}

CHAR* AfMessage::GetTypeValue(CHAR* pMsg, char* Type, CHAR* Value)
{
    if (*pMsg++ != '{')
    {
        return NULL;
    }

    /* type */
    while (*pMsg != 0 && *pMsg != ':')
    {
        *Type++ = *pMsg++;
    }

    if (*pMsg++ != ':')
    {
        return NULL;
    }

    /* value */
    while (*pMsg != 0 && *pMsg != '}')
    {
        *Value++ = *pMsg++;
    }

    if (*pMsg++ != '}')
    {
        return NULL;
    }

    return pMsg; 
}


DWORD AfMessage::AfDecode (CHAR* Msg)
{
    CHAR Type[32];
    CHAR Value[1024];

    m_MsgVec.clear();

    CHAR* pData = Msg;
    while (*pData != 0)
    {
        memset(Type, 0, sizeof(Type));
        memset(Value, 0, sizeof(Value));
            
        pData = GetTypeValue(pData, Type, Value);
        if (pData == NULL)
        {
            return AF_FAIL;
        }

        if (!IsTypeValid(Type))
        {
            cout<<"Type: "<<Type<<" Is Invalid!!\r\n";
            return AF_FAIL;            
        }

        m_MsgVec.push_back (make_pair (string(Type), string(Value)));
    }

    return AF_SUCCESS;
}

AfMessage::~AfMessage ()
{
}


AfMessage::AfMessage (CHAR* Msg)
{
    DWORD Ret = AfDecode(Msg);
    assert (Ret != AF_FAIL);   
}

AfMessage::AfMessage ()
{
}


