#include "kmcupsjobmanager.h"
#include "kmjob.h"
#include "cupsinfos.h"
#include "ipprequest.h"
#include "kmjob.h"

KMCupsJobManager::KMCupsJobManager(QObject *parent, const char *name)
: KMJobManager(parent,name)
{
}

KMCupsJobManager::~KMCupsJobManager()
{
}

int KMCupsJobManager::actions()
{
	return KMJob::All;
}

bool KMCupsJobManager::sendCommand(const QList<KMJob>& jobs, int action, const QString& argstr)
{
	IppRequest	req;
	QString		uri;
	bool		value(true);

	QListIterator<KMJob>	it(jobs);
	for (;it.current() && value;++it)
	{
		req.addURI(IPP_TAG_OPERATION,"job-uri",it.current()->uri());
		req.addName(IPP_TAG_OPERATION,"requesting-user-name", CupsInfos::self()->login());

		switch (action)
		{
			case KMJob::Remove:
				req.setOperation(IPP_CANCEL_JOB);
				break;
			case KMJob::Hold:
				req.setOperation(IPP_HOLD_JOB);
				break;
			case KMJob::Resume:
				req.setOperation(IPP_RELEASE_JOB);
				break;
			case KMJob::Move:
				if (argstr.isEmpty()) return false;
				req.setOperation(CUPS_MOVE_JOB);
				uri = QString("ipp://%1:%2/printers/%3").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port()).arg(argstr);
				req.addURI(IPP_TAG_OPERATION,"job-printer-uri",uri);
				break;
			default:
				return false;
		}

		value = req.doRequest("/jobs/");
	}

	return value;
}

bool KMCupsJobManager::listJobs()
{
	IppRequest	req;
	QString		uri;
	QStringList	keys;

	req.setOperation(IPP_GET_JOBS);
	uri = QString("ipp://%1:%2/jobs/").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port());
	req.addURI(IPP_TAG_OPERATION,"job-uri",uri);
	keys.append("job-id");
	keys.append("job-uri");
	keys.append("job-printer-uri");
	keys.append("job-name");
	keys.append("job-state");
	keys.append("job-k-octets");
	keys.append("job-originating-user-name");
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

	if (req.doRequest("/jobs/"))
	{
		ipp_attribute_t	*attr = req.first();
		KMJob		*job = new KMJob();
		while (attr)
		{
			QString	name(attr->name);
			if (name == "job-id") job->setId(attr->values[0].integer);
			else if (name == "job-uri") job->setUri(QString::fromLatin1(attr->values[0].string.text));
			else if (name == "job-name") job->setName(QString::fromLocal8Bit(attr->values[0].string.text));
			else if (name == "job-state")
			{
				switch (attr->values[0].integer)
				{
					case IPP_JOB_PENDING:
						job->setState(KMJob::Queued);
						break;
					case IPP_JOB_HELD:
						job->setState(KMJob::Held);
						break;
					case IPP_JOB_PROCESSING:
						job->setState(KMJob::Printing);
						break;
					case IPP_JOB_STOPPED:
						job->setState(KMJob::Error);
						break;
					default:
						job->setState(KMJob::Unknown);
						break;
				}
			}
			else if (name == "job-k-octets") job->setSize(attr->values[0].integer);
			else if (name == "job-originating-user-name") job->setOwner(QString::fromLocal8Bit(attr->values[0].string.text));
			else if (name == "job-printer-uri")
			{
				uri = QString::fromLatin1(attr->values[0].string.text);
				int	p = uri.findRev('/');
				if (p != -1)
					job->setPrinter(uri.right(uri.length()-p-1));
			}

			if (name.isEmpty() || attr == req.last())
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
				job = new KMJob();
			}

			attr = attr->next;
		}
		return true;
	}
	else
		return false;
}
