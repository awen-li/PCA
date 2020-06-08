//===- llaf/tool/daf/include/AfQueue.h -   dynamic online analysis server  -------*- C++ -*-===//
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
#ifndef _AFQUEUE_H_
#define _AFQUEUE_H_ 
#include "common/BasicMacro.h"

using namespace std;

class AfQueue
{
private:
    DWORD m_DataLen;
    DWORD m_BufNum;
    CHAR* m_Buffer;
    
    queue <CHAR*> m_BufQueue;
    queue <CHAR*> m_DataQueue;
    
	pthread_mutex_t m_BufLock;
    pthread_mutex_t m_DataLock;

private:
    VOID AfQueueInit ();

    CHAR* AfAllocBuf ();
    VOID AfReleaseBuf (CHAR* Buf);

public:
    AfQueue (DWORD Size, DWORD DataLen);
    AfQueue ();
    ~AfQueue ();

    DWORD AfQueuePush (CHAR*    Data, DWORD DataLen);
    DWORD AfQueuePop    (CHAR* Data, DWORD DataLen);
};


#endif







