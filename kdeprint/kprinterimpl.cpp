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
#include "kmfiltermanager.h"
#include "kmthreadjob.h"
#include "kmprinter.h"
#include "kprintfilter.h"

#include <qfile.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kdatastream.h>
#include <kdebug.h>
#include <kprocess.h>

#include <stdlib.h>

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
}

KPrinterImpl::~KPrinterImpl()
{
}

void KPrinterImpl::preparePrinting(KPrinter*)
{
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
					p->setErrorMessage(i18n("Cannot save print file. Check that you have write access to it."));
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
	QList<KMPrinter>	*printers = KMFactory::self()->manager()->printerList(false);
	if (printers)
	{
		QListIterator<KMPrinter>	it(*printers);
		for (;it.current();++it)
		{
			initEditPrinter(it.current());
			it.current()->setEditedOption(key,value);
		}
	}
}

int KPrinterImpl::dcopPrint(const QString& cmd, const QStringList& files, bool removeflag)
{
	kdDebug() << "kdeprint: print command: " << cmd << endl;

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

bool KPrinterImpl::startPrinting(const QString& cmd, KPrinter *printer, const QStringList& files, bool flag)
{
	QString	command(cmd);
	QStringList	printfiles;
	if (command.find("%in") == -1) command.append(" %in");

	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		if (QFile::exists(*it))
			printfiles.append(*it);
		else
			kdDebug() << "File not found: " << (*it) << endl;

	if (printfiles.count() > 0)
	{
		command.replace(QRegExp("%in"),printfiles.join(" "));
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
	else
	{
		printer->setErrorMessage(i18n("No valid file was found for printing. Operation aborted."));
		return false;
	}
}

QString KPrinterImpl::tempFile()
{
	QString	f;
	// be sure the file doesn't exist
	do f = locateLocal("tmp","kdeprint_") + KApplication::randomString(8); while (QFile::exists(f));
	return f;
}

bool KPrinterImpl::filterFiles(KPrinter *printer, QStringList& files, bool flag)
{
	QStringList	flist = QStringList::split(',',printer->option("_kde-filters"),false);
	if (flist.count() == 0)
		return true;

	QString	filtercmd;
	KMFilterManager	*fmgr = KMFactory::self()->filterManager();
	for (uint i=0;i<flist.count();i++)
	{
		KPrintFilter	*filter = fmgr->filter(flist[i]);
		QString		subcmd = filter->buildCommand(printer->options(),(i>0),(i<(flist.count()-1)));
		if (!subcmd.isEmpty())
		{
			filtercmd.append(subcmd);
			if (i < flist.count()-1)
				filtercmd.append("| ");
		}
		else
		{
			printer->setErrorMessage(i18n("Error while reading filter description for <b>%1</b>. Empty command line received.").arg(filter->idName()));
			return false;
		}
	}
	kdDebug() << "kdeprint: filter command: " << filtercmd << endl;

	QRegExp	rin("%in"), rout("%out");
	for (QStringList::Iterator it=files.begin(); it!=files.end(); ++it)
	{
		QString	tmpfile = tempFile();
		QString	cmd(filtercmd);
		cmd.replace(rin,*it);
		cmd.replace(rout,tmpfile);
		if (system(cmd.latin1()) != 0)
		{
			printer->setErrorMessage(i18n("Error while filtering. Command was: <b>%1</b>.").arg(filtercmd));
			return false;
		}
		if (flag) QFile::remove(*it);
		*it = tmpfile;
	}
	return true;
}

bool KPrinterImpl::setupSpecialCommand(QString& cmd, KPrinter *p, const QStringList& files)
{
	QString	s(p->option("kde-special-command"));
	if (s.isEmpty())
	{
		p->setErrorMessage("Empty command.");
		return false;
	}
	s.replace(QRegExp("%out"),p->outputFileName());
	s.replace(QRegExp("%psl"),QString::fromLatin1(pageSizeToPageName(p->pageSize())).lower());
	cmd = s;
	return true;
}
#include "kprinterimpl.moc"
