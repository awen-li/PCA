//===- llaf/tool/daf/include/DynAnalysis.h -   dynamic online analysis server  -------*- C++ -*-===//
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
#ifndef _DYNANALYSIS_H_
#define _DYNANALYSIS_H_ 
#include "DynFlow.h"
#include "AfQueue.h"
#include "AfMessage.h"

using namespace std;

class DynAnalysis
{
private:
    static DynCg *m_DynCg;
    AfQueue *m_MsgQueue;

private:
    DWORD GetInst (AfMessage &Msg, DynInst &Inst);
    
public:
    DynAnalysis(AfQueue *MsgQueue);
    ~DynAnalysis ();

    DWORD DynCgAnalysis ();
};

#endif







