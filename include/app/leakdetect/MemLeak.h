//===- MemLeak.h -- Detecting memory leaks--------------------------------//
//
//                     OAF: Optimiazed Analysis Framework
//
// Copyright (C) <2019-2024>  <Wen Li>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

#ifndef _MEMLEAK_H_
#define _MEMLEAK_H_
#include "app/leakdetect/LeakDetect.h"
#include "analysis/ProgramSlice.h"


using namespace std;


typedef enum
{
    F_MALLOC = 1,
    F_FREE,
}F_TYPE;


class MemLeak:public LeakDetector 
{
private:
    static vector<string> m_Malloc;
    static vector<string> m_Free;

    map<string, DWORD> m_MemFuncMap;

    map<string, DWORD> m_CheckResult;
    
public:
    MemLeak(DgGraph *Dg):LeakDetector (Dg)
    {
        InitFuncMap ();
    }

    ~MemLeak() 
    {
        
    }

protected:  
    bool IsNormalCheck(llvm::Function *CallFunc);
    bool IsSrcInAWrapper(DgNode *Root, ComQueue<DgNode *>* SrcQueue);
    VOID CollectSources();
    VOID CollectSinks();

    inline VOID InsertCheckResult (string BugInfo, DWORD Type)
    {
        m_CheckResult[BugInfo] = Type;
        return;
    }
    
    inline BOOL IsSourceFunc(const Function* Func)
    {
        auto fIt = m_MemFuncMap.find (Func->getName().str());
        if (fIt == m_MemFuncMap.end ())
        {
            return AF_FALSE;
        }

        return (BOOL)(fIt->second == F_MALLOC);
    }

    inline BOOL IsSinkFunc(const Function* Func)
    {
        auto fIt = m_MemFuncMap.find (Func->getName().str());
        if (fIt == m_MemFuncMap.end ())
        {
            return AF_FALSE;
        }

        return (BOOL)(fIt->second == F_FREE);
    }

    VOID InitFuncMap ();
    VOID InitFuncMap (std::vector<std::string> FuncVec, F_TYPE Type);


public:
    DWORD RunDetector ();
    VOID  ReportBug(ProgramSlice *PgSlice, DgNode *Source);

    inline map<string, DWORD>::iterator BugBegin ()
    {
        return m_CheckResult.begin();
    }

    
    inline map<string, DWORD>::iterator BugEnd ()
    {
        return m_CheckResult.end();
    }

};

#endif
