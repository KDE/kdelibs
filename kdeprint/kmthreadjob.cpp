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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kmthreadjob.h"
#include "kmjob.h"
#include "kmjobmanager.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <kstandarddirs.h>

#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#define CHARSEP	'$'

KMThreadJob::KMThreadJob(QObject *parent, const char *name)
: QObject(parent,name)
{
	m_jobs.setAutoDelete(true);
}

KMThreadJob::~KMThreadJob()
{
}

QString KMThreadJob::jobFile()
{
	QString	f = locateLocal("data","kdeprint/printjobs");
	return f;
}

bool KMThreadJob::saveJobs()
{
	QFile	f(jobFile());
	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		QIntDictIterator<KMJob>	it(m_jobs);
		for (;it.current();++it)
			t << it.current()->id() << CHARSEP << it.current()->name() << CHARSEP << it.current()->printer() << CHARSEP << it.current()->owner() << CHARSEP << it.current()->size() << endl;
		return true;
	}
	return false;
}

bool KMThreadJob::loadJobs()
{
	QFile	f(jobFile());
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		line;

		m_jobs.clear();
		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.isEmpty())
				continue;
			QStringList	ll = QStringList::split(CHARSEP,line,true);
			if (ll.count() == 5)
			{
				KMJob	*job = new KMJob();
				job->setId(ll[0].toInt());
				job->setName(ll[1]);
				job->setPrinter(ll[2]);
				job->setOwner(ll[3]);
				job->setSize(ll[4].toInt());
				job->setState(KMJob::Printing);
				job->setType(KMJob::Threaded);
				job->setUri("proc:/"+ll[0]);
				if (job->id() > 0 && checkJob(job->id()))
					m_jobs.insert(job->id(),job);
				else
					delete job;
			}
		}
		return true;
	}
	return false;
}

bool KMThreadJob::checkJob(int ID)
{
	return (kill((pid_t)ID,0) == 0 || errno == EPERM);
}

KMJob* KMThreadJob::findJob(int ID)
{
	return m_jobs.find(ID);
}

KMJob* KMThreadJob::findJob(const QString& uri)
{
	if (uri.startsWith("proc:/"))
	{
		int	pid = uri.mid(6).toInt();
		if (pid > 0)
			return m_jobs.find(pid);
	}
	return NULL;
}

bool KMThreadJob::removeJob(int ID)
{
	if (!checkJob(ID) || kill((pid_t)ID, SIGTERM) == 0)
	{
		m_jobs.remove(ID);
		saveJobs();
		return true;
	}
	else
		return false;
}

void KMThreadJob::createJob(int ID, const QString& printer, const QString& name, const QString& owner, int size)
{
	KMThreadJob	mth(0);
	KMJob	*job = new KMJob();
	job->setId(ID);
	job->setPrinter(printer);
	job->setName(name);
	job->setOwner(owner);
	job->setSize(size);
	job->setType(KMJob::Threaded);
	mth.createJob(job);
}

void KMThreadJob::createJob(KMJob *job)
{
	if (job->id() > 0)
	{
		loadJobs();
		if (!m_jobs.find(job->id()))
		{
			m_jobs.insert(job->id(),job);
			saveJobs();
		}
	}
}

void KMThreadJob::updateManager(KMJobManager *mgr)
{
	loadJobs();
	QIntDictIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
	{
		KMJob	*job = new KMJob(*(it.current()));
		mgr->addJob(job);
	}
}
