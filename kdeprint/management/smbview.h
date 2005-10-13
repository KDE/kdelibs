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

#ifndef SMBVIEW_H
#define SMBVIEW_H

#include <klistview.h>

class KProcess;
class KTempFile;

class SmbView : public KListView
{
	Q_OBJECT
public:
	SmbView(QWidget *parent = 0, const char *name = 0);
	~SmbView();

	void setLoginInfos(const QString& login, const QString& password);
	void setOpen(QListViewItem*, bool);
	void init();
	void abort();

signals:
	void printerSelected(const QString& work, const QString& server, const QString& printer);
	void running(bool);

protected:
	void startProcess(int);
	void endProcess();
	void processGroups();
	void processServers();
	void processShares();

protected slots:
	void slotReceivedStdout(KProcess*, char*, int);
	void slotProcessExited(KProcess*);
	void slotSelectionChanged(QListViewItem*);

private:
	enum State { GroupListing, ServerListing, ShareListing, Idle };
	int 		m_state;
	QListViewItem	*m_current;
	KProcess	*m_proc;
	QString		m_buffer;
	QString		m_login, m_password;
	KTempFile	*m_passwdFile;
	QString		m_wins_server;
};

#endif
