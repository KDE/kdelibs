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

#include "kmfiltermanager.h"
#include "kprintfilter.h"
#include "kdeprintcheck.h"

#include <qdir.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>

KMFilterManager::KMFilterManager(QObject *parent, const char *name)
: QObject(parent,name)
{
}

KMFilterManager::~KMFilterManager()
{
}

KPrintFilter* KMFilterManager::filter(const QString& idname)
{
	return new KPrintFilter(idname,this);
}

QStringList KMFilterManager::filterList()
{
	if (m_flist.count() == 0)
	{
		QDir	d(locate("data","kdeprint/filters/"));
		QStringList	files = d.entryList("*.desktop",QDir::Files,QDir::Unsorted);
		for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		{
			KSimpleConfig	conf(d.absFilePath(*it));
			conf.setGroup("KDE Print Filter Entry");
			if (KdeprintChecker::check(&conf))
			{
				QString	value = conf.readEntry("Name",QString::null);
				if (!value.isEmpty())
				{
					m_flist.append(value);
					value = conf.readEntry("Comment",QString::null);
					if (value.isEmpty())
						value = m_flist.last();	// use idname by default
					m_flist.append(value);
				}
			}
		}
	}
	return m_flist;
}
