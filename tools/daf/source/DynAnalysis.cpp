
#include "DynAnalysis.h"


DynCg* DynAnalysis::m_DynCg = NULL;


DynAnalysis::DynAnalysis (AfQueue *MsgQueue)
{
    m_MsgQueue = MsgQueue;
    assert (MsgQueue != NULL);
    
    if (m_DynCg == NULL)
    {
        m_DynCg = new DynCg ();
        assert (m_DynCg != NULL);
    }

}

DynAnalysis::~DynAnalysis ()
{

}

DWORD DynAnalysis::GetInst (AfMessage &Msg, DynInst &Inst)
{
    DWORD Ret;
    T_Field *Field;
    T_MsgVector::iterator It, End;
   
    for (It = Msg.begin(), End = Msg.end(); It != End; It++)
    {
        Field = &(*It);
        Inst.DynInstSetValue(Field->first, Field->second);
        if (Ret != AF_SUCCESS)
        {
            return Ret;
        }
    }

    return AF_SUCCESS;  
}


DWORD DynAnalysis::DynCgAnalysis ()
{
    DWORD Ret;
    CHAR Data[1024];

    while (1)
    {
        Ret = m_MsgQueue->AfQueuePop(Data, sizeof(Data));
        if (Ret != AF_SUCCESS);
        {
            sleep (1);
            continue;
        }

        AfMessage Msg(Data);
        DynInst Inst;
        Ret = GetInst (Msg, Inst);
        assert (Ret != AF_FAIL);

        m_DynCg->AddInst(Inst);        
    }
}


