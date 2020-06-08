//===- PointsTo.cpp -- point-to analysis ----//
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

#include "analysis/points-to/PointsTo.h"

Anderson* PointsTo::m_Andersen = NULL;
T_PTS PointsTo::m_PtsType     = T_NULL;

DWORD PointsTo::RunPtsAnalysis (T_PTS Type)
{
    switch (Type)
    {
        case T_ANDRESEN:
        {
            if (m_Andersen == NULL)
            {
                m_Andersen = new Anderson(m_ModMange);
                m_Andersen->RunPtsAnalysis ();

                m_PtsType = T_ANDRESEN;
            }
            break;
        }
        default:
        {
            assert(0);
        }
                
    }

    return AF_SUCCESS;
}

VOID PointsTo::GetPtsTo (llvm::Value *Src, std::vector<llvm::Value*>& Dst)
{
    if (m_PtsType == T_ANDRESEN)
    {
        m_Andersen->GetPtsTo (Src, Dst);
    }

    return;
}

inline llvm::SparseBitVector<>::iterator PointsTo::PtsBegin(llvm::Value* Val)
{
    if (m_PtsType == T_ANDRESEN)
    {
        m_Andersen->PtsBegin (Val);
    }
}

inline llvm::SparseBitVector<>::iterator PointsTo::PtsEnd(llvm::Value* Val)
{
    if (m_PtsType == T_ANDRESEN)
    {
        m_Andersen->PtsEnd (Val);
    }
}




