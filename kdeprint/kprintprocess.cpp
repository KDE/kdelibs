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

#include "kprintprocess.h"
#include <kapp.h>

KPrintProcess::KPrintProcess()
: KShellProcess()
{
	// redirect everything to a single buffer
	connect(this,SIGNAL(receivedStdout(KProcess*,char*,int)),SLOT(slotReceivedStderr(KProcess*,char*,int)));
	connect(this,SIGNAL(receivedStderr(KProcess*,char*,int)),SLOT(slotReceivedStderr(KProcess*,char*,int)));
}

KPrintProcess::~KPrintProcess()
{
}

QString KPrintProcess::errorMessage() const
{
	return m_buffer;
}

bool KPrintProcess::print()
{
	m_buffer = QString::null;
	return start(NotifyOnExit,All);
}

void KPrintProcess::slotReceivedStderr(KProcess *proc, char *buf, int len)
{
	if (proc == this)
	{
		QCString	str = QCString(buf,len).stripWhiteSpace();
		if (str.find("?password:") == 0)
		{
			QString	login(str.mid(10));
			emit passwordRequested(this, login);
		}
		else
			m_buffer.append(str.append("\n"));
	}
}
#include "kprintprocess.moc"
