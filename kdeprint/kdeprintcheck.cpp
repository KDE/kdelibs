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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/*
 * Implementation of simple checking mechanism. Rules are defined in
 * the form of an URI. Available syntax is:
 *	- exec:/<execname>	->	check for an executable in
 *					$PATH variable.
 *	- config:/path/to/file	->      check for the existence of a file
 *					or directory in KDE or standard
 *					UNIX config locations
 *	- file:/path/to/file
 *	- dir:/path/to/dir	->	simply check the existence of the
 *					a file or directory
 *	- service:/serv 	->	try to connect to a port on the
 *					specified host (usually localhost)
 *					"serv" can be a port value or service name
 *
 * TO BE IMPLEMENTED:
 *	- run:/<execname>	->	check for a running executable
 */

#include "kdeprintcheck.h"

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kextsock.h>
#include <qfile.h>
#include <unistd.h>

static const char* const config_stddirs[] = {
	"/etc/",
	"/usr/etc/",
	"/usr/local/etc/",
	"/opt/etc/",
	"/opt/local/etc/",
	0
};

bool KdeprintChecker::check(KConfig *conf, const QString& group)
{
	if (!group.isEmpty())
		conf->setGroup(group);
	QStringList	uris = conf->readListEntry("Require");
	return check(uris);
}

bool KdeprintChecker::check(const QStringList& uris)
{
	bool	state(true);
	for (QStringList::ConstIterator it=uris.begin(); it!=uris.end() && state; ++it)
	{
		state = (state && checkURL(KURL(*it)));
		// kdDebug( 500 ) << "auto-detection uri=" << *it << ", state=" << state << endl;
	}
	return state;
}

bool KdeprintChecker::checkURL(const KURL& url)
{
	QString	prot(url.protocol());
	if (prot == "config")
		return checkConfig(url);
	else if (prot == "exec")
		return checkExec(url);
	else if (prot == "file" || prot == "dir")
		return KStandardDirs::exists(url.url());
	else if (prot == "service")
		return checkService(url);
	return false;
}

bool KdeprintChecker::checkConfig(const KURL& url)
{
	// get the config filename (may contain a path)
	QString	f(url.path().mid(1));
	bool	state(false);

	// first check for standard KDE config file
	if (!locate("config",f).isEmpty())
		state = true;
	else
	// otherwise check in standard UNIX config directories
	{
		const char* const *p = config_stddirs;
		while (*p)
		{
			// kdDebug( 500 ) << "checkConfig() with " << QString::fromLatin1( *p ) + f << endl;
			if ( ::access( QFile::encodeName( QString::fromLatin1( *p ) + f ), F_OK ) == 0 )
			{
				state = true;
				break;
			}
			else
				p++;
		}
	}
	return state;
}

bool KdeprintChecker::checkExec(const KURL& url)
{
	QString	execname(url.path().mid(1));
	return !(KStandardDirs::findExe(execname).isEmpty());
}

bool KdeprintChecker::checkService(const KURL& url)
{
	QString	serv(url.path().mid(1));
	KExtendedSocket	sock;

	bool	ok;
	int	port = serv.toInt(&ok);

	if (ok) sock.setAddress("localhost", port);
	else sock.setAddress("localhost", serv);
	return (sock.connect() == 0);
}
