/* This file is part of the KDE libraries
   Copyright (C) 2001 Michael Häckel <Michael@Haeckel.Net>
   $Id$
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qstrlist.h>
#include <kmdcodec.h>
#include <kurl.h>
#include "kdesasl.h"

KDESasl::KDESasl(const KURL &aUrl)
{
  mUser = aUrl.user();
  mPass = aUrl.pass();
  mFirst = TRUE;
}

KDESasl::KDESasl(const QString &aUser, const QString &aPass)
{
  mUser = aUser;
  mPass = aPass;
  mFirst = TRUE;
}

QCString KDESasl::chooseMethod(const QStrIList aMethods)
{
  if (aMethods.contains("CRAM-MD5")) mMethod = "CRAM-MD5";
  else if (aMethods.contains("LOGIN")) mMethod = "LOGIN";
  else if (aMethods.contains("PLAIN")) mMethod = "PLAIN";
  else mMethod = QCString();
  return mMethod;
}

void KDESasl::setMethod(const QCString &aMethod)
{
  mMethod = aMethod;
}

QByteArray KDESasl::getPlainResponse()
{
  QCString user = mUser.utf8();
  QCString pass = mPass.utf8();
  int userlen = user.length();
  int passlen = pass.length();
  QByteArray result(2 * userlen + passlen + 2);
  for (int i = 0; i <= userlen; i++) result[i+userlen+1] = result[i] = user[i];
  for (int i = 0; i < passlen; i++) result[i+2*userlen+2] = pass[i];
  return result;
}

QByteArray KDESasl::getLoginResponse()
{
  QByteArray result = (mFirst) ? mUser.utf8() : mPass.utf8();
  mFirst = !mFirst;
  if (result.size()) result.resize(result.size() - 1);
  return result;
}

QByteArray KDESasl::getCramMd5Response(const QByteArray &aChallenge)
{
  uint i;
  QByteArray secret = mPass.utf8();
  int len = mPass.utf8().length();
  secret.resize(len);
  if (secret.size() > 64)
  {
    KMD5 md5(secret);
    secret.duplicate((char*)md5.rawDigest(), 16);
    len = 16;
  }
  secret.resize(64);
  for (i = len; i < 64; i++) secret[i] = 0;
  QByteArray XorOpad(64);
  for (i = 0; i < 64; i++) XorOpad[i] = secret[i] ^ 0x5C;
  QByteArray XorIpad(64);
  for (i = 0; i < 64; i++) XorIpad[i] = secret[i] ^ 0x36;
  KMD5 md5;
  md5.update(XorIpad);
  md5.update(aChallenge);
  md5.finalize();
  KMD5 md5a;
  md5a.update(XorOpad);
  md5a.update(md5.rawDigest(), 16);
  md5a.finalize();
  QByteArray result = mUser.utf8();
  len = mUser.utf8().length();
  result.resize(len + 33);
  result[len] = ' ';
  char *ch = md5a.hexDigest();
  for (i = 0; i < 32; i++) result[i+len+1] = *(ch + i);
  return result;
}

/*
QByteArray KDESasl::getDigestMd5Response(const QByteArray &aChallenge)
{
  return QByteArray();
} */

QByteArray KDESasl::getBinaryResponse(const QByteArray &aChallenge, bool aBase64)
{
  if (aBase64)
  {
    QByteArray ba;
    KCodecs::base64Decode(aChallenge, ba);
    KCodecs::base64Encode(getBinaryResponse(ba, FALSE), ba);
    return ba;
  }
  if (stricmp(mMethod, "PLAIN") == 0) return getPlainResponse();
  if (stricmp(mMethod, "LOGIN") == 0) return getLoginResponse();
  if (stricmp(mMethod, "CRAM-MD5") == 0) return getCramMd5Response(aChallenge);
  return QByteArray();
}

QCString KDESasl::getResponse(const QByteArray &aChallenge, bool aBase64)
{
  QByteArray ba = getBinaryResponse(aChallenge, aBase64);
  return QCString(ba.data(), ba.size() + 1);
}
