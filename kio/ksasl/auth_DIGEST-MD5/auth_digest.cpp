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

#include <sys/types.h>
#include <sys/uio.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream.h>

#include <qfile.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include <kdebug.h>
#include <kurl.h>

#include "auth_digest.h"

QString DigestAuthModule::COLON;

DigestAuthModule::DigestAuthModule (QObject *parent, const char *name)
	: KSASLAuthModule (parent, name), nonce_count(0)
{
	COLON = ASCII(":");
}

DigestAuthModule::~DigestAuthModule ()
{
}

QString DigestAuthModule::ASCII (const char *text)
{
	return QString::fromLatin1(text);
}

QString DigestAuthModule::auth_method ()
{
	return ASCII("DIGEST-MD5");
}

QString DigestAuthModule::auth_response(const QString &challenge, const KURL &auth_url)
{
	UseAuthString(challenge);
	char nc[9];
	QString uri, response;
	KMD5 A1, A2, FINAL;

	GetA1(auth_url.user(), keys[ASCII("realm")], auth_url.pass(), keys[ASCII("nonce")], ASCII(cnonce), A1);

	uri=auth_url.protocol()+QChar('/')+auth_url.host()+QChar('/')+auth_url.host();
	GetA2(ASCII("auth"), uri, A2);

	FINAL.update(reinterpret_cast<unsigned char *>(A1.hexDigest()), 32);
	FINAL.update(COLON);
	FINAL.update(keys[ASCII("nonce")]);
	FINAL.update(COLON);
	sprintf(nc, "%.8d", ++nonce_count);
	FINAL.update(ASCII(nc));
	FINAL.update(COLON);
	FINAL.update(reinterpret_cast<unsigned char *>(cnonce), strlen(cnonce));
	FINAL.update(COLON);
	FINAL.update(ASCII("auth"));
	FINAL.update(COLON);
	FINAL.update((unsigned char *)A2.hexDigest(), 32);
	FINAL.finalize();

	response=ASCII("charset=utf-8,");
	response+=ASCII("username=\"%1\",").arg(auth_url.user());
	response+=ASCII("realm=\"%1\",").arg(keys[ASCII("realm")]);
	response+=ASCII("nonce=\"%1\",").arg(keys[ASCII("nonce")]);
	response+=ASCII("nc=%1,").arg(ASCII(nc));
	response+=ASCII("cnonce=\"%1\",").arg(ASCII(cnonce));
	response+=ASCII("digest-uri=\"%1\",").arg(uri);
	response+=ASCII("response=%1,").arg(ASCII(FINAL.hexDigest()));
	response+=ASCII("qop=auth");
	return response;
}

void DigestAuthModule::GetA1(const QString &user, const QString &realm, const QString &pass, const QString &nonce, const QString &cnonce, KMD5 &A1)
{
	KMD5 md5;

	md5.update(user);
	md5.update(COLON);
	md5.update(realm);
	md5.update(COLON);
	md5.update(pass);
	md5.finalize();

	A1.update(md5.rawDigest(), 16);
	A1.update(COLON);
	A1.update(nonce);
	A1.update(COLON);
	A1.update(cnonce);
	A1.finalize();
}

void DigestAuthModule::GetA2(const QString &qop, const QString &uri, KMD5 &A2)
{
	A2.update(ASCII("AUTHENTICATE"));
	A2.update(COLON);
	A2.update(uri);
	if ( (qop == ASCII("auth-int")) ||  (qop == ASCII("auth-conf")) ) {
		char buf[34];
		sprintf(buf, ":%.32d", 0);
		A2.update(ASCII(buf));
	}
	A2.finalize();
}

void DigestAuthModule::reset()
{
	// TODO: Replace this with a sane random class
	//memset(&cnonce, 0, 128);
	QFile f(ASCII("/dev/random"));
	if (f.open(IO_ReadOnly)) {
		// Read up to 127 bits of randomness
		ssize_t ret = ::read(f.handle(), cnonce, 127); 
		if (ret <= 0) {
			sprintf(cnonce, "%s", "AHHYOUNEEDRANDOMINYOURLIFE");
		} else {
			cnonce[ret] = 0;
		}

		f.close();
	} else {
		sprintf(cnonce, "%s", "AHHYOUNEEDRANDOMINYOURLIFE");
	}
	keys.clear();
	nonce_count = 0;
}

void DigestAuthModule::AddKey(const QString &k, const QString &v, QMap<QString, QString> &m)
{
	kdDebug() << "Adding " << k.latin1() << ":" << v.latin1() << endl;
	m.insert(k, v);
}

void DigestAuthModule::AddKey(const QString &line, QMap<QString, QString> &)
{
	bool open_quote=false;
	unsigned int i=0;
	QString authstr(line);
	while (authstr.length() >= i) {
		if (authstr.at(i) == QChar('\"'))
			if (open_quote) open_quote=false;
			else open_quote=true;
		else if (authstr.at(i) == '=') {
			if (!open_quote) {
				QString left, right;
			left=authstr.mid(0, i);
			right=authstr.mid(i+1, authstr.length());
			if (right.at(0) == '\"')
				right=right.mid(1, right.length());
			if (right.at(right.length()-1) == '\"' )
				right=right.mid(0, right.length()-1);
			AddKey(left, right, keys);	
			}
		}
		i++;
	}
}

void DigestAuthModule::UseAuthString(const QString &challenge)
{
	bool open_quote=false;
	unsigned int i=0, comma=0;
	while (challenge.length() >= i) {
		if (challenge.at(i) == '\"') {
			if (open_quote) {
				open_quote=false;
			} else {
				open_quote=true;
			}
		} else if (challenge.at(i) == ',') {
			if (!open_quote) {
				AddKey(challenge.mid(comma, i-comma), keys);
				comma=i+1;
				if (challenge.find(ASCII(","), i+1) == -1) {
					AddKey(challenge.mid(i+1, challenge.length()), keys);
					break;
				}
			}
		}
		i++;
	}
}

// Module factory stuff.. should be automated
DigestAuthModuleFactory::DigestAuthModuleFactory (QObject *parent, const char *name)
	: KLibFactory (parent, name)
{
	s_instance = new KInstance("authdigestmodule");
}

DigestAuthModuleFactory::~DigestAuthModuleFactory ()
{
	delete s_instance;
}

QObject *DigestAuthModuleFactory::create (QObject *parent, const char *name, const char *, const QStringList &) {
	QObject *obj = new DigestAuthModule (parent, name);
	emit objectCreated(obj);
	return obj;
}

KInstance *DigestAuthModuleFactory::instance ()
{
	return s_instance;
}

extern "C" {
	void *init_ksasl_auth_digest () {
		return new DigestAuthModuleFactory;
	}
}


#include "auth_digest.moc"
