/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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

#include "kmlprjobmanager.h"
#include "kmlprmanager.h"
#include "lpqhelper.h"
#include "lpchelper.h"
#include "kmjob.h"
#include "lprsettings.h"

#include <qptrlist.h>
#include <klocale.h>

KMLprJobManager::KMLprJobManager(QObject *parent, const char *name)
: KMJobManager(parent, name)
{
	m_lpqhelper = new LpqHelper(this, "LpqHelper");
}

bool KMLprJobManager::listJobs(const QString& prname, JobType, int limit)
{
	QPtrList<KMJob>	jobList;
	jobList.setAutoDelete(false);
	m_lpqhelper->listJobs(jobList, prname, limit);
	QPtrListIterator<KMJob>	it(jobList);
	for (; it.current(); ++it)
		addJob(it.current());
	return false;
}

LpcHelper* KMLprJobManager::lpcHelper()
{
	return static_cast<KMLprManager*>(KMManager::self())->lpcHelper();
}

int KMLprJobManager::actions()
{
	if (LprSettings::self()->mode() == LprSettings::LPR)
		return KMJob::Remove;
	else
		// some additional actions to be added here
		return (KMJob::Remove | KMJob::Hold | KMJob::Resume);
}

bool KMLprJobManager::sendCommandSystemJob(const QPtrList<KMJob>& jobs, int action, const QString& arg)
{
	QString	msg;
	QPtrListIterator<KMJob>	it(jobs);
	bool	status(true);
	LpcHelper	*helper = lpcHelper();

	for (; it.current() && status; ++it)
	{
		switch (action)
		{
			case KMJob::Remove:
				status = helper->removeJob(it.current(), msg);
				break;
			case KMJob::Hold:
				status = helper->changeJobState(it.current(), KMJob::Held, msg);
				break;
			case KMJob::Resume:
				status = helper->changeJobState(it.current(), KMJob::Queued, msg);
				break;
			default:
				status = false;
				msg = i18n("Unsupported operation.");
				break;
		}
	}
	if (!status && !msg.isEmpty())
		KMManager::self()->setErrorMsg(msg);
	return status;
}
