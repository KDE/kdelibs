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

#ifndef	CUPSDCONF_H
#define	CUPSDCONF_H

#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qtextstream.h>

#include "cupsdcomment.h"

enum LogLevelType { LOGLEVEL_DEBUG2 = 0, LOGLEVEL_DEBUG, LOGLEVEL_INFO, LOGLEVEL_WARN, LOGLEVEL_ERROR, LOGLEVEL_NONE };
enum OrderType { ORDER_ALLOW_DENY = 0, ORDER_DENY_ALLOW };
enum AuthTypeType { AUTHTYPE_NONE = 0, AUTHTYPE_BASIC, AUTHTYPE_DIGEST };
enum AuthClassType { AUTHCLASS_ANONYMOUS = 0, AUTHCLASS_USER, AUTHCLASS_SYSTEM, AUTHCLASS_GROUP };
enum EncryptionType { ENCRYPT_ALWAYS = 0, ENCRYPT_NEVER, ENCRYPT_REQUIRED, ENCRYPT_IFREQUESTED };
enum BrowseProtocolType { BROWSE_ALL = 0, BROWSE_CUPS, BROWSE_SLP };

struct CupsLocation;
struct CupsResource;
enum ResourceType { RESOURCE_GLOBAL, RESOURCE_PRINTER, RESOURCE_CLASS, RESOURCE_ADMIN };

struct CupsdConf
{
// functions member
	CupsdConf();
	~CupsdConf();

	bool loadFromFile(const QString& filename);
	bool saveToFile(const QString& filename);
	bool parseOption(const QString& line);
	bool parseLocation(CupsLocation *location, QTextStream& file);

	bool loadAvailableResources();

	static CupsdConf* get();
	static void release();

// data members
	static CupsdConf	*unique_;

	// Server/Identity
	QString	servername_;
	QString	serveradmin_;
	QString	user_;
	QString	group_;
	QString	remroot_;

	// Server/Log
	QString	accesslog_;
	QString	errorlog_;
	QString	pagelog_;
	int	loglevel_;
	int	maxlogsize_;

	// Server/Directories
	QString	datadir_;
	QString	requestroot_;
	QString	serverbin_;
	QString	serverroot_;
	QString	tempdir_;
	QString	fontpath_;

	// Server/HTTP
	QString	documentroot_;
	QString	defaultcharset_;
	QString	defaultlanguage_;

	// Server/Jobs
	int	preservejobhistory_;
	int	preservejobfiles_;
	int	autopurgejobs_;
	int	maxjobs_;
	int	filterlimit_;
	QString	classification_;
	int	classifyoverride_;

	// Server/Misc
	QString	printcap_;
	QString	ripcache_;

	// Network
	QValueList<int>	port_;
	int	hostnamelookups_;
	int	keepalive_;
	int	keepalivetimeout_;
	int	maxclients_;
	int	maxrequestsize_;
	int	timeout_;

	// Browsing/General
	int	browsing_;
	int	browseprotocols_;
	int	browseshortnames_;
	int	implicitclasses_;
	int	hideimplicitmembers_;
	int	implicitanyclasses_;

	// Browsing/Connection
	QStringList	browseaddress_;
	int	browseport_;
	QStringList	browserelay_;
	QStringList	browsepoll_;

	// Browsing/Masks
	QStringList	browseallow_;
	QStringList	browsedeny_;
	int	browseorder_;

	// Browsing/Timeouts
	int	browseinterval_;
	int	browsetimeout_;

	// Security
	QString	systemgroup_;
	QPtrList<CupsLocation>	locations_;
	QPtrList<CupsResource>	resources_;

        // Encryption
        QString	servercertificate_;
        QString	serverkey_;

	// cupsd.conf file comments
	CupsdComment	comments_;
};

struct CupsLocation
{
	CupsLocation();
	CupsLocation(const CupsLocation& loc);

	bool parseOption(const QString& line);
	bool parseResource(const QString& line);

	CupsResource	*resource_;
	QString	resourcename_;
	int	authtype_;
	int	authclass_;
	QString	authgroupname_;
	int	order_;
	QStringList	allow_;
	QStringList	deny_;
        int	encryption_;
};

struct CupsResource
{
	CupsResource();
	CupsResource(const QString& path);

	void setPath(const QString& path);

	int	type_;
	QString	path_;
	QString	text_;

	static QString textToPath(const QString& text);
	static QString pathToText(const QString& path);
	static int typeFromPath(const QString& path);
	static int typeFromText(const QString& text);
	static QString typeToIconName(int type);
};

#endif
