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

#include "kprinterimpl.h"
#include "kprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmuimanager.h"
#include "kxmlcommand.h"
#include "kmspecialmanager.h"
#include "kmthreadjob.h"
#include "kmprinter.h"
#include "driver.h"

#include <qfile.h>
#include <qregexp.h>
#include <qinputdialog.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdatastream.h>
#include <kdebug.h>
#include <kmimemagic.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kconfig.h>

#include <stdlib.h>

void dumpOptions(const QMap<QString,QString>&);
void initEditPrinter(KMPrinter *p)
{
	if (!p->isEdited())
	{
		p->setEditedOptions(p->defaultOptions());
		p->setEdited(true);
	}
}

//****************************************************************************************

KPrinterImpl::KPrinterImpl(QObject *parent, const char *name)
: QObject(parent,name)
{
	loadAppOptions();
}

KPrinterImpl::~KPrinterImpl()
{
}

void KPrinterImpl::preparePrinting(KPrinter *printer)
{
	// page size -> try to find page size and margins from driver file
	// use "PageSize" as option name to find the wanted page size. It's
	// up to the driver loader to use that option name.
	KMManager	*mgr = KMFactory::self()->manager();
	DrMain	*driver = mgr->loadPrinterDriver(mgr->findPrinter(printer->printerName()), false);
	if (driver)
	{
		QString	psname = printer->option("PageSize");
		if (psname.isEmpty())
		{
			DrListOption	*opt = (DrListOption*)driver->findOption("PageSize");
			if (opt) psname = opt->get("default");
		}
		if (!psname.isEmpty())
		{
			printer->setOption("kde-pagesize",QString::number((int)pageNameToPageSize(psname)));
			DrPageSize	*ps = driver->findPageSize(psname);
			if (ps)
			{
				printer->setRealPageSize(ps->pageSize());
				printer->setMargins(ps->margins());
				printer->setRealDrawableArea( ps->pageRect() );
			}
		}
		delete driver;
	}

}

bool KPrinterImpl::setupCommand(QString&, KPrinter*)
{
	return false;
}

bool KPrinterImpl::printFiles(KPrinter *p, const QStringList& f, bool flag)
{
	QString	cmd;
	if (p->option("kde-isspecial") == "1")
	{
		if (p->option("kde-special-command").isEmpty() && p->outputToFile())
		{
			if (f.count() > 1)
			{
				p->setErrorMessage(i18n("Cannot copy multiple files into one file."));
				return false;
			}
			else
			{
				KProcess proc;
				proc << (flag?"mv":"cp") << f[0] << p->outputFileName();
				if (!proc.start(KProcess::Block) || !proc.normalExit() || proc.exitStatus() != 0)
				{
					p->setErrorMessage(i18n("Cannot save print file to %1. Check that you have write access to it.").arg(p->outputFileName()));
					return false;
				}
			}
			return true;
		}
		if (!setupSpecialCommand(cmd,p,f))
			return false;
	}
	else if (!setupCommand(cmd,p))
		return false;
	return startPrinting(cmd,p,f,flag);
}

void KPrinterImpl::broadcastOption(const QString& key, const QString& value)
{
	// force printer listing if not done yet (or reload needed)
	QPtrList<KMPrinter>	*printers = KMFactory::self()->manager()->printerListComplete(false);
	if (printers)
	{
		QPtrListIterator<KMPrinter>	it(*printers);
		for (;it.current();++it)
		{
			initEditPrinter(it.current());
			it.current()->setEditedOption(key,value);
		}
	}
}

int KPrinterImpl::dcopPrint(const QString& cmd, const QStringList& files, bool removeflag)
{
	kdDebug(500) << "kdeprint: print command: " << cmd << endl;

	int result = 0;
	DCOPClient	*dclient = kapp->dcopClient();
	if (!dclient || (!dclient->isAttached() && !dclient->attach()))
	{
		return result;
	}

	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg( data, IO_WriteOnly );
	arg << cmd;
	arg << files;
	arg << removeflag;
	if (dclient->call( "kded", "kdeprintd", "print(QString,QStringList,bool)", data, replyType, replyData ))
	{
		if (replyType == "int")
		{
			QDataStream _reply_stream( replyData, IO_ReadOnly );
			_reply_stream >> result;
		}
	}
	return result;
}

void KPrinterImpl::statusMessage(const QString& msg, KPrinter *printer)
{
	kdDebug(500) << "kdeprint: status message: " << msg << endl;
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	if (!conf->readBoolEntry("ShowStatusMsg", true))
		return;

	QString	message(msg);
	if (printer && !msg.isEmpty())
		message.prepend(i18n("Printing document: %1").arg(printer->docName())+"\n");

	DCOPClient	*dclient = kapp->dcopClient();
	if (!dclient || (!dclient->isAttached() && !dclient->attach()))
	{
		return;
	}

	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg( data, IO_WriteOnly );
	arg << message;
	arg << (int)getpid();
	arg << kapp->caption();
	dclient->call( "kded", "kdeprintd", "statusMessage(QString,int,QString)", data, replyType, replyData );
}

bool KPrinterImpl::startPrinting(const QString& cmd, KPrinter *printer, const QStringList& files, bool flag)
{
	statusMessage(i18n("Sending print data to printer: %1").arg(printer->printerName()), printer);

	QString	command(cmd), filestr;
	QStringList	printfiles;
	if (command.find("%in") == -1) command.append(" %in");

	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		if (QFile::exists(*it))
		{
			// quote and encode filenames
			filestr.append(quote(QFile::encodeName(*it))).append(" ");;
			printfiles.append(*it);
		}
		else
			kdDebug(500) << "File not found: " << (*it) << endl;

	if (printfiles.count() > 0)
	{
		command.replace(QRegExp("%in"),filestr);
		int pid = dcopPrint(command,files,flag);
		if (pid > 0)
		{
			if (printer)
				KMThreadJob::createJob(pid,printer->printerName(),printer->docName(),getenv("USER"),0);
			return true;
		}
		else
		{
			QString	msg = i18n("Unable to start child print process. ");
			if (pid == 0)
				msg += i18n("The KDE print server (<b>kdeprintd</b>) could not be contacted. Check that this server is running.");
			else
				msg += i18n("1 is the command that <files> is given to", "Check the command syntax:\n%1 <files>").arg(cmd);
			printer->setErrorMessage(msg);
			return false;
		}
	}
	//else
	//{
		printer->setErrorMessage(i18n("No valid file was found for printing. Operation aborted."));
		return false;
	//}
}

QString KPrinterImpl::tempFile()
{
	QString	f;
	// be sure the file doesn't exist
	do f = locateLocal("tmp","kdeprint_") + KApplication::randomString(8); while (QFile::exists(f));
	return f;
}

int KPrinterImpl::filterFiles(KPrinter *printer, QStringList& files, bool flag)
{
	QStringList	flist = QStringList::split(',',printer->option("_kde-filters"),false);
	QMap<QString,QString>	opts = printer->options();

	// generic page selection mechanism (using psselect filter)
	// do it only if:
	//	- using system-side page selection
	//	- special printer or regular printer without page selection support in current plugin
	//	- one of the page selection option has been selected to non default value
	// Action -> add the psselect filter to the filter chain.
	if (printer->pageSelection() == KPrinter::SystemSide &&
	    (printer->option("kde-isspecial") == "1" || !(KMFactory::self()->uiManager()->pluginPageCap() & KMUiManager::PSSelect)) &&
	    (printer->pageOrder() == KPrinter::LastPageFirst ||
	     !printer->option("kde-range").isEmpty() ||
	     printer->pageSet() != KPrinter::AllPages))
	{
		if (flist.findIndex("psselect") == -1)
		{
			int	index = KXmlCommandManager::self()->insertCommand(flist, "psselect", false);
			if (index == -1 || !KXmlCommandManager::self()->checkCommand("psselect"))
			{
				printer->setErrorMessage(i18n("<p>Unable to perform the requested page selection. The filter <b>psselect</b> "
							      "cannot be inserted in the current filter chain. See <b>Filter</b> tab in the "
							      "printer properties dialog for further information.</p>"));
				return -1;
			}
		}
		if (printer->pageOrder() == KPrinter::LastPageFirst)
			opts["_kde-psselect-order"] = "r";
		if (!printer->option("kde-range").isEmpty())
			opts["_kde-psselect-range"] = printer->option("kde-range");
		if (printer->pageSet() != KPrinter::AllPages)
			opts["_kde-psselect-set"] = (printer->pageSet() == KPrinter::OddPages ? "-o" : "-e");
	}

	return doFilterFiles(printer, files, flist, opts, flag);
}

int KPrinterImpl::doFilterFiles(KPrinter *printer, QStringList& files, const QStringList& flist, const QMap<QString,QString>& opts, bool flag)
{
	// nothing to do
	if (flist.count() == 0)
		return 0;

	QString	filtercmd;
	QStringList	inputMimeTypes;
	for (uint i=0;i<flist.count();i++)
	{
		KXmlCommand	*filter = KXmlCommandManager::self()->loadCommand(flist[i]);
		if (i == 0)
			inputMimeTypes = filter->inputMimeTypes();

		QString		subcmd = filter->buildCommand(opts,(i>0),(i<(flist.count()-1)));
		delete filter;
		if (!subcmd.isEmpty())
		{
			filtercmd.append(subcmd);
			if (i < flist.count()-1)
				filtercmd.append("| ");
		}
		else
		{
			printer->setErrorMessage(i18n("<p>Error while reading filter description for <b>%1</b>. Empty command line received.</p>").arg(flist[i]));
			return -1;
		}
	}
	kdDebug(500) << "kdeprint: filter command: " << filtercmd << endl;

	QRegExp	rin("%in"), rout("%out"), rpsl("%psl"), rpsu("%psu");
	QString	ps = pageSizeToPageName( printer->option( "kde-printsize" ).isEmpty() ? printer->pageSize() : ( KPrinter::PageSize )printer->option( "kde-printsize" ).toInt() );
	for (QStringList::Iterator it=files.begin(); it!=files.end(); ++it)
	{
		QString	mime = KMimeMagic::self()->findFileType(*it)->mimeType();
		if (inputMimeTypes.find(mime) == inputMimeTypes.end())
		{
			if (KMessageBox::warningContinueCancel(0,
				"<p>" + i18n("The MIME type %1 is not supported as input of the filter chain "
				     "(this may happen with non-CUPS spoolers when performing page selection "
				     "on a non-PostScript file). Do you want KDE to convert the file to a supported "
				     "format?</p>").arg(mime),
				QString::null, i18n("Convert")) == KMessageBox::Continue)
			{
				QStringList	ff;
				int	done(0);

				ff << *it;
				while (done == 0)
				{
					bool	ok(false);
					QString	targetMime = QInputDialog::getItem(
						i18n("Select MIME Type"),
						i18n("Select the target format for the conversion:"),
						inputMimeTypes, 0, false, &ok);
					if (!ok)
					{
						printer->setErrorMessage(i18n("Operation aborted."));
						return -1;
					}
					QStringList	filters = KXmlCommandManager::self()->autoConvert(mime, targetMime);
					if (filters.count() == 0)
					{
						KMessageBox::error(0, i18n("No appropriate filter found. Select another target format."));
					}
					else
					{
						int	result = doFilterFiles(printer, ff, filters, QMap<QString,QString>(), flag);
						if (result == 1)
						{
							*it = ff[0];
							done = 1;
						}
						else
						{
							KMessageBox::error(0,
								i18n("<qt>Operation failed with message:<br>%1<br>Select another target format.</qt>").arg(printer->errorMessage()));
						}
					}
				}
			}
			else
			{
				printer->setErrorMessage(i18n("Operation aborted."));
				return -1;
			}
		}

		QString	tmpfile = tempFile();
		QString	cmd(filtercmd);
		cmd.replace(rout,quote(tmpfile));
		cmd.replace(rpsl,ps.lower());
		cmd.replace(rpsu,ps);
		cmd.replace(rin,quote(*it)); // Replace as last, filename could contain "%psl"
		statusMessage(i18n("Filtering print data"), printer);
		int status = system(QFile::encodeName(cmd));
		if (status < 0 || WEXITSTATUS(status) == 127)
		{
			printer->setErrorMessage(i18n("Error while filtering. Command was: <b>%1</b>.").arg(filtercmd));
			return -1;
		}
		if (flag) QFile::remove(*it);
		*it = tmpfile;
	}
	return 1;
}

int KPrinterImpl::autoConvertFiles(KPrinter *printer, QStringList& files, bool flag)
{
	KMFactory::PluginInfo	info = KMFactory::self()->pluginInfo(KMFactory::self()->printSystem());
	int		status(0), result;
	for (QStringList::Iterator it=files.begin(); it!=files.end(); )
	{
		QString	mime = KMimeMagic::self()->findFileType(*it)->mimeType();
		if (info.mimeTypes.findIndex(mime) == -1)
		{
			if ((result=KMessageBox::warningYesNoCancel(NULL,
					       i18n("The file format %1 is not directly supported by the current print system. "
					            "KDE can try to convert this file automatically to a supported format. But you can "
						    "still try to send the file to the printer without any conversion. Do you want KDE "
						    "to try to convert this file to %2?").arg(mime).arg(info.primaryMimeType),
					       QString::null,
					       i18n("Convert"),
					       i18n("Keep"),
					       QString::fromLatin1("kdeprintAutoConvert"))) == KMessageBox::Yes)
			{
				// find the filter chain
				QStringList	flist = KXmlCommandManager::self()->autoConvert(mime, info.primaryMimeType);
				if (flist.count() == 0)
				{
					if (KMessageBox::warningYesNo(NULL,
								      i18n("No appropriate filter was found to convert the file "
								           "format %1 into %2. Do you want to print the "
									   "file using its original format?").arg(mime).arg(info.primaryMimeType),
								      QString::null,
								      i18n("Print"),
								      i18n("Skip")) == KMessageBox::No)
					{
						if (flag)
							QFile::remove(*it);
						it = files.remove(it);
					}
					else
						++it;
					continue;
				}
				QStringList	l(*it);
				switch (doFilterFiles(printer, l, flist, QMap<QString,QString>(), flag))
				{
					case -1:
						return -1;
					case 0:
						break;
					case 1:
						status = 1;
						*it = l[0];
						break;
				}
			}
			else if (result == KMessageBox::Cancel)
			{
				files.clear();
				return 0;
			}
		}
		++it;
	}
	return status;
}

bool KPrinterImpl::setupSpecialCommand(QString& cmd, KPrinter *p, const QStringList&)
{
	QString	s(p->option("kde-special-command"));
	if (s.isEmpty())
	{
		p->setErrorMessage("Empty command.");
		return false;
	}

	s = KMFactory::self()->specialManager()->setupCommand(s, p->options());

	QString	ps = pageSizeToPageName( p->option( "kde-printsize" ).isEmpty() ? p->pageSize() : ( KPrinter::PageSize )p->option( "kde-printsize" ).toInt() );
	s.replace(QRegExp("%psl"), ps.lower());
	s.replace(QRegExp("%psu"), ps);
	s.replace(QRegExp("%out"), quote(p->outputFileName())); // Replace as last
	cmd = s;
	return true;
}

QString KPrinterImpl::quote(const QString& s)
{ return KProcess::quote(s); }

void KPrinterImpl::saveOptions(const QMap<QString,QString>& opts)
{
	m_options = opts;
	saveAppOptions();
}

void KPrinterImpl::loadAppOptions()
{
	KConfig	*conf = KGlobal::config();
	conf->setGroup("KPrinter Settings");
	QStringList	opts = conf->readListEntry("ApplicationOptions");
	for (uint i=0; i<opts.count(); i+=2)
		if (opts[i].startsWith("app-"))
			m_options[opts[i]] = opts[i+1];
}

void KPrinterImpl::saveAppOptions()
{
	QStringList	optlist;
	for (QMap<QString,QString>::ConstIterator it=m_options.begin(); it!=m_options.end(); ++it)
		if (it.key().startsWith("app-"))
			optlist << it.key() << it.data();

	KConfig	*conf = KGlobal::config();
	conf->setGroup("KPrinter Settings");
	conf->writeEntry("ApplicationOptions", optlist);
}

#include "kprinterimpl.moc"
