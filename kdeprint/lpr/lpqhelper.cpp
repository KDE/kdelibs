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
#include "kpipeprocess.h"

#include <kstandarddirs.h>
#include <kdebug.h>

LpqHelper::LpqHelper(QObject *parent, const char *name)
: QObject(parent, name)
{
	m_exepath = KStandardDirs::findExe("lpq");
}

LpqHelper::~LpqHelper()
{
}

KMJob* LpqHelper::parseLineLpr(const QString& line)
{
	QString	rank = line.left(7);
	if (!rank[0].isDigit() && rank != "active")
		return NULL;
	KMJob	*job = new KMJob;
	job->setState((rank[0].isDigit() ? KMJob::Queued : KMJob::Printing));
	job->setOwner(line.mid(7, 11).stripWhiteSpace());
	job->setId(line.mid(18, 5).toInt());
	job->setName(line.mid(23, 38).stripWhiteSpace());
	int	p = line.find(' ', 61);
	if (p != -1)
	{
		job->setSize(line.mid(61, p-61).toInt() / 1000);
	}
	return job;
}

void LpqHelper::listJobs(QPtrList<KMJob>& jobs, const QString& prname)
{
	KPipeProcess	proc;
	if (!m_exepath.isEmpty() && proc.open(m_exepath + " -P" + prname))
	{
		QTextStream	t(&proc);
		QString		line;
		while (!t.atEnd())
		{
			line = t.readLine();
			if (line.startsWith("Rank"))
				break;
		}
		while (!t.atEnd())
		{
			line = t.readLine();
			KMJob	*job = parseLineLpr(line);
			if (job)
			{
				job->setPrinter(prname);
				job->setUri("lpd://"+prname+"/"+QString::number(job->id()));
				jobs.append(job);
			}
			else
				break;
		}
		proc.close();
	}
}
