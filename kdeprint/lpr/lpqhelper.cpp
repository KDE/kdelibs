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
#include "lprsettings.h"

#include <kstandarddirs.h>
#include <kprocess.h>
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

KMJob* LpqHelper::parseLineLPRng(const QString& line)
{
	QString	rank = line.left(7).stripWhiteSpace();
	if (!rank[0].isDigit() && rank != "active" && rank != "hold")
		return NULL;
	KMJob	*job = new KMJob;
	job->setState((rank[0].isDigit() ? KMJob::Queued : (rank == "hold" ? KMJob::Held : KMJob::Printing)));
	int	p = line.find('@', 7), q = line.find(' ', 7);
	job->setOwner(line.mid(7, QMIN(p,q)-7));
	while (line[q].isSpace())
		q++;
	q++;
	while (line[q].isSpace())
		q++;
	p = line.find(' ', q);
	job->setId(line.mid(q, p-q).toInt());
	while (line[p].isSpace())
		p++;
	q = p+25;
	while (line[q].isDigit())
		q--;
	job->setName(line.mid(p, q-p).stripWhiteSpace());
	job->setSize(line.mid(q+1, p+26-q).toInt() / 1000);
	return job;
}

void LpqHelper::listJobs(QPtrList<KMJob>& jobs, const QString& prname, int limit)
{
	KPipeProcess	proc;
	if (!m_exepath.isEmpty() && proc.open(m_exepath + " -P " + KProcess::quote(prname)))
	{
		QTextStream	t(&proc);
		QString		line;
		bool	lprng = (LprSettings::self()->mode() == LprSettings::LPRng);
		int count = 0;

		while (!t.atEnd())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.startsWith("Rank"))
				break;
		}
		while (!t.atEnd())
		{
			line = t.readLine();
			if ( limit > 0 && count >= limit )
				continue;
			KMJob	*job = (lprng ? parseLineLPRng(line) : parseLineLpr(line));
			if (job)
			{
				job->setPrinter(prname);
				job->setUri("lpd://"+prname+"/"+QString::number(job->id()));
				jobs.append(job);
				count++;
			}
			else
				break;
		}
		proc.close();
	}
}
