#include "kmjobmanager.h"
#include "kmjob.h"

KMJobManager::KMJobManager(QObject *parent, const char *name)
: QObject(parent,name)
{
	m_jobs.setAutoDelete(true);
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

bool KMJobManager::sendCommand(const QList<KMJob>&, int, const QString&)
{
	return false;
}

bool KMJobManager::listJobs()
{
	return false;
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
