/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ktoolinvocation_x11.cpp" 


#if 0  // TODO: merge with relating parts from x11 

#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>

#include "kcheckaccelerators.h"
#include "kappdcopiface.h"

#include <qassistantclient.h>
#include <qdir.h>

#include "windows.h"
#include "shellapi.h"


void KApplication::invokeHelp( const QString& anchor,
	const QString& _appname, const QByteArray& startup_id ) const
{
	if (!d->qassistantclient) {
		d->qassistantclient = new QAssistantClient(
			KStandardDirs::findExe( "assistant" ), 0);
		QStringList args;
		args << "-profile";
		args << QDir::convertSeparators( locate("html", QString(name())+"/"+QString(name())+".adp") );
		d->qassistantclient->setArguments(args);
	}
	d->qassistantclient->openAssistant();
}


// on win32, for invoking browser we're using win32 API
// see kapplication_win.cpp
void KApplication::invokeBrowser( const QString &url, const QByteArray& startup_id )
{
	QByteArray s = url.latin1();
	const unsigned short *l = (const unsigned short *)s.data();
	ShellExecuteA(0, "open", s.data(), 0, 0, SW_NORMAL);
}

void KApplication::invokeMailer(const QString &to, const QString &cc, const QString &bcc,
                                const QString &subject, const QString &body,
                                const QString & /*messageFile TODO*/, const QStringList &attachURLs,
                                const QByteArray& startup_id )
{
	KURL url("mailto:"+to);
	url.setQuery("?subject="+subject);
	url.addQueryItem("cc", cc);
	url.addQueryItem("bcc", bcc);
	url.addQueryItem("body", body);
	for (QStringList::ConstIterator it = attachURLs.constBegin(); it != attachURLs.constEnd(); ++it)
		url.addQueryItem("attach", KURL::encode_string(*it));

	QByteArray s = url.url().latin1();
	const unsigned short *l = (const unsigned short *)s.data();
	ShellExecuteA(0, "open", s.data(), 0, 0, SW_NORMAL);
}
#endif
