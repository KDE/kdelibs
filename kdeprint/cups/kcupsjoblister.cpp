#include "kcupsjoblister.h"
#include "cupsinfos.h"
#include "ipprequest.h"

#include <kdebug.h>
#include <cups/cups.h>

KCupsJobLister::KCupsJobLister()
: KJobLister()
{
}

KCupsJobLister::~KCupsJobLister()
{
}

bool KCupsJobLister::sendCommand(const QList<KJob>& jobs, int action, const QString& argstr)
{
	IppRequest	req;
	QString		uri;
	bool		value(true);

	QListIterator<KJob>	it(jobs);
	for (;it.current() && value;++it)
	{
		req.addURI(IPP_TAG_OPERATION,"job-uri",it.current()->uri);
		req.addName(IPP_TAG_OPERATION,"requesting-user-name",QString::fromLocal8Bit(cupsUser()));

		switch (action)
		{
			case KJob::Remove:
				req.setOperation(IPP_CANCEL_JOB);
				break;
			case KJob::Hold:
				req.setOperation(IPP_HOLD_JOB);
				break;
			case KJob::Resume:
				req.setOperation(IPP_RELEASE_JOB);
				break;
			case KJob::Move:
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

bool KCupsJobLister::listJobs()
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
		KJob		*job = new KJob();
		while (attr)
		{
			QString	name(attr->name);
			if (name == "job-id") job->ID = attr->values[0].integer;
			else if (name == "job-uri") job->uri = QString::fromLatin1(attr->values[0].string.text);
			else if (name == "job-name") job->name = QString::fromLocal8Bit(attr->values[0].string.text);
			else if (name == "job-state")
			{
				switch (attr->values[0].integer)
				{
					case IPP_JOB_PENDING:
						job->state = KJob::Queued;
						break;
					case IPP_JOB_HELD:
						job->state = KJob::Held;
						break;
					case IPP_JOB_PROCESSING:
						job->state = KJob::Printing;
						break;
					case IPP_JOB_STOPPED:
						job->state = KJob::Error;
						break;
					default:
						job->state = KJob::Unknown;
						break;
				}
			}
			else if (name == "job-k-octets") job->size = attr->values[0].integer;
			else if (name == "job-originating-user-name") job->user = QString::fromLocal8Bit(attr->values[0].string.text);
			else if (name == "job-printer-uri")
			{
				uri = QString::fromLatin1(attr->values[0].string.text);
				int	p = uri.findRev('/');
				if (p != -1)
					job->printer = uri.right(uri.length()-p-1);
			}

			if (name.isEmpty() || attr == req.last())
			{
				// only keep it if "printer" is not empty, and in printer filter
				if (job->ID > 0 && !job->printer.isEmpty() && printers_.contains(job->printer) > 0)
				{
					KJob	*aJob = findJob(job->ID);
					if (aJob)
					{
						jobCopy(job,aJob);
						delete job;
					}
					else
					{
						job->discarded = false;
						jobs_.append(job);
					}
				}
				else
					delete job;
				job = new KJob();
			}

			attr = attr->next;
		}
		return true;
	}
	else
		return false;
}

int KCupsJobLister::actions()
{
	return KJob::All;
}
