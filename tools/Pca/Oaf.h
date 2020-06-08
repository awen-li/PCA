
#ifndef _OAFPASS_H_
#define _OAFPASS_H_

#include <llvm/Pass.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Instructions.h>
#include "callgraph/CallGraph.h"
#include "common/VisitDir.h"
#include "analysis/Dependence.h"
#include "llvmadpt/ModuleSet.h"

class Oaf 
{

public:

    Oaf()
    {
        m_Cg = NULL;
    }

    virtual ~Oaf()
    {
        if (m_Cg != NULL)
        {
            delete m_Cg;
        }
    }

    bool runOnModule(ModuleManage& ModMng);

    virtual llvm::StringRef getPassName() const 
    {
        return "Oaf pass";
    }

private:
    CallGraph *m_Cg;

    

};

#endif /* MTA_H_ */
