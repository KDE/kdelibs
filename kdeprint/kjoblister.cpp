#include "kjoblister.h"

#include <kdebug.h>

KJobLister::KJobLister()
{
	jobs_.setAutoDelete(true);
}

KJobLister::~KJobLister()
{
}

void KJobLister::discardAllJobs()
{
	QListIterator<KJob>	it(jobs_);
	for (;it.current();++it)
		it.current()->discarded = true;
}

void KJobLister::removeDiscardedJobs()
{
	for (uint i=0;i<jobs_.count();i++)
		if (jobs_.at(i)->discarded)
		{
			jobs_.remove(i);
			i--;
		}
}

KJob* KJobLister::findJob(int ID)
{
	QListIterator<KJob>	it(jobs_);
	for (;it.current();++it)
		if (it.current()->ID == ID)
			return it.current();
	return 0;
}

bool KJobLister::sendCommand(int ID, int action, const QString& arg)
{
	KJob	*job = findJob(ID);
	if (job)
	{
		QList<KJob>	l;
		l.setAutoDelete(false);
		l.append(job);
		return sendCommand(l,action,arg);
	}
	return false;
}

bool KJobLister::sendCommand(const QList<KJob>&, int, const QString&)
{
	return false;
}

bool KJobLister::listJobs()
{
	return false;
}

const QList<KJob>& KJobLister::jobList()
{
	discardAllJobs();
	if (!listJobs())
		kdDebug() << "Unable to retrieve job list !" << endl;
	removeDiscardedJobs();
	return jobs_;
}

int KJobLister::actions()
{
	return 0;
}

//*************************************************************************************************************

void jobCopy(KJob *src, KJob *dest)
{
	dest->ID = src->ID;
	dest->name = src->name;
	dest->printer = src->printer;
	dest->user = src->user;
	dest->state = src->state;
	dest->size = src->size;

	dest->discarded = false;
	dest->uri = src->uri;
}
