/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "util.h"
#include <qstringlist.h>

KURL smbToUrl(const QString& work, const QString& server, const QString& printer)
{
	KURL	url;
	url.setProtocol("smb");
	if (!work.isEmpty())
	{
		url.setHost(work);
		url.setPath("/" + server + "/" + printer);
	}
	else
	{
		url.setHost(server);
		url.setPath("/" + printer);
	}
	return url;
}

void urlToSmb(const KURL& url, QString& work, QString& server, QString& printer)
{
	if (url.protocol() != "smb")
		return;
	QString	h = url.host();
	QStringList	l = QStringList::split('/', url.path(), false);
	if (l.count() > 1)
	{
		work = h;
		server = l[0];
		printer = l[1];
	}
	else
	{
		work = QString::null;
		server = h;
		printer = l[0];
	}
}

KURL smbToUrl(const QString& s)
{
	// allow to handle non-encoded chars in login/password
	KURL	url;
	int	p = s.find('@');
	if (p == -1)
	{
		url = KURL(s);
	}
	else
	{
		// assumes URL starts with "smb://"
		QString	username = s.mid(6, p-6);
		url = KURL("smb://" + s.mid(p+1));
		int	q = username.find(':');
		if (q == -1)
			url.setUser(username);
		else
		{
			url.setUser(username.left(q));
			url.setPass(username.mid(q+1));
		}
	}
	return url;
}

QString urlToSmb(const KURL& url)
{
	// do not encode special chars in login/password
	QString	s = "smb://";
	if (!url.user().isEmpty())
	{
		s.append(url.user());
		if (!url.pass().isEmpty())
			s.append(":").append(url.pass());
		s.append("@");
	}
	s.append(url.host()).append(url.path());
	return s;
}

int findIndex(int ID)
{
	for (int i=0; i<KPrinter::NPageSize-1; i++)
		if (page_sizes[i].ID == ID)
			return i;
	return 4;
}
