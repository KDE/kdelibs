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

bool KMCupsJobManager::sendCommandSystemJob(const QPtrList<KMJob>& jobs, int action, const QString& argstr)
{
	IppRequest	req;
	QString		uri;
	bool		value(true);

	QPtrListIterator<KMJob>	it(jobs);
	for (;it.current() && value;++it)
	{
		req.addURI(IPP_TAG_OPERATION,"job-uri",it.current()->uri());
		req.addName(IPP_TAG_OPERATION,"requesting-user-name",CupsInfos::self()->realLogin());

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
			case KMJob::Restart:
				req.setOperation(IPP_RESTART_JOB);
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

	// printers jobs
	req.setOperation(IPP_GET_JOBS);
	uri = QString("ipp://%1:%2/printers/").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	keys.append("job-id");
	keys.append("job-uri");
	keys.append("job-printer-uri");
	keys.append("job-name");
	keys.append("job-state");
	keys.append("job-k-octets");
	keys.append("job-originating-user-name");
	keys.append("job-k-octets-completed");
	keys.append("job-media-sheets");
	keys.append("job-media-sheets-completed");
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);
	if (jobType() == KMJobManager::CompletedJobs)
		req.addKeyword(IPP_TAG_OPERATION,"which-jobs",QString::fromLatin1("completed"));

	if (req.doRequest("/"))
		parseListAnswer(req);
	else
		return false;

	// classes jobs
	req.init();
	req.setOperation(IPP_GET_JOBS);
	uri = QString("ipp://%1:%2/classes/").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);
	if (jobType() == KMJobManager::CompletedJobs)
		req.addKeyword(IPP_TAG_OPERATION,"which-jobs",QString::fromLatin1("completed"));

	if (req.doRequest("/"))
		parseListAnswer(req);
	else
		return false;

	return KMJobManager::listJobs();
}

void KMCupsJobManager::parseListAnswer(IppRequest& req)
{
	ipp_attribute_t	*attr = req.first();
	KMJob		*job = new KMJob();
	QString		uri;
	while (attr)
	{
		QString	name(attr->name);
		if (name == "job-id") job->setId(attr->values[0].integer);
		else if (name == "job-uri") job->setUri(QString::fromLocal8Bit(attr->values[0].string.text));
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
				case IPP_JOB_CANCELLED:
					job->setState(KMJob::Cancelled);
					break;
				case IPP_JOB_ABORTED:
					job->setState(KMJob::Aborted);
					break;
				case IPP_JOB_COMPLETED:
					job->setState(KMJob::Completed);
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
			uri = QString::fromLocal8Bit(attr->values[0].string.text);
			int	p = uri.findRev('/');
			if (p != -1)
				job->setPrinter(uri.right(uri.length()-p-1));
		}
		else if (name == "job-k-octets-completed") job->setProcessedSize(attr->values[0].integer);
		else if (name == "job-media-sheets") job->setPages(attr->values[0].integer);
		else if (name == "job-media-sheets-completed") job->setProcessedPages(attr->values[0].integer);

		if (name.isEmpty() || attr == req.last())
		{
		addJob(job);	// don't use job after this call !!!
			job = new KMJob();
		}

		attr = attr->next;
	}
}
