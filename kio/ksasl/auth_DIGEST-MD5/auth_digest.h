#ifndef _KDIGEST_AUTH_MODULE_H
#define _KDIGEST_AUTH_MODULE_H "$Id$"

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

#include <qmap.h>

#include <kinstance.h>
#include <klibloader.h>
#include <kmdcodec.h>
#include <saslmodule.h>

class DigestAuthModule
	: public KSASLAuthModule
{
	Q_OBJECT

public:
	DigestAuthModule (QObject *parent = 0, const char *name = 0);
	virtual ~DigestAuthModule ();
	virtual QString auth_method ();
	virtual QString auth_response (const QString &challenge, const KURL &auth_url);
	virtual void get_usercredentials() {;}

protected:
	// Get us to a useable state;
	void reset ();

	// In case something fails
	void decNonceCount () {nonce_count--;}
	bool generateResponse (QString &s);

	void UseAuthString (const QString &challenge);

	void AddKey (const QString &line, QMap<QString, QString> &map);
	void AddKey (const QString &k, const QString &v, QMap<QString, QString> &map);

	static void GetA1 (const QString &user, const QString &realm, const QString &pass, const QString &nonce, const QString &cnonce, KMD5 &A1);
	static void GetA2 (const QString &qop, const QString &uri, KMD5 &A2);

	static inline QString ASCII (const char *text);

protected:
	unsigned int nonce_count;
	char cnonce[128];
	KURL request_url;
	QMap<QString, QString> keys;
	static QString COLON;
};

class DigestAuthModuleFactory
	: public KLibFactory
{
	Q_OBJECT

public:
	DigestAuthModuleFactory (QObject *parent = 0, const char *name = 0);
	~DigestAuthModuleFactory ();

	virtual QObject *create (QObject *parent = 0, const char *name = 0,
				 const char *classname = "QObject",
				 const QStringList &args = QStringList());

	KInstance *instance ();

private:
	KInstance *s_instance;
};
#endif
