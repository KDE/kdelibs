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
#include "kprintprocess.h"

#include <qfile.h>
#include <knotifyclient.h>
#include <klocale.h>
#include <kapp.h>
#include <kstddirs.h>

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

	// initialize process pool
	m_processpool.setAutoDelete(true);
}

KPrinterImpl::~KPrinterImpl()
{
	delete m_fileprinter;
	if (m_processpool.count() > 0)
		KNotifyClient::event("printerror",i18n("There were still %1 print process(es) running. Printing aborted.").arg(m_processpool.count()));
	m_processpool.clear();
	cleanTempFiles();
}

void KPrinterImpl::preparePrinting(KPrinter*)
{
}

bool KPrinterImpl::printFiles(KPrinter*, const QStringList&)
{
	return false;
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

void KPrinterImpl::slotProcessExited(KProcess *proc)
{
	KPrintProcess	*pproc = (KPrintProcess*)proc;
	if (m_processpool.findRef(pproc) == -1)
		return;

	QString	msg;
	if (!proc->normalExit())
		msg = i18n("<nobr>Abnormal process termination (<b>%1</b>).</nobr>").arg(pproc->args()->first());
	else if (proc->exitStatus() != 0)
		msg = i18n("The execution of <b>%1</b> failed with message:<p>%2</p>").arg(pproc->args()->first()).arg(pproc->errorMessage());

	m_processpool.removeRef(pproc);
	if (!msg.isEmpty())
		KNotifyClient::event("printerror",i18n("<p><nobr>A print error occured. Error message received from system:</nobr></p><br>%1").arg(msg));

	if (m_processpool.count() == 0)
		// last child process exited, clean up temporary files
		cleanTempFiles();
}

bool KPrinterImpl::startPrintProcess(KPrintProcess *proc, KPrinter *printer)
{
	connect(proc,SIGNAL(processExited(KProcess*)),SLOT(slotProcessExited(KProcess*)));
	if (proc->print())
	{
		m_processpool.append(proc);
		if (printer) KMThreadJob::createJob(proc->pid(),printer->printerName(),printer->docName(),getenv("USER"),0);
		return true;
	}
	else
	{
		delete proc;
		return false;
	}
}

bool KPrinterImpl::startPrinting(KPrintProcess *proc, KPrinter *printer, const QStringList& files)
{
	bool	canPrint(false);
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		if (QFile::exists(*it))
		{
			*proc << *it;
			canPrint = true;
		}
		else
			qDebug("File not found: %s",(*it).latin1());
	if (canPrint)
		if (!startPrintProcess(proc,printer))
		{
			printer->setErrorMessage(i18n("Unable to start child print process."));
			return false;
		}
		else return true;
	else
	{
		printer->setErrorMessage(i18n("No valid file was found for printing. Operation aborted."));
		return false;
	}
}

void KPrinterImpl::cleanTempFiles()
{
	for (QStringList::Iterator it=m_tempfiles.begin(); it!=m_tempfiles.end();)
		if (QFile::remove(*it)) it = m_tempfiles.remove(it);
		else ++it;
}

QString KPrinterImpl::tempFile()
{
	QString	f = locateLocal("tmp","kdeprint_") + KApplication::randomString(8);
	return f;
}
#include "kprinterimpl.moc"
