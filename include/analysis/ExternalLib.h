//===- ExternalLib.h -- External function packet ---------------------------------//
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


#ifndef _EXTERNALLIB_H_
#define _EXTERNALLIB_H_
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CallSite.h"
#include "common/BasicMacro.h"

typedef enum
{
    EXT_Null = 0,
    EXT_Normal,
    EXT_Malloc,
    EXT_ReMalloc,
    EXT_Memcpy,
    EXT_RetArg0,
    EXT_Cast
}EXT_TYPE;

class ExternalLib 
{
private:
    static std::vector<std::string> m_Normal;
    static std::vector<std::string> m_Malloc;
    static std::vector<std::string> m_Realloc;
    static std::vector<std::string> m_Memcpy;
    static std::vector<std::string> m_RetArg0;
    static std::vector<std::string> m_Cast;


    std::map<std::string, EXT_TYPE> m_ExtFuncMap;

    EXT_TYPE m_CacheType;
    
public:
    
    ExternalLib()
    {
        m_CacheType = EXT_Null;
        InitExtLib ();
    }

    ~ ExternalLib()
    {
    }

    VOID CacheExtType(const llvm::Function *Func);
    
    bool IsDealWithPts ();
    bool IsRealloc ();
    bool IsMalloc ();
    bool IsRetArg0 ();
    bool IsMemcpy ();
    bool IsCast ();

private:

    EXT_TYPE Search (std::string Str);
    VOID InitExtLib ();

    
};


class DebugLib 
{
private:
    static std::vector<std::string> m_Debug;
    std::set<std::string> m_DebugFuncSet;
    
public:
    
    DebugLib()
    {
        InitDebugLib ();
    }

    ~ DebugLib()
    {
    }

    bool IsDebugFunction (std::string FuncName);

private:

    VOID InitDebugLib ();
};


#endif 
