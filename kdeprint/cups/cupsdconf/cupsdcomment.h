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

#ifndef CUPSDCOMMENT_H
#define	CUPSDCOMMENT_H

#include <qstringlist.h>
#include <qlist.h>

#define	HEADER_COMM		0
#define	SERVERNAME_COMM		1
#define	SERVERADMIN_COMM	2
#define	ACCESSLOG_COMM		3
#define	DATADIR_COMM		4
#define	DEFAULTCHARSET_COMM	5
#define	DEFAULTLANG_COMM	6
#define	DOCUMENTROOT_COMM	7
#define	ERRORLOG_COMM		8
#define	FONTPATH_COMM		9
#define	LOGLEVEL_COMM		10
#define	MAXLOGSIZE_COMM		11
#define	PAGELOG_COMM		12
#define	PRESERVEJOBHIST_COMM	13
#define	PRESERVEJOBFILE_COMM	14
#define	PRINTCAP_COMM		15
#define	REQUESTROOT_COMM	16
#define	REMOTEROOT_COMM		17
#define	SERVERBIN_COMM		18
#define	SERVERROOT_COMM		19
#define	USER_COMM		20
#define	GROUP_COMM		21
#define	RIPCACHE_COMM		22
#define	TEMPDIR_COMM		23
#define	FILTERLIMIT_COMM	24
#define	PORT_COMM		25
#define	HOSTNAMELOOKUPS_COMM	26
#define	KEEPALIVE_COMM		27
#define	KEEPALIVETIMEOUT_COMM	28
#define	MAXCLIENTS_COMM		29
#define	MAXREQUESTSIZE_COMM	30
#define	TIMEOUT_COMM		31
#define	BROWSING_COMM		32
#define	BROWSESHORTNAMES_COMM	33
#define	BROWSEADDRESS_COMM	34
#define	BROWSEALLOWDENY_COMM	35
#define	BROWSEINTERVAL_COMM	36
#define	BROWSEORDER_COMM	37
#define	BROWSEPOLL_COMM		38
#define	BROWSEPORT_COMM		39
#define	BROWSERELAY_COMM	40
#define	BROWSETIMEOUT_COMM	41
#define	IMPLICITCLASSES_COMM	42
#define	SYSTEMGROUP_COMM	43
#define	SERVERCERTIFICATE_COMM	44
#define	SERVERKEY_COMM		45
#define	LOCATIONSCOMPLETE_COMM	46
#define	LOCAUTHTYPE_COMM	47
#define	LOCAUTHCLASS_COMM	48
#define	LOCAUTHGROUPNAME_COMM	49
#define	LOCAUTHORDER_COMM	50
#define	LOCAUTHALLOW_COMM	51
#define	LOCAUTHDENY_COMM	52
#define	LOCENCRYPTION_COMM	53
#define	LOCATIONS_COMM		54
#define	AUTOPURGEJOBS_COMM	55
#define	BROWSEPROTOCOLS_COMM	56
#define	CLASSIFICATION_COMM	57
#define	CLASSIFYOVERRIDE_COMM	58
#define	HIDEIMPLICITMEMBERS_COMM	59
#define	IMPLICITANYCLASSES_COMM	60
#define	MAXJOBS_COMM		61

#define	LAST_COMM		61

class QFile;

class Comment
{
public:
        bool load(QFile* f);
        QString toolTip();
        QString comment();
private:
        QString comment_;
        QString example_;
};

class CupsdComment
{
public:
	QString operator[] (unsigned int index);
        QString comment(uint index);
        QString toolTip(uint index);

private:
	bool loadComments();

private:
	QList<Comment>  comments_;
};

#endif
