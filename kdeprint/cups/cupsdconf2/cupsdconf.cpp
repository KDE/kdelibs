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
#include <qregexp.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>

#include <stdlib.h>
#include <cups/cups.h>
#include <cups/ipp.h>
#include <cups/language.h>

QString findDir(const QStringList& list)
{
	for (QStringList::ConstIterator it=list.begin(); it!=list.end(); ++it)
		if (QFile::exists(*it))
			return *it;
	// if nothing found, just use the first as default value
	return list[0];
}

void splitSizeSpec(const QString& s, int& sz, int& suff)
{
	int	p = s.find(QRegExp("\\D"));
	sz = s.mid(0, p).toInt();
	if (p != -1)
	{
		switch (s[p].latin1())
		{
			case 'k': suff = UNIT_KB; break;
			default:
			case 'm': suff = UNIT_MB; break;
			case 'g': suff = UNIT_GB; break;
			case 't': suff = UNIT_TILE; break;
		}
	}
	else
		suff = UNIT_MB;
}

CupsdConf::CupsdConf()
{
	// start by trying to find CUPS directories (useful later)
	datadir_ = findDir(QStringList("/usr/share/cups")
			<< "/usr/local/share/cups"
			<< "/opt/share/cups"
			<< "/opt/local/share/cups");
	documentdir_ = findDir(QStringList(datadir_+"/doc")
			<< datadir_.left(datadir_.length()-5)+"/doc/cups");
	//fontpath_ << (datadir_+"/fonts");
	requestdir_ = findDir(QStringList("/var/spool/cups")
			<< "/var/cups");
	serverbin_ = findDir(QStringList("/usr/lib/cups")
			<< "/usr/local/lib/cups"
			<< "/opt/lib/cups"
			<< "/opt/local/lib/cups");
	serverfiles_ = findDir(QStringList("/etc/cups")
			<< "/usr/local/etc/cups");
	tmpfiles_ = requestdir_+"/tmp";

	// other options
	servername_ = "localhost";
	serveradmin_ = "root@"+servername_;
	classification_ = CLASS_NONE;
	otherclassname_ = QString::null;
	classoverride_ = false;
	charset_ = "utf-8";
	language_ = "en";
	printcap_ = "/etc/printcap";
	printcapformat_ = PRINTCAP_BSD;
	remoteroot_ = "remroot";
	systemgroup_ = "sys";
	encryptcert_ = serverfiles_+"/ssl/server.crt";
	encryptkey_ = serverfiles_+"/ssl/server.key";
	hostnamelookup_ = HOSTNAME_OFF;
	keepalive_ = true;
	keepalivetimeout_ = 60;
	maxclients_ = 100;
	maxrequestsize_ = "0";
	clienttimeout_ = 300;
	// listenaddresses_
	QString	logdir = findDir(QStringList("/var/log/cups")
			<< "/var/spool/cups/log"
			<< "/var/cups/log");
	accesslog_ = logdir+"/access_log";
	errorlog_ = logdir+"/error_log";
	pagelog_ = logdir+"/page_log";
	maxlogsize_ = "1m";
	loglevel_ = LOGLEVEL_INFO;
	keepjobhistory_ = true;
	keepjobfiles_ = false;
	autopurgejobs_ = false;
	maxjobs_ = 0;
	maxjobsperprinter_ = 0;
	maxjobsperuser_ = 0;
	user_ = "lp";
	group_ = "sys";
	ripcache_ = "8m";
	filterlimit_ = 0;
	browsing_ = true;
	browseprotocols_ << "CUPS";
	browseport_ = ippPort();
	browseinterval_ = 30;
	browsetimeout_ = 300;
	// browseaddresses_
	browseorder_ = ORDER_ALLOW_DENY;
	useimplicitclasses_ = true;
	hideimplicitmembers_ = true;
	useshortnames_ = true;
	useanyclasses_ = false;

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
	if (!f.open(IO_WriteOnly))
		return false;
	else
	{
		QTextStream	t(&f);
		t << comments_["header"] << endl;
		t << "# Server" << endl << endl;

		t << comments_["servername"] << endl;
		t << "ServerName " << servername_ << endl;

		t << endl << comments_["serveradmin"] << endl;
		t << "ServerAdmin " << serveradmin_ << endl;

		t << endl << comments_["classification"] << endl;
		t << "Classification ";
		switch (classification_)
		{
			default:
			case CLASS_NONE: t << "none"; break;
			case CLASS_CLASSIFIED: t << "classified"; break;
			case CLASS_CONFIDENTIAL: t << "confidential"; break;
			case CLASS_SECRET: t << "secret"; break;
			case CLASS_TOPSECRET: t << "topsecret"; break;
			case CLASS_UNCLASSIFIED: t << "unclassified"; break;
			case CLASS_OTHER: t << otherclassname_; break;
		}
		t << endl;

		t << endl << comments_["classifyoverride"] << endl;
		if (classification_ != CLASS_NONE) t << "ClassifyOverride " << (classoverride_ ? "Yes" : "No") << endl;

		t << endl << comments_["defaultcharset"] << endl;
		t << "DefaultCharset " << charset_.upper() << endl;

		t << endl << comments_["defaultlanguage"] << endl;
		t << "DefaultLanguage " << language_.lower() << endl;

		t << endl << comments_["printcap"] << endl;
		t << "Printcap " << printcap_ << endl;

		t << endl << comments_["printcapformat"] << endl;
		t << "PrintcapFormat " << (printcapformat_ == PRINTCAP_SOLARIS ? "Solaris" : "BSD") << endl;

		t << endl << "# Security" << endl;
		t << endl << comments_["remoteroot"] << endl;
		t << "RemoteRoot " << remoteroot_ << endl;

		t << endl << comments_["systemgroup"] << endl;
		t << "SystemGroup " << systemgroup_ << endl;

		t << endl << comments_["servercertificate"] << endl;
		t << "ServerCertificate " << encryptcert_ << endl;

		t << endl << comments_["serverkey"] << endl;
		t << "ServerKey " << encryptkey_ << endl;

		t << endl << comments_["locations"] << endl;
		for (locations_.first(); locations_.current(); locations_.next())
		{
			CupsLocation *loc = locations_.current();
			t << "<Location " << loc->resourcename_ << ">" << endl;
			if (loc->authtype_ != AUTHTYPE_NONE)
			{
				t << "AuthType ";
				switch (loc->authtype_)
				{
					case AUTHTYPE_BASIC: t << "Basic"; break;
					case AUTHTYPE_DIGEST: t << "Digest"; break;
				}
				t << endl;
			}
			if (loc->authclass_ != AUTHCLASS_ANONYMOUS)
			{
				switch (loc->authclass_)
				{
					case AUTHCLASS_USER:
						if (!loc->authname_.isEmpty())
							t << "Require user " << loc->authname_ << endl;
						else
							t << "AuthClass User" << endl;
						break;
					case AUTHCLASS_GROUP:
						if (!loc->authname_.isEmpty())
							t << "Require group " << loc->authname_ << endl;
						else
							t << "AuthClass Group" << endl;
						break;
					case AUTHCLASS_SYSTEM:
						t << "AuthClass System" << endl;
						break;
				}
			}
			t << "Encryption ";
			switch (loc->encryption_)
			{
				case ENCRYPT_ALWAYS: t << "Always"; break;
				case ENCRYPT_NEVER: t << "Never"; break;
				case ENCRYPT_REQUIRED: t << "Required"; break;
				default:
				case ENCRYPT_IFREQUESTED: t << "IfRequested"; break;
			}
			t << endl;
			t << "Satisfy " << (loc->satisfy_ == SATISFY_ALL ? "All" : "Any") << endl;
			t << "Order " << (loc->order_ == ORDER_ALLOW_DENY ? "allow,deny" : "deny,allow") << endl;
			for (QStringList::ConstIterator it=loc->addresses_.begin(); it!=loc->addresses_.end(); ++it)
				t << *it << endl;
			t << "</Location>" << endl;
		}

		t << endl << "# Network" << endl;
		t << endl << comments_["hostnamelookups"] << endl;
		t << "HostnameLookups ";
		switch (hostnamelookup_)
		{
			default:
			case HOSTNAME_OFF: t << "Off"; break;
			case HOSTNAME_ON: t << "On"; break;
			case HOSTNAME_DOUBLE: t << "Double"; break;
		}
		t << endl;

		t << endl << comments_["keepalive"] << endl;
		t << "KeepAlive " << (keepalive_ ? "On" : "Off") << endl;

		t << endl << comments_["keepalivetimeout"] << endl;
		t << "KeepAliveTimeout " << keepalivetimeout_ << endl;

		t << endl << comments_["maxclients"] << endl;
		t << "MaxClients " << maxclients_ << endl;

		t << endl << comments_["maxrequestsize"] << endl;
		t << "MaxRequestSize " << maxrequestsize_ << endl;

		t << endl << comments_["timeout"] << endl;
		t << "Timeout " << clienttimeout_ << endl;

		t << endl << comments_["listen"] << endl;
		for (QStringList::ConstIterator it=listenaddresses_.begin(); it!=listenaddresses_.end(); ++it)
			t << *it << endl;

		t << endl << "# Log" << endl;
		t << endl << comments_["accesslog"] << endl;
		t << "AccessLog " << accesslog_ << endl;

		t << endl << comments_["errorlog"] << endl;
		t << "ErrorLog " << errorlog_ << endl;

		t << endl << comments_["pagelog"] << endl;
		t << "PageLog " << pagelog_ << endl;

		t << endl << comments_["maxlogsize"] << endl;
		//t << "MaxLogSize " << maxlogsize_ << "m" << endl;
		t << "MaxLogSize " << maxlogsize_ << endl;

		t << endl << comments_["loglevel"] << endl;
		t << "LogLevel ";
		switch (loglevel_)
		{
			case LOGLEVEL_NONE: t << "none"; break;
			default:
			case LOGLEVEL_INFO: t << "info"; break;
			case LOGLEVEL_ERROR: t << "error"; break;
			case LOGLEVEL_WARN: t << "warn"; break;
			case LOGLEVEL_DEBUG: t << "debug"; break;
			case LOGLEVEL_DEBUG2: t << "debug2"; break;
		}
		t << endl;

		t << endl << "# Jobs" << endl;
		t << endl << comments_["preservejobhistory"] << endl;
		t << "PreserveJobHistory " << (keepjobhistory_ ? "On" : "Off") << endl;

		t << endl << comments_["preservejobfiles"] << endl;
		if (keepjobhistory_) t << "PreserveJobFiles " << (keepjobfiles_ ? "On" : "Off") << endl;

		t << endl << comments_["autopurgejobs"] << endl;
		if (keepjobhistory_) t << "AutoPurgeJobs " << (autopurgejobs_ ? "Yes" : "No") << endl;

		t << endl << comments_["maxjobs"] << endl;
		t << "MaxJobs " << maxjobs_ << endl;

		t << endl << comments_["maxjobsperprinter"] << endl;
		t << "MaxJobsPerPrinter " << maxjobsperprinter_ << endl;

		t << endl << comments_["maxjobsperuser"] << endl;
		t << "MaxJobsPerUser " << maxjobsperuser_ << endl;

		t << endl << "# Filter" << endl;
		t << endl << comments_["user"] << endl;
		t << "User " << user_ << endl;

		t << endl << comments_["group"] << endl;
		t << "Group " << group_ << endl;

		t << endl << comments_["ripcache"] << endl;
		t << "RIPCache " << ripcache_ << endl;

		t << endl << comments_["filterlimit"] << endl;
		t << "FilterLimit " << filterlimit_ << endl;

		t << endl << "# Directories" << endl;
		t << endl << comments_["datadir"] << endl;
		t << "DataDir " << datadir_ << endl;

		t << endl << comments_["documentroot"] << endl;
		t << "DocumentRoot " << documentdir_ << endl;

		t << endl << comments_["fontpath"] << endl;
		for (QStringList::ConstIterator it=fontpath_.begin(); it!=fontpath_.end(); ++it)
			t << "FontPath " << *it << endl;

		t << endl << comments_["requestroot"] << endl;
		t << "RequestRoot " << requestdir_ << endl;

		t << endl << comments_["serverbin"] << endl;
		t << "ServerBin " << serverbin_ << endl;

		t << endl << comments_["serverroot"] << endl;
		t << "ServerRoot " << serverfiles_ << endl;

		t << endl << comments_["tempdir"] << endl;
		t << "TempDir " << tmpfiles_ << endl;

		t << endl << "# Browsing" << endl;
		t << endl << comments_["browsing"] << endl;
		t << "Browsing " << (browsing_ ? "On" : "Off") << endl;

		t << endl << comments_["browseprotocols"] << endl;
		if (browsing_)
		{
			t << "BrowseProtocols ";
			for (QStringList::ConstIterator it=browseprotocols_.begin(); it!=browseprotocols_.end(); ++it)
				t << (*it).upper() << " ";
			t << endl;
		}

		t << endl << comments_["browseport"] << endl;
		if (browsing_) t << "BrowsePort " << browseport_ << endl;

		t << endl << comments_["browseinterval"] << endl;
		if (browsing_) t << "BrowseInterval " << browseinterval_ << endl;

		t << endl << comments_["browsetimeout"] << endl;
		if (browsing_) t << "BrowseTimeout " << browsetimeout_ << endl;

		t << endl << comments_["browseaddress"] << endl;
		if (browsing_)
			for (QStringList::ConstIterator it=browseaddresses_.begin(); it!=browseaddresses_.end(); ++it)
				if ((*it).startsWith("Send"))
					t << "BrowseAddress " << (*it).mid(5) << endl;
				else
					t << "Browse" << (*it) << endl;

		t << endl << comments_["browseorder"] << endl;
		if (browsing_) t << "BrowseOrder " << (browseorder_ == ORDER_ALLOW_DENY ? "allow,deny" : "deny,allow") << endl;

		t << endl << comments_["implicitclasses"] << endl;
		if (browsing_) t << "ImplicitClasses " << (useimplicitclasses_ ? "On" : "Off") << endl;

		t << endl << comments_["implicitanyclasses"] << endl;
		if (browsing_) t << "ImplicitAnyClasses " << (useanyclasses_ ? "On" : "Off") << endl;

		t << endl << comments_["hideimplicitmembers"] << endl;
		if (browsing_) t << "HideImplicitMembers " << (hideimplicitmembers_ ? "Yes" : "No") << endl;

		t << endl << comments_["browseshortnames"] << endl;
		if (browsing_) t << "BrowseShortNames " << (useshortnames_ ? "Yes" : "No") << endl;
		
		t << endl << "# Unknown" << endl;
		for (QValueList< QPair<QString,QString> >::ConstIterator it=unknown_.begin(); it!=unknown_.end(); ++it)
			t << (*it).first << " " << (*it).second << endl;

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
	int p(-1);
	QString keyword, value, l(line.simplifyWhiteSpace());

	if ((p=l.find(' ')) != -1)
	{
		keyword = l.left(p).lower();
		value = l.mid(p+1);
	}
	else
	{
		keyword = l.lower();
	}

	//kdDebug() << "cupsd.conf keyword=" << keyword << endl;
	if (keyword == "accesslog") accesslog_ = value;
	else if (keyword == "autopurgejobs") autopurgejobs_ = (value.lower() == "yes");
	else if (keyword == "browseaddress") browseaddresses_.append("Send "+value);
	else if (keyword == "browseallow") browseaddresses_.append("Allow "+value);
	else if (keyword == "browsedeny") browseaddresses_.append("Deny "+value);
	else if (keyword == "browseinterval") browseinterval_ = value.toInt();
	else if (keyword == "browseorder") browseorder_ = (value.lower() == "deny,allow" ? ORDER_DENY_ALLOW : ORDER_ALLOW_DENY);
	else if (keyword == "browsepoll") browseaddresses_.append("Poll "+value);
	else if (keyword == "browseport") browseport_ = value.toInt();
	else if (keyword == "browseprotocols")
	{
		browseprotocols_.clear();
		QStringList prots = QStringList::split(QRegExp("\\s"), value, false);
		if (prots.find("all") != prots.end())
			browseprotocols_ << "CUPS" << "SLP";
		else
			for (QStringList::ConstIterator it=prots.begin(); it!=prots.end(); ++it)
				browseprotocols_ << (*it).upper();
	}
	else if (keyword == "browserelay") browseaddresses_.append("Relay "+value);
	else if (keyword == "browseshortnames") useshortnames_ = (value.lower() != "no");
	else if (keyword == "browsetimeout") browsetimeout_ = value.toInt();
	else if (keyword == "browsing") browsing_ = (value.lower() != "off");
	else if (keyword == "classification")
	{
		QString	cl = value.lower();
		if (cl == "none") classification_ = CLASS_NONE;
		else if (cl == "classified") classification_ = CLASS_CLASSIFIED;
		else if (cl == "confidential") classification_ = CLASS_CONFIDENTIAL;
		else if (cl == "secret") classification_ = CLASS_SECRET;
		else if (cl == "topsecret") classification_ = CLASS_TOPSECRET;
		else if (cl == "unclassified") classification_ = CLASS_UNCLASSIFIED;
		else
		{
			classification_ = CLASS_OTHER;
			otherclassname_ = cl;
		}
	}
	else if (keyword == "classifyoverride") classoverride_ = (value.lower() == "yes");
	else if (keyword == "datadir") datadir_ = value;
	else if (keyword == "defaultcharset") charset_ = value;
	else if (keyword == "defaultlanguage") language_ = value;
	else if (keyword == "documentroot") documentdir_ = value;
	else if (keyword == "errorlog") errorlog_ = value;
	else if (keyword == "filterlimit") filterlimit_ = value.toInt();
	else if (keyword == "fontpath") fontpath_ += QStringList::split(':', value, false);
	else if (keyword == "group") group_ = value;
	else if (keyword == "hideimplicitmembers") hideimplicitmembers_ = (value.lower() != "no");
	else if (keyword == "hostnamelookups")
	{
		QString h = value.lower();
		if (h == "on") hostnamelookup_ = HOSTNAME_ON;
		else if (h == "double") hostnamelookup_ = HOSTNAME_DOUBLE;
		else hostnamelookup_ = HOSTNAME_OFF;
	}
	else if (keyword == "implicitclasses") useimplicitclasses_ = (value.lower() != "off");
	else if (keyword == "implicitanyclasses") useanyclasses_ = (value.lower() == "on");
	else if (keyword == "keepalive") keepalive_ = (value.lower() != "off");
	else if (keyword == "keepalivetimeout") keepalivetimeout_ = value.toInt();
	else if (keyword == "listen") listenaddresses_.append("Listen "+value);
	else if (keyword == "loglevel")
	{
		QString ll = value.lower();
		if (ll == "none") loglevel_ = LOGLEVEL_NONE;
		else if (ll == "error") loglevel_ = LOGLEVEL_ERROR;
		else if (ll == "warn") loglevel_ = LOGLEVEL_WARN;
		else if (ll == "info") loglevel_ = LOGLEVEL_INFO;
		else if (ll == "debug") loglevel_ = LOGLEVEL_DEBUG;
		else if (ll == "debug2") loglevel_ = LOGLEVEL_DEBUG2;
	}
	else if (keyword == "maxclients") maxclients_ = value.toInt();
	else if (keyword == "maxjobs") maxjobs_ = value.toInt();
	else if (keyword == "maxjobsperprinter") maxjobsperprinter_ = value.toInt();
	else if (keyword == "maxjobsperuser") maxjobsperuser_ = value.toInt();
	else if (keyword == "maxrequestsize") maxrequestsize_ = value;
	else if (keyword == "maxlogsize") maxlogsize_ = value;
	/*{
		// FIXME: support for suffixes
		int suffix;
		splitSizeSpec( value, maxlogsize_, suffix );
	}*/
	else if (keyword == "pagelog") pagelog_ = value;
	else if (keyword == "port") listenaddresses_.append("Listen *:"+value);
	else if (keyword == "preservejobhistory") keepjobhistory_ = (value != "off");
	else if (keyword == "preservejobfiles") keepjobfiles_ = (value == "on");
	else if (keyword == "printcap") printcap_ = value;
	else if (keyword == "printcapformat") printcapformat_ = (value.lower() == "solaris" ? PRINTCAP_SOLARIS : PRINTCAP_BSD);
	else if (keyword == "requestroot") requestdir_ = value;
	else if (keyword == "remoteroot") remoteroot_ = value;
	else if (keyword == "ripcache") ripcache_ = value;
	else if (keyword == "serveradmin") serveradmin_ = value;
	else if (keyword == "serverbin") serverbin_ = value;
	else if (keyword == "servercertificate") encryptcert_ = value;
	else if (keyword == "serverkey") encryptkey_ = value;
	else if (keyword == "servername") servername_ = value;
	else if (keyword == "serverroot") serverfiles_ = value;
	else if (keyword == "ssllisten") listenaddresses_.append("SSLListen "+value);
	else if (keyword == "sslport") listenaddresses_.append("SSLListen *:"+value);
	else if (keyword == "systemgroup") systemgroup_ = value;
	else if (keyword == "tempdir") tmpfiles_ = value;
	else if (keyword == "timeout") clienttimeout_ = value.toInt();
	else if (keyword == "user") user_ = value;
	else
	{
		// unrecognized option
		unknown_ << QPair<QString,QString>(keyword, value);
	}
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
	resources_.append(new CupsResource("/jobs"));

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
	authtype_ = AUTHTYPE_NONE;
	authclass_ = AUTHCLASS_ANONYMOUS;
	authname_ = QString::null;
	encryption_ = ENCRYPT_IFREQUESTED;
	satisfy_ = SATISFY_ALL;
	order_ = ORDER_ALLOW_DENY;
	// addresses_
}

CupsLocation::CupsLocation(const CupsLocation& loc)
: resource_(loc.resource_),
  resourcename_(loc.resourcename_),
  authtype_(loc.authtype_),
  authclass_(loc.authclass_),
  authname_(loc.authname_),
  encryption_(loc.encryption_),
  satisfy_(loc.satisfy_),
  order_(loc.order_),
  addresses_(loc.addresses_)
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
	int p(-1);
	QString keyword, value, l(line.simplifyWhiteSpace());

	if ((p=l.find(' ')) != -1)
	{
		keyword = l.left(p).lower();
		value = l.mid(p+1);
	}
	else
	{
		keyword = l.lower();
	}

	if (keyword == "authtype")
	{
		QString a = value.lower();
		if (a == "basic") authtype_ = AUTHTYPE_BASIC;
		else if (a == "digest") authtype_ = AUTHTYPE_DIGEST;
		else authtype_ = AUTHTYPE_NONE;
	}
	else if (keyword == "authclass")
	{
		QString a = value.lower();
		if (a == "user") authclass_ = AUTHCLASS_USER;
		else if (a == "system") authclass_ = AUTHCLASS_SYSTEM;
		else if (a == "group") authclass_ = AUTHCLASS_GROUP;
		else authclass_ = AUTHCLASS_ANONYMOUS;
	}
	else if (keyword == "authgroupname") authname_ = value;
	else if (keyword == "require")
	{
		int p = value.find(' ');
		if (p != -1)
		{
			authname_ = value.mid(p+1);
			QString cl = value.left(p).lower();
			if (cl == "user")
				authclass_ = AUTHCLASS_USER;
			else if (cl == "group")
				authclass_ = AUTHCLASS_GROUP;
		}
	}
	else if (keyword == "allow") addresses_.append("Allow "+value);
	else if (keyword == "deny") addresses_.append("Deny "+value);
	else if (keyword == "order") order_ = (value.lower() == "deny,allow" ? ORDER_DENY_ALLOW : ORDER_ALLOW_DENY);
	else if (keyword == "encryption")
	{
		QString e = value.lower();
		if (e == "always") encryption_ = ENCRYPT_ALWAYS;
		else if (e == "never") encryption_ = ENCRYPT_NEVER;
		else if (e == "required") encryption_ = ENCRYPT_REQUIRED;
		else encryption_ = ENCRYPT_IFREQUESTED;
	}
	else if (keyword == "satisfy") satisfy_ = (value.lower() == "any" ? SATISFY_ANY : SATISFY_ALL);
	else return false;
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
	if (text == i18n("Base", "Root") || text == i18n("All printers") || text == i18n("All classes") || text == i18n("Print jobs")) return RESOURCE_GLOBAL;
	else if (text == i18n("Administration")) return RESOURCE_ADMIN;
	else if (text.find(i18n("Class")) == 0) return RESOURCE_CLASS;
	else if (text.find(i18n("Printer")) == 0) return RESOURCE_PRINTER;
	else return RESOURCE_PRINTER;
}

int CupsResource::typeFromPath(const QString& path)
{
	if (path == "/admin") return RESOURCE_ADMIN;
	else if (path == "/printers" || path == "/classes" || path == "/" || path == "/jobs") return RESOURCE_GLOBAL;
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
	else if (text == i18n("Print jobs")) path = "/jobs";
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
	else if (path == "/jobs") text = i18n("Print jobs");
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
