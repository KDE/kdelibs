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

#ifndef KDESASL_H
#define KDESASL_H

#include <qstring.h>

class KURL;
class QStrIList;

/**
 * This library can create responses for SASL authentication for a given
 * challenge and a given secret. This way of authentication is common for
 * SMTP, POP3 and IMAP.
 *
 * Currently PLAIN (RFC 2595), LOGIN and CRAM-MD5 (RFC 2195) authentication
 * are supported.
 *
 * @author Michael Häckel <Michael@Haeckel.Net>
 * @version $Id$
 */

class KDESasl
{

public:
  /**
   * Construct a sasl object and initialize it with the username and password
   * passed via the url.
   */
  KDESasl(const KURL &aUrl);
  /**
   * This is a conveniece funtion and differs from the above function only by
   * what arguments it accepts.
   */
  KDESasl(const QString &aUser, const QString &aPass);
  /*
   * You need to have a virtual destructor!
   */
  virtual ~KDESasl();
  /**
   * @returns the most secure method from the given methods and use it for
   * further operations.
   */
  virtual QCString chooseMethod(const QStrIList aMethods);
  /**
   * Explicitely set the SASL method used.
   */
  virtual void setMethod(const QCString &aMethod);
  /**
   * Creates a response using the formerly chosen SASL method.
   * For LOGIN authentication you have to call this function twice. KDESasl
   * realizes on its own, if you are calling it for the first or for the
   * second time.
   * @param aChallenge is the challenge sent to create a response for
   * @param aBase64 specifies, whether the authentication protocol uses base64
   * encoding. The challenge is decoded from base64 and the response is
   * encoded base64 if set to TRUE.
   */
   QCString getResponse(const QByteArray &aChallenge, bool aBase64 = TRUE);
  /**
   * Create a response as above but place it in a QByteArray
   */
  QByteArray getBinaryResponse(const QByteArray &aChallenge, bool aBase64); 

protected:
  /**
   * PLAIN authentication as described in RFC 2595
   */
  virtual QByteArray getPlainResponse();
  /**
   * LOGIN authentication
   */
  virtual QByteArray getLoginResponse();
  /**
   * CRAM-MD5 authentication as described in RFC 2195
   */
  virtual QByteArray getCramMd5Response(const QByteArray &aChallenge);
  /*
   * DIGEST-MD5 authentication as described in RFC 2831 and RFC 2617
   * (Not implemented yet)
   */
//  virtual QByteArray getDigestMd5Response(const QByteArray &aChallenge);

private:
  QString mUser, mPass;
  QCString mMethod;
  bool mFirst;
};

#endif
