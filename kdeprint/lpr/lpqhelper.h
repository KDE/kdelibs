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

#ifndef LPQHELPER_H
#define LPQHELPER_H

#include <qobject.h>
#include <qdict.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qvaluelist.h>

class KProcess;
struct JobInfo;
struct LpqInfo;
class KMJob;

class LpqHelper : public QObject
{
	Q_OBJECT
public:
	LpqHelper(QObject *parent = 0, const char *name = 0);
	~LpqHelper();

	void listJobs(QPtrList<KMJob>& jobs, const QString& prname);

protected slots:
	void slotExited(KProcess*);
	void slotReceivedOutput(KProcess*, char*, int);
	void processNext();
	void slotTimeout();

protected:
	JobInfo* splitLine(const QString&, const QValueList<int>&);
	void parseOutput(const QString& buf, const QString& prname);
	void updateNow(const QString&);

private:
	KProcess	*m_proc;
	QDict<LpqInfo>	m_lpq;
	QStringList	m_updatelist;
	QString		m_exepath;
	QString		m_buffer;
};

#endif
