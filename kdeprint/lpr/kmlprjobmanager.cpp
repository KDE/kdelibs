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
#include "lpqhelper.h"
#include "kmjob.h"

#include <qptrlist.h>

KMLprJobManager::KMLprJobManager(QObject *parent, const char *name)
: KMJobManager(parent, name)
{
	m_lpqhelper = new LpqHelper(this, "LpqHelper");
}

bool KMLprJobManager::listJobs(const QString& prname, JobType)
{
	QPtrList<KMJob>	jobList;
	jobList.setAutoDelete(false);
	m_lpqhelper->listJobs(jobList, prname);
	QPtrListIterator<KMJob>	it(jobList);
	for (; it.current(); ++it)
		addJob(it.current());
	return false;
}
