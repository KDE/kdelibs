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

#ifndef KMJOBMANAGER_H
#define KMJOBMANAGER_H

#include <qobject.h>
#include <qptrlist.h>
#include <qdict.h>
#include <qvaluelist.h>

class KMJob;
class KMThreadJob;
class KActionCollection;
class KAction;

class KMJobManager : public QObject
{
	Q_OBJECT

public:
	enum JobType { ActiveJobs = 0, CompletedJobs = 1 };
	struct JobFilter
	{
		JobFilter() { m_type[0] = m_type[1] = 0; }
		int	m_type[2];
	};

	KMJobManager(QObject *parent = 0, const char *name = 0);
	virtual ~KMJobManager();

	static KMJobManager* self();

	void addPrinter(const QString& pr, JobType type = ActiveJobs);
	void removePrinter(const QString& pr, JobType type = ActiveJobs);
	void clearFilter();
	QDict<JobFilter>* filter();
	int limit();
	void setLimit(int val);

	//KMJob* findJob(int ID);
	KMJob* findJob(const QString& uri);
	//bool sendCommand(int ID, int action, const QString& arg = QString::null);
	bool sendCommand(const QString& uri, int action, const QString& arg = QString::null);
	bool sendCommand(const QPtrList<KMJob>& jobs, int action, const QString& arg = QString::null);
	const QPtrList<KMJob>& jobList(bool reload = true);
	void addJob(KMJob*);
	KMThreadJob* threadJob();

	virtual int actions();
	virtual QValueList<KAction*> createPluginActions(KActionCollection*);
	virtual void validatePluginActions(KActionCollection*, const QPtrList<KMJob>&);
	virtual bool doPluginAction(int, const QPtrList<KMJob>&);

protected:
	void discardAllJobs();
	void removeDiscardedJobs();

protected:
	virtual bool listJobs(const QString& prname, JobType type, int limit = 0);
	virtual bool sendCommandSystemJob(const QPtrList<KMJob>& jobs, int action, const QString& arg = QString::null);
	bool sendCommandThreadJob(const QPtrList<KMJob>& jobs, int action, const QString& arg = QString::null);

protected:
	QPtrList<KMJob>	m_jobs;
	QDict<JobFilter>	m_filter;
	KMThreadJob	*m_threadjob;
};

inline QDict<KMJobManager::JobFilter>* KMJobManager::filter()
{ return &m_filter; }

inline void KMJobManager::clearFilter()
{ m_filter.clear(); }

inline KMThreadJob* KMJobManager::threadJob()
{ return m_threadjob; }

#endif
