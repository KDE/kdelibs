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

int KMFilterManager::insertFilter(QStringList& list, const QString& filtername, bool defaultToStart)
{
	int	pos(0);
	KPrintFilter	*f1(filter(filtername)), *f2(0);
	if (f1 && f1->inputMimeTypes().count() > 0)
	{
		QString	mimetype = f1->inputMimeTypes()[0];
		for (QStringList::Iterator it=list.begin(); it!=list.end(); ++it, pos++)
		{
			f2 = filter(*it);
			if (f2->acceptMimeType(f1->mimeType()) && f1->acceptMimeType(mimetype))
			{
				list.insert(it, filtername);
				break;
			}
			else
			{
				mimetype = f2->mimeType();
				delete f2;
				f2 = 0;
			}
		}
		if (pos == list.count())
		{
			if (list.count() == 0 || f1->acceptMimeType(mimetype))
				list.append(filtername);
			else if (defaultToStart)
			{
				pos = 0;
				list.prepend(filtername);
			}
			else
				pos = -1;
		}
	}
	delete f1;
	delete f2;
	return pos;
}
