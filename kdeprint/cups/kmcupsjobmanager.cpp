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
#include "kmcupsmanager.h"
#include "kmjob.h"
#include "cupsinfos.h"
#include "ipprequest.h"
#include "pluginaction.h"

#include <klocale.h>

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
		// hypothesis: job operation are always done on local jobs. The only operation
		// allowed on remote jobs is listing (done elsewhere).

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
				uri = QString::fromLatin1("ipp://%1:%2/printers/%3").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port()).arg(argstr);
				req.addURI(IPP_TAG_OPERATION, "job-printer-uri", uri);
				break;
			default:
				return false;
		}

		value = req.doRequest("/jobs/");
	}

	return value;
}

bool KMCupsJobManager::listJobs(const QString& prname, KMJobManager::JobType type)
{
	IppRequest	req;
	QString		uri("ipp://%1:%2/%3/%4");
	QStringList	keys;
	CupsInfos	*infos = CupsInfos::self();

	// wanted attributes
	keys.append("job-id");
	keys.append("job-uri");
	keys.append("job-name");
	keys.append("job-state");
	keys.append("job-printer-uri");
	keys.append("job-k-octets");
	keys.append("job-originating-user-name");
	keys.append("job-k-octets-completed");
	keys.append("job-media-sheets");
	keys.append("job-media-sheets-completed");
	keys.append("job-priority");

	req.setOperation(IPP_GET_JOBS);

	// add printer-uri
	KMPrinter *mp = KMManager::self()->findPrinter(prname);
	if (!mp)
		return false;

	if (!mp->uri().isEmpty())
	{
		req.addURI(IPP_TAG_OPERATION, "printer-uri", mp->uri().prettyURL());
		req.setHost(mp->uri().host());
		req.setPort(mp->uri().port());
	}
	else
		req.addURI(IPP_TAG_OPERATION, "printer-uri", uri.arg(infos->host()).arg(infos->port()).arg(((mp&&mp->isClass())?"classes":"printers")).arg(prname));

	// other attributes
	req.addKeyword(IPP_TAG_OPERATION, "requested-attributes", keys);
	if (type == KMJobManager::CompletedJobs)
		req.addKeyword(IPP_TAG_OPERATION,"which-jobs",QString::fromLatin1("completed"));

	// send request
	if (req.doRequest("/"))
		parseListAnswer(req, mp);
	else
		return false;

	return true;
}

void KMCupsJobManager::parseListAnswer(IppRequest& req, KMPrinter *pr)
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
		else if (name == "job-k-octets-completed") job->setProcessedSize(attr->values[0].integer);
		else if (name == "job-media-sheets") job->setPages(attr->values[0].integer);
		else if (name == "job-media-sheets-completed") job->setProcessedPages(attr->values[0].integer);
		else if (name == "job-printer-uri" && !pr->isRemote())
		{
			QString	str(attr->values[0].string.text);
			int	p = str.findRev('/');
			if (p != -1)
				job->setPrinter(str.mid(p+1));
		}
		else if (name == "job-priority")
		{
			job->setAttribute(0, QString::fromLatin1("%1").arg(attr->values[0].integer, 3));
		}

		if (name.isEmpty() || attr == req.last())
		{
			if (job->printer().isEmpty())
				job->setPrinter(pr->printerName());
			job->setRemote(pr->isRemote());
			addJob(job);	// don't use job after this call !!!
			job = new KMJob();
		}

		attr = attr->next;
	}
}

bool KMCupsJobManager::doPluginAction(int ID, const QPtrList<KMJob>& jobs)
{
	switch (ID)
	{
		case 0:
			if (jobs.count() == 1)
				jobIppReport(jobs.getFirst());
			return true;
		case 1:
			return increasePriority(jobs);
		case 2:
			return decreasePriority(jobs);
	}
	return false;
}

void KMCupsJobManager::jobIppReport(KMJob *j)
{
	IppRequest	req;

	req.setOperation(IPP_GET_JOB_ATTRIBUTES);
	req.addURI(IPP_TAG_OPERATION, "job-uri", j->uri());
	if (req.doRequest("/"))
	{
		static_cast<KMCupsManager*>(KMManager::self())->ippReport(req, IPP_TAG_JOB, i18n("Job Report"));
	}
}

QValueList<KAction*> KMCupsJobManager::createPluginActions(KActionCollection *coll)
{
	QValueList<KAction*>	list;
	KAction	*act(0);

	list <<  (act = new PluginAction(0, i18n("Job IPP Report..."), "editpaste", 0, coll, "plugin_ipp"));
	act->setGroup("plugin");
	list << (act = new PluginAction(1, i18n("Increase Priority"), "up", 0, coll, "plugin_prioup"));
	act->setGroup("plugin");
	list << (act = new PluginAction(2, i18n("Decrease Priority"), "down", 0, coll, "plugin_priodown"));
	act->setGroup("plugin");

	return list;
}

void KMCupsJobManager::validatePluginActions(KActionCollection *coll, const QPtrList<KMJob>& joblist)
{
	QPtrListIterator<KMJob>	it(joblist);
	bool	syst(true);
	for (; it.current(); ++it)
	{
		syst = (syst && it.current()->type() == KMJob::System);
	}
	syst = (syst && joblist.count() > 0);
	coll->action("plugin_ipp")->setEnabled(joblist.count() == 1);
	coll->action("plugin_prioup")->setEnabled(syst && false);
	coll->action("plugin_priodown")->setEnabled(syst && false);
}

bool KMCupsJobManager::increasePriority(const QPtrList<KMJob>&)
{
	return false;
}

bool KMCupsJobManager::decreasePriority(const QPtrList<KMJob>&)
{
	return false;
}

#include "kmcupsjobmanager.moc"
