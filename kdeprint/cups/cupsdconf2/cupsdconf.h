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
#include <qpair.h>

#include "cupsdcomment.h"

enum LogLevelType { LOGLEVEL_DEBUG2 = 0, LOGLEVEL_DEBUG, LOGLEVEL_INFO, LOGLEVEL_WARN, LOGLEVEL_ERROR, LOGLEVEL_NONE };
enum OrderType { ORDER_ALLOW_DENY = 0, ORDER_DENY_ALLOW };
enum AuthTypeType { AUTHTYPE_NONE = 0, AUTHTYPE_BASIC, AUTHTYPE_DIGEST };
enum AuthClassType { AUTHCLASS_ANONYMOUS = 0, AUTHCLASS_USER, AUTHCLASS_SYSTEM, AUTHCLASS_GROUP };
enum EncryptionType { ENCRYPT_ALWAYS = 0, ENCRYPT_NEVER, ENCRYPT_REQUIRED, ENCRYPT_IFREQUESTED };
enum BrowseProtocolType { BROWSE_ALL = 0, BROWSE_CUPS, BROWSE_SLP };
enum PrintcapFormatType { PRINTCAP_BSD = 0, PRINTCAP_SOLARIS };
enum HostnameLookupType { HOSTNAME_OFF = 0, HOSTNAME_ON, HOSTNAME_DOUBLE };
enum ClassificationType { CLASS_NONE = 0, CLASS_CLASSIFIED, CLASS_CONFIDENTIAL, CLASS_SECRET, CLASS_TOPSECRET, CLASS_UNCLASSIFIED, CLASS_OTHER };
enum SatisfyType { SATISFY_ALL = 0, SATISFY_ANY };
enum UnitType { UNIT_KB = 0, UNIT_MB, UNIT_GB, UNIT_TILE };

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

	// Server
	QString	servername_;
	QString	serveradmin_;
	int classification_;
	QString otherclassname_;
	bool classoverride_;
	QString charset_;
	QString language_;
	QString printcap_;
	int printcapformat_;

	// Security
	QString remoteroot_;
	QString systemgroup_;
	QString encryptcert_;
	QString encryptkey_;
	QPtrList<CupsLocation> locations_;
	QPtrList<CupsResource> resources_;

	// Network
	int hostnamelookup_;
	bool keepalive_;
	int keepalivetimeout_;
	int maxclients_;
	QString maxrequestsize_;
	int clienttimeout_;
	QStringList listenaddresses_;

	// Log
	QString accesslog_;
	QString errorlog_;
	QString pagelog_;
	QString maxlogsize_;
	int loglevel_;

	// Jobs
	bool keepjobhistory_;
	bool keepjobfiles_;
	bool autopurgejobs_;
	int maxjobs_;
	int maxjobsperprinter_;
	int maxjobsperuser_;

	// Filter
	QString user_;
	QString group_;
	QString ripcache_;
	int filterlimit_;

	// Directories
	QString datadir_;
	QString documentdir_;
	QStringList fontpath_;
	QString requestdir_;
	QString serverbin_;
	QString serverfiles_;
	QString tmpfiles_;

	// Browsing
	bool browsing_;
	QStringList browseprotocols_;
	int browseport_;
	int browseinterval_;
	int browsetimeout_;
	QStringList browseaddresses_;
	int browseorder_;
	bool useimplicitclasses_;
	bool hideimplicitmembers_;
	bool useshortnames_;
	bool useanyclasses_;
	
	// cupsd.conf file comments
	CupsdComment	comments_;
	
	// unrecognized options
	QValueList< QPair<QString,QString> >	unknown_;
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
	QString	authname_;
	int	encryption_;
	int	satisfy_;
	int	order_;
	QStringList	addresses_;
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
