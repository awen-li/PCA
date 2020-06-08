
#include "AfQueue.h"

VOID AfQueue::AfQueueInit ()
{
    DWORD Id;
    CHAR* Buf;

    m_Buffer = new CHAR(m_BufNum * m_DataLen);
    assert (m_Buffer != NULL);

    Buf = m_Buffer;
    for (Id = 0; Id < m_BufNum; Id++)
    {
        m_BufQueue.push (Buf);
        Buf += m_DataLen;        
    }

    pthread_mutex_init(&m_BufLock, NULL);
    pthread_mutex_init(&m_DataLock, NULL);

    return;
}


AfQueue::AfQueue (DWORD Size, DWORD DataLen)
{
    m_BufNum  = Size?Size:1024;
    m_DataLen = DataLen?DataLen:1024;   

    AfQueueInit ();
}

AfQueue::AfQueue ()
{
    m_BufNum  = 1024;
    m_DataLen = 1024;  
    
    AfQueueInit ();
}

AfQueue::~AfQueue ()
{
    if (m_Buffer != NULL)
    {
        delete m_Buffer;
        m_Buffer = NULL;
    }
}

CHAR* AfQueue::AfAllocBuf ()
{
    CHAR* Buf;
    
    pthread_mutex_lock(&m_BufLock);
    Buf = m_BufQueue.front ();
    m_BufQueue.pop ();
    pthread_mutex_unlock(&m_BufLock);

    return Buf;
}

VOID AfQueue::AfReleaseBuf (CHAR* Buf)
{
    memset (Buf, 0, m_DataLen);
    
    pthread_mutex_lock(&m_BufLock);
    m_BufQueue.push (Buf);
    pthread_mutex_unlock(&m_BufLock);

    return;
}


DWORD AfQueue::AfQueuePush (CHAR*    Data, DWORD DataLen)
{
    CHAR *Buf = AfAllocBuf();
    
    assert (Buf != NULL && "queue buffer full!!!");
    assert(DataLen <= m_DataLen);

    memcpy (Buf, Data, DataLen);
    pthread_mutex_lock(&m_DataLock);
    m_DataQueue.push (Buf);
    pthread_mutex_unlock(&m_DataLock);
    
    return AF_SUCCESS;
}

DWORD AfQueue::AfQueuePop    (CHAR* Data, DWORD DataLen)
{
    CHAR* Buf;

    assert (Data != NULL && "queue buffer full!!!");
    assert(DataLen >= m_DataLen);

    if (m_DataQueue.empty())
    {
        return AF_FAIL;
    }

    pthread_mutex_lock(&m_DataLock);
    Buf = m_DataQueue.front ();
    m_DataQueue.pop ();
    pthread_mutex_unlock(&m_DataLock);

    memcpy (Data, Buf, m_DataLen);
    AfReleaseBuf (Buf);
    
    return AF_SUCCESS;
}



