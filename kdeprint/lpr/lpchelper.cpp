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
#include "lprsettings.h"

#include <kstandarddirs.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <stdlib.h>

static QString execute(const QString& cmd)
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
	m_checkpcpath = KStandardDirs::findExe("checkpc", PATH);
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

void LpcHelper::parseStatusLPR(QTextStream &t)
{
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
		else if (line.find("printing is disabled") != -1)
		{
			if (!printer.isEmpty())
				m_state[printer] = KMPrinter::PrinterState((KMPrinter::Stopped) | (m_state[printer] & ~KMPrinter::StateMask));
		}
		else if (line.find("queuing is disabled") != -1)
		{
			if (!printer.isEmpty())
				m_state[printer] = KMPrinter::PrinterState((KMPrinter::Rejecting) | (m_state[printer] & KMPrinter::StateMask));
		}
		else if (line.find("entries") != -1)
		{
			if (!printer.isEmpty() &&
			    (m_state[printer] & KMPrinter::StateMask) != KMPrinter::Stopped &&
			    line.find("no entries") == -1)
				m_state[printer] = KMPrinter::PrinterState((m_state[printer] & ~KMPrinter::StateMask) | KMPrinter::Processing);
		}
	}
}

void LpcHelper::parseStatusLPRng(QTextStream& t)
{
	QStringList	l;
	int	p(-1);
	QString	printer;

	while (!t.atEnd())
		if (t.readLine().stripWhiteSpace().startsWith("Printer"))
			break;
	while (!t.atEnd())
	{
		l = QStringList::split(QRegExp("\\s"), t.readLine(), false);
		if (l.count() < 4)
			continue;
		p = l[0].find('@');
		if (p == 0)
			printer = l[0];
		else
			printer = l[0].left(p);
		int	st(0);
		if (l[1] == "disabled")
			st = KMPrinter::Stopped;
		else if (l[3] != "0")
			st = KMPrinter::Processing;
		else
			st = KMPrinter::Idle;
		if (l[2] == "disabled")
			st |= KMPrinter::Rejecting;
		m_state[printer] = KMPrinter::PrinterState(st);
	}
}

void LpcHelper::updateStates()
{
	KPipeProcess	proc;

	m_state.clear();
	if (!m_exepath.isEmpty() && proc.open(m_exepath + " status all"))
	{
		QTextStream	t(&proc);

		switch (LprSettings::self()->mode())
		{
			default:
			case LprSettings::LPR:
				parseStatusLPR(t);
				break;
			case LprSettings::LPRng:
				parseStatusLPRng(t);
				break;
		}
		proc.close();
	}

}

bool LpcHelper::enable(KMPrinter *prt, bool state, QString& msg)
{
	int	st = m_state[prt->printerName()] & KMPrinter::StateMask;
	if (changeState(prt->printerName(), (state ? "enable" : "disable"), msg))
	{
		m_state[prt->printerName()] = KMPrinter::PrinterState((state ? KMPrinter::Rejecting : 0) | st);
		return true;
	}
	return false;
}

bool LpcHelper::start(KMPrinter *prt, bool state, QString& msg)
{
	int	rej = m_state[prt->printerName()] & ~KMPrinter::StateMask;
	if (changeState(prt->printerName(), (state ? "start" : "stop"), msg))
	{
		m_state[prt->printerName()] = KMPrinter::PrinterState((state ? KMPrinter::Idle : KMPrinter::Stopped) | rej);
		return true;
	}
	return false;
}

// status
//    0 : success
//   -1 : permission denied
//   -2 : unknown printer
//    1 : unknown error
int LpcHelper::parseStateChangeLPR(const QString& result, const QString& printer)
{
	if (result.startsWith(printer + ":"))
		return 0;
	else if (result.startsWith("?Privileged"))
		return -1;
	else if (result.startsWith("unknown"))
		return -2;
	else
		return 1;
}

static QString lprngAnswer(const QString& result, const QString& printer)
{
	int	p, q;

	p = result.find("\n" + printer);
	if (p != -1)
	{
		q = result.find(':', p)+2;
		p = result.find('\n', q);
		QString	answer = result.mid(q, p-q).stripWhiteSpace();
		return answer;
	}
	return QString::null;
}

int LpcHelper::parseStateChangeLPRng(const QString& result, const QString& printer)
{
	QString	answer = lprngAnswer(result, printer);
	if (answer == "no")
		return -1;
	else if (answer == "disabled" || answer == "enabled" || answer == "started" || answer == "stopped")
		return 0;
	else
		return 1;
}

bool LpcHelper::changeState(const QString& printer, const QString& op, QString& msg)
{
	if (m_exepath.isEmpty())
	{
		msg = i18n("Unable to locate executable %1 in your PATH.").arg("lpc");
		return false;
	}
	QString	result = execute(m_exepath + " " + op + " " + KProcess::quote(printer));
	int	status;

	switch (LprSettings::self()->mode())
	{
		default:
		case LprSettings::LPR:
			status = parseStateChangeLPR(result, printer);
			break;
		case LprSettings::LPRng:
			status = parseStateChangeLPRng(result, printer);
			break;
	}
	switch (status)
	{
		case 0:
			break;
		case -1:
			msg = i18n("Permission denied");
			break;
		case -2:
			msg = i18n("Printer %1 does not exist.").arg(printer);
			break;
		default:
		case 1:
			msg = i18n("Unknown error: %1").arg(result.replace(QRegExp("\\n"), " "));
			break;
	}
	return (status == 0);
}

bool LpcHelper::removeJob(KMJob *job, QString& msg)
{
	if (m_lprmpath.isEmpty())
	{
		msg = i18n("The executable %1 couldn't be find in your PATH.").arg("lprm");
		return false;
	}
	QString	result = execute(m_lprmpath + " -P " + KProcess::quote(job->printer()) + " " + QString::number(job->id()));
	if (result.find("dequeued") != -1)
		return true;
	else if (result.find("Permission denied") != -1 || result.find("no permissions") != -1)
		msg = i18n("Permission denied.");
	else
		msg = i18n("Execution of lprm failed: %1").arg(result);
	return false;
}

// LPRng only
bool LpcHelper::changeJobState(KMJob *job, int state, QString& msg)
{
	if (m_lprmpath.isEmpty())
	{
		msg = i18n("The executable %1 couldn't be find in your PATH.").arg("lpc");
		return false;
	}
	QString	result = execute(m_exepath + (state == KMJob::Held ? " hold " : " release ") + KProcess::quote(job->printer()) + " " + QString::number(job->id()));
	QString	answer = lprngAnswer(result, job->printer());
	if (answer == "no")
	{
		msg = i18n("Permission denied.");
		return false;
	}
	else
		return true;
}

bool LpcHelper::restart(QString& msg)
{
	QString	s;
	if (m_exepath.isEmpty())
		s = "lpc";
	else if (m_checkpcpath.isEmpty())
		s = "checkpc";
	if (!s.isEmpty())
	{
		msg = i18n("The executable %1 couldn't be find in your PATH.").arg(s);
		return false;
	}
	::system(QFile::encodeName(m_exepath + " reread"));
	::system(QFile::encodeName(m_checkpcpath + " -f"));
	return true;
}
