/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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





#include "nsUniversalDetector.h"

#include "nsMBCSGroupProber.h"
#include "nsSBCSGroupProber.h"
#include "nsEscCharsetProber.h"
#include "nsLatin1Prober.h"

nsUniversalDetector::nsUniversalDetector()
{
  mDone = false;
  mBestGuess = -1;   //illegal value as signal
  mInTag = false;
  mEscCharSetProber = 0;

  mStart = true;
  mDetectedCharset = 0;
  mGotData = false;
  mInputState = ePureAscii;
  mLastChar = '\0';

  unsigned int i;
  for (i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
    mCharSetProbers[i] = 0;
}

nsUniversalDetector::~nsUniversalDetector() 
{
  for (int i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
    if (mCharSetProbers[i])      
      delete mCharSetProbers[i];
  if (mEscCharSetProber)
    delete mEscCharSetProber;
}

void 
nsUniversalDetector::Reset()
{
  mDone = false;
  mBestGuess = -1;   //illegal value as signal
  mInTag = false;

  mStart = true;
  mDetectedCharset = 0;
  mGotData = false;
  mInputState = ePureAscii;
  mLastChar = '\0';

  if (mEscCharSetProber)
    mEscCharSetProber->Reset();

  unsigned int i;
  for (i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
    if (mCharSetProbers[i])
      mCharSetProbers[i]->Reset();
}

//---------------------------------------------------------------------
#define SHORTCUT_THRESHOLD      (float)0.95
#define MINIMUM_THRESHOLD      (float)0.20

nsProbingState nsUniversalDetector::HandleData(const char* aBuf, unsigned int aLen)
{
  if(mDone) 
    return eFoundIt;

  if (aLen > 0)
    mGotData = true;

  unsigned int i;
  for (i = 0; i < aLen; i++)
  {
    //other than 0xa0, if every othe character is ascii, the page is ascii
    if (aBuf[i] & '\x80' && aBuf[i] != '\xA0')  //Since many Ascii only page contains NBSP 
    {
      //we got a non-ascii byte (high-byte)
      if (mInputState != eHighbyte)
      {
        //adjust state
        mInputState = eHighbyte;

        //kill mEscCharSetProber if it is active
        if (mEscCharSetProber) {
          delete mEscCharSetProber;
          mEscCharSetProber = 0;
        }

        //start multibyte and singlebyte charset prober
        if (0 == mCharSetProbers[0])
          mCharSetProbers[0] = new nsMBCSGroupProber;
        if (0 == mCharSetProbers[1])
          mCharSetProbers[1] = new nsSBCSGroupProber;
        if (0 == mCharSetProbers[2])
          mCharSetProbers[2] = new nsLatin1Prober; 
      }
    }
    else
    {
      //ok, just pure ascii so far
      if ( ePureAscii == mInputState &&
        (aBuf[i] == '\033' || (aBuf[i] == '{' && mLastChar == '~')) )
      {
        //found escape character or HZ "~{"
        mInputState = eEscAscii;
      }
          
      mLastChar = aBuf[i];
    }
  }

  nsProbingState st = eDetecting;
  switch (mInputState)
  {
  case eEscAscii:
    if (0 == mEscCharSetProber) {
      mEscCharSetProber = new nsEscCharSetProber;
    }
    st = mEscCharSetProber->HandleData(aBuf, aLen);
    if (st == eFoundIt)
    {
      mDone = true;
      mDetectedCharset = mEscCharSetProber->GetCharSetName();
    }
    break;
  case eHighbyte:
    for (i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
    {
      st = mCharSetProbers[i]->HandleData(aBuf, aLen);
      if (st == eFoundIt) 
      {
        mDone = true;
        mDetectedCharset = mCharSetProbers[i]->GetCharSetName();
      } 
    }
    break;

  default:  //pure ascii
    mDetectedCharset = "UTF-8";
  }
  return st;
}


//---------------------------------------------------------------------
const char* nsUniversalDetector::GetCharSetName()
{
  if (mDetectedCharset)
    return mDetectedCharset;
  switch (mInputState)
  {
  case eHighbyte:
    {
      float proberConfidence;
      float maxProberConfidence = (float)0.0;
      int maxProber = 0;

      for (int i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
      {
        proberConfidence = mCharSetProbers[i]->GetConfidence();
        if (proberConfidence > maxProberConfidence)
        {
          maxProberConfidence = proberConfidence;
          maxProber = i;
        }
      }
      //do not report anything because we are not confident of it, that's in fact a negative answer
      if (maxProberConfidence > MINIMUM_THRESHOLD)
        return mCharSetProbers[maxProber]->GetCharSetName();
    }
  case eEscAscii:
    break;
  default:           // pure ascii
      ;
  }
  return "UTF-8";

}

//---------------------------------------------------------------------
float nsUniversalDetector::GetConfidence()
{
  if (!mGotData)
  {
    // we haven't got any data yet, return immediately
    // caller program sometimes call DataEnd before anything has been sent to detector
    return MINIMUM_THRESHOLD;
  }
  if (mDetectedCharset)
    return 0.99;
switch (mInputState)
  {
  case eHighbyte:
    {
      float proberConfidence;
      float maxProberConfidence = (float)0.0;
      int maxProber = 0;

      for (int i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
      {
        proberConfidence = mCharSetProbers[i]->GetConfidence();
        if (proberConfidence > maxProberConfidence)
        {
          maxProberConfidence = proberConfidence;
          maxProber = i;
        }
      }
      //do not report anything because we are not confident of it, that's in fact a negative answer
      if (maxProberConfidence > MINIMUM_THRESHOLD)
        return mCharSetProbers[maxProber]->GetConfidence();
    }
  case eEscAscii:
    break;
  default:           // pure ascii
      ;
  }
  return MINIMUM_THRESHOLD;
}

nsProbingState nsUniversalDetector::GetState()
{
    if (mDone)
        return eFoundIt;
    else
        return eDetecting;
}



