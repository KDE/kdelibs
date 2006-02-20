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

#include "kmlprjobmanager.h"
#include "kmlprmanager.h"
#include "lpqhelper.h"
#include "lpchelper.h"
#include "kmjob.h"
#include "lprsettings.h"

#include <klocale.h>

KMLprJobManager::KMLprJobManager(QObject *parent, const char *name, const QStringList & /*args*/)
    : KMJobManager(parent)
{
    m_lpqhelper = new LpqHelper(this );
    m_lpqhelper->setObjectName( "LpqHelper" );
}

bool KMLprJobManager::listJobs(const QString& prname, JobType, int limit)
{
	QList<KMJob*>	jobList;
	m_lpqhelper->listJobs(jobList, prname, limit);
	QListIterator<KMJob*>	it(jobList);
	while(it.hasNext())
		addJob(it.next());
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

bool KMLprJobManager::sendCommandSystemJob(const QList<KMJob*>& jobs, int action, const QString& arg)
{
	QString	msg;
	QListIterator<KMJob*>	it(jobs);
	bool	status(true);
	LpcHelper	*helper = lpcHelper();

	while(it.hasNext() && status )
	{
		KMJob *item = it.next();
		switch (action)
		{
			case KMJob::Remove:
				status = helper->removeJob(item, msg);
				break;
			case KMJob::Hold:
				status = helper->changeJobState(item, KMJob::Held, msg);
				break;
			case KMJob::Resume:
				status = helper->changeJobState(item, KMJob::Queued, msg);
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
