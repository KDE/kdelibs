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

#include "lpchelper.h"
#include "kmtimer.h"

#include <kprocess.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <stdlib.h>

LpcHelper::LpcHelper(QObject *parent, const char *name)
: QObject(parent, name)
{
	m_proc = 0;

	// look for the "lpc" executable. Use the PATH variable and
	// add some specific dirs.
	QString	PATH = getenv("PATH");
	PATH.append(":/usr/sbin:/usr/local/sbin:/sbin:/opt/sbin:/opt/local/sbin");
	m_exepath = KStandardDirs::findExe("lpc", PATH);

	// found, create the KProcIO and establish connections
	if (!m_exepath.isEmpty())
	{
		m_proc = new KProcess;
		connect(m_proc, SIGNAL(receivedStdout(KProcess*,char*,int)), SLOT(slotReceivedOutput(KProcess*,char*,int)));
		connect(m_proc, SIGNAL(processExited(KProcess*)), SLOT(slotExited(KProcess*)));
		connect(KMTimer::self(), SIGNAL(timeout()), SLOT(slotTimeout()));
		// start it immediately
		slotTimeout();
	}
}

LpcHelper::~LpcHelper()
{
	delete m_proc;
}

KMPrinter::PrinterState LpcHelper::state(const QString& prname) const
{
	if (m_state.contains(prname))
		return m_state[prname];
	return KMPrinter::Unknown;
}

KMPrinter::PrinterState LpcHelper::state(KMPrinter *prt) const
{
	return state(prt->printerName());
}

void LpcHelper::slotReceivedOutput(KProcess*, char *buf, int len)
{
	m_buffer.append(QCString(buf, len+1));
}

void LpcHelper::slotExited(KProcess*)
{
	m_state.clear();
	if (!m_proc->normalExit() || m_proc->exitStatus() != 0)
		return;

	parseStatusOutput(m_buffer);
}

void LpcHelper::parseStatusOutput(const QString& buf)
{
	QStringList	lines = QStringList::split("\n", buf, false);
	QString	prname;
	int	p;

	m_state.clear();
	for (QStringList::ConstIterator it=lines.begin(); it!=lines.end(); ++it)
	{
		if ((*it).isEmpty())
			continue;
		else if (!(*it)[0].isSpace() && (p=(*it).find(':')) != -1)
		{
			prname = (*it).left(p).stripWhiteSpace();
			m_state[prname] = KMPrinter::Processing;
		}
		else if ((*it).find("disabled") != -1)
		{
			if (!prname.isEmpty())
			{
				m_state[prname] = KMPrinter::Stopped;
			}
		}
		else if ((*it).find("no entries") != -1)
		{
			if (!prname.isEmpty() && m_state[prname] != KMPrinter::Stopped)
			{
				m_state[prname] = KMPrinter::Idle;
			}
		}
	}
}

void LpcHelper::slotTimeout()
{
	// if already running, just skip this call
	if (!m_proc || m_proc->isRunning())
		return;

	m_proc->clearArguments();
	m_buffer = QString::null;
	*m_proc << m_exepath << "status";
	m_proc->start((m_state.count() == 0 ? KProcess::Block : KProcess::NotifyOnExit), KProcess::Stdout);
}

#include "lpchelper.moc"
