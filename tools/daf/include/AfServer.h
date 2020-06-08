//===- llaf/tool/daf/include/server.h -   dynamic online analysis server  -------*- C++ -*-===//
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
#ifndef _AFSERVER_H_
#define _AFSERVER_H_ 
#include "AfQueue.h"

using namespace std;

class AfServer
{
private:
    DWORD  m_LsnPort;
    SDWORD m_Socket;
    struct sockaddr_in m_skAddr;

    AfQueue *m_MsgQueue;

private:
    DWORD AfInit ();
    VOID AfPushOneMsg (CHAR* Msg, DWORD MsgLen);

public:
    AfServer (DWORD Port, AfQueue *Queue);
    ~AfServer ();
    DWORD AfStart ();
    
};


#endif







