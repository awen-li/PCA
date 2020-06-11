
#ifndef _MEMCHECK_H_
#define _MEMCHECK_H_

#include <llvm/Pass.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Instructions.h>
#include "common/VisitDir.h"
#include "app/leakdetect/MemLeak.h"
#include "llvmadpt/ModuleSet.h"

class MemCheck 
{
public:
    typedef map<string, DWORD> T_Case;
    typedef map<string, T_Case> T_CaseSet;
    
private:
    MemLeak *m_MemLeak;
    string m_CaseName;

    T_CaseSet m_CaseSet;

public:

    MemCheck(string CaseName="")
    {
        m_MemLeak = NULL;
        if (CaseName != "")
        {
            m_CaseName = CaseName;
            InitCase ();
        }
    }

    ~MemCheck()
    {
        if (m_MemLeak != NULL)
        {
            delete m_MemLeak;
        }
    }

    bool runOnModule(ModuleManage& ModMng);

    virtual llvm::StringRef getPassName() const 
    {
        return "Memcheck Pass";
    }

private:
    VOID AssertPrint (bool Con, string Msg);
    VOID InitCase ();
    VOID CaseAssert();
};

#endif /* MTA_H_ */
