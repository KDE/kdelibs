#ifndef SASLCONTEXT_H
#define SASLCONTEXT_H "$Id$"

/*-
 * Copyright (c) 2000,2001 Alex Zepeda <jazepeda@pacbell.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Redistributions of source code or in binary form must consent to
 *    future terms and conditions as set forth by the original author.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#include <qdict.h>
#include <qobject.h>
#include <qstring.h>

#include <kurl.h>

#include <saslmodule.h>

/**
  * Provides a generic client side authentication API suitable for use 
  * with any SASL compatible protocol or authentication method.
  *
  * @author Alex Zepeda <jazepeda@pacbell.net>
  * @version $Revision$
  **/
class KSASLContext
{
public:

	/**
	  * Default ctor
	  **/
	KSASLContext ();

	/**
	  * Returns the best possible authentication method
	  *
	  * @param available A space separated list of authentication 
	  * methods supported by the server.
	  *
	  * @param isBASE64 Set to true if available is Base 64 encoded.
	  **/
	QString chooseMethod (const QString &available, bool isBASE64 = false);

	/**
	  * Instructs the current authentication context to operate on
	  * a given URL.
	  *
	  * @param url The URL.
	  **/
	void setURL (const KURL &url);

	/**
	  * An overloaded version of generateResponse.  Why it's here I don't know..
	  *
	  * @param challenge
	  * @param isBASE64
	  **/
	QString generateResponse (const QString &challenge = QString::null, bool isBASE64 = false);

	/**
	  * Returns a response based on the given URL, challenge, and 
	  * authentication scheme.  If isBASE64 is true, challenge is
	  * decoded from Base 64, and the returned value is Base 64 encoded.
	  *
	  * @param challenge
	  * @param isBASE64
	  **/
	QString generateResponse (const QString &challenge, bool isBASE64, bool &more);

protected:

	/**
	  * Load all of the available authentication modules into memory
	  * so we've got an idea what is supported on the client side.
	  **/
	void loadModules ();

	QDict<KSASLAuthModule> m_lstModules;
	KURL m_urlAuthPath;
	QString m_sAuthMethod;
};

#endif
