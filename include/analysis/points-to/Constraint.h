//===- Constraint.h -- COnstraints for anderson points-to analysis algorithm ---------------------//
//
// Copyright (C) <2019-2024>  <Wen Li>

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


#ifndef _CONSTRAINT_H_
#define _CONSTRAINT_H_
#include "common/BasicMacro.h"

/*
 copy:      "A = B", 
 load:      "A = *B",
 store:     "*A = B",
 AddressOf: "A = alloca",  
 Offset:    "*(A + K) = B-> stores", "A = *(B + K) -> loads", "A = B + K" -> copies
*/
class  Constraint 
{
public:
    typedef enum  { E_COPY, E_LOAD, E_STORE, E_ADDR_OF } ConstraintType;
    
private:

    DWORD m_Dest;
    DWORD m_Src;
    DWORD m_Offset;
    ConstraintType m_CttType;

public:

    Constraint(ConstraintType Ty, DWORD D, DWORD S, DWORD O = 0)
                   : m_CttType(Ty), m_Dest(D), m_Src(S), m_Offset(O) 
    {
        assert((m_Offset == 0 || Ty != E_ADDR_OF) &&
               "Offset is illegal on addressof constraints");
    }

    bool operator==(const Constraint &RHS) const 
    {
        return RHS.m_CttType == m_CttType
            && RHS.m_Dest == m_Dest
            && RHS.m_Src == m_Src
            && RHS.m_Offset == m_Offset;
    }

    bool operator!=(const Constraint &RHS) const 
    {
        return !(*this == RHS);
    }

    bool operator<(const Constraint &RHS) const 
    {
        if (RHS.m_CttType != m_CttType)
        {
              return RHS.m_CttType < m_CttType;
        }
        else if (RHS.m_Dest != m_Dest)
        {
              return RHS.m_Dest < m_Dest;
        }
        else if (RHS.m_Src != m_Src)
        {
              return RHS.m_Src < m_Src;
        }

        return RHS.m_Offset < m_Offset;
    }

    inline DWORD GetDst ()
    {
        return m_Dest;
    }

    inline DWORD GetSrc ()
    {
        return m_Src;
    }

    inline ConstraintType GetType ()
    {
        return m_CttType;
    }

    inline DWORD GetOffset ()
    {
         return m_Offset;
    }
};

#endif 
