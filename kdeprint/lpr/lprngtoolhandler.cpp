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

#include "lprngtoolhandler.h"
#include "printcapentry.h"
#include "kmprinter.h"
#include "util.h"
#include "lprsettings.h"
#include "driver.h"
#include "kmmanager.h"
#include "kprinter.h"

#include <qfile.h>
#include <qtextstream.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

LPRngToolHandler::LPRngToolHandler(KMManager *mgr)
: LprHandler("lprngtool", mgr)
{
}

bool LPRngToolHandler::validate(PrintcapEntry *entry)
{
	if (entry->comment.startsWith("##LPRNGTOOL##") &&
	    entry->comment.find("UNKNOWN") == -1)
		return true;
	return false;
}

bool LPRngToolHandler::completePrinter(KMPrinter *prt, PrintcapEntry *entry, bool shortmode)
{
	QString	str, lp;

	// look for type in comment
	QStringList	l = QStringList::split(' ', entry->comment, false);
	lp = entry->field("lp");
	if (l.count() < 1)
		return false;

	if (l[1] == "DEVICE" || l[1] == "SOCKET" || l[1] == "QUEUE")
		LprHandler::completePrinter(prt, entry, shortmode);
	else if (l[1] == "SMB")
	{
		QMap<QString,QString>	opts = parseXferOptions(entry->field("xfer_options"));
		QString	user, pass;
		loadAuthFile(LprSettings::self()->baseSpoolDir() + "/" + entry->name + "/" + opts["authfile"], user, pass);
		QString uri = buildSmbURI(
				opts[ "workgroup" ],
				opts[ "host" ],
				opts[ "printer" ],
				user,
				pass );
		prt->setDevice( uri );
		prt->setLocation(i18n("Network printer (%1)").arg("smb"));
	}

	// look for comment
	if (!(str=entry->field("cm")).isEmpty())
		prt->setDescription(str);

	// driver
	//if (!shortmode)
	//{
		if (!(str=entry->field("ifhp")).isEmpty())
		{
			QString	model;
			int	p = str.find("model");
			if (p != -1)
			{
				p = str.find('=', p);
				if (p != -1)
				{
					p++;
					int	q = str.find(',', p);
					if (q == -1)
						model = str.mid(p);
					else
						model = str.mid(p, q-p);
				}
			}
			prt->setDriverInfo(i18n("IFHP Driver (%1)").arg((model.isEmpty() ? i18n("unknown") : model)));
			prt->setOption("driverID", model);
		}
	//}
	return true;
}

QMap<QString,QString> LPRngToolHandler::parseXferOptions(const QString& str)
{
	int	p(0), q;
	QMap<QString,QString>	opts;
	QString	key, val;

	while (p < str.length())
	{
		key = val = QString();
		// skip spaces
		while (p < str.length() && str[p].isSpace())
			p++;
		q = p;
		while (q < str.length() && str[q] != '=')
			q++;
		key = str.mid(p, q-p);
		p = q+2;
		while (p < str.length() && str[p] != '"')
			p++;
		val = str.mid(q+2, p-q-2);
		if (!key.isEmpty())
			opts[key] = val;
		p++;
	}
	return opts;
}

void LPRngToolHandler::loadAuthFile(const QString& filename, QString& user, QString& pass)
{
	QFile	f(filename);
	if (f.open(QIODevice::ReadOnly))
	{
		QTextStream	t(&f);
		QString	line;
		while (!t.atEnd())
		{
			line = t.readLine().trimmed();
			if (line.isEmpty())
				continue;
			int	p = line.find('=');
			if (p != -1)
			{
				QString	key = line.left(p);
				if (key == "username")
					user = line.mid(p+1);
				else if (key == "password")
					pass = line.mid(p+1);
			}
		}
	}
}

DrMain* LPRngToolHandler::loadDriver(KMPrinter *prt, PrintcapEntry *entry, bool config)
{
	if (entry->field("lprngtooloptions").isEmpty())
	{
		manager()->setErrorMsg(i18n("No driver defined for that printer. It might be a raw printer."));
		return NULL;
	}

	DrMain*	driver = loadToolDriver(locate("data", "kdeprint/lprngtooldriver1"));
	if (driver)
	{
		QString	model = prt->option("driverID");
		driver->set("text", i18n("LPRngTool Common Driver (%1)").arg((model.isEmpty() ? i18n("unknown") : model)));
		if (!model.isEmpty())
			driver->set("driverID", model);
		QMap<QString,QString>	opts = parseZOptions(entry->field("prefix_z"));
		opts["lpr"] = entry->field("lpr");
		driver->setOptions(opts);
		// if not configuring, don't show the "lpr" options
		if (!config)
			driver->removeOptionGlobally("lpr");
	}
	return driver;
}

DrMain* LPRngToolHandler::loadDbDriver(const QString& s)
{
	int	p = s.find('/');
	DrMain*	driver = loadToolDriver(locate("data", "kdeprint/lprngtooldriver1"));
	if (driver)
		driver->set("driverID", s.mid(p+1));
	return driver;
}

QList< QPair<QString,QStringList> > LPRngToolHandler::loadChoiceDict(const QString& filename)
{
	QFile	f(filename);
	QList< QPair<QString,QStringList> >	dict;
	if (f.open(QIODevice::ReadOnly))
	{
		QTextStream	t(&f);
		QString	line, key;
		QStringList	l;
		while (!t.atEnd())
		{
			line = t.readLine().trimmed();
			if (line.startsWith("OPTION"))
			{
				if (l.count() > 0 && !key.isEmpty())
					dict << QPair<QString,QStringList>(key, l);
				l.clear();
				key.clear();
				if (line.contains('|') == 2 || line.right(7) == "BOOLEAN")
				{
					int	p = line.find('|', 7);
					key = line.mid(7, p-7);
				}
			}
			else if (line.startsWith("CHOICE"))
			{
				int	p = line.find('|', 7);
				l << line.mid(7, p-7);
			}
		}
	}
	return dict;
}

QMap<QString,QString> LPRngToolHandler::parseZOptions(const QString& optstr)
{
	QMap<QString,QString>	opts;
	QStringList	l = QStringList::split(',', optstr, false);
	if (l.count() == 0)
		return opts;
	
	if (m_dict.count() == 0)
		m_dict = loadChoiceDict(locate("data", "kdeprint/lprngtooldriver1"));

	QString	unknown;
	for (QStringList::ConstIterator it=l.begin(); it!=l.end(); ++it)
	{
		bool	found(false);
		for (QList< QPair<QString,QStringList> >::ConstIterator p=m_dict.begin(); p!=m_dict.end() && !found; ++p)
		{
			if ((*p).second.find(*it) != (*p).second.end())
			{
				opts[(*p).first] = (*it);
				found = true;
			}
		}
		if (!found)
		{
			unknown.append(*it).append(QLatin1Char( ',' ));
		}
	}
	if (!unknown.isEmpty())
	{
		unknown.truncate(unknown.length()-1);
		opts["filter"] = unknown;
	}
	return opts;
}

QString LPRngToolHandler::filterDir()
{
	return driverDirectory();
}

QString LPRngToolHandler::driverDirInternal()
{
	return locateDir("filters", "/usr/lib:/usr/local/lib:/opt/lib:/usr/libexec:/usr/local/libexec:/opt/libexec");
}

PrintcapEntry* LPRngToolHandler::createEntry(KMPrinter *prt)
{
	QString	prot = prt->deviceProtocol();
	if (prot != "parallel" && prot != "lpd" && prot != "smb" && prot != "socket")
	{
		manager()->setErrorMsg(i18n("Unsupported backend: %1.").arg(prot));
		return NULL;
	}
	PrintcapEntry	*entry = new PrintcapEntry;
	entry->addField("cm", Field::String, prt->description());
	QString	lp, comment("##LPRNGTOOL## ");
	if (prot == "parallel")
	{
		comment.append("DEVICE ");
		lp = prt->device().mid( 9 );
		entry->addField("rw@", Field::Boolean);
	}
	else if (prot == "socket")
	{
		comment.append("SOCKET ");
		KUrl url( prt->device() );
		lp = url.host();
		if (url.port() == 0)
			lp.append("%9100");
		else
			lp.append("%").append(QString::number(url.port()));
	}
	else if (prot == "lpd")
	{
		comment.append("QUEUE ");
		KUrl url( prt->device() );
		lp = url.path().mid(1) + "@" + url.host();
	}
	else if (prot == "smb")
	{
		comment.append("SMB ");
		lp = "| " + filterDir() + "/smbprint";
		QString	work, server, printer, user, passwd;
		if ( splitSmbURI( prt->device(), work, server, printer, user, passwd ) )
		{
			entry->addField("xfer_options", Field::String, QString::fromLatin1("authfile=\"auth\" crlf=\"0\" hostip=\"\" host=\"%1\" printer=\"%2\" remote_mode=\"SMB\" share=\"//%3/%4\" workgroup=\"%5\"").arg(server).arg(printer).arg(server).arg(printer).arg(work));
			QFile	authfile(LprSettings::self()->baseSpoolDir() + "/" + prt->printerName() + "/auth");
			if (authfile.open(QIODevice::WriteOnly))
			{
				QTextStream	t(&authfile);
				t << "username=" << user << endl;
				t << "password=" << passwd << endl;
				authfile.close();
			}
		}
		else
		{
			manager()->setErrorMsg( i18n( "Invalid printer backend specification: %1" ).arg( prt->device() ) );
			delete entry;
			return NULL;
		}
	}

	if (prt->driver())
	{
		DrMain	*driver = prt->driver();
		comment.append("filtertype=IFHP ifhp_options=status@,sync@,pagecount@,waitend@ printerdb_entry=");
		comment.append(driver->get("driverID"));
		entry->addField("ifhp", Field::String, QString::fromLatin1("model=%1,status@,sync@,pagecount@,waitend@").arg(driver->get("driverID")));
		entry->addField("lprngtooloptions", Field::String, QString::fromLatin1("FILTERTYPE=\"IFHP\" IFHP_OPTIONS=\"status@,sync@,pagecount@,waitend@\" PRINTERDB_ENTRY=\"%1\"").arg(driver->get("driverID")));
		QMap<QString,QString>	opts;
		QString	optstr;
		driver->getOptions(opts, false);
		for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
			if (it.key() != "lpr")
				optstr.append(*it).append(",");
		if (!optstr.isEmpty())
		{
			optstr.truncate(optstr.length()-1);
			entry->addField("prefix_z", Field::String, optstr);
		}
		if (!opts["lpr"].isEmpty())
			entry->addField("lpr", Field::String, opts["lpr"]);
	}

	entry->addField("lp", Field::String, lp);
	entry->comment = comment;

	return entry;
}

bool LPRngToolHandler::savePrinterDriver(KMPrinter*, PrintcapEntry *entry, DrMain *driver, bool *mustSave)
{
	// save options in the "prefix_z" field and tell the manager to save the printcap file
	QMap<QString,QString>	opts;
	QString	optstr;
	driver->getOptions(opts, false);
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		if (it.key() != "lpr")
			optstr.append(*it).append(",");
	if (!optstr.isEmpty())
		optstr.truncate(optstr.length()-1);
	// save options in any case, otherwise nothing will happen whn
	// options are reset to their default value
	entry->addField("prefix_z", Field::String, optstr);
	entry->addField("lpr", Field::String, opts["lpr"]);
	if (mustSave)
		*mustSave = true;
	return true;
}

QString LPRngToolHandler::printOptions(KPrinter *printer)
{
	QString	optstr;
	QMap<QString,QString>	opts = printer->options();
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
	{
		if (it.key().startsWith("kde-") || it.key().startsWith("_kde-") || it.key() == "lpr" || it.key().startsWith( "app-" ))
			continue;
		optstr.append(*it).append(",");
	}
	if (!optstr.isEmpty())
	{
		optstr.truncate(optstr.length()-1);
		optstr.prepend("-Z '").append("'");
	}
	return optstr;
}
