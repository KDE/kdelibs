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

#include "kpipeprocess.h"

KPipeProcess::KPipeProcess(const QString& cmd, int mode)
: QFile()
{
	m_pipe = NULL;
	if (!cmd.isEmpty())
		open(cmd,mode);
}

KPipeProcess::~KPipeProcess()
{
	close();
}

bool KPipeProcess::open(const QString& cmd, int mode)
{
	// close first if needed
	close();
	// check supported modes
	if (mode != IO_ReadOnly && mode != IO_WriteOnly)
		return false;

	// create the pipe
	m_pipe = popen(cmd.latin1(),(mode == IO_WriteOnly ? "w" : "r"));
	if (m_pipe)
		if (!QFile::open(mode,m_pipe))
			close();
	return (m_pipe != NULL);
}

void KPipeProcess::close()
{
	if (m_pipe != NULL)
	{
		QFile::close();
		m_pipe = NULL;
	}
}
