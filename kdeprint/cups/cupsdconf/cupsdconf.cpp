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

#include "cupsdconf.h"

#include <qfile.h>
#include <klocale.h>
#include <kconfig.h>

#include <stdlib.h>
#include <cups/cups.h>
#include <cups/ipp.h>
#include <cups/language.h>

CupsdConf::CupsdConf()
{
	servername_ = QString::null;
	serveradmin_ = QString::null;
	user_ = QString::null;
	group_ = QString::null;
	remroot_ = QString::null;
	accesslog_ = QString::null;
	errorlog_ = QString::null;
	pagelog_ = QString::null;
	loglevel_ = -1;
	maxlogsize_ = -1;
	datadir_ = QString::null;
	requestroot_ = QString::null;
	serverbin_ = QString::null;
	serverroot_ = QString::null;
	tempdir_ = QString::null;
	fontpath_ = QString::null;
	documentroot_ = QString::null;
	defaultcharset_ = QString::null;
	defaultlanguage_ = QString::null;
	preservejobhistory_ = -1;
	preservejobfiles_ = -1;
	printcap_ = QString::null;
	ripcache_ = QString::null;
	filterlimit_ = -1;
	hostnamelookups_ = -1;
	keepalive_ = -1;
	keepalivetimeout_ = -1;
	maxclients_ = -1;
	maxrequestsize_ = -1;
	timeout_ = -1;
	browsing_ = -1;
	browseshortnames_ = -1;
	implicitclasses_ = -1;
	// browseaddress_
	browseport_ = -1;
	// browserelay
	// browsepoll
	// browseallow_
	// browsedeny_
	browseorder_ = -1;
	browseinterval_ = -1;
	browsetimeout_ = -1;
	systemgroup_ = QString::null;
	locations_.setAutoDelete(true);
	resources_.setAutoDelete(true);
        servercertificate_ = QString::null;
        serverkey_ = QString::null;

	loadAvailableResources();
}

CupsdConf::~CupsdConf()
{
}

bool CupsdConf::loadFromFile(const QString& filename)
{
	QFile	f(filename);
	if (!f.exists() || !f.open(IO_ReadOnly)) return false;
	else
	{
		QTextStream	t(&f);
		QString	line;
		bool	done(false), value(true);
		while (!done && value)
		{
			line = t.readLine().simplifyWhiteSpace();
			if (line.isEmpty())
			{
				if (t.atEnd()) done = true;
				else continue;
			}
			else if (line[0] == '#') continue;
			else if (line.left(9).lower() == "<location")
			{
				CupsLocation	*location = new CupsLocation();
				locations_.append(location);
				if (!location->parseResource(line) || !parseLocation(location, t))
					value = false;
				// search corresponding resource
				for (resources_.first();resources_.current();resources_.next())
					if (resources_.current()->path_ == location->resourcename_)
						location->resource_ = resources_.current();
			}
			else value = parseOption(line);
		}
		f.close();
		return value;
	}
}

bool CupsdConf::saveToFile(const QString& filename)
{
	QFile	f(filename);
	if (!f.open(IO_WriteOnly)) return false;
	else
	{
		QTextStream	t(&f);
		t << comments_[HEADER_COMM] << endl;
		t << comments_[SERVERNAME_COMM] << endl;
		if (!servername_.isNull()) t << "ServerName " << servername_ << endl;
		t << endl << comments_[SERVERADMIN_COMM] << endl;
		if (!serveradmin_.isNull()) t << "ServerAdmin " << serveradmin_ << endl;
		t << endl << comments_[USER_COMM] << endl;
		if (!user_.isNull()) t << "User " << user_ << endl;
		t << endl << comments_[GROUP_COMM] << endl;
		if (!group_.isNull()) t << "Group " << group_ << endl;
		t << endl << comments_[REMOTEROOT_COMM] << endl;
		if (!remroot_.isNull()) t << "RemoteRoot " << remroot_ << endl;
		t << endl << comments_[ACCESSLOG_COMM] << endl;
		if (!accesslog_.isNull()) t << "AccessLog " << accesslog_ << endl;
		t << endl << comments_[ERRORLOG_COMM] << endl;
		if (!errorlog_.isNull()) t << "ErrorLog " << errorlog_ << endl;
		t << endl << comments_[PAGELOG_COMM] << endl;
		if (!pagelog_.isNull()) t << "PageLog " << pagelog_ << endl;
		t << endl << comments_[LOGLEVEL_COMM] << endl;
		if (loglevel_ != -1)
		{
			t << "LogLevel ";
			switch (loglevel_)
			{
			   case LOGLEVEL_DEBUG2: t << "debug2" << endl; break;
			   case LOGLEVEL_DEBUG: t << "debug" << endl; break;
			   case LOGLEVEL_INFO: t << "info" << endl; break;
			   case LOGLEVEL_WARN: t << "warn" << endl; break;
			   case LOGLEVEL_ERROR: t << "error" << endl; break;
			   case LOGLEVEL_NONE: t << "none" << endl; break;
			   default: t << "info" << endl; break;
			}
		}
		t << endl << comments_[MAXLOGSIZE_COMM] << endl;
		if (maxlogsize_ != -1) t << "MaxLogSize " << maxlogsize_ << endl;
		t << endl << comments_[DATADIR_COMM] << endl;
		if (!datadir_.isNull()) t << "DataDir " << datadir_ << endl;
		t << endl << comments_[REQUESTROOT_COMM] << endl;
		if (!requestroot_.isNull()) t << "RequestRoot " << requestroot_ << endl;
		t << endl << comments_[SERVERBIN_COMM] << endl;
		if (!serverbin_.isNull()) t << "ServerBin " << serverbin_ << endl;
		t << endl << comments_[SERVERROOT_COMM] << endl;
		if (!serverroot_.isNull()) t << "ServerRoot " << serverroot_ << endl;
		t << endl << comments_[SERVERCERTIFICATE_COMM] << endl;
		if (!servercertificate_.isNull()) t << "ServerCertificate " << servercertificate_ << endl;
		t << endl << comments_[SERVERKEY_COMM] << endl;
		if (!serverkey_.isNull()) t << "ServerKey " << serverkey_ << endl;
		t << endl << comments_[TEMPDIR_COMM] << endl;
		if (!tempdir_.isNull()) t << "TempDir " << tempdir_ << endl;
		t << endl << comments_[FONTPATH_COMM] << endl;
		if (!fontpath_.isNull()) t << "FontPath " << fontpath_ << endl;
		t << endl << comments_[DOCUMENTROOT_COMM] << endl;
		if (!documentroot_.isNull()) t << "DocumentRoot " << documentroot_ << endl;
		t << endl << comments_[DEFAULTCHARSET_COMM] << endl;
		if (!defaultcharset_.isNull()) t << "DefaultCharset " << defaultcharset_ << endl;
		t << endl << comments_[DEFAULTLANG_COMM] << endl;
		if (!defaultlanguage_.isNull()) t << "DefaultLanguage " << defaultlanguage_ << endl;
		t << endl << comments_[PRESERVEJOBHIST_COMM] << endl;
		if (preservejobhistory_ != -1) t << "PreserveJobHistory " << (preservejobhistory_ == 1 ? "Yes" : "No") << endl;
		t << endl << comments_[PRESERVEJOBFILE_COMM] << endl;
		if (preservejobfiles_ != -1) t << "PreserveJobFiles " << (preservejobfiles_ == 1 ? "Yes" : "No") << endl;
		t << endl << comments_[PRINTCAP_COMM] << endl;
		if (!printcap_.isNull()) t << "Printcap " << printcap_ << endl;
		t << endl << comments_[RIPCACHE_COMM] << endl;
		if (!ripcache_.isNull()) t << "RIPCache " << ripcache_ << endl;
		t << endl << comments_[FILTERLIMIT_COMM] << endl;
		if (filterlimit_ != -1) t << "FilterLimit " << filterlimit_ << endl;
		t << endl << comments_[PORT_COMM] << endl;
		QValueList<int>::Iterator	it;
		for (it=port_.begin();it!=port_.end();++it)
			t << "Port " << (*it) << endl;
		t << endl << comments_[HOSTNAMELOOKUPS_COMM] << endl;
		if (hostnamelookups_ != -1) t << "HostNameLookups " << (hostnamelookups_ == 1 ? "On" : "Off") << endl;
		t << endl << comments_[KEEPALIVE_COMM] << endl;
		if (keepalive_ != -1) t << "KeepAlive " << (keepalive_ == 1 ? "On" : "Off") << endl;
		t << endl << comments_[KEEPALIVETIMEOUT_COMM] << endl;
		if (keepalivetimeout_ != -1) t << "KeepAliveTimeout " << keepalivetimeout_ << endl;
		t << endl << comments_[MAXCLIENTS_COMM] << endl;
		if (maxclients_ != -1) t << "MaxClients " << maxclients_ << endl;
		t << endl << comments_[MAXREQUESTSIZE_COMM] << endl;
		if (maxrequestsize_ != -1) t << "MaxRequestSize " << maxrequestsize_ << endl;
		t << endl << comments_[TIMEOUT_COMM] << endl;
		if (timeout_ != -1) t << "Timeout " << timeout_ << endl;
		t << endl << comments_[BROWSING_COMM] << endl;
		if (browsing_ != -1) t << "Browsing " << (browsing_ == 1 ? "On" : "Off") << endl;
		t << endl << comments_[BROWSESHORTNAMES_COMM] << endl;
		if (browseshortnames_ != -1) t << "BrowseShortNames " << (browseshortnames_ == 1 ? "Yes" : "No") << endl;
		t << endl << comments_[IMPLICITCLASSES_COMM] << endl;
		if (implicitclasses_ != -1) t << "ImplicitClasses " << (implicitclasses_ == 1 ? "On" : "Off") << endl;
		t << endl << comments_[BROWSEADDRESS_COMM] << endl;
		QStringList::Iterator	sit;
		for (sit=browseaddress_.begin();sit!=browseaddress_.end();++sit)
			t << "BrowseAddress " << (*sit) << endl;
		t << endl << comments_[BROWSEPORT_COMM] << endl;
		if (browseport_ != -1) t << "BrowsePort " << browseport_ << endl;
		t << endl << comments_[BROWSERELAY_COMM] << endl;
		for (sit=browserelay_.begin();sit!=browserelay_.end();++sit)
		{
			t << "BrowseRelay " << (*sit);
			++sit;
			t << " " << (*sit) << endl;
		}
		t << endl << comments_[BROWSEPOLL_COMM] << endl;
		for (sit=browsepoll_.begin();sit!=browsepoll_.end();++sit)
			t << "BrowsePoll " << (*sit) << endl;
		t << endl << comments_[BROWSEALLOWDENY_COMM] << endl;
		for (sit=browseallow_.begin();sit!=browseallow_.end();++sit)
			t << "BrowseAllow " << (*sit) << endl;
		for (sit=browsedeny_.begin();sit!=browsedeny_.end();++sit)
			t << "BrowseDeny " << (*sit) << endl;
		t << endl << comments_[BROWSEORDER_COMM] << endl;
		if (browseorder_ != -1) t << "BrowseOrder " << (browseorder_ == ORDER_ALLOW_DENY ? "allow,deny" : "deny,allow") << endl;
		t << endl << comments_[BROWSEINTERVAL_COMM] << endl;
		if (browseinterval_ != -1) t << "BrowseInterval " << browseinterval_ << endl;
		t << endl << comments_[BROWSETIMEOUT_COMM] << endl;
		if (browsetimeout_ != -1) t << "BrowseTimeout " << browsetimeout_ << endl;
		t << endl << comments_[SYSTEMGROUP_COMM] << endl;
		if (!systemgroup_.isNull()) t << "SystemGroup " << systemgroup_ << endl;
		t << endl << comments_[LOCATIONSCOMPLETE_COMM] << endl;
		for (locations_.first();locations_.current();locations_.next())
		{
			CupsLocation	*loc = locations_.current();
			t << "<Location " << loc->resourcename_ << ">" << endl;
			if (loc->authtype_ != -1)
			{
				t << "AuthType ";
				switch (loc->authtype_)
				{
				   case AUTHTYPE_NONE: t << "None" << endl; break;
				   case AUTHTYPE_BASIC: t << "Basic" << endl; break;
				   case AUTHTYPE_DIGEST: t << "Digest" << endl; break;
				   default: t << "None" << endl; break;
				}
			}
                        if (loc->authtype_ != AUTHTYPE_NONE)
                        { // print other settings only if type is not "None"
			        if (loc->authclass_ != -1)
			        {
					t << "AuthClass ";
					switch (loc->authclass_)
					{
					   case AUTHCLASS_ANONYMOUS: t << "Anonymous" << endl; break;
					   case AUTHCLASS_USER: t << "User" << endl; break;
					   case AUTHCLASS_SYSTEM: t << "System" << endl; break;
					   case AUTHCLASS_GROUP: t << "Group" << endl; break;
					   default: t << "User" << endl; break;
					}
				}
				if (!loc->authgroupname_.isEmpty()) t << "AuthGroupName " << loc->authgroupname_ << endl;
				if (loc->order_ != -1) t << "Order " << (loc->order_ == ORDER_ALLOW_DENY ? "Allow,Deny" : "Deny,Allow") << endl;
				for (sit=loc->allow_.begin();sit!=loc->allow_.end();++sit)
					t << "Allow From " << (*sit) << endl;
				for (sit=loc->deny_.begin();sit!=loc->deny_.end();++sit)
					t << "Deny From " << (*sit) << endl;
			}
			if (loc->encryption_ != -1)
                        {
                        	t << "Encryption ";
                                switch (loc->encryption_)
                                {
                                   case ENCRYPT_ALWAYS: t << "Always" << endl; break;
                                   case ENCRYPT_NEVER: t << "Never" << endl; break;
                                   case ENCRYPT_REQUIRED: t << "Required" << endl; break;
                                   case ENCRYPT_IFREQUESTED: t << "IfRequested" << endl; break;
                                   default: t << "IfRequested" << endl; break;
                                }
                        }
			t << "</Location>" << endl;
		}
		return true;
	}
}

bool CupsdConf::parseLocation(CupsLocation *location, QTextStream& file)
{
	QString	line;
	bool	done(false);
	bool	value(true);
	while (!done && value)
	{
		line = file.readLine().simplifyWhiteSpace();
		if (line.isEmpty())
		{
			if (file.atEnd())
			{
				value = false;
				done = true;
			}
			else continue;
		}
		else if (line[0] == '#') continue;
		else if (line.lower() == "</location>") done = true;
		else value = location->parseOption(line);
	}
	return value;
}

bool CupsdConf::parseOption(const QString& line)
{
	QStringList	wordlist_ = QStringList::split(' ', line.simplifyWhiteSpace().append(' '), false);
	if (wordlist_.count() == 0)
		return false;
	if (wordlist_.count() == 1)
	{
qDebug("warning: empty option \"%s\", adding a pseudo empty argument",(*(wordlist_.at(0))).latin1());
		wordlist_.append(QString(""));
	}
	QString	opt = *(wordlist_.at(0));
	if (opt.lower() == "servername") servername_ = *(wordlist_.at(1));
	else if (opt.lower() == "serveradmin") serveradmin_ = *(wordlist_.at(1));
	else if (opt.lower() == "user") user_ = *(wordlist_.at(1));
	else if (opt.lower() == "group") group_ = *(wordlist_.at(1));
	else if (opt.lower() == "remoteroot") remroot_ = *(wordlist_.at(1));
	else if (opt.lower() == "accesslog") accesslog_ = *(wordlist_.at(1));
	else if (opt.lower() == "errorlog") errorlog_ = *(wordlist_.at(1));
	else if (opt.lower() == "pagelog") pagelog_ = *(wordlist_.at(1));
	else if (opt.lower() == "loglevel")
	{
		QString	log = *(wordlist_.at(1));
		if (log.lower() == "debug2") loglevel_ = LOGLEVEL_DEBUG2;
		else if (log.lower() == "debug") loglevel_ = LOGLEVEL_DEBUG;
		else if (log.lower() == "info") loglevel_ = LOGLEVEL_INFO;
		else if (log.lower() == "warn") loglevel_ = LOGLEVEL_WARN;
		else if (log.lower() == "error") loglevel_ = LOGLEVEL_ERROR;
		else if (log.lower() == "none") loglevel_ = LOGLEVEL_NONE;
		else return false;
	}
	else if (opt.lower() == "maxlogsize") maxlogsize_ = (*(wordlist_.at(1))).toInt();
	else if (opt.lower() == "datadir") datadir_ = *(wordlist_.at(1));
	else if (opt.lower() == "requestroot") requestroot_ = *(wordlist_.at(1));
	else if (opt.lower() == "serverbin") serverbin_ = *(wordlist_.at(1));
	else if (opt.lower() == "serverroot") serverroot_ = *(wordlist_.at(1));
	else if (opt.lower() == "servercertificate") servercertificate_ = *(wordlist_.at(1));
	else if (opt.lower() == "serverkey") serverkey_ = *(wordlist_.at(1));
	else if (opt.lower() == "tempdir") tempdir_ = *(wordlist_.at(1));
	else if (opt.lower() == "fontpath") fontpath_ = *(wordlist_.at(1));
	else if (opt.lower() == "documentroot") documentroot_ = *(wordlist_.at(1));
	else if (opt.lower() == "defaultcharset") defaultcharset_ = *(wordlist_.at(1));
	else if (opt.lower() == "defaultlanguage") defaultlanguage_ = *(wordlist_.at(1));
	else if (opt.lower() == "preservejobhistory") preservejobhistory_ = (*(wordlist_.at(1)) == "Yes" ? 1 : 0);
	else if (opt.lower() == "preservejobfiles") preservejobfiles_ = (*(wordlist_.at(1)) == "Yes" ? 1 : 0);
	else if (opt.lower() == "printcap") printcap_ = *(wordlist_.at(1));
	else if (opt.lower() == "ripcache") ripcache_ = *(wordlist_.at(1));
	else if (opt.lower() == "filterlimit") filterlimit_ = (*(wordlist_.at(1))).toInt();
	else if (opt.lower() == "port") port_.append((*(wordlist_.at(1))).toInt());
	else if (opt.lower() == "hostnamelookups") hostnamelookups_ = (*(wordlist_.at(1)) == "On" ? 1 : 0);
	else if (opt.lower() == "keepalive") keepalive_ = (*(wordlist_.at(1)) == "On" ? 1 : 0);
	else if (opt.lower() == "keepalivetimeout") keepalivetimeout_ = (*(wordlist_.at(1))).toInt();
	else if (opt.lower() == "maxclients") maxclients_ = (*(wordlist_.at(1))).toInt();
	else if (opt.lower() == "maxrequestsize") maxrequestsize_ = (*(wordlist_.at(1))).toInt();
	else if (opt.lower() == "timeout") timeout_ = (*(wordlist_.at(1))).toInt();
	else if (opt.lower() == "browsing") browsing_ = (*(wordlist_.at(1)) == "On" ? 1 : 0);
	else if (opt.lower() == "browseshortnames") browseshortnames_ = (*(wordlist_.at(1)) == "Yes" ? 1 : 0);
	else if (opt.lower() == "implicitclasses") implicitclasses_ = (*(wordlist_.at(1)) == "On" ? 1 : 0);
	else if (opt.lower() == "browseaddress") browseaddress_.append(*(wordlist_.at(1)));
	else if (opt.lower() == "browseport") browseport_ = (*(wordlist_.at(1))).toInt();
	else if (opt.lower() == "browserelay")
	{
		if (wordlist_.count() < 3) return false;
		browserelay_.append(*(wordlist_.at(1)));
		browserelay_.append(*(wordlist_.at(2)));
	}
	else if (opt.lower() == "browsepoll") browsepoll_.append(*(wordlist_.at(1)));
	else if (opt.lower() == "browseallow") browseallow_.append(*(wordlist_.at(1)));
	else if (opt.lower() == "browsedeny") browsedeny_.append(*(wordlist_.at(1)));
	else if (opt.lower() == "browseorder")
	{
		QString	value = *(wordlist_.at(1));
		if (value.lower() == "allow,deny") browseorder_ = ORDER_ALLOW_DENY;
		else if (value.lower() == "deny,allow") browseorder_ = ORDER_DENY_ALLOW;
		else return false;
	}
	else if (opt.lower() == "browseinterval") browseinterval_ = (*(wordlist_.at(1))).toInt();
	else if (opt.lower() == "browsetimeout") browsetimeout_ = (*(wordlist_.at(1))).toInt();
	else if (opt.lower() == "systemgroup") systemgroup_ = *(wordlist_.at(1));
	else return false;
	return true;
}

bool CupsdConf::loadAvailableResources()
{
	KConfig	conf("kdeprintrc");
	conf.setGroup("CUPS");
	QString	host = conf.readEntry("Host",cupsServer());
	int 	port = conf.readNumEntry("Port",ippPort());
	http_t	*http_ = httpConnect(host.local8Bit(),port);

	resources_.clear();
	// standard resources
	resources_.append(new CupsResource("/"));
	resources_.append(new CupsResource("/admin"));
	resources_.append(new CupsResource("/printers"));
	resources_.append(new CupsResource("/classes"));

	if (!http_)
		return false;

	// printer resources
	ipp_t	*request_ = ippNew();
	cups_lang_t*	lang = cupsLangDefault();
	ippAddString(request_, IPP_TAG_OPERATION, IPP_TAG_CHARSET, "attributes-charset", NULL, cupsLangEncoding(lang));
	ippAddString(request_, IPP_TAG_OPERATION, IPP_TAG_LANGUAGE, "attributes-natural-language", NULL, lang->language);
	request_->request.op.operation_id = CUPS_GET_PRINTERS;
	request_ = cupsDoRequest(http_, request_, "/printers/");
	if (request_)
	{
		QString	name;
		int	type(0);
		ipp_attribute_t	*attr = request_->attrs;
		while (attr)
		{
			// check new printer (keep only local non-implicit printers)
			if (!attr->name)
			{
				if (!(type & CUPS_PRINTER_REMOTE) && !(type & CUPS_PRINTER_IMPLICIT) && !name.isEmpty())
					resources_.append(new CupsResource("/printers/"+name));
				name = "";
				type = 0;
			}
			else if (strcmp(attr->name, "printer-name") == 0) name = attr->values[0].string.text;
			else if (strcmp(attr->name, "printer-type") == 0) type = attr->values[0].integer;
			attr = attr->next;
		}
		if (!(type & CUPS_PRINTER_REMOTE) && !(type & CUPS_PRINTER_IMPLICIT) && !name.isEmpty())
			resources_.append(new CupsResource("/printers/"+name));
		ippDelete(request_);
	}
	// class resources
	request_ = ippNew();
	ippAddString(request_, IPP_TAG_OPERATION, IPP_TAG_CHARSET, "attributes-charset", NULL, cupsLangEncoding(lang));
	ippAddString(request_, IPP_TAG_OPERATION, IPP_TAG_LANGUAGE, "attributes-natural-language", NULL, lang->language);
	request_->request.op.operation_id = CUPS_GET_CLASSES;
	request_ = cupsDoRequest(http_, request_, "/classes/");
	if (request_)
	{
		QString	name;
		int	type(0);
		ipp_attribute_t	*attr = request_->attrs;
		while (attr)
		{
			// check new class (keep only local classes)
			if (!attr->name)
			{
				if (!(type & CUPS_PRINTER_REMOTE) && !name.isEmpty())
					resources_.append(new CupsResource("/classes/"+name));
				name = "";
				type = 0;
			}
			else if (strcmp(attr->name, "printer-name") == 0) name = attr->values[0].string.text;
			else if (strcmp(attr->name, "printer-type") == 0) type = attr->values[0].integer;
			attr = attr->next;
		}
		if (!(type & CUPS_PRINTER_REMOTE) && !name.isEmpty())
			resources_.append(new CupsResource("/classes/"+name));
		ippDelete(request_);
	}
	httpClose(http_);
	return true;
}

//------------------------------------------------------------------------------------------------

CupsLocation::CupsLocation()
{
	resource_ = 0;
	resourcename_ = "";
	authtype_ = -1;
	authclass_ = -1;
	authgroupname_ = QString::null;
	order_ = -1;
	// allow_
	// deny_
        encryption_ = -1;
}

CupsLocation::CupsLocation(const CupsLocation& loc)
: resource_(loc.resource_),
  resourcename_(loc.resourcename_),
  authtype_(loc.authtype_),
  authclass_(loc.authclass_),
  authgroupname_(loc.authgroupname_),
  order_(loc.order_),
  allow_(loc.allow_),
  deny_(loc.deny_),
  encryption_(loc.encryption_)
{
}

bool CupsLocation::parseResource(const QString& line)
{
	QString	str = line.simplifyWhiteSpace();
	int	p1 = line.find(' '), p2 = line.find('>');
	if (p1 != -1 && p2 != -1)
	{
		resourcename_ = str.mid(p1+1,p2-p1-1);
		return true;
	}
	else return false;
}

bool CupsLocation::parseOption(const QString& line)
{
	QStringList	wordlist_ = QStringList::split(' ', line.simplifyWhiteSpace().append(' '), false);
	if (wordlist_.count() == 0) return false;
	if (wordlist_.count() == 1)
	{
qDebug("warning: empty option \"%s\", adding a pseudo empty argument",(*(wordlist_.at(0))).latin1());
		wordlist_.append(QString(""));
	}
	QString	opt = *(wordlist_.at(0)), value = *(wordlist_.at(1));;
	if (opt.lower() == "authtype")
	{
		if (value.lower() == "none") authtype_ = AUTHTYPE_NONE;
		else if (value.lower() == "basic") authtype_ = AUTHTYPE_BASIC;
		else if (value.lower() == "digest") authtype_ = AUTHTYPE_DIGEST;
		else return false;
	}
	else if (opt.lower() == "authclass")
	{
		if (value.lower() == "anonymous") authclass_ = AUTHCLASS_ANONYMOUS;
		else if (value.lower() == "user") authclass_ = AUTHCLASS_USER;
		else if (value.lower() == "system") authclass_ = AUTHCLASS_SYSTEM;
		else if (value.lower() == "group") authclass_ = AUTHCLASS_GROUP;
		else return false;
	}
	else if (opt.lower() == "authgroupname") authgroupname_ = value;
	else if (opt.lower() == "allow") allow_.append(*(wordlist_.at(2)));
	else if (opt.lower() == "deny") deny_.append(*(wordlist_.at(2)));
	else if (opt.lower() == "order")
	{
		if (value.lower() == "allow,deny") order_ = ORDER_ALLOW_DENY;
		else if (value.lower() == "deny,allow") order_ = ORDER_DENY_ALLOW;
		else return false;
	}
        else if (opt.lower() == "encryption")
        {
        	if (value.lower() == "always") encryption_ = ENCRYPT_ALWAYS;
        	else if (value.lower() == "never") encryption_ = ENCRYPT_NEVER;
        	else if (value.lower() == "required") encryption_ = ENCRYPT_REQUIRED;
        	else if (value.lower() == "ifrequested") encryption_ = ENCRYPT_IFREQUESTED;
                else return false;
        }
	return true;
}

//------------------------------------------------------------------------------------------------

CupsResource::CupsResource()
{
	type_ = RESOURCE_GLOBAL;
}

CupsResource::CupsResource(const QString& path)
{
	setPath(path);
}

void CupsResource::setPath(const QString& path)
{
	path_ = path;
	type_ = typeFromPath(path_);
	text_ = pathToText(path_);
}

int CupsResource::typeFromText(const QString& text)
{
	if (text == i18n("Base", "Root") || text == i18n("All printers") || text == i18n("All classes")) return RESOURCE_GLOBAL;
	else if (text == i18n("Administration")) return RESOURCE_ADMIN;
	else if (text.find(i18n("Class")) == 0) return RESOURCE_CLASS;
	else if (text.find(i18n("Printer")) == 0) return RESOURCE_PRINTER;
	else return RESOURCE_PRINTER;
}

int CupsResource::typeFromPath(const QString& path)
{
	if (path == "/admin") return RESOURCE_ADMIN;
	else if (path == "/printers" || path == "/classes" || path == "/") return RESOURCE_GLOBAL;
	else if (path.left(9) == "/printers") return RESOURCE_PRINTER;
	else if (path.left(8) == "/classes") return RESOURCE_CLASS;
	else return RESOURCE_GLOBAL;
}

QString CupsResource::textToPath(const QString& text)
{
	QString	path("/");
	if (text == i18n("Administration")) path = "/admin";
	else if (text == i18n("All printers")) path = "/printers";
	else if (text == i18n("All classes")) path = "/classes";
	else if (text == i18n("Base", "Root")) path = "/";
	else if (text.find(i18n("Printer")) == 0)
	{
		path = "/printers/";
		path.append(text.right(text.length()-i18n("Printer").length()-1));
	}
	else if (text.find(i18n("Class")) == 0)
	{
		path = "/classes/";
		path.append(text.right(text.length()-i18n("Class").length()-1));
	}
	return path;
}

QString CupsResource::pathToText(const QString& path)
{
	QString	text(i18n("Base", "Root"));
	if (path == "/admin") text = i18n("Administration");
	else if (path == "/printers") text = i18n("All printers");
	else if (path == "/classes") text = i18n("All classes");
	else if (path == "/") text = i18n("Root");
	else if (path.find("/printers/") == 0)
	{
		text = i18n("Printer");
		text.append(" ");
		text.append(path.right(path.length()-10));
	}
	else if (path.find("/classes/") == 0)
	{
		text = i18n("Class");
		text.append(" ");
		text.append(path.right(path.length()-9));
	}
	return text;
}

QString CupsResource::typeToIconName(int type)
{
	switch (type)
	{
	   case RESOURCE_ADMIN:
	   case RESOURCE_GLOBAL:
	   	return QString("folder");
	   case RESOURCE_PRINTER:
	   	return QString("kdeprint_printer");
	   case RESOURCE_CLASS:
	   	return QString("kdeprint_printer_class");
	}
	return QString("folder");
}
