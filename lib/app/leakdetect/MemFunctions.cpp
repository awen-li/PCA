//===- MemFunction.cpp -- Memory related functions ---------------------------//
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

#include "app/leakdetect/MemLeak.h"

vector<string> MemLeak::m_Malloc = 
{
    "malloc", 
    "valloc", 
    "calloc", 
    "strdup", 
    "strndup",
    "alloc",
    "alloc_check",
    "alloc_clear",
    "calloc",
    "jpeg_alloc_huff_table",
    "jpeg_alloc_quant_table",
    "lalloc",
    "lalloc_clear",
    "nhalloc",
    "oballoc",
    "permalloc",
    "png_create_info_struct",
    "png_create_write_struct",
    "safe_calloc",
    "safe_malloc",
    "safecalloc",
    "safemalloc",
    "safexcalloc",
    "safexmalloc",
    "savealloc",
    "xalloc",
    "xcalloc",
    "xmalloc",
    "SSL_CTX_new",
    "SSL_new"
};

vector<string> MemLeak::m_Free = 
{
   "cfree",
   "free",
   "free_all_mem",
   "freeaddrinfo",
   "gcry_mpi_release",
   "gcry_sexp_release",
   "globfree",
   "nhfree",
   "obstack_free",
   "safe_cfree",
   "safe_free",
   "safefree",
   "safexfree",
   "sm_free",
   "vim_free",
   "xfree",
   "SSL_CTX_free",
   "SSL_free"
};


VOID MemLeak::InitFuncMap (vector<string> FuncVec, F_TYPE Type)
{
    for (auto it  = FuncVec.begin(), ie =  FuncVec.end(); it !=ie; it++)
    {
        m_MemFuncMap[*it] = (DWORD)Type;
    }
}

VOID MemLeak::InitFuncMap ()
{
    InitFuncMap (m_Malloc, F_MALLOC);

    InitFuncMap (m_Free, F_FREE);
}


