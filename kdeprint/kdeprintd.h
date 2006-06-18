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

#ifndef KDEPRINTD_H
#define KDEPRINTD_H

#include <kdedmodule.h>
#include <qlist.h>
#include <qstringlist.h>
#include <qpointer.h>
#include <qhash.h>
#include <dbus/qdbus.h>

class KPrintProcess;
class KProcess;
class StatusWindow;

class KDEPrintd : public KDEDModule
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.KDEPrintd")

public:
	KDEPrintd();
	~KDEPrintd();

public Q_SLOTS:
	Q_SCRIPTABLE int print(const QString& cmd, const QStringList& files, bool remove);
	Q_SCRIPTABLE QString openPassDlg(const QString& user);
	Q_SCRIPTABLE QString requestPassword( const QString& user, const QString& host, int port, int seqNbr, const QDBusMessage& msg );
	Q_SCRIPTABLE void initPassword( const QString& user, const QString& passwd, const QString& host, int port );
	Q_SCRIPTABLE Q_ASYNC void statusMessage(const QString& msg, int pid = -1, const QString& appName = QString());

protected Q_SLOTS:
	void slotPrintTerminated( KPrintProcess* );
	void slotPrintError( KPrintProcess*, const QString& );
	void slotClosed();
	void processRequest();

protected:
	bool checkFiles(QString& cmd, const QStringList& files);

private:
	class Request;
	QList<KPrintProcess*>	m_processpool;
	QMultiHash<int,StatusWindow*>	m_windows;
	QList<Request*>       m_requestsPending;
};

#endif
