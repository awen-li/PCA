//===- PointsTo.h -- point-to analysis  ---------------------------------------//
//
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


#ifndef _POINTSTO_H_
#define _POINTSTO_H_

#include <llvm/IR/Module.h>
#include <llvm/ADT/GraphTraits.h>
#include <llvm/ADT/STLExtras.h>	
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SparseBitVector.h>
#include "llvmadpt/ModuleSet.h"
#include "analysis/Analysis.h"
#include "analysis/points-to/Anderson.h"

typedef enum
{
    T_NULL,
    T_ANDRESEN,
}T_PTS;

class PointsTo 
{
private:    
    ModuleManage m_ModMange;

    static T_PTS m_PtsType;
    static Anderson *m_Andersen;

public:

    PointsTo(ModuleManage &Mm, T_PTS Type) 
    {
        m_ModMange = Mm;

        RunPtsAnalysis (Type);
    }

    ~ PointsTo() 
    {
    }

    VOID GetPtsTo (llvm::Value *Src, std::vector<llvm::Value*>& Dst);
    inline llvm::SparseBitVector<>::iterator PtsBegin(llvm::Value* Val);
    inline llvm::SparseBitVector<>::iterator PtsEnd(llvm::Value* Val);

    T_PTS GetPtsType ()
    {
        return m_PtsType;
    }


    static VOID DeletePointTo ()
    {
        if (m_Andersen)
        {
            delete m_Andersen;
            m_Andersen = NULL;
        }
    }

    inline bool IsDebugFunction (std::string FuncName)
    {
        return m_Andersen->IsDebugFunction (FuncName);
    } 

private:
    DWORD RunPtsAnalysis (T_PTS Type);


};

#endif 
