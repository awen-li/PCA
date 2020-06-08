//===- llaf/tool/daf/include/AfMessage.h - dynamic online analysis server -===//
//
//                     The LLAF framework
//
// This file is distributed under the University of WSU Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the dynamic online analysis server class.
//
//===----------------------------------------------------------------------===//
#ifndef _AFMESAGE_H_
#define _AFMESAGE_H_ 
#include "common/BasicMacro.h"

using namespace std;

typedef pair<string, string> T_Field;
typedef vector<T_Field> T_MsgVector;

class AfMessage
{
private:
    T_MsgVector m_MsgVec;
    static vector<string> m_MsgType; 
    
private:
    CHAR* GetTypeValue(CHAR* pMsg, char* Type, CHAR* Value);
    DWORD AfDecode (CHAR* Msg);

    BOOL IsTypeValid(CHAR* Type);

public:
    AfMessage (CHAR* Msg);
    AfMessage ();
    ~AfMessage ();

    inline T_MsgVector::iterator begin() 
    {
        return m_MsgVec.begin();
    }
    
    inline T_MsgVector::iterator end() 
    {
        return m_MsgVec.end();
    }
  
};


#endif



