#ifndef AUTH_PLAIN_H
#define AUTH_PLAIN_H "$Id$"

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

#include <kinstance.h>
#include <klibloader.h>

#include <kio/ksasl/saslmodule.h>

class PlainAuthModule
	: public KSASLAuthModule
{
public:
	PlainAuthModule (QObject *parent = 0, const char *name = 0);
	virtual ~PlainAuthModule ();
	virtual QString auth_method ();
	virtual QString auth_response (const QString &challenge, const KURL &auth_url);
	virtual void get_usercredentials () {;}
};

class PlainAuthModuleFactory : public KLibFactory
{
	Q_OBJECT

public:
	PlainAuthModuleFactory (QObject *parent = 0, const char *name = 0);
	~PlainAuthModuleFactory ();
	virtual QObject *createObject (QObject *parent = 0, const char *name = 0,
				 const char *classname = "QObject",
				 const QStringList &args = QStringList());
	KInstance *instance ();

private:
	KInstance *s_instance;
};

#endif
