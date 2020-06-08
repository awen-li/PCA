//===- LeakDetect.h -- Detecting data leaks--------------------------------//
//
//                     OAF: Optimiazed Analysis Framework
//
// Copyright (C) <2019-2023>  <Wen Li>
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

#ifndef _LEAKDETECT_H_
#define _LEAKDETECT_H_
#include "common/WorkList.h"
#include "analysis/Dependence.h"

using namespace std;

typedef enum  
{
    MUST_LEAK,
    MAY_LEAK,
    NEVER_LEAK
}LEAK_TYPE;

class LeakDetector 
{
public:
    typedef set<DgNode *> DgNodeSet;
    
    
protected:
    DgGraph *m_DgGraph;
    DgNodeSet m_SrcSet;
    DgNodeSet m_SinkSet;

public:
    LeakDetector(DgGraph *Dg) 
    {
        m_DgGraph = Dg;
    }

    ~LeakDetector() 
    {
        if (m_DgGraph != NULL)
        {
            delete m_DgGraph;
        }
    }
  
    virtual VOID CollectSources() = 0;
    virtual VOID CollectSinks() = 0;
    
    virtual BOOL IsSourceFunc(const Function* fun) = 0;   
    virtual BOOL IsSinkFunc(const Function* fun) = 0;

    virtual DWORD RunDetector () = 0;
};

#endif
