/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Universal charset detector code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *          Shy Shalom <shooshX@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */



#include <stdio.h>
#include <stdlib.h>

#include "JapaneseGroupProber.h"

#ifdef DEBUG_chardet
char *ProberName[] = 
{
  "UTF8",
  "GB18030",
  "Big5",
};

#endif

JapaneseGroupProber::JapaneseGroupProber()
{
  mProbers[0] = new nsUTF8Prober();
  mProbers[1] = new nsSJISProber();
  mProbers[2] = new nsEUCJPProber();
  Reset();
}

JapaneseGroupProber::~JapaneseGroupProber()
{
  for (unsigned int i = 0; i < JP_NUM_OF_PROBERS; i++)
  {
    delete mProbers[i];
  }
}

const char* JapaneseGroupProber::GetCharSetName()
{
  if (mBestGuess == -1)
  {
    GetConfidence();
    if (mBestGuess == -1)
      mBestGuess = 1;       // assume it's GB18030
  }
  return mProbers[mBestGuess]->GetCharSetName();
}

void  JapaneseGroupProber::Reset(void)
{
  mActiveNum = 0;
  for (unsigned int i = 0; i < JP_NUM_OF_PROBERS; i++)
  {
    if (mProbers[i])
    {
      mProbers[i]->Reset();
      mIsActive[i] = true;
      ++mActiveNum;
    }
    else
      mIsActive[i] = false;
  }
  mBestGuess = -1;
  mState = eDetecting;
}

nsProbingState JapaneseGroupProber::HandleData(const char* aBuf, unsigned int aLen)
{
  nsProbingState st;
  unsigned int i;

  //do filtering to reduce load to probers
  char *highbyteBuf;
  char *hptr;
  bool keepNext = true;   //assume previous is not ascii, it will do no harm except add some noise
  hptr = highbyteBuf = (char*)malloc(aLen);
  if (!hptr)
      return mState;
  for (i = 0; i < aLen; i++)
  {
    if (aBuf[i] & 0x80)
    {
      *hptr++ = aBuf[i];
      keepNext = true;
    }
    else
    {
      //if previous is highbyte, keep this even it is a ASCII
      if (keepNext)
      {
          *hptr++ = aBuf[i];
          keepNext = false;
      }
    }
  }

  for (i = 0; i < JP_NUM_OF_PROBERS; i++)
  {
     if (!mIsActive[i])
       continue;
     st = mProbers[i]->HandleData(highbyteBuf, hptr - highbyteBuf);
     if (st == eFoundIt)
     {
       mBestGuess = i;
       mState = eFoundIt;
       break;
     }
     else if (st == eNotMe)
     {
       mIsActive[i] = false;
       mActiveNum--;
       if (mActiveNum <= 0)
       {
         mState = eNotMe;
         break;
       }
     }
  }

  free(highbyteBuf);

  return mState;
}

float JapaneseGroupProber::GetConfidence(void)
{
  unsigned int i;
  float bestConf = 0.0, cf;

  switch (mState)
  {
  case eFoundIt:
    return (float)0.99;
  case eNotMe:
    return (float)0.01;
  default:
    for (i = 0; i < JP_NUM_OF_PROBERS; i++)
    {
      if (!mIsActive[i])
        continue;
      cf = mProbers[i]->GetConfidence();
      if (bestConf < cf)
      {
        bestConf = cf;
        mBestGuess = i;
      }
    }
  }
  return bestConf;
}

#ifdef DEBUG_chardet
void JapaneseGroupProber::DumpStatus()
{
  unsigned int i;
  float cf;
  
  GetConfidence();
  for (i = 0; i < JP_NUM_OF_PROBERS; i++)
  {
    if (!mIsActive[i])
      printf("  Chinese group inactive: [%s] (confidence is too low).\r\n", ProberName[i]);
    else
    {
      cf = mProbers[i]->GetConfidence();
      printf("  Chinese group %1.3f: [%s]\r\n", cf, ProberName[i]);
    }
  }
}
#endif



