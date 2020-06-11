

#include <llvm/Support/CommandLine.h>	
#include <llvm/IR/InstIterator.h>
#include "MemCheck.h"
#include "common/Stat.h"


using namespace llvm;


bool MemCheck::runOnModule(ModuleManage& ModMng) 
{
    Stat::StartTime ("Compute MemCheck");

    DgGraph *Dg = new DgGraph (ModMng);

    m_MemLeak = new MemLeak(Dg);
    assert (m_MemLeak != NULL);

    printf ("#==========================================================\r\n");
    printf ("#start mempory leak detection....\r\n");
    printf ("#==========================================================\r\n");
    m_MemLeak->RunDetector ();

    Stat::EndTime ("Compute MemCheck");

    if (m_CaseName != "")
    {
        CaseAssert();        
    }

    return AF_FALSE;
}

VOID MemCheck::InitCase ()
{
    string CaseName = "case1";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 36 file: source/main.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 37 file: source/main.c", REACH_PARITAL));

    CaseName = "case2";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 5 file: source/main.c", REACH_PARITAL));

    CaseName = "case3";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 10 file: source/main.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 11 file: source/main.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 15 file: source/main.c", REACH_PARITAL));

    CaseName = "case5";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 7 file: source/main.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 16 file: source/main.c", REACH_PARITAL));

    CaseName = "case6";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 16 file: source/main.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 7 file: source/main.c", REACH_ALL));

    CaseName = "case7";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 7 file: source/main.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 16 file: source/main.c", REACH_PARITAL));

    CaseName = "case8";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 21 file: source/main.c", REACH_PARITAL));

    CaseName = "case9";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 29 file: source/main.c", REACH_PARITAL));

    CaseName = "case10";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 29 file: source/main.c", REACH_PARITAL));

    CaseName = "malloc0";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc0.c", REACH_NONE));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc0.c", REACH_NONE));

    CaseName = "malloc1.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 11 file: malloc1.c", REACH_ALL));

    CaseName = "malloc2.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 16 file: malloc2.c", REACH_ALL));

    CaseName = "malloc3.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 19 file: malloc3.c", REACH_ALL));

    CaseName = "malloc4.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 27 file: malloc4.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 28 file: malloc4.c", REACH_NONE));
    
    CaseName = "malloc5.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 18 file: malloc5.c", REACH_ALL));

    CaseName = "malloc6.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 18 file: malloc6.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 20 file: malloc6.c", REACH_ALL));

    CaseName = "malloc7.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc7.c", REACH_NONE));

    CaseName = "malloc8.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 15 file: malloc8.c", REACH_ALL));

    CaseName = "malloc9.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 21 file: malloc9.c", REACH_NONE));

    CaseName = "malloc10.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 20 file: malloc10.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 22 file: malloc10.c", REACH_NONE));

    CaseName = "malloc11.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc11.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 14 file: malloc11.c", REACH_ALL));

    CaseName = "malloc12.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 10 file: malloc12.c", REACH_ALL));

    CaseName = "malloc13.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 14 file: malloc13.c", REACH_NONE));

    CaseName = "malloc14.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc14.c", REACH_NONE));

    CaseName = "malloc15.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 11 file: malloc15.c", REACH_NONE));

    CaseName = "malloc16.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 27 file: malloc16.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 28 file: malloc16.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 29 file: malloc16.c", REACH_ALL));
    
    CaseName = "malloc17.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc17.c", REACH_ALL));

    CaseName = "malloc18.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 19 file: malloc18.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 21 file: malloc18.c", REACH_NONE));

    CaseName = "malloc19.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 17 file: malloc19.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 19 file: malloc19.c", REACH_ALL));

    CaseName = "malloc20.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc20.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 18 file: malloc20.c", REACH_PARITAL));

    CaseName = "malloc21.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 27 file: malloc21.c", REACH_NONE));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 28 file: malloc21.c", REACH_NONE));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 29 file: malloc21.c", REACH_NONE));

    CaseName = "malloc22.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 28 file: malloc22.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 29 file: malloc22.c", REACH_NONE));

    CaseName = "malloc23.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 11 file: malloc23.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 15 file: malloc23.c", REACH_PARITAL));

    CaseName = "malloc24.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc24.c", REACH_PARITAL));

    CaseName = "malloc25.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc25.c", REACH_NONE));

    CaseName = "malloc26.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 14 file: malloc26.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 16 file: malloc26.c", REACH_ALL));

    CaseName = "malloc27.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 11 file: malloc27.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 21 file: malloc27.c", REACH_NONE));

    CaseName = "malloc28.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 31 file: malloc28.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 21 file: malloc28.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 24 file: malloc28.c", REACH_NONE));

    CaseName = "malloc29.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 11 file: malloc29.c", REACH_NONE));

    CaseName = "malloc30.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc30.c", REACH_ALL));

    CaseName = "malloc31.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 22 file: malloc31.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 23 file: malloc31.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 14 file: malloc31.c", REACH_NONE));

    CaseName = "malloc32.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 18 file: malloc32.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 19 file: malloc32.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc32.c", REACH_NONE));

    CaseName = "malloc33.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 15 file: malloc33.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 16 file: malloc33.c", REACH_PARITAL));

    CaseName = "malloc34.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc34.c", REACH_ALL));

    CaseName = "malloc35.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 14 file: malloc35.c", REACH_NONE));

    CaseName = "malloc36.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 14 file: malloc36.c", REACH_NONE));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 19 file: malloc36.c", REACH_PARITAL));

    CaseName = "malloc37.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 18 file: malloc37.c", REACH_NONE));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 19 file: malloc37.c", REACH_NONE));

    CaseName = "malloc37.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 18 file: malloc37.c", REACH_NONE));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 19 file: malloc37.c", REACH_NONE));

    CaseName = "malloc38.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 18 file: malloc38.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 19 file: malloc38.c", REACH_ALL));

    CaseName = "malloc39.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc39.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc39.c", REACH_PARITAL));

    CaseName = "malloc40.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc40.c", REACH_NONE));

    CaseName = "malloc41.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 28 file: malloc41.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 29 file: malloc41.c", REACH_NONE));

    CaseName = "malloc42.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 14 file: malloc42.c", REACH_ALL));

    CaseName = "malloc43.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 16 file: malloc43.c", REACH_ALL));

    CaseName = "malloc44.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 18 file: malloc44.c", REACH_NONE));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 19 file: malloc44.c", REACH_ALL));

    CaseName = "malloc45.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc45.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc45.c", REACH_NONE));

    CaseName = "malloc46.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc46.c", REACH_ALL));

    CaseName = "malloc47.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc47.c", REACH_PARITAL));

    CaseName = "malloc48.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 11 file: malloc48.c", REACH_PARITAL));

    CaseName = "malloc49.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc49.c", REACH_PARITAL));

    CaseName = "malloc50.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc50.c", REACH_PARITAL));

    CaseName = "malloc51.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 25 file: malloc51.c", REACH_ALL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 26 file: malloc51.c", REACH_NONE));

    CaseName = "malloc52.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 176 file: malloc52.c", REACH_NONE));

    CaseName = "malloc53.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 57 file: malloc53.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 58 file: malloc53.c", REACH_PARITAL));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 54 file: malloc53.c", REACH_PARITAL));

    CaseName = "malloc54.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc54.c", REACH_ALL));

    CaseName = "malloc55.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc55.c", REACH_ALL));

    CaseName = "malloc56.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc56.c", REACH_PARITAL));

    CaseName = "malloc57.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 10 file: malloc57.c", REACH_PARITAL));
    
    CaseName = "malloc58.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 13 file: malloc58.c", REACH_NONE));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 14 file: malloc58.c", REACH_NONE));
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 17 file: malloc58.c", REACH_NONE));

    CaseName = "malloc59.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 12 file: malloc59.c", REACH_ALL));

    CaseName = "malloc60.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 11 file: malloc60.c", REACH_PARITAL));

    CaseName = "malloc61.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 15 file: malloc61.c", REACH_PARITAL));

    CaseName = "malloc62.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 16 file: malloc62.c", REACH_PARITAL));

    CaseName = "malloc63.c";
    m_CaseSet[CaseName].insert(map<string, DWORD>::value_type ("line: 17 file: malloc63.c", REACH_PARITAL));

}

VOID MemCheck::AssertPrint (bool Con, string Msg)
{
    string AsertMsg = m_CaseName + " " + Msg; 
    if (Con)
    {
        llvm::errs()<<"===> "<<LightMsg(AsertMsg)<<" success!!!\r\n";
    }
    else
    {
        llvm::errs()<<"===> "<<ErrMsg(AsertMsg)<<" fail!!!\r\n";
    }
}


VOID MemCheck::CaseAssert()
{
    auto It = m_CaseSet.find (m_CaseName);
    if (It == m_CaseSet.end())
    {
        return;
    }
    
    T_Case *Case = &(It->second);
    
    for (auto bIt = m_MemLeak->BugBegin (), bEnd = m_MemLeak->BugEnd (); bIt != bEnd; bIt++)
    {
        auto Result = Case->find (bIt->first);
        if (Result == Case->end())
        {
            AssertPrint (Result != Case->end(), Result->first);
            continue;
        }
                
        AssertPrint (Result->second == bIt->second, Result->first);
    }
}




