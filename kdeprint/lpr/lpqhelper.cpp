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

#include "lpqhelper.h"
#include "kmjob.h"
#include "kmtimer.h"

#include <kstandarddirs.h>
#include <kprocess.h>
#include <kdebug.h>
#include <stdlib.h>
#include <stdio.h>

struct JobInfo
{
	int	rank;
	QString	owner;
	int	ID;
	QString	name;
	int	size;
	
	KMJob* toJob(const QString& prname);
};

KMJob* JobInfo::toJob(const QString& prname)
{
	KMJob	*job = new KMJob;
	job->setId(ID);
	job->setName(name);
	job->setOwner(owner);
	job->setState((rank == -1 ? KMJob::Printing : KMJob::Queued));
	job->setSize(size);
	job->setPrinter(prname);
	job->setUri("lpd://"+prname+"/"+QString::number(ID));
	return job;
}

struct LpqInfo
{
	LpqInfo(const QString& s) : prname(s), update(1)
	{
		jobs.setAutoDelete(true);
	}
	QString	prname;
	QPtrList<JobInfo>	jobs;
	int	update;
};

LpqHelper::LpqHelper(QObject *parent, const char *name)
: QObject(parent, name)
{
	m_lpq.setAutoDelete(true);
	m_exepath = KStandardDirs::findExe("lpq");

	if (!m_exepath.isEmpty())
	{
		m_proc = new KProcess;
		connect(m_proc, SIGNAL(processExited(KProcess*)), SLOT(slotExited(KProcess*)));
		connect(m_proc, SIGNAL(receivedStdout(KProcess*,char*,int)), SLOT(slotReceivedOutput(KProcess*,char*,int)));
		connect(KMTimer::self(), SIGNAL(timeout()), SLOT(slotTimeout()));
	}
}

LpqHelper::~LpqHelper()
{
	delete m_proc;
}

JobInfo* LpqHelper::splitLine(const QString& line, const QValueList<int>& fields)
{
	if (line.isEmpty())
		return NULL;

	int	pos(0);
	QStringList	l;
	for (QValueList<int>::ConstIterator it=fields.begin(); it!=fields.end(); ++it)
	{
		l << line.mid(pos, (*it));
		pos += (*it);
	}
	if (pos < line.length())
		l << line.mid(pos);

	if (l.count() == 5)
	{
		JobInfo	*info = new JobInfo;
		if (l[0].lower() == "active")
			info->rank = -1;
		else
			info->rank = l[0].toInt();
		info->owner = l[1];
		info->ID = l[2].toInt();
		info->name = l[3];
		info->size = l[4].toInt();
		return info;
	}
	return NULL;
}

void LpqHelper::parseOutput(const QString& buf, const QString& prname)
{
	QStringList	lines = QStringList::split("\n", buf, false);
	int	l(0);

	LpqInfo	*info = m_lpq.find(prname);
	if (!info)
	{
		info = new LpqInfo(prname);
		m_lpq.insert(prname, info);
	}
	else
	{
		info->jobs.clear();
	}

	while (l < lines.count())
	{
		if (lines[l++].left(4).lower() == "rank")
			break;
	}
	QValueList<int>	fields;
	fields << 8 << 8 << 8 << 32;
	while (l < lines.count())
	{
		info->jobs.append(splitLine(lines[l++], fields));
	}
	info->update--;
}

void LpqHelper::slotExited(KProcess*)
{
	QString	prname = m_updatelist.first();
	m_updatelist.remove(m_updatelist.begin());
	parseOutput(m_buffer, prname);
	processNext();
}

void LpqHelper::slotReceivedOutput(KProcess*, char *buf, int len)
{
	m_buffer.append(QCString(buf, len+1));
}

void LpqHelper::processNext()
{
	if (m_updatelist.isEmpty() || m_proc->isRunning())
		return;
	m_proc->clearArguments();
	m_buffer = QString::null;
	*m_proc << m_exepath << "-P" << m_updatelist.first();
	m_proc->start(KProcess::NotifyOnExit, KProcess::Stdout);
}

void LpqHelper::slotTimeout()
{
	if (m_proc->isRunning() || !m_updatelist.isEmpty())
		return;

	QDictIterator<LpqInfo>	it(m_lpq);
	while (it.current())
	{
		if (it.current()->update > 0)
		{
			m_updatelist.append(it.current()->prname);
			++it;
		}
		else
		{
			m_lpq.remove(it.currentKey());
		}
	}
	processNext();
}

void LpqHelper::updateNow(const QString& prname)
{
	if (m_exepath.isEmpty() || m_lpq.find(prname))
		return;

	char	buf[256] = {0};
	QString	cmd = (m_exepath + " -P " + KShellProcess::quote(prname)), sbuf;
	FILE	*f = popen(cmd.local8Bit(), "r");
	if (!f)
	{
		return;
	}
	while (fgets(buf, 255,f) != NULL)
		sbuf.append(buf);
	pclose(f);
	parseOutput(sbuf, prname);
}

void LpqHelper::listJobs(QPtrList<KMJob>& jobs, const QString& prname)
{
	updateNow(prname);

	LpqInfo	*info = m_lpq.find(prname);
	if (!info)
		return;
	info->update++;

	QPtrListIterator<JobInfo>	it(info->jobs);
	for (; it.current(); ++it)
		jobs.append(it.current()->toJob(prname));
}

#include "lpqhelper.moc"
