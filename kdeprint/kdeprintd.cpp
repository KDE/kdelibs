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

#include "kdeprintd.h"
#include "kprintprocess.h"

#include <qfile.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kio/passdlg.h>

extern "C"
{
	KDEDModule *create_kdeprintd(const QCString& name)
	{
		return new KDEPrintd(name);
	}
}

static void cleanFileList(const QStringList& files)
{
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		QFile::remove(*it);
}

//*****************************************************************************************************

KDEPrintd::KDEPrintd(const QCString& obj)
: KDEDModule(obj)
{
	m_processpool.setAutoDelete(true);
	m_tempfiles.setAutoDelete(true);
}

KDEPrintd::~KDEPrintd()
{
	cleanTempFiles();
}

int KDEPrintd::print(const QString& cmd, const QStringList& files, bool remflag)
{
	KPrintProcess	*proc = new KPrintProcess;
	connect(proc,SIGNAL(processExited(KProcess*)),SLOT(slotProcessExited(KProcess*)));
	connect(proc,SIGNAL(passwordRequested(KProcess*,const QString&)),SLOT(slotPasswordRequested(KProcess*,const QString&)));
	*proc << cmd;
	if (remflag)
		m_tempfiles.insert(proc,new QStringList(files));
	if (proc->print())
	{
		m_processpool.append(proc);
		return (int)(proc->pid());
	}
	else
	{
		cleanTempFile(proc);
		delete proc;
		return (-1);
	}
}

void KDEPrintd::cleanTempFiles()
{
	QPtrDictIterator<QStringList>	it(m_tempfiles);
	for (;it.current();++it)
		cleanFileList(*(it.current()));
}

void KDEPrintd::cleanTempFile(KProcess *p)
{
	QStringList	*l = m_tempfiles.find(p);
	if (l)
		cleanFileList(*l);
}

void KDEPrintd::slotProcessExited(KProcess *proc)
{
	KPrintProcess	*pproc = (KPrintProcess*)proc;
	if (m_processpool.findRef(pproc) != -1)
	{
		m_processpool.take();
		QString		msg;
		if (!pproc->normalExit())
			msg = i18n("Abnormal process termination (<b>%1</b>).").arg(pproc->args()->first());
		else if (pproc->exitStatus() != 0)
			msg = i18n("<b>%1</b>: execution failed with message:<p>%2</p>").arg(pproc->args()->first()).arg(pproc->errorMessage());
		cleanTempFile(pproc);

		delete pproc;
		if (!msg.isEmpty())
			KNotifyClient::event("printerror",i18n("<p><nobr>A print error occured. Error message received from system:</nobr></p><br>%1").arg(msg));
	}
}

void KDEPrintd::slotPasswordRequested(KProcess *proc, const QString& login)
{
	QString	user(login), passwd, str;
	if (KIO::PasswordDialog::getNameAndPassword(user, passwd, NULL) == KDialog::Accepted)
		str.append(user).append(":").append(passwd);
	str.append("\n");
	proc->writeStdin(str.local8Bit().data(), str.length());
}

#include "kdeprintd.moc"
