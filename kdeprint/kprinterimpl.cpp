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
#include "kmthreadjob.h"
#include "kmprinter.h"

#include <qfile.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kdatastream.h>

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
	// initialize file printer
	m_fileprinter = new KMPrinter();
	m_fileprinter->setPrinterName(QString::fromLatin1("File"));
	m_fileprinter->setName("__kdeprint_file");
	m_fileprinter->setDefaultOption("kde-orientation","Portrait");
	m_fileprinter->setDefaultOption("kde-colormode","Color");
	m_fileprinter->setDefaultOption("kde-pagesize",QString::number((int)KPrinter::A4));
}

KPrinterImpl::~KPrinterImpl()
{
	delete m_fileprinter;
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
	if (!setupCommand(cmd,p))
		return false;
	else
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

	// update also "file" printer
	initEditPrinter(m_fileprinter);
	m_fileprinter->setEditedOption(key,value);
}

int KPrinterImpl::dcopPrint(const QString& cmd, const QStringList& files, bool removeflag)
{
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
	bool	canPrint(false);
	QString	command(cmd);
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		if (QFile::exists(*it))
		{
			command.append(" ").append(*it);
			canPrint = true;
		}
		else
			qDebug("File not found: %s",(*it).latin1());
	if (canPrint)
	{
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
				msg += i18n("Check the command syntax:\n%1 <files>").arg(cmd);
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
#include "kprinterimpl.moc"
