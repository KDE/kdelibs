/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#include <config.h>

#include "kmcupsmanager.h"
#include "kmprinter.h"
#include "ipprequest.h"
#include "cupsinfos.h"
#include "driver.h"
#include "matic.h"
#include "kmfactory.h"
#include "kmdbentry.h"
#include "cupsaddsmb2.h"
#include "ippreportdlg.h"
#include "kpipeprocess.h"
#include "util.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qsocket.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kdialogbase.h>
#include <kextendedsocket.h>
#include <cups/cups.h>
#include <cups/ppd.h>

#define ppdi18n(s)	i18n(QString::fromLocal8Bit(s).utf8())

void extractMaticData(QString& buf, const QString& filename);
QString printerURI(KMPrinter *p, bool useExistingURI = false);
QString downloadDriver(KMPrinter *p);

static int trials = 5;

//*****************************************************************************************************

KMCupsManager::KMCupsManager(QObject *parent, const char *name)
: KMManager(parent,name)
{
	// be sure to create the CupsInfos object -> password
	// management is handled correctly.
	CupsInfos::self();
	m_cupsdconf = 0;
	m_currentprinter = 0;
	m_socket = 0;

	setHasManagement(true);
	setPrinterOperationMask(KMManager::PrinterAll);
	setServerOperationMask(KMManager::ServerAll);

	// change LANG variable so that CUPS is always using
	// english language: translation may only come from the PPD
	// itself, or from KDE.
	setenv("LANG", "en", 1);
}

KMCupsManager::~KMCupsManager()
{
	//delete m_socket;
}

QString KMCupsManager::driverDbCreationProgram()
{
	return QString::fromLatin1("make_driver_db_cups");
}

QString KMCupsManager::driverDirectory()
{
	QString	d = cupsInstallDir();
	if (d.isEmpty())
		d = "/usr";
	d.append("/share/cups/model");
	// raw foomatic support
	d.append(":/usr/share/foomatic/db/source");
	return d;
}

QString KMCupsManager::cupsInstallDir()
{
	KConfig	*conf=  KMFactory::self()->printConfig();
	conf->setGroup("CUPS");
	QString	dir = conf->readEntry("InstallDir",QString::null);
	return dir;
}

void KMCupsManager::reportIppError(IppRequest *req)
{
	setErrorMsg(req->statusMessage());
}

bool KMCupsManager::createPrinter(KMPrinter *p)
{
	bool isclass = p->isClass(false), result(false);
	IppRequest	req;
	QString		uri;

	uri = printerURI(p,false);
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	// needed to avoid problems when changing printer name
	p->setUri(KURL(uri));

	if (isclass)
	{
		req.setOperation(CUPS_ADD_CLASS);
		QStringList	members = p->members(), uris;
		QString		s = QString::fromLocal8Bit("ipp://%1:%2/printers/").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port());
		for (QStringList::ConstIterator it=members.begin(); it!=members.end(); ++it)
			uris.append(s+(*it));
		req.addURI(IPP_TAG_PRINTER,"member-uris",uris);
	}
	else
	{
		req.setOperation(CUPS_ADD_PRINTER);
		// only set the device-uri if needed, otherwise you may loose authentification
		// data (login/password in URI's like smb or ipp).
		KMPrinter	*otherP = findPrinter(p->printerName());
		if (!otherP || otherP->device() != p->device())
		{
			// do not encode special chars in login/passwd in SMB devices
			// to be compatible with CUPS/smbspool
			if (p->device().protocol() == "smb")
				req.addURI(IPP_TAG_PRINTER, "device-uri", urlToSmb(p->device()));
			else
				req.addURI(IPP_TAG_PRINTER,"device-uri",p->device().url());
		}
		if (!p->option("kde-banners").isEmpty())
		{
			QStringList	bans = QStringList::split(',',p->option("kde-banners"),false);
			while (bans.count() < 2)
				bans.append("none");
			req.addName(IPP_TAG_PRINTER,"job-sheets-default",bans);
		}
		req.addInteger(IPP_TAG_PRINTER,"job-quota-period",p->option("job-quota-period").toInt());
		req.addInteger(IPP_TAG_PRINTER,"job-k-limit",p->option("job-k-limit").toInt());
		req.addInteger(IPP_TAG_PRINTER,"job-page-limit",p->option("job-page-limit").toInt());
		if (!p->option("requesting-user-name-denied").isEmpty())
			req.addName(IPP_TAG_PRINTER,"requesting-user-name-denied",QStringList::split(",",p->option("requesting-user-name-denied"),false));
		else if (!p->option("requesting-user-name-allowed").isEmpty())
			req.addName(IPP_TAG_PRINTER,"requesting-user-name-allowed",QStringList::split(",",p->option("requesting-user-name-allowed"),false));
		else
			req.addName(IPP_TAG_PRINTER,"requesting-user-name-allowed",QString::fromLatin1("all"));
	}
	req.addText(IPP_TAG_PRINTER,"printer-info",p->description());
	req.addText(IPP_TAG_PRINTER,"printer-location",p->location());

	if (req.doRequest("/admin/"))
	{
		result = true;
		if (p->driver())
			result = savePrinterDriver(p,p->driver());
		if (result)
			upPrinter(p, true);
	}
	else reportIppError(&req);

	return result;
}

bool KMCupsManager::removePrinter(KMPrinter *p)
{
	bool	result = setPrinterState(p,CUPS_DELETE_PRINTER);
	return result;
}

bool KMCupsManager::enablePrinter(KMPrinter *p, bool state)
{
	return setPrinterState(p, (state ? CUPS_ACCEPT_JOBS : CUPS_REJECT_JOBS));
}

bool KMCupsManager::startPrinter(KMPrinter *p, bool state)
{
	return setPrinterState(p, (state ? IPP_RESUME_PRINTER : IPP_PAUSE_PRINTER));
}

bool KMCupsManager::setDefaultPrinter(KMPrinter *p)
{
	return setPrinterState(p,CUPS_SET_DEFAULT);
}

bool KMCupsManager::setPrinterState(KMPrinter *p, int state)
{
	IppRequest	req;
	QString		uri;

	req.setOperation(state);
	uri = printerURI(p);
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	if (req.doRequest("/admin/"))
		return true;
	reportIppError(&req);
	return false;
}

bool KMCupsManager::completePrinter(KMPrinter *p)
{
	if (completePrinterShort(p))
	{
		// driver informations
		QString	ppdname = downloadDriver(p);
		ppd_file_t	*ppd = (ppdname.isEmpty() ? NULL : ppdOpenFile(ppdname.local8Bit()));
		if (ppd)
		{
			KMDBEntry	entry;
			// use the validation mechanism of KMDBEntry to
			// fill possible missing entries like manufacturer
			// or model.
			entry.manufacturer = ppd->manufacturer;
			entry.model = ppd->shortnickname;
			entry.modelname = ppd->modelname;
			// do not check the driver regarding the manager
			entry.validate(false);
			// update the KMPrinter object
			p->setManufacturer(entry.manufacturer);
			p->setModel(entry.model);
			p->setDriverInfo(QString::fromLocal8Bit(ppd->nickname));
			ppdClose(ppd);
		}
		if (!ppdname.isEmpty())
			QFile::remove(ppdname);

		return true;
	}
	return false;
}

bool KMCupsManager::completePrinterShort(KMPrinter *p)
{
	IppRequest	req;
	QStringList	keys;
	QString		uri;

	req.setOperation(IPP_GET_PRINTER_ATTRIBUTES);
	uri = printerURI(p, true);
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	// change host and port for remote stuffs
	if (!p->uri().isEmpty())
	{
		req.setHost(p->uri().host());
		req.setPort(p->uri().port());
	}
	// disable location as it has been transferred to listing (for filtering)
	//keys.append("printer-location");
	keys.append("printer-info");
	keys.append("printer-make-and-model");
	keys.append("job-sheets-default");
	keys.append("job-sheets-supported");
	keys.append("job-quota-period");
	keys.append("job-k-limit");
	keys.append("job-page-limit");
	keys.append("requesting-user-name-allowed");
	keys.append("requesting-user-name-denied");
	if (p->isClass(true))
	{
		keys.append("member-uris");
		keys.append("member-names");
	}
	else
		keys.append("device-uri");
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

	if (req.doRequest("/printers/"))
	{
		QString	value;
		if (req.text("printer-info",value)) p->setDescription(value);
		// disabled location
		//if (req.text("printer-location",value)) p->setLocation(value);
		if (req.text("printer-make-and-model",value)) p->setDriverInfo(value);
		if (req.uri("device-uri",value))
		{
			// in case of SMB, encode special chars in login/passwd
			if (value.startsWith("smb://"))
				p->setDevice(smbToUrl(value));
			else
				p->setDevice(KURL(value));
		}
		QStringList	values;
/*		if (req.uri("member-uris",values))
		{
			QStringList	members;
			for (QStringList::ConstIterator it=values.begin(); it!=values.end(); ++it)
			{
				int	p = (*it).findRev('/');
				if (p != -1)
					members.append((*it).right((*it).length()-p-1));
			}
			p->setMembers(members);
		}*/
		if (req.name("member-names",values))
			p->setMembers(values);
		// banners
		req.name("job-sheets-default",values);
		while (values.count() < 2) values.append("none");
		p->setOption("kde-banners",values.join(QString::fromLatin1(",")));
		if (req.name("job-sheets-supported",values)) p->setOption("kde-banners-supported",values.join(QString::fromLatin1(",")));

		// quotas
		int	ival;
		if (req.integer("job-quota-period",ival)) p->setOption("job-quota-period",QString::number(ival));
		if (req.integer("job-k-limit",ival)) p->setOption("job-k-limit",QString::number(ival));
		if (req.integer("job-page-limit",ival)) p->setOption("job-page-limit",QString::number(ival));

		// access permissions (allow and deny are mutually exclusives)
		if (req.name("requesting-user-name-allowed",values) && values.count() > 0)
		{
			p->removeOption("requesting-user-name-denied");
			p->setOption("requesting-user-name-allowed",values.join(","));
		}
		if (req.name("requesting-user-name-denied",values) && values.count() > 0)
		{
			p->removeOption("requesting-user-name-allowed");
			p->setOption("requesting-user-name-denied",values.join(","));
		}

		return true;
	}

	reportIppError(&req);
	return false;
}

bool KMCupsManager::testPrinter(KMPrinter *p)
{
	return KMManager::testPrinter(p);
	/*
	QString	testpage = testPage();
	if (testpage.isEmpty())
	{
		setErrorMsg(i18n("Unable to locate test page."));
		return false;
	}

	IppRequest	req;
	QString		uri;

	req.setOperation(IPP_PRINT_JOB);
	uri = printerURI(p);
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	req.addMime(IPP_TAG_OPERATION,"document-format","application/postscript");
	if (!CupsInfos::self()->realLogin().isEmpty()) req.addName(IPP_TAG_OPERATION,"requesting-user-name",CupsInfos::self()->realLogin());
	req.addName(IPP_TAG_OPERATION,"job-name",QString::fromLatin1("KDE Print Test"));
	if (req.doFileRequest("/printers/",testpage))
		return true;
	reportIppError(&req);
	return false;
	*/
}

void KMCupsManager::listPrinters()
{
	loadServerPrinters();
}

void KMCupsManager::loadServerPrinters()
{
	IppRequest	req;
	QStringList	keys;

	// get printers
	req.setOperation(CUPS_GET_PRINTERS);
	keys.append("printer-name");
	keys.append("printer-type");
	keys.append("printer-state");
	// location needed for filtering
	keys.append("printer-location");
	keys.append("printer-uri-supported");
	keys.append("printer-is-accepting-jobs");
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

	if (req.doRequest("/printers/"))
	{
		processRequest(&req);

		// get classes
		req.init();
		req.setOperation(CUPS_GET_CLASSES);
		req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

		if (req.doRequest("/classes/"))
		{
			processRequest(&req);

			// load default
			req.init();
			req.setOperation(CUPS_GET_DEFAULT);
			req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",QString::fromLatin1("printer-name"));
			if (req.doRequest("/printers/"))
			{
				QString	s = QString::null;
				req.name("printer-name",s);
				setHardDefault(findPrinter(s));

				// everything went OK, just returns
				return;
			}
		}
	}

	// something went wrong if we get there, report the error
	reportIppError(&req);
}

void KMCupsManager::processRequest(IppRequest* req)
{
	ipp_attribute_t	*attr = req->first();
	KMPrinter	*printer = new KMPrinter();
	while (attr)
	{
		QString	attrname(attr->name);
		if (attrname == "printer-name")
		{
			QString	value = QString::fromLocal8Bit(attr->values[0].string.text);
			printer->setName(value);
			printer->setPrinterName(value);
		}
		else if (attrname == "printer-type")
		{
			int	value = attr->values[0].integer;
			printer->setType(0);
			printer->addType(((value & CUPS_PRINTER_CLASS) || (value & CUPS_PRINTER_IMPLICIT) ? KMPrinter::Class : KMPrinter::Printer));
			if ((value & CUPS_PRINTER_REMOTE)) printer->addType(KMPrinter::Remote);
			if ((value & CUPS_PRINTER_IMPLICIT)) printer->addType(KMPrinter::Implicit);
		}
		else if (attrname == "printer-state")
		{
			switch (attr->values[0].integer)
			{
				case IPP_PRINTER_IDLE: printer->setState(KMPrinter::Idle); break;
				case IPP_PRINTER_PROCESSING: printer->setState(KMPrinter::Processing); break;
				case IPP_PRINTER_STOPPED: printer->setState(KMPrinter::Stopped); break;
			}
		}
		else if (attrname == "printer-uri-supported")
		{
			printer->setUri(KURL(attr->values[0].string.text));
		}
		else if (attrname == "printer-location")
		{
			printer->setLocation(QString::fromLocal8Bit(attr->values[0].string.text));
		}
		else if (attrname == "printer-is-accepting-jobs")
		{
			printer->setAcceptJobs(attr->values[0].boolean);
		}
		if (attrname.isEmpty() || attr == req->last())
		{
			addPrinter(printer);
			printer = new KMPrinter();
		}
		attr = attr->next;
	}
}

DrMain* KMCupsManager::loadPrinterDriver(KMPrinter *p, bool)
{
	if (!p || p->isClass(true))
		return NULL;

	QString	fname = downloadDriver(p);
	DrMain	*driver(0);
	if (!fname.isEmpty())
	{
		driver = loadDriverFile(fname);
		if (driver)
			driver->set("temporary",fname);
	}

	return driver;
}

DrMain* KMCupsManager::loadFileDriver(const QString& filename)
{
	if (filename.startsWith("ppd:"))
		return loadDriverFile(filename.mid(4));
	else if (filename.startsWith("foomatic/"))
		return loadMaticDriver(filename);
	else
		return loadDriverFile(filename);
}

DrMain* KMCupsManager::loadMaticDriver(const QString& drname)
{
	QStringList	comps = QStringList::split('/', drname, false);
	QString	tmpFile = locateLocal("tmp", "foomatic_" + kapp->randomString(8));
	QString	PATH = getenv("PATH") + QString::fromLatin1(":/usr/sbin:/usr/local/sbin:/opt/sbin:/opt/local/sbin");
	QString	exe = KStandardDirs::findExe("foomatic-datafile", PATH);
	if (exe.isEmpty())
	{
		setErrorMsg(i18n("Unable to find the executable foomatic-datafile "
		                 "in your PATH. Check that Foomatic is correctly installed."));
		return NULL;
	}

	KPipeProcess	in;
	QFile		out(tmpFile);
	if (in.open(exe + " -t cups -d " + comps[2] + " -p " + comps[1]) && out.open(IO_WriteOnly))
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

		DrMain	*driver = loadDriverFile(tmpFile);
		if (driver)
		{
			driver->set("template", tmpFile);
			driver->set("temporary", tmpFile);
			return driver;
		}
	}
	setErrorMsg(i18n("Unable to create the Foomatic driver [%1,%2]. "
	                 "Either that driver does not exist, or you don't have "
	                 "the required permissions to perform that operation.").arg(comps[1]).arg(comps[2]));
	QFile::remove(tmpFile);
	return NULL;
}

DrMain* KMCupsManager::loadDriverFile(const QString& fname)
{
	if (QFile::exists(fname))
	{
		QString	unzipfname;
		if (!uncompressFile(fname,unzipfname))
			return NULL;

		DrMain	*driver = new DrMain();
		if (unzipfname.isEmpty()) unzipfname = fname;
		else driver->set("temporary",unzipfname);

		// at this point we can only work with unzipfname
		ppd_file_t	*ppd = ppdOpenFile(unzipfname.local8Bit());
		if (ppd)
		{
			ppdMarkDefaults(ppd);

			driver->set("text",QString::fromLocal8Bit(ppd->nickname));
			driver->set("manufacturer",QString::fromLocal8Bit(ppd->manufacturer));
			driver->set("model",QString::fromLocal8Bit(ppd->shortnickname));
			driver->set("description",QString::fromLocal8Bit(ppd->nickname));
			driver->set("template",unzipfname);
			driver->set("colordevice", QString::number(ppd->color_device));
			for (int i=0;i<ppd->num_groups;i++)
			{
				ppd_group_t	*grp = ppd->groups+i;
				DrGroup	*gr = new DrGroup();
				gr->set("text",ppdi18n(grp->text));
				bool 	fixed = QString::fromLocal8Bit(grp->text).contains("install",false);
				driver->addGroup(gr);
				for (int k=0;k<grp->num_options;k++)
				{
					ppd_option_t	*opt = grp->options+k;
					if (strcmp(opt->keyword,"PageRegion") == 0) continue;
					DrListOption	*op;
					switch (opt->ui)
					{
					   case PPD_UI_BOOLEAN:
						op = new DrBooleanOption();
						break;
					   case PPD_UI_PICKONE:
						op = new DrListOption();
						break;
					   default:
						continue;	// skip option
					}
					op->setName(QString::fromLocal8Bit(opt->keyword));
					op->set("text",ppdi18n(opt->text));
					op->set("default",QString::fromLocal8Bit(opt->defchoice));
					if (fixed) op->set("fixed","1");
					gr->addOption(op);
					for (int n=0;n<opt->num_choices;n++)
					{
						ppd_choice_t	*cho = opt->choices+n;
						DrBase	*ch = new DrBase();
						ch->setName(QString::fromLocal8Bit(cho->choice));
						ch->set("text",ppdi18n(cho->text));
						op->addChoice(ch);
					}
					op->setValueText(QString::fromLocal8Bit(opt->defchoice));
				}
			}

			// add constraints
			for (int i=0; i<ppd->num_consts; i++)
			{
				ppd_const_t	*cst = ppd->consts+i;
				driver->addConstraint(new DrConstraint(QString::fromLatin1(cst->option1),QString::fromLatin1(cst->option2),QString::fromLatin1(cst->choice1),QString::fromLatin1(cst->choice2)));
			}

			// add page sizes
			for (int i=0; i<ppd->num_sizes; i++)
			{
				ppd_size_t	*sz = ppd->sizes+i;
				driver->addPageSize(new DrPageSize(QString::fromLatin1(sz->name),(int)sz->width,(int)sz->length,(int)sz->left,(int)sz->bottom,(int)sz->right,(int)sz->top));
			}

			ppdClose(ppd);

			// try to extract Matic data
			QString	maticdata;
			DrGroup	*adjgrp(0);
			extractMaticData(maticdata,unzipfname);
			if (!maticdata.isEmpty())
			{
				driver->set("matic","1");

				MaticBlock	*blk = loadMaticData(maticdata.latin1()), *varblk(0), *argblk(0);
				if (blk) varblk = blk->block("$VAR1");
				if (varblk) argblk = varblk->block("args_byname");
				for (int i=0;i<2;i++)
				{
					if (argblk)
					{
						QDictIterator<MaticBlock>	it(argblk->m_blocks);
						for (;it.current();++it)
						{
							QString	type = it.current()->arg("type");
							if (type != "float" && type != "int" && type != "string")
								continue;	// skip it
							if (!adjgrp)
							{
								adjgrp = new DrGroup();
								adjgrp->set("text",i18n("Adjustments"));
								driver->addGroup(adjgrp);
							}
							DrBase	*opt(0);
							if (type == "float")
								opt = new DrFloatOption();
							else if (type == "int")
								opt = new DrIntegerOption();
							else
								opt = new DrStringOption();
							opt->setName(it.current()->arg("name"));
							opt->set("text",i18n(it.current()->arg("comment").local8Bit()));
							if (type == "float" || type == "int")
							{
								opt->set("minval",it.current()->arg("min"));
								opt->set("maxval",it.current()->arg("max"));
							}
							opt->setValueText(it.current()->arg("default"));
							opt->set("default",it.current()->arg("default"));
							// remove any option (list-type) with the same name (compliant
							// with the new PPD structure in Foomatic)
							DrBase	*oldOpt(0);
							if ((oldOpt=driver->findOption(opt->name())) && oldOpt->type() == DrBase::List)
							{
								DrListOption	*oldLOpt = static_cast<DrListOption*>(oldOpt);
								QString	fixedvals;
								QPtrListIterator<DrBase>	it(*(oldLOpt->choices()));
								for (; it.current(); ++it)
								{
									fixedvals.append(it.current()->name());
									if (!it.atLast())
										fixedvals.append("|");
								}
								opt->set("fixedvals", fixedvals);
							}
							driver->removeOptionGlobally(opt->name());
							// finally add the (new) numerical option
							adjgrp->addOption(opt);
						}
					}
					if (varblk) argblk = varblk->block("args");	// for new Foomatic structure
				}
				delete blk;
			}

			return driver;
		}
		else delete driver;
	}
	return NULL;
}

void KMCupsManager::saveDriverFile(DrMain *driver, const QString& filename)
{
	QFile	in(driver->get("template")), out(filename);
	if (in.exists() && in.open(IO_ReadOnly) && out.open(IO_WriteOnly))
	{
		QTextStream	tin(&in), tout(&out);
		QString		line, keyword;
		bool 		isnumeric(false);
		DrBase		*opt(0);

		while (!tin.eof())
		{
			line = tin.readLine();
			if (line.startsWith("*% COMDATA #"))
			{
				int	p(-1), q(-1);
				if ((p=line.find("'name'")) != -1)
				{
					p = line.find('\'',p+6)+1;
					q = line.find('\'',p);
					keyword = line.mid(p,q-p);
					opt = driver->findOption(keyword);
					if (opt && (opt->type() == DrBase::Integer || opt->type() == DrBase::Float))
						isnumeric = true;
					else
						isnumeric = false;
				}
				/*else if ((p=line.find("'type'")) != -1)
				{
					p = line.find('\'',p+6)+1;
					if (line.find("float",p) != -1 || line.find("int",p) != -1)
						isnumeric = true;
					else
						isnumeric = false;
				}*/
				else if ((p=line.find("'default'")) != -1 && !keyword.isEmpty() && opt && isnumeric)
				{
					QString	prefix = line.left(p+9);
					tout << prefix << " => '" << opt->valueText() << '\'';
					if (line.find(',',p) != -1)
						tout << ',';
					tout << endl;
					continue;
				}
				tout << line << endl;
			}
			else if (line.startsWith("*Default"))
			{
				int	p = line.find(':',8);
				keyword = line.mid(8,p-8);
				DrBase	*bopt = driver->findOption((keyword == "PageRegion" ? QString::fromLatin1("PageSize") : keyword));
				if (bopt)
					switch (bopt->type())
					{
						case DrBase::List:
						case DrBase::Boolean:
							{
								DrListOption	*opt = static_cast<DrListOption*>(bopt);
								if (opt && opt->currentChoice())
									tout << "*Default" << keyword << ": " << opt->currentChoice()->name() << endl;
								else
									tout << line << endl;
							}
							break;
						case DrBase::Integer:
							{
								DrIntegerOption	*opt = static_cast<DrIntegerOption*>(bopt);
								tout << "*Default" << keyword << ": " << opt->fixedVal() << endl;
							}
							break;
						case DrBase::Float:
							{
								DrFloatOption	*opt = static_cast<DrFloatOption*>(bopt);
								tout << "*Default" << keyword << ": " << opt->fixedVal() << endl;
							}
							break;
						default:
							tout << line << endl;
							break;
					}
				else
					tout << line << endl;
			}
			else
				tout << line << endl;
		}
	}
}

bool KMCupsManager::savePrinterDriver(KMPrinter *p, DrMain *d)
{
	QString	tmpfilename = locateLocal("tmp","print_") + kapp->randomString(8);

	// first save the driver in a temporary file
	saveDriverFile(d,tmpfilename);

	// then send a request
	IppRequest	req;
	QString		uri;
	bool		result(false);

	req.setOperation(CUPS_ADD_PRINTER);
	uri = printerURI(p, true);
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	result = req.doFileRequest("/admin/",tmpfilename);

	// remove temporary file
	QFile::remove(tmpfilename);

	if (!result)
		reportIppError(&req);
	return result;
}

void* KMCupsManager::loadCupsdConfFunction(const char *name)
{
	if (!m_cupsdconf)
	{
		m_cupsdconf = KLibLoader::self()->library("libcupsdconf");
		if (!m_cupsdconf)
		{
			setErrorMsg(i18n("Library libcupsdconf not found. Check your installation."));
			return NULL;
		}
	}
	void*	func = m_cupsdconf->symbol(name);
	if (!func)
		setErrorMsg(i18n("Symbol %1 not found in libcupsdconf library.").arg(name));
	return func;
}

void KMCupsManager::unloadCupsdConf()
{
	if (m_cupsdconf)
	{
		KLibLoader::self()->unloadLibrary("libcupsdconf");
		m_cupsdconf = 0;
	}
}

bool KMCupsManager::restartServer()
{
	QString	msg;
	bool (*f1)(QString&) = (bool(*)(QString&))loadCupsdConfFunction("restartServer");
	bool 	result(false);
	if (f1)
	{
		result = f1(msg);
		if (!result) setErrorMsg(msg);
	}
	unloadCupsdConf();
	return result;
}

bool KMCupsManager::configureServer(QWidget *parent)
{
	QString msg;
	bool (*f2)(QWidget*, QString&) = (bool(*)(QWidget*, QString&))loadCupsdConfFunction("configureServer");
	bool 	result(false);
	if (f2)
	{
		result = f2(parent, msg);
		if ( !result )
			setErrorMsg( msg );
	}
	unloadCupsdConf();
	return result;
}

QStringList KMCupsManager::detectLocalPrinters()
{
	QStringList	list;
	IppRequest	req;
	req.setOperation(CUPS_GET_DEVICES);
	if (req.doRequest("/"))
	{
		QString	desc, uri, printer, cl;
		ipp_attribute_t	*attr = req.first();
		while (attr)
		{
			QString	attrname(attr->name);
			if (attrname == "device-info") desc = attr->values[0].string.text;
			else if (attrname == "device-make-and-model") printer = attr->values[0].string.text;
			else if (attrname == "device-uri") uri = attr->values[0].string.text;
			else if ( attrname == "device-class" ) cl = attr->values[ 0 ].string.text;
			if (attrname.isEmpty() || attr == req.last())
			{
				if (!uri.isEmpty())
				{
					if (printer == "Unknown") printer = QString::null;
					list << cl << uri << desc << printer;
				}
				uri = desc = printer = cl = QString::null;
			}
			attr = attr->next;
		}
	}
	return list;
}

void KMCupsManager::createPluginActions(KActionCollection *coll)
{
	KAction	*act = new KAction(i18n("&Export Driver..."), "kdeprint_uploadsmb", 0, this, SLOT(exportDriver()), coll, "plugin_export_driver");
	act->setGroup("plugin");
	act = new KAction(i18n("&Printer IPP Report..."), "kdeprint_report", 0, this, SLOT(printerIppReport()), coll, "plugin_printer_ipp_report");
	act->setGroup("plugin");
}

void KMCupsManager::validatePluginActions(KActionCollection *coll, KMPrinter *pr)
{
	// save selected printer for future use in slots
	m_currentprinter = pr;
	coll->action("plugin_export_driver")->setEnabled(pr && pr->isLocal() && !pr->isClass(true) && !pr->isSpecial());
	coll->action("plugin_printer_ipp_report")->setEnabled(pr && !pr->isSpecial());
}

void KMCupsManager::exportDriver()
{
	if (m_currentprinter && m_currentprinter->isLocal() &&
	    !m_currentprinter->isClass(true) && !m_currentprinter->isSpecial())
	{
		QString	path = cupsInstallDir();
		if (path.isEmpty())
			path = "/usr/share/cups";
		else
			path += "/share/cups";
		CupsAddSmb::exportDest(m_currentprinter->printerName(), path);
	}
}

void KMCupsManager::printerIppReport()
{
	if (m_currentprinter && !m_currentprinter->isSpecial())
	{
		IppRequest	req;
		QString	uri;

		req.setOperation(IPP_GET_PRINTER_ATTRIBUTES);
		uri = printerURI(m_currentprinter, true);
		req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
		if (!m_currentprinter->uri().isEmpty())
		{
			req.setHost(m_currentprinter->uri().host());
			req.setPort(m_currentprinter->uri().port());
		}
		req.dump(2);
		if (req.doRequest("/printers/"))
		{
			ippReport(req, IPP_TAG_PRINTER, i18n("IPP report for %1").arg(m_currentprinter->printerName()));
		}
		else
		{
			KMessageBox::error(0, "<p>"+i18n("Unable to retrieve printer information. Error received:")+"</p>"+req.statusMessage());
		}
	}
}

void KMCupsManager::ippReport(IppRequest& req, int group, const QString& caption)
{
	IppReportDlg::report(&req, group, caption);
}

QString KMCupsManager::stateInformation()
{
	return i18n("Connected to %1:%2").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port());
}

void KMCupsManager::checkUpdatePossibleInternal()
{
	kdDebug(500) << "Checking for update possible" << endl;
	delete m_socket;
	/*m_socket = new KExtendedSocket( CupsInfos::self()->host(), CupsInfos::self()->port() );
	connect( m_socket, SIGNAL( connectionSuccess() ), SLOT( slotConnectionSuccess() ) );
	connect( m_socket, SIGNAL( connectionFailed( int ) ), SLOT( slotConnectionFailed( int ) ) );
	m_socket->setTimeout( 1 );*/
	m_socket = new QSocket( this );
	connect( m_socket, SIGNAL( connected() ), SLOT( slotConnectionSuccess() ) );
	connect( m_socket, SIGNAL( error( int ) ), SLOT( slotConnectionFailed( int ) ) );
	trials = 5;
	QTimer::singleShot( 1, this, SLOT( slotAsyncConnect() ) );
}

void KMCupsManager::slotConnectionSuccess()
{
	kdDebug(500) << "Connection success, trying to send a request..." << endl;
	m_socket->close();
	
	IppRequest req;
	req.setOperation( CUPS_GET_PRINTERS );
	req.addKeyword( IPP_TAG_OPERATION, "requested-attributes", QString::fromLatin1( "printer-name" ) );
	if ( req.doRequest( "/printers/" ) )
		setUpdatePossible( true );
	else
	{
		kdDebug(500) << "Unable to get printer list" << endl;
		if ( trials > 0 )
		{
			trials--;
			QTimer::singleShot( 1000, this, SLOT( slotAsyncConnect() ) );
		}
		else
		{
			setErrorMsg( i18n( "Connection to CUPS server failed. Check that the CUPS server is correctly installed and running. "
				"Error: %1." ).arg( i18n( "the IPP request failed for an unknown reason" ) ) );
			setUpdatePossible( false );
		}
	}
}

void KMCupsManager::slotAsyncConnect()
{
	kdDebug(500) << "Starting async connect" << endl;
	//m_socket->startAsyncConnect();
	m_socket->connectToHost( CupsInfos::self()->host(), CupsInfos::self()->port() );
}

void KMCupsManager::slotConnectionFailed( int errcode )
{
	kdDebug(500) << "Connection failed trials=" << trials << endl;
	if ( trials > 0 )
	{
		//m_socket->setTimeout( ++to );
		//m_socket->cancelAsyncConnect();
		trials--;
		m_socket->close();
		QTimer::singleShot( 1000, this, SLOT( slotAsyncConnect() ) );
		return;
	}

	setErrorMsg( i18n( "Connection to CUPS server failed. Check that the CUPS server is correctly installed and running. "
				"Error: %1." ).arg( errcode == QSocket::ErrConnectionRefused ? i18n( "connection refused" ) : i18n( "host not found" ) ) );
	setUpdatePossible( false );
}

//*****************************************************************************************************

void extractMaticData(QString& buf, const QString& filename)
{
	QFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		line;
		while (!t.eof())
		{
			line = t.readLine();
			if (line.startsWith("*% COMDATA #"))
				buf.append(line.right(line.length()-12)).append('\n');
		}
	}
}

QString printerURI(KMPrinter *p, bool use)
{
	QString	uri;
	if (use && !p->uri().isEmpty())
		uri = p->uri().prettyURL();
	else
		uri = QString("ipp://%1:%2/%4/%3").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port()).arg(p->printerName()).arg((p->isClass(false) ? "classes" : "printers"));
	return uri;
}

QString downloadDriver(KMPrinter *p)
{
	QString	driverfile, prname = p->printerName();
	bool	changed(false);

	if (!p->uri().isEmpty())
	{
		// try to load the driver from the host:port
		// specified in its URI. Doing so may also change
		// the printer name to use. Note that for remote
		// printer, this operation is read-only, no counterpart
		// for saving operation.
		cupsSetServer(p->uri().host().local8Bit());
		ippSetPort(p->uri().port());
		// strip any "@..." from the printer name
		prname = prname.replace(QRegExp("@.*"), "");
		changed = true;
	}

	// download driver
	driverfile = cupsGetPPD(prname.local8Bit());

	// restore host:port (if they have changed)
	if (changed)
	{
		cupsSetServer(CupsInfos::self()->host().local8Bit());
		ippSetPort(CupsInfos::self()->port());
	}

	return driverfile;
}

#include "kmcupsmanager.moc"
