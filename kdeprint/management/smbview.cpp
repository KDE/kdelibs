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

#include "smbview.h"

#include <config.h>
#include <stdlib.h>

#include <kprocess.h>
#include <qheader.h>
#include <qapplication.h>

#include <kiconloader.h>
#include <klocale.h>

QString smbPasswordString(const QString& login, const QString& password)
{
	QString	s;
	if (login.isEmpty()) s = "-N";
	else if (password.isEmpty()) s = QString("-U %1").arg(login);
	else s = QString("-U %1%%2").arg(login).arg(password);
	return s;
}

//*********************************************************************************************

SmbView::SmbView(QWidget *parent, const char *name)
: KListView(parent,name)
{
	addColumn(i18n("Printer"));
	addColumn(i18n("Comment"));
	setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	setLineWidth(1);
	setAllColumnsShowFocus(true);
	setRootIsDecorated(true);

	m_state = Idle;
	m_current = 0;
	m_proc = new KShellProcess();
	connect(m_proc,SIGNAL(processExited(KProcess*)),SLOT(slotProcessExited(KProcess*)));
	connect(m_proc,SIGNAL(receivedStdout(KProcess*,char*,int)),SLOT(slotReceivedStdout(KProcess*,char*,int)));
	connect(this,SIGNAL(selectionChanged(QListViewItem*)),SLOT(slotSelectionChanged(QListViewItem*)));
}

SmbView::~SmbView()
{
	delete m_proc;
}

void SmbView::setLoginInfos(const QString& login, const QString& password)
{
	m_login = login;
	m_password = password;
}

void SmbView::startProcess(int state)
{
	m_buffer = QString::null;
	m_state = state;
	QApplication::setOverrideCursor(waitCursor);
	m_proc->start(KProcess::NotifyOnExit,KProcess::Stdout);
	emit running(true);
}

void SmbView::endProcess()
{
	switch (m_state)
	{
		case GroupListing:
			processGroups();
			break;
		case ServerListing:
			processServers();
			break;
		case ShareListing:
			processShares();
			break;
		default:
			break;
	}
	m_state = Idle;
	QApplication::restoreOverrideCursor();
	emit running(false);
}

void SmbView::slotProcessExited(KProcess*)
{
	endProcess();
}

void SmbView::slotReceivedStdout(KProcess*, char *buf, int len)
{
	m_buffer.append(QString::fromLocal8Bit(buf,len));
}

void SmbView::init()
{
	QString	cmd("nmblookup -M - | grep '<01>' | awk '{print $1}' | xargs nmblookup -A | grep '<1d>' | awk '{print $1}'");
	m_proc->setExecutable(cmd);
	startProcess(GroupListing);
}

void SmbView::setOpen(QListViewItem *item, bool on)
{
	if (on && item->childCount() == 0)
	{
		QCString oldpw = getenv("PASSWD");
		QCString olduser = getenv("USER");
		QCString pw = m_password.local8Bit();
		setenv("PASSWD", pw, 1);
		QCString user = m_login.local8Bit();
		setenv("USER", user, 1);
		if (item->depth() == 0)
		{ // opening group
			m_current = item;
			QString	cmd = QString("nmblookup -M %1 -S | grep '<20>' | awk '{print $1}' | xargs -iserv_name ").arg(KShellProcess::quote(item->text(0)));
			cmd += QString("smbclient -L serv_name -N -W %1").arg(KShellProcess::quote(item->text(0)));
			m_proc->setExecutable(cmd);
			startProcess(ServerListing);
		}
		else if (item->depth() == 1)
		{ // opening server
			m_current = item;
			QString	cmd = QString("smbclient -L %1 ").arg(KShellProcess::quote(item->text(0)));
			cmd += QString("-N -W %1").arg(KShellProcess::quote(item->parent()->text(0)));
			m_proc->setExecutable(cmd);
			startProcess(ShareListing);
		}
		if (oldpw.isNull())
		   unsetenv("PASSWD");
		else
		   setenv("PASSWD", oldpw, 1);
		if (olduser.isNull())
		   unsetenv("USER");
		else
		   setenv("USER", olduser, 1);
	}
	QListView::setOpen(item,on);
}

void SmbView::processGroups()
{
	QStringList	grps = QStringList::split('\n',m_buffer,false);
	clear();
	for (QStringList::ConstIterator it=grps.begin(); it!=grps.end(); ++it)
	{
		QListViewItem	*item = new QListViewItem(this,*it);
		item->setExpandable(true);
		item->setPixmap(0,SmallIcon("network"));
	}
}

void SmbView::processServers()
{
	QStringList	lines = QStringList::split('\n',m_buffer,true);
	QString		line;
	uint 		index(0);
	for (;index < lines.count();index++)
		if (lines[index].stripWhiteSpace().startsWith("Server"))
			break;
	index += 2;
	while (index < lines.count())
	{
		line = lines[index++].stripWhiteSpace();
		if (line.isEmpty())
			break;
		QStringList	words = QStringList::split(' ',line,false);
		QListViewItem	*item = new QListViewItem(m_current,words[0]);
		item->setExpandable(true);
		item->setPixmap(0,SmallIcon("kdeprint_computer"));
	}
}

void SmbView::processShares()
{
	QStringList	lines = QStringList::split('\n',m_buffer,true);
	QString		line;
	uint 		index(0);
	for (;index < lines.count();index++)
		if (lines[index].stripWhiteSpace().startsWith("Sharename"))
			break;
	index += 2;
	while (index < lines.count())
	{
		line = lines[index++].stripWhiteSpace();
		if (line.isEmpty())
			break;
		QStringList	words = QStringList::split(' ',line,false);
		if (words[1] == "Printer")
		{
			QString	comm;
			for (uint i=2; i<words.count(); i++)
				comm += (words[i]+" ");
			QListViewItem	*item = new QListViewItem(m_current,words[0],comm);
			item->setPixmap(0,SmallIcon("kdeprint_printer"));
		}
	}
}

void SmbView::slotSelectionChanged(QListViewItem *item)
{
	if (item && item->depth() == 2)
		emit printerSelected(item->parent()->parent()->text(0),item->parent()->text(0),item->text(0));
}

void SmbView::abort()
{
	if (m_proc->isRunning())
		m_proc->kill();
}
#include "smbview.moc"
