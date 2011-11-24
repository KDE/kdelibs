/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*  -*- C++ -*-
*  Copyright (C) 1998 <developer@mozilla.org>
*
*
*  Permission is hereby granted, free of charge, to any person obtaining
*  a copy of this software and associated documentation files (the
*  "Software"), to deal in the Software without restriction, including
*  without limitation the rights to use, copy, modify, merge, publish,
*  distribute, sublicense, and/or sell copies of the Software, and to
*  permit persons to whom the Software is furnished to do so, subject to
*  the following conditions:
*
*  The above copyright notice and this permission notice shall be included 
*  in all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
*  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
*  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
*  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
*  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef nsCodingStateMachine_h__
#define nsCodingStateMachine_h__

#include "kencodingprober.h"

#include "kdemacros.h"

#include "nsPkgInt.h"
namespace kencodingprober {
typedef enum {
   eStart = 0,
   eError = 1,
   eItsMe = 2 
} nsSMState;

#define GETCLASS(c) GETFROMPCK(((unsigned char)(c)), mModel->classTable)

//state machine model
typedef struct 
{
  nsPkgInt classTable;
  unsigned int classFactor;
  nsPkgInt stateTable;
  const unsigned int* charLenTable;
  const char* name;
} SMModel;

class KDE_NO_EXPORT nsCodingStateMachine {
public:
  nsCodingStateMachine(SMModel* sm){
          mCurrentState = eStart;
          mModel = sm;
        };
  nsSMState NextState(char c){
    //for each byte we get its class KDE_NO_EXPORT , if it is first byte, we also get byte length
    unsigned int byteCls = GETCLASS(c);
    if (mCurrentState == eStart)
    { 
      mCurrentBytePos = 0; 
      mCurrentCharLen = mModel->charLenTable[byteCls];
    }
    //from byte's class KDE_NO_EXPORT and stateTable, we get its next state
    mCurrentState=(nsSMState)GETFROMPCK(mCurrentState*(mModel->classFactor)+byteCls,
                                       mModel->stateTable);
    mCurrentBytePos++;
    return mCurrentState;
  };
  unsigned int  GetCurrentCharLen(void) {return mCurrentCharLen;};
  void      Reset(void) {mCurrentState = eStart;};
  const char * GetCodingStateMachine() {return mModel->name;};
#ifdef DEBUG_PROBE
  const char * DumpCurrentState(){
    switch (mCurrentState) {
        case eStart:
            return "eStart";
        case eError:
            return "eError";
        case eItsMe:
            return "eItsMe";
        default:
            return "OK";
    }
  }
#endif

protected:
  nsSMState mCurrentState;
  unsigned int mCurrentCharLen;
  unsigned int mCurrentBytePos;

  SMModel *mModel;
};

extern KDE_NO_EXPORT SMModel UTF8SMModel;
extern KDE_NO_EXPORT SMModel Big5SMModel;
extern KDE_NO_EXPORT SMModel EUCJPSMModel;
extern KDE_NO_EXPORT SMModel EUCKRSMModel;
extern KDE_NO_EXPORT SMModel EUCTWSMModel;
extern KDE_NO_EXPORT SMModel GB18030SMModel;
extern KDE_NO_EXPORT SMModel SJISSMModel;
extern KDE_NO_EXPORT SMModel UCS2LESMModel;
extern KDE_NO_EXPORT SMModel UCS2BESMModel;


extern KDE_NO_EXPORT SMModel HZSMModel;
extern KDE_NO_EXPORT SMModel ISO2022CNSMModel;
extern KDE_NO_EXPORT SMModel ISO2022JPSMModel;
extern KDE_NO_EXPORT SMModel ISO2022KRSMModel;
}
#endif /* nsCodingStateMachine_h__ */

