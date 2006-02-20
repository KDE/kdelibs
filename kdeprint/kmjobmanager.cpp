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

#include <kdebug.h>
#include <kconfig.h>

class KAction;
class KActionCollection;

KMJobManager::KMJobManager(QObject *parent)
: QObject(parent)
{
	m_threadjob = new KMThreadJob(this );
        m_threadjob->setObjectName( "ThreadJob" );
  m_filter.setAutoDelete(true);
}

KMJobManager::~KMJobManager()
{
  qDeleteAll(m_jobs);
}

KMJobManager* KMJobManager::self()
{
	return KMFactory::self()->jobManager();
}

void KMJobManager::discardAllJobs()
{
	QListIterator<KMJob*>	it(m_jobs);
	while (it.hasNext())
		it.next()->setDiscarded(true);
}

void KMJobManager::removeDiscardedJobs()
{
	for (int i=0;i<m_jobs.count();i++)
		if (m_jobs.at(i)->isDiscarded())
		{
			m_jobs.removeAt(i);
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
	QListIterator<KMJob*>	it(m_jobs);
	while (it.hasNext()) {
    KMJob *job(it.next());
		if (job->uri() == uri)
			return job;
  }
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
		QList<KMJob*>	l;
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
		QList<KMJob*>	l;
		l.append(job);
		return sendCommand(l,action,arg);
	}
	return false;
}

bool KMJobManager::sendCommand(const QList<KMJob*>& jobs, int action, const QString& args)
{
	// split jobs in 2 classes
	QList<KMJob*>	csystem, cthread;
	QListIterator<KMJob*>	it(jobs);
	while (it.hasNext()) {
    KMJob *job(it.next());
		if (job->type() == KMJob::Threaded) cthread.append(job);
		else csystem.append(job);
  }

	// perform operation on both classes
	if (cthread.count() > 0 && !sendCommandThreadJob(cthread, action, args))
		return false;
	if (csystem.count() > 0 && !sendCommandSystemJob(csystem, action, args))
		return false;
	return true;
}

bool KMJobManager::sendCommandSystemJob(const QList<KMJob*>&, int, const QString&)
{
	return false;
}

bool KMJobManager::sendCommandThreadJob(const QList<KMJob*>& jobs, int action, const QString&)
{
	if (action != KMJob::Remove)
		return false;

	QListIterator<KMJob*>	it(jobs);
	bool	result(true);
	while (it.hasNext() && result)
		result = m_threadjob->removeJob(it.next()->id());
	return result;
}

bool KMJobManager::listJobs(const QString&, KMJobManager::JobType, int)
{
	return true;
}

const QList<KMJob*>& KMJobManager::jobList(bool reload)
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

void KMJobManager::validatePluginActions(KActionCollection*, const QList<KMJob*>&)
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
		jf->m_type[type] = qMax(0, jf->m_type[type]-1);
		if (!jf->m_type[0] && !jf->m_type[1])
			m_filter.remove(pr);
	}
}

bool KMJobManager::doPluginAction(int, const QList<KMJob*>&)
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
	return conf->readEntry("Limit", 0);
}

#include "kmjobmanager.moc"
