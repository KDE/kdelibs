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

#include "kmjobmanager.h"
#include "kmjob.h"
#include "kmthreadjob.h"

KMJobManager::KMJobManager(QObject *parent, const char *name)
: QObject(parent,name)
{
	m_jobs.setAutoDelete(true);
	m_threadjob = new KMThreadJob(this, "ThreadJob");
}

KMJobManager::~KMJobManager()
{
}

void KMJobManager::discardAllJobs()
{
	QListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
		it.current()->setDiscarded(true);
}

void KMJobManager::removeDiscardedJobs()
{
	for (uint i=0;i<m_jobs.count();i++)
		if (m_jobs.at(i)->isDiscarded())
		{
			m_jobs.remove(i);
			i--;
		}
}

KMJob* KMJobManager::findJob(int ID)
{
	QListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
		if (it.current()->id() == ID)
			return it.current();
	return 0;
}

void KMJobManager::addJob(KMJob *job)
{
	// only keep it if "printer" is not empty, and in printer filter
	if (job->id() > 0 && !job->printer().isEmpty() && m_printers.contains(job->printer()) > 0)
	{
		KMJob	*aJob = findJob(job->id());
		if (aJob)
		{
			aJob->copy(*job);
			delete job;
		}
		else
		{
			job->setDiscarded(false);
			m_jobs.append(job);
		}
	}
	else
		delete job;
}

bool KMJobManager::sendCommand(int ID, int action, const QString& arg)
{
	KMJob	*job = findJob(ID);
	if (job)
	{
		QList<KMJob>	l;
		l.setAutoDelete(false);
		l.append(job);
		return sendCommand(l,action,arg);
	}
	return false;
}

bool KMJobManager::sendCommand(const QList<KMJob>& jobs, int action, const QString& args)
{
	// split jobs in 2 classes
	QList<KMJob>	csystem, cthread;
	csystem.setAutoDelete(false);
	cthread.setAutoDelete(false);
	QListIterator<KMJob>	it(jobs);
	for (;it.current();++it)
		if (it.current()->type() == KMJob::Threaded) cthread.append(it.current());
		else csystem.append(it.current());

	// perform operation on both classes
	if (cthread.count() > 0 && !sendCommandThreadJob(cthread, action, args))
		return false;
	if (csystem.count() > 0 && !sendCommandSystemJob(csystem, action, args))
		return false;
	return true;;
}

bool KMJobManager::sendCommandSystemJob(const QList<KMJob>&, int, const QString&)
{
	return false;
}

bool KMJobManager::sendCommandThreadJob(const QList<KMJob>& jobs, int action, const QString&)
{
	if (action != KMJob::Remove)
		return false;

	QListIterator<KMJob>	it(jobs);
	bool	result(true);
	for (;it.current() && result; ++it)
		result = m_threadjob->removeJob(it.current()->id());
	return result;
}

bool KMJobManager::listJobs()
{
	m_threadjob->updateManager(this);
	return true;
}

const QList<KMJob>& KMJobManager::jobList()
{
	discardAllJobs();
	listJobs();
	removeDiscardedJobs();
	return m_jobs;
}

int KMJobManager::actions()
{
	return 0;
}
