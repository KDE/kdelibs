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

#include "matichandler.h"
#include "printcapentry.h"
#include "kmprinter.h"
#include "matic.h"
#include "matichelper.h"
#include "driver.h"
#include "kpipeprocess.h"
#include "kmmanager.h"
#include "kprinter.h"
#include "lprsettings.h"
#include "util.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kprocess.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <stdlib.h>
#include <sys/wait.h>

MaticHandler::MaticHandler(KMManager *mgr)
: LprHandler("foomatic", mgr)
{
	QString	PATH = getenv("PATH");
	PATH.append(":/usr/sbin:/usr/local/sbin:/opt/sbin:/opt/local/sbin");
	m_exematicpath = KStandardDirs::findExe("lpdomatic", PATH);
	m_ncpath = KStandardDirs::findExe("nc");
	m_smbpath = KStandardDirs::findExe("smbclient");
	m_rlprpath = KStandardDirs::findExe("rlpr");
}

bool MaticHandler::validate(PrintcapEntry *entry)
{
	if (entry)
		return (entry->field("if").right(9) == "lpdomatic");
	return false;
}

KMPrinter* MaticHandler::createPrinter(PrintcapEntry *entry)
{
	if (entry && validate(entry))
	{
		KMPrinter	*prt = new KMPrinter;
		prt->setName(entry->name);
		prt->setPrinterName(entry->name);
		prt->setType(KMPrinter::Printer);
		//if (entry->field("lp") == "/dev/null" || entry->field("lp").isEmpty())
		//	prt->addType(KMPrinter::Remote);
		return prt;
	}
	return NULL;
}

bool MaticHandler::completePrinter(KMPrinter *prt, PrintcapEntry *entry, bool shortmode)
{
	QString	val = entry->field("lp");
	if (val == "/dev/null" || val.isEmpty())
	{
		prt->setLocation(i18n("Network printer"));
	}
	else
	{
		prt->setLocation(i18n("Local printer on %1").arg(val));
		KURL	url(val);
		if (val.find("usb") != -1)
			url.setProtocol("usb");
		else
			url.setProtocol("parallel");
		prt->setDevice(url);
	}
	prt->setDescription(entry->aliases.join(", "));

	if (!shortmode)
	{
		MaticBlock	*blk = loadMaticData(maticFile(entry));
		if (blk)
		{
			QString	postpipe = blk->arg("$postpipe");
			if (!postpipe.isEmpty())
			{
				KURL	url = parsePostpipe(postpipe);
				if (!url.isEmpty())
				{
					QString	ds = QString::fromLatin1("%1 (%2)").arg(prt->location()).arg(url.protocol());
					prt->setDevice(url);
					prt->setLocation(ds);
				}
			}

			MaticBlock	*varblk = blk->block("$VAR1");
			if (varblk)
			{
				prt->setManufacturer(varblk->arg("make"));
				prt->setModel(varblk->arg("model"));
				prt->setDriverInfo(QString::fromLatin1("%1 %2 (%3)").arg(prt->manufacturer()).arg(prt->model()).arg(varblk->arg("driver")));
			}
		}
	}

	return true;
}

MaticBlock* MaticHandler::loadMaticData(const QString& fname)
{
	QFile	f(fname);
	if (!f.exists() || !f.open(IO_ReadOnly))
	{
		kdDebug() << "unable to load matic data: " << fname << endl;
		return NULL;
	}

	QCString	buffer(f.size()+1);
	f.readBlock(buffer.data(), f.size());
	f.close();

	MaticBlock	*blk = ::loadMaticData(buffer.data());
	return blk;
}

DrMain* MaticHandler::loadMaticDriver(const QString& fname)
{
	MaticBlock	*blk = loadMaticData(fname);
	if (blk)
	{
		DrMain	*driver = maticToDriver(blk);
		delete blk;
		return driver;
	}
	else
		return NULL;
}

KURL MaticHandler::parsePostpipe(const QString& s)
{
	KURL	url;
	int	p = s.findRev('|');
	QStringList	args = QStringList::split(" ", s.right(s.length()-p-1));

	if (args.count() != 0)
	{
		// socket printer
		if (args[0].right(3) == "/nc")
		{
			url.setProtocol("socket");
			url.setHost(args[1]);
			if (args.count() > 2)
				url.setPort(args[2].toInt());
			else
				url.setPort(9100);
		}
		// smb printer
		else if (args[0].right(10) == "/smbclient")
		{
			url.setProtocol("smb");
			QStringList	host_components = QStringList::split(QRegExp("/|\\\\\""), args[1], false);
			QString	workgrp, user, pass;
			for (uint i=2; i<args.count(); i++)
			{
				if (args[i] == "-U")
					user = args[++i];
				else if (args[i] == "-W")
					workgrp = args[++i];
				else if (args[i][0] != '-' && i == 2)
					pass = args[i];
			}
			if (!workgrp.isEmpty())
			{
				url.setHost(workgrp);
				url.setPath("/"+host_components[0]+"/"+host_components[1]);
			}
			else
			{
				url.setHost(host_components[0]);
				url.setPath("/"+host_components[1]);
			}
			if (!user.isEmpty())
				url.setUser(user);
			if (!pass.isEmpty())
				url.setPass(pass);
		}
		// remote printer
		else if (args[0].right(5) == "/rlpr")
		{
			uint	i=1;
			while (i < args.count())
			{
				if (args[i].left(2) != "-P")
					i++;
				else
				{
					QString	host = (args[i].length() == 2 ? args[i+1] : args[i].right(args[i].length()-2));
					int	p = host.find("\\@");
					if (p != -1)
					{
						url.setProtocol("lpd");
						url.setHost(host.right(host.length()-p-2));
						url.setPath("/"+host.left(p));
					}
					break;
				}
			}
		}
	}

	return url;
}

QString MaticHandler::createPostpipe(const KURL& url)
{
	QString	prot = url.protocol();
	QString	str;
	if (prot == "socket")
	{
		str += ("| " + m_ncpath);
		str += (" " + url.host());
		if (url.port() != 0)
			str += (" " + QString::number(url.port()));
	}
	else if (prot == "lpd")
	{
		str += ("| " + m_rlprpath + " -q -h");
		QString	h = url.host(), p = url.path().mid(1);
		str += (" -P " + p + "\\@" + h);
	}
	else if (prot == "smb")
	{
		str += ("| (\\n echo \\\"print -\\\"\\n cat \\n) | " + m_smbpath);
		QString	work, server, printer;
		urlToSmb(url, work, server, printer);
		str += (" \\\"//" + server + "/" + printer + "\\\"");
		if (!url.pass().isEmpty())
			str += (" " + url.pass());
		if (!url.user().isEmpty())
			str += (" -U " + url.user());
		if (!work.isEmpty())
			str += (" -W " + work);
		str += " -N -P";
	}
	return str;
}

DrMain* MaticHandler::loadDriver(KMPrinter*, PrintcapEntry *entry, bool)
{
	// we need to use a copy of the driver, as the driver
	// is not self-contained. If the printer is removed (when
	// changing printer name), the template would be also removed
	QString	origfilename = maticFile(entry);
	QString	filename = locateLocal("tmp", "foomatic_" + kapp->randomString(8));
	::system(QFile::encodeName("cp " + KProcess::quote(origfilename) + " " + KProcess::quote(filename)));
	DrMain	*driver = loadMaticDriver(filename);
	if (driver)
	{
		driver->set("template", filename);
		driver->set("temporary", "true");
		return driver;
	}
	else
		return NULL;
}

DrMain* MaticHandler::loadDbDriver(const QString& path)
{
	QStringList	comps = QStringList::split('/', path, false);
	if (comps.count() < 3 || comps[0] != "foomatic")
	{
		manager()->setErrorMsg(i18n("Internal error."));
		return NULL;
	}

	QString	tmpFile = locateLocal("tmp", "foomatic_" + kapp->randomString(8));
	QString	PATH = getenv("PATH") + QString::fromLatin1(":/usr/sbin:/usr/local/sbin:/opt/sbin:/opt/local/sbin");
	QString	exe = KStandardDirs::findExe("foomatic-datafile", PATH);
	if (exe.isEmpty())
	{
		manager()->setErrorMsg(i18n("Unable to find the executable foomatic-datafile "
		                            "in your PATH. Check that Foomatic is correctly installed."));
		return NULL;
	}

	KPipeProcess	in;
	QFile		out(tmpFile);
	QString cmd = KProcess::quote(exe);
	cmd += " -t lpd -d ";
	cmd += KProcess::quote(comps[2]);
	cmd += " -p ";
	cmd += KProcess::quote(comps[1]);
	if (in.open(cmd) && out.open(IO_WriteOnly))
	{
		QTextStream	tin(&in), tout(&out);
		QString	line;
		while (!tin.atEnd())
		{
			line = tin.readLine();
			tout << line << endl;
		}
		in.close();
		out.close();

		DrMain	*driver = loadMaticDriver(tmpFile);
		if (driver)
		{
			driver->set("template", tmpFile);
			driver->set("temporary", tmpFile);
			return driver;
		}
	}
	manager()->setErrorMsg(i18n("Unable to create the Foomatic driver [%1,%2]. "
	                            "Either that driver does not exist, or you don't have "
	                            "the required permissions to perform that operation.").arg(comps[1]).arg(comps[2]));
	return NULL;
}

bool MaticHandler::savePrinterDriver(KMPrinter *prt, PrintcapEntry *entry, DrMain *driver, bool*)
{
	QFile	tmpFile(locateLocal("tmp", "foomatic_" + kapp->randomString(8)));
	QFile	inFile(driver->get("template"));
	QString	outFile = maticFile(entry);
	bool	result(false);
	QString	postpipe = createPostpipe(prt->device());

	if (inFile.open(IO_ReadOnly) && tmpFile.open(IO_WriteOnly))
	{
		QTextStream	tin(&inFile), tout(&tmpFile);
		QString	line, optname;
		int	p(-1), q(-1);
		if (!postpipe.isEmpty())
			tout << "$postpipe = \"" << postpipe << "\";" << endl;
		while (!tin.atEnd())
		{
			line = tin.readLine();
			if (line.stripWhiteSpace().startsWith("$postpipe"))
				continue;
			else if ((p = line.find("'name'")) != -1)
			{
				p = line.find('\'', p+6)+1;
				q = line.find('\'', p);
				optname = line.mid(p, q-p);
			}
			else if ((p = line.find("'default'")) != -1)
			{
				DrBase	*opt = driver->findOption(optname);
				if (opt)
				{
					tout << line.left(p+9) << " => '" << opt->valueText() << "'," << endl;
					continue;
				}
			}
			tout << line << endl;
		}
		inFile.close();
		tmpFile.close();

		QString	cmd = "mv " + KProcess::quote(tmpFile.name()) + " " + KProcess::quote(outFile);
		int	status = ::system(QFile::encodeName(cmd).data());
		QFile::remove(tmpFile.name());
		result = (status != -1 && WEXITSTATUS(status) == 0);
	}

	if (!result)
		manager()->setErrorMsg(i18n("You probably don't have the required permissions "
		                            "to perform that operation."));
	QFile::remove(tmpFile.name());
	if (!result || entry->field("ppdfile").isEmpty())
		return result;
	else
		return savePpdFile(driver, entry->field("ppdfile"));
}

bool MaticHandler::savePpdFile(DrMain *driver, const QString& filename)
{
	QString	mdriver(driver->get("matic_driver")), mprinter(driver->get("matic_printer"));
	if (mdriver.isEmpty() || mprinter.isEmpty())
		return true;

	QString	PATH = getenv("PATH") + QString::fromLatin1(":/usr/sbin:/usr/local/sbin:/opt/sbin:/opt/local/sbin");
	QString	exe = KStandardDirs::findExe("foomatic-datafile", PATH);
	if (exe.isEmpty())
	{
		manager()->setErrorMsg(i18n("Unable to find the executable foomatic-datafile "
		                            "in your PATH. Check that Foomatic is correctly installed."));
		return false;
	}

	KPipeProcess	in;
	QFile		out(filename);
	if (in.open(exe + " -t cups -d " + mdriver + " -p " + mprinter) && out.open(IO_WriteOnly))
	{
		QTextStream	tin(&in), tout(&out);
		QString	line, optname;
		QRegExp	re("^\\*Default(\\w+):"), foo("'name'\\s+=>\\s+'(\\w+)'"), foo2("'\\w+'\\s*,\\s*$");
		while (!tin.atEnd())
		{
			line = tin.readLine();
			if (line.startsWith("*% COMDATA #"))
			{
				if (line.find("'default'") != -1)
				{
					DrBase	*opt = (optname.isEmpty() ? NULL : driver->findOption(optname));
					if (opt)
					{
						line.replace(foo2, "'"+opt->valueText()+"',");
					}
				}
				else if (foo.search(line) != -1)
					optname = foo.cap(1);
			}
			else if (re.search(line) != -1)
			{
				DrBase	*opt = driver->findOption(re.cap(1));
				if (opt)
				{
					QString	val = opt->valueText();
					if (opt->type() == DrBase::Boolean)
						val = (val == "1" ? "True" : "False");
					tout << "*Default" << opt->name() << ": " << val << endl;
					continue;
				}
			}
			tout << line << endl;
		}
		in.close();
		out.close();

		return true;
	}
	manager()->setErrorMsg(i18n("Unable to create the Foomatic driver [%1,%2]. "
	                            "Either that driver does not exist, or you don't have "
	                            "the required permissions to perform that operation.").arg(mdriver).arg(mprinter));
	
	return false;
}

PrintcapEntry* MaticHandler::createEntry(KMPrinter *prt)
{
	QString	prot = prt->device().protocol();
	if ((prot != "lpd" || m_rlprpath.isEmpty()) &&
		(prot != "socket" || m_ncpath.isEmpty()) &&
		(prot != "smb" || m_smbpath.isEmpty()) &&
		 prot != "parallel")
	{
		manager()->setErrorMsg(i18n("Unsupported backend: %1.").arg(prot));
		return NULL;
	}
	if (m_exematicpath.isEmpty())
	{
		manager()->setErrorMsg(i18n("Unable to find executable lpdomatic. "
		                            "Check that Foomatic is correctly installed "
		                            "and that lpdomatic is installed in a standard "
		                            "location."));
		return NULL;
	}
	PrintcapEntry	*entry = new PrintcapEntry;
	entry->addField("lf", Field::String, "/var/log/lp-errs");
	entry->addField("lp", Field::String, (prot != "parallel" ? "/dev/null" : prt->device().path()));
	entry->addField("if", Field::String, m_exematicpath);
	if (LprSettings::self()->mode() == LprSettings::LPRng)
	{
		entry->addField("filter_options", Field::String, " --lprng $Z /etc/foomatic/lpd/"+prt->printerName()+".lom");
		entry->addField("force_localhost", Field::Boolean);
		entry->addField("ppdfile", Field::String, "/etc/foomatic/"+prt->printerName()+".ppd");
	}
	else
		entry->addField("af", Field::String, "/etc/foomatic/lpd/"+prt->printerName()+".lom");
	if (!prt->description().isEmpty())
		entry->aliases << prt->description();
	return entry;
}

bool MaticHandler::removePrinter(KMPrinter *prt, PrintcapEntry *entry)
{
	// remove Foomatic driver
	QString	af = entry->field("af");
	if (af.isEmpty())
		return true;
	if (!QFile::remove(af))
	{
		manager()->setErrorMsg(i18n("Unable to remove driver file %1.").arg(af));
		return false;
	}
	return true;
}

QString MaticHandler::printOptions(KPrinter *printer)
{
	QMap<QString,QString>	opts = printer->options();
	QString	str;
	for (QMap<QString,QString>::Iterator it=opts.begin(); it!=opts.end(); ++it)
	{
		if (it.key().startsWith("kde-") || it.key().startsWith("_kde"))
			continue;
		str += (" " + it.key() + "=" + (*it));
	}
	if (!str.isEmpty())
		str.prepend("-J '").append("'");
	return str;
}

QString MaticHandler::driverDirInternal()
{
	return locateDir("foomatic/db/source", "/usr/share:/usr/local/share:/opt/share");
}
