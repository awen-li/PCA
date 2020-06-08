//===- Analysis.h -- Generic graph ---------------------------------------//
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


#ifndef _ANALYSIS_H_
#define _ANALYSIS_H_

#include <llvm/ADT/GraphTraits.h>
#include <llvm/ADT/STLExtras.h>	
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SparseBitVector.h>
#include "common/BasicMacro.h"

class Analysis 
{
public:
    
    Analysis()
    {
    }

    ~ Analysis()
    {
    }

    virtual VOID RunAnalysis ();
};

#endif 
