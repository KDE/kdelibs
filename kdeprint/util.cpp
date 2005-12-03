/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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
		// assumes url starts with "smb://". Use encoding in
		// case the printer name contains chars like '#'.
		url = KURL("smb://" + KURL::encode_string(s.mid(6)));
	}
	else
	{
		// assumes URL starts with "smb://"
		QString	username = s.mid(6, p-6);
		url = KURL("smb://" + KURL::encode_string(s.mid(p+1)));
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
	s.append(url.host()).append(KURL::decode_string(url.path()));
	return s;
}

int findIndex(int ID)
{
	for (int i=0; i<KPrinter::NPageSize-1; i++)
		if (page_sizes[i].ID == ID)
			return i;
	return 4;
}

QString buildSmbURI( const QString& work, const QString& server, const QString& printer, const QString& user, const QString& passwd )
{
	QString uri = server + "/" + printer;
	if ( !work.isEmpty() )
		uri.prepend( work + "/" );
	if ( !user.isEmpty() )
	{
		uri.prepend( "@" );
		if ( !passwd.isEmpty() )
			uri.prepend( ":" + passwd );
		uri.prepend( user );
	}
	uri.prepend( "smb://" );
	return uri;
}

bool splitSmbURI( const QString& uri, QString& work, QString& server, QString& printer, QString& user, QString& passwd )
{
	int p( 0 );
	if ( !uri.startsWith( "smb://" ) )
		return false;
	p = 6;

	int p1 = uri.find( '/', p );
	if ( p1 != -1 )
	{
		int p2 = uri.find( '@', p );
		if ( p2 != -1 && p2 < p1 )
		{
			// Got a user
			int p3 = uri.find( ':', p );
			if ( p3 != -1 && p3 < p2 )
			{
				// Got a password
				user = uri.mid( p, p3-p );
				passwd = uri.mid( p3+1, p2-p3-1 );
			}
			else
				user = uri.mid( p, p2-p );
		}
		else
			p2 = p-1;
		QStringList l = QStringList::split( '/', uri.mid( p2+1 ), false );
		switch ( l.count() )
		{
			case 3:
				work = l[ 0 ];
				server = l[ 1 ];
				printer = l[ 2 ];
				break;
			case 2:
				server = l[ 0 ];
				printer = l[ 1 ];
				break;
			default:
				return false;
		}
		return true;
	}
	return false;
}

QString shadowPassword( const QString& uri )
{
	QString result = uri;
	int p = result.find( ':' );
	if ( p != -1 )
	{
		while ( result[ ++p ] == '/' );
		int p1 = result.find( '@', p );
		if ( p1 != -1 )
		{
			int p2 = result.find( ':', p );
			if ( p2 != -1 && p2 < p1 )
			{
				result.replace( p2, p1-p2, "" );
			}
		}
	}
	return result;
}
