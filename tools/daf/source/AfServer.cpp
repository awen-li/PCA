
#include "AfServer.h"

AfServer::AfServer (DWORD Port, AfQueue *Queue)
{
    m_LsnPort   = Port;
    m_Socket    = 0;
    memset (&m_skAddr, 0, sizeof(m_skAddr));

    m_MsgQueue = Queue;
    assert (Queue != NULL);
}

AfServer::~AfServer ()
{
    if (m_Socket != 0)
    {
        close (m_Socket);
    }

    if (m_MsgQueue != NULL)
    {
        delete m_MsgQueue;
    }
}

DWORD AfServer::AfInit ()
{
    SDWORD Ret;
    SDWORD Socket;  

    Socket = socket(AF_INET, SOCK_STREAM, 0);
    assert (Socket > 0);

    m_skAddr.sin_family = AF_INET; 
	m_skAddr.sin_port =	htons(m_LsnPort);
    m_skAddr.sin_addr.s_addr = htons(INADDR_ANY);
	Ret = bind(Socket, (struct sockaddr*)&m_skAddr, sizeof(struct sockaddr_in));
	assert(Ret >= 0);
    
    Ret = listen (Socket, 1);
    assert (Ret >= 0);

    m_Socket = Socket;

    return AF_SUCCESS;    
}

DWORD AfServer::AfStart ()
{
    DWORD Ret;
    DWORD SkSize;
    SDWORD SkConnect;
    CHAR MsgBuf[1024];

    Ret = AfInit();
    assert(Ret == AF_SUCCESS);

    cout <<"==============================================="<<endl;
    cout <<"==                analysis Server            =="<<endl;   
    cout <<"==============================================="<<endl;
    
    SkSize = sizeof(m_skAddr);
	while (1)
	{	    
	    SkConnect = accept(m_Socket, (struct sockaddr*)&m_skAddr, &SkSize);
	    assert(SkConnect > 0);
	    
	    memset (MsgBuf, 0, sizeof(MsgBuf));
			
	    /* recv from client */
	 	Ret = recv(SkConnect, MsgBuf, sizeof(MsgBuf), 0);
        assert(Ret >= 0);

        AfPushOneMsg (MsgBuf, Ret);
	}
    ;
    return AF_SUCCESS;
}

VOID AfServer::AfPushOneMsg (CHAR* Msg, DWORD MsgLen)
{  
    m_MsgQueue->AfQueuePush(Msg, MsgLen);
    return;
}



