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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KDEPRINTD_H
#define KDEPRINTD_H

#include <kdedmodule.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qptrdict.h>
#include <qguardedptr.h>
#include <qintdict.h>

class KPrintProcess;
class KProcess;
class StatusWindow;

class KDEPrintd : public KDEDModule
{
	Q_OBJECT
	K_DCOP

public:
	KDEPrintd(const QCString& obj);
	~KDEPrintd();

k_dcop:
	int print(const QString& cmd, const QStringList& files, bool remove);
	QString openPassDlg(const QString& user);
	ASYNC statusMessage(const QString& msg, int pid = -1, const QString& appName = QString::null);
	QString requestPassword( const QString& user, const QString& host, int port, int seqNbr );
	void initPassword( const QString& user, const QString& passwd, const QString& host, int port );

protected slots:
	void slotPrintTerminated( KPrintProcess* );
	void slotPrintError( KPrintProcess*, const QString& );
	void slotClosed();
	void processRequest();

protected:
	bool checkFiles(QString& cmd, const QStringList& files);

private:
	class Request;
	QPtrList<KPrintProcess>	m_processpool;
	QIntDict<StatusWindow>	m_windows;
	QPtrList<Request>       m_requestsPending;
};

#endif
