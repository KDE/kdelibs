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

#include "kmdbcreator.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <qprogressdialog.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>

KMDBCreator::KMDBCreator(QObject *parent, const char *name)
: QObject(parent,name)
{
	m_dlg = 0;
	m_status = true;

	connect(&m_proc,SIGNAL(receivedStdout(KProcess*,char*,int)),SLOT(slotReceivedStdout(KProcess*,char*,int)));
	connect(&m_proc,SIGNAL(receivedStderr(KProcess*,char*,int)),SLOT(slotReceivedStderr(KProcess*,char*,int)));
	connect(&m_proc,SIGNAL(processExited(KProcess*)),SLOT(slotProcessExited(KProcess*)));
}

KMDBCreator::~KMDBCreator()
{
	if (m_proc.isRunning())
		m_proc.kill();
	if (m_dlg) delete m_dlg;
}

bool KMDBCreator::checkDriverDB(const QString& dirname, const QDateTime& d)
{
	// don't block GUI
	kapp->processEvents();

	// first check current directory
	QFileInfo	dfi(dirname);
	if (dfi.lastModified() > d)
		return false;

	// then check most recent file in current directory
	QDir	dir(dirname);
	const QFileInfoList	*list = dir.entryInfoList(QDir::Files,QDir::Time);
	if (list && list->count() > 0 && list->getFirst()->lastModified() > d)
		return false;

	// then loop into subdirs
	QStringList	slist = dir.entryList(QDir::Dirs,QDir::Time);
	for (QStringList::ConstIterator it=slist.begin(); it!=slist.end(); ++it)
		if ((*it) != "." && (*it) != ".." && !checkDriverDB(dir.absFilePath(*it),d))
			return false;

	// everything is OK
	return true;
}

bool KMDBCreator::createDriverDB(const QString& dirname, const QString& filename, QWidget *parent)
{
	bool	started(true);

	// initialize status
	m_status = false;
	m_firstflag = true;

	// start the child process
	m_proc.clearArguments();
	QString	exestr = KMFactory::self()->manager()->driverDbCreationProgram();
	m_proc.setExecutable(exestr);
	m_proc << dirname << filename;
	if (exestr.isEmpty() || KStandardDirs::findExe(exestr).isEmpty() || !m_proc.start(KProcess::NotifyOnExit, KProcess::AllOutput))
	{
		KMFactory::self()->manager()->setErrorMsg(i18n("Unable to start child process '%1' !").arg(exestr));
		started = false;
	}

	// Create the dialog is the process is running
	if (started)
	{
		if (m_dlg)
		{
			delete m_dlg;
			m_dlg = 0;
		}
		m_dlg = new QProgressDialog(parent,"progress-dialog",true);
		m_dlg->setLabelText(i18n("Please wait while KDE rebuilds a driver database."));
		m_dlg->setCaption(i18n("Driver database"));
		m_dlg->setMinimumDuration(0);	// always show the dialog
		m_dlg->setProgress(0);		// to force showing
		connect(m_dlg,SIGNAL(cancelled()),SLOT(slotCancelled()));
	}
	else
		// be sure to emit this signal otherwise the DB widget won't never be notified
		emit dbCreated();

	return started;
}

void KMDBCreator::slotReceivedStdout(KProcess*, char *buf, int len)
{
	// save buffer
	QString	str( QCString(buf, len) );

	// get the number
	bool	ok;
	int	n = str.toInt(&ok);

	// process the number received
	if (ok && m_dlg)
	{
		if (m_firstflag)
		{
			m_dlg->setTotalSteps(n);
			m_firstflag = false;
		}
		else
		{
			m_dlg->setProgress(n);
		}
	}
}

void KMDBCreator::slotReceivedStderr(KProcess*, char*, int)
{
	// just discard it for the moment
}

void KMDBCreator::slotProcessExited(KProcess*)
{
	// delete the progress dialog
	if (m_dlg)
	{
		delete m_dlg;
		m_dlg = 0;
	}

	// set exit status
	m_status = (m_proc.normalExit() && m_proc.exitStatus() == 0);
	if (!m_status)
		KMFactory::self()->manager()->setErrorMsg(i18n("Abnormal child process termination !"));
	else
		emit dbCreated();
}

void KMDBCreator::slotCancelled()
{
	if (m_proc.isRunning())
		m_proc.kill();
	else
		emit dbCreated();
}
#include "kmdbcreator.moc"
