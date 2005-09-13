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

#include "gschecker.h"
#include "kpipeprocess.h"

#include <qfile.h>
#include <qtextstream.h>

GsChecker::GsChecker(QObject *parent, const char *name)
: QObject(parent,name)
{
}

bool GsChecker::checkGsDriver(const QString& name)
{
	if (m_driverlist.count() == 0)
		loadDriverList();
	return m_driverlist.contains(name);
}

void GsChecker::loadDriverList()
{
	KPipeProcess	proc;
	if (proc.open("gs -h",QIODevice::ReadOnly))
	{
		QTextStream	t(&proc);
		QString	buffer, line;
		bool	ok(false);
		while (!t.atEnd())
		{
			line = t.readLine().stripWhiteSpace();
			if (ok)
			{
				if (line.find(':') != -1)
					break;
				else
					buffer.append(line).append(" ");
			}
			else if (line.startsWith(QLatin1String("Available devices:")))
				ok = true;
		}
		m_driverlist = QStringList::split(' ',buffer,false);
	}
}
