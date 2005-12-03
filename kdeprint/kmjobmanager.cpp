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

#include "kmjobmanager.h"
#include "kmjob.h"
#include "kmthreadjob.h"
#include "kmfactory.h"

#include <kaction.h>
#include <kdebug.h>
#include <kconfig.h>

KMJobManager::KMJobManager(QObject *parent)
: QObject(parent)
{
	m_jobs.setAutoDelete(true);
	m_threadjob = new KMThreadJob(this );
        m_threadjob->setObjectName( "ThreadJob" );
	m_filter.setAutoDelete(true);
}

KMJobManager::~KMJobManager()
{
}

KMJobManager* KMJobManager::self()
{
	return KMFactory::self()->jobManager();
}

void KMJobManager::discardAllJobs()
{
	Q3PtrListIterator<KMJob>	it(m_jobs);
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

/*KMJob* KMJobManager::findJob(int ID)
{
	QPtrListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
		if (it.current()->id() == ID)
			return it.current();
	return 0;
}*/

KMJob* KMJobManager::findJob(const QString& uri)
{
	Q3PtrListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
		if (it.current()->uri() == uri)
			return it.current();
	return 0;
}

void KMJobManager::addJob(KMJob *job)
{
	// only keep it if "printer" is not empty, and in printer filter
	if (!job->uri().isEmpty() && !job->printer().isEmpty())
	{
		KMJob	*aJob = findJob(job->uri());
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

/*bool KMJobManager::sendCommand(int ID, int action, const QString& arg)
{
	KMJob	*job = findJob(ID);
	if (job)
	{
		QPtrList<KMJob>	l;
		l.setAutoDelete(false);
		l.append(job);
		return sendCommand(l,action,arg);
	}
	return false;
}*/

bool KMJobManager::sendCommand(const QString& uri, int action, const QString& arg)
{
	KMJob	*job = findJob(uri);
	if (job)
	{
		Q3PtrList<KMJob>	l;
		l.setAutoDelete(false);
		l.append(job);
		return sendCommand(l,action,arg);
	}
	return false;
}

bool KMJobManager::sendCommand(const Q3PtrList<KMJob>& jobs, int action, const QString& args)
{
	// split jobs in 2 classes
	Q3PtrList<KMJob>	csystem, cthread;
	csystem.setAutoDelete(false);
	cthread.setAutoDelete(false);
	Q3PtrListIterator<KMJob>	it(jobs);
	for (;it.current();++it)
		if (it.current()->type() == KMJob::Threaded) cthread.append(it.current());
		else csystem.append(it.current());

	// perform operation on both classes
	if (cthread.count() > 0 && !sendCommandThreadJob(cthread, action, args))
		return false;
	if (csystem.count() > 0 && !sendCommandSystemJob(csystem, action, args))
		return false;
	return true;
}

bool KMJobManager::sendCommandSystemJob(const Q3PtrList<KMJob>&, int, const QString&)
{
	return false;
}

bool KMJobManager::sendCommandThreadJob(const Q3PtrList<KMJob>& jobs, int action, const QString&)
{
	if (action != KMJob::Remove)
		return false;

	Q3PtrListIterator<KMJob>	it(jobs);
	bool	result(true);
	for (;it.current() && result; ++it)
		result = m_threadjob->removeJob(it.current()->id());
	return result;
}

bool KMJobManager::listJobs(const QString&, KMJobManager::JobType, int)
{
	return true;
}

const Q3PtrList<KMJob>& KMJobManager::jobList(bool reload)
{
	if (reload || m_jobs.count() == 0)
	{
		discardAllJobs();
		Q3DictIterator<JobFilter>	it(m_filter);
		int	joblimit = limit();
		bool threadjobs_updated = false;
		for (; it.current(); ++it)
		{
			if ( it.current()->m_isspecial )
			{
				if ( !threadjobs_updated )
				{
					threadJob()->updateManager( this );
					threadjobs_updated = true;
				}
			}
			else
			{
				if (it.current()->m_type[ActiveJobs] > 0)
					listJobs(it.currentKey(), ActiveJobs, joblimit);
				if (it.current()->m_type[CompletedJobs] > 0)
					listJobs(it.currentKey(), CompletedJobs, joblimit);
			}
		}
		m_threadjob->updateManager(this);
		removeDiscardedJobs();
	}
	return m_jobs;
}

int KMJobManager::actions()
{
	return 0;
}

QList<KAction*> KMJobManager::createPluginActions(KActionCollection*)
{
	return QList<KAction*>();
}

void KMJobManager::validatePluginActions(KActionCollection*, const Q3PtrList<KMJob>&)
{
}

void KMJobManager::addPrinter(const QString& pr, KMJobManager::JobType type, bool isSpecial)
{
	struct JobFilter	*jf = m_filter.find(pr);
	if (!jf)
	{
		jf = new JobFilter;
		m_filter.insert(pr, jf);
	}
	jf->m_type[type]++;
	jf->m_isspecial = isSpecial;
}

void KMJobManager::removePrinter(const QString& pr, KMJobManager::JobType type)
{
	struct JobFilter	*jf = m_filter.find(pr);
	if (jf)
	{
		jf->m_type[type] = QMAX(0, jf->m_type[type]-1);
		if (!jf->m_type[0] && !jf->m_type[1])
			m_filter.remove(pr);
	}
}

bool KMJobManager::doPluginAction(int, const Q3PtrList<KMJob>&)
{
	return true;
}

void KMJobManager::setLimit(int val)
{
	KConfig *conf = KMFactory::self()->printConfig();
	conf->setGroup("Jobs");
	conf->writeEntry("Limit", val);
}

int KMJobManager::limit()
{
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("Jobs");
	return conf->readNumEntry("Limit", 0);
}

#include "kmjobmanager.moc"
