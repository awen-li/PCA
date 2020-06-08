#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "AfServer.h"
#include "DynAnalysis.h"


VOID* StartSrvThread (VOID *pPara)
{
    AfQueue *Queue = (AfQueue*)pPara;
    
    AfServer afSrv(200902, Queue);
    afSrv.AfStart();

    return NULL;
}

int main (int argc,char *argv[])
{
    INT Ret;
    pthread_t Ts;

    AfQueue *Queue = new AfQueue (1024, 1024);
    assert (Queue != NULL);
    
    Ret = pthread_create(&Ts, NULL, StartSrvThread, Queue);
    if(Ret != 0)  
    {
        printf("Create pthread error!\n");
        return -1;
    }

    DynAnalysis DynAl (Queue);
    DynAl.DynCgAnalysis ();

    delete Queue;
    return 0;
}




