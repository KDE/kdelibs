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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>

#include <kdebug.h>
#include <kinstance.h>
#include <kmdcodec.h>

#include "auth_plain.h"

PlainAuthModule::PlainAuthModule (QObject *parent, const char *name)
	: KSASLAuthModule (parent, name)
{
}

PlainAuthModule::~PlainAuthModule ()
{
}

QString PlainAuthModule::auth_method ()
{
	return QString::fromLatin1("PLAIN");
}

QString PlainAuthModule::auth_response (const QString &, const KURL &auth_url)
{
	QString line;
	line.append(auth_url.user());
	line.append(QChar(static_cast<char>(0)));
	line.append(auth_url.user());
	line.append(QChar(static_cast<char>(0)));
	line.append(auth_url.pass());
	return KBase64::encodeString(line);
}

// Module factory stuff.. should be automated

PlainAuthModuleFactory::PlainAuthModuleFactory (QObject *parent, const char *name)
	: KLibFactory (parent, name)
{
	s_instance = new KInstance("authplainmodule");
}

PlainAuthModuleFactory::~PlainAuthModuleFactory ()
{
	delete s_instance;
}

QObject *PlainAuthModuleFactory::createObject (QObject *parent, const char *name, const char *, const QStringList &)
{
	QObject *obj = new PlainAuthModule(parent, name);
	return obj;
}

KInstance *PlainAuthModuleFactory::instance ()
{
	return s_instance;
}

extern "C" {
	void *init_ksasl_auth_plain() {
		return new PlainAuthModuleFactory;
	}
}
