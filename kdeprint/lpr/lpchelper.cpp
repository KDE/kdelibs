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
#include "kpipeprocess.h"
#include "kmjob.h"

#include <kstandarddirs.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>
#include <stdlib.h>

QString execute(const QString& cmd)
{
	KPipeProcess	proc;
	QString		output;
	if (proc.open(cmd))
	{
		QTextStream	t(&proc);
		while (!t.atEnd())
			output.append(t.readLine()).append("\n");
		proc.close();
	}
	return output;
}

LpcHelper::LpcHelper(QObject *parent, const char *name)
: QObject(parent, name)
{
	// look for the "lpc" executable. Use the PATH variable and
	// add some specific dirs.
	QString	PATH = getenv("PATH");
	PATH.append(":/usr/sbin:/usr/local/sbin:/sbin:/opt/sbin:/opt/local/sbin");
	m_exepath = KStandardDirs::findExe("lpc", PATH);
	m_lprmpath = KStandardDirs::findExe("lprm");
}

LpcHelper::~LpcHelper()
{
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

void LpcHelper::updateStates()
{
	KPipeProcess	proc;

	m_state.clear();
	if (!m_exepath.isEmpty() && proc.open(m_exepath + " status all"))
	{
		QTextStream	t(&proc);
		QString		printer, line;
		int		p(-1);

		while (!t.atEnd())
		{
			line = t.readLine();
			if (line.isEmpty())
				continue;
			else if (!line[0].isSpace() && (p = line.find(':')) != -1)
			{
				printer = line.left(p);
				m_state[printer] = KMPrinter::Idle;
			}
			else if (line.find("disabled") != -1)
			{
				if (!printer.isEmpty())
					m_state[printer] = KMPrinter::Stopped;
			}
			else if (line.find("entries") != -1)
			{
				if (!printer.isEmpty() &&
				    m_state[printer] != KMPrinter::Stopped &&
				    line.find("no entries") == -1)
					m_state[printer] = KMPrinter::Processing;
			}
		}
		proc.close();
	}

}

bool LpcHelper::disable(KMPrinter *prt, QString& msg)
{
	return changeState(prt->printerName(), false, msg);
}

bool LpcHelper::enable(KMPrinter *prt, QString& msg)
{
	return changeState(prt->printerName(), true, msg);
}

bool LpcHelper::changeState(const QString& printer, bool state, QString& msg)
{
	QString	result = execute(m_exepath + (state ? " up " : " down ") + printer);
	if (result.startsWith(printer + ":"))
	{
		m_state[printer] = (state ? KMPrinter::Idle : KMPrinter::Stopped);
		return true;
	}
	else if (result.startsWith("?Privileged"))
		msg = i18n("Permission denied.");
	else if (result.startsWith("unknown"))
		msg = i18n("Printer %1 does not exist.").arg(printer);
	else
		msg = i18n("Unknown error: %1").arg(result.replace(QRegExp("\\n"), " "));
	return false;
}

bool LpcHelper::removeJob(KMJob *job, QString& msg)
{
	if (m_lprmpath.isEmpty())
	{
		msg = i18n("The executable lprm couldn't be find in your PATH.");
		return false;
	}
	QString	result = execute(m_lprmpath + " -P" + job->printer() + " " + QString::number(job->id()));
	if (result.find("dequeued") != -1)
		return true;
	else if (result.find("Permission denied") != -1)
		msg = i18n("Permission denied.");
	else
		msg = i18n("Execution of lprm failed: %1").arg(result);
	return false;
}
