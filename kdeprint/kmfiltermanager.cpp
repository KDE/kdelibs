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

struct KMFilterManager::FilterInfo
{
	QString	name;
	QString	desc;
	QStringList	mimeSrc;
	QString		mimeDest;
	QStringList	requirements;
};

//-------------------------------------------------------------------------------------------

KMFilterManager::KMFilterManager(QObject *parent, const char *name)
: QObject(parent,name)
{
	m_filters.setAutoDelete(true);
	m_filterdict.setAutoDelete(true);
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
	loadFilters();
	return m_flist;
}

KMFilterManager::FilterInfo* KMFilterManager::load(const QString& filename)
{
	KSimpleConfig	conf(filename);
	conf.setGroup("KDE Print Filter Entry");
	if (KdeprintChecker::check(&conf))
	{
		FilterInfo	*fi = new FilterInfo;
		fi->name = conf.readEntry("Name");
		fi->desc = conf.readEntry("Comment");
		fi->requirements = conf.readEntry("Requires");
		fi->mimeSrc = conf.readListEntry("MimeTypeIn");
		fi->mimeDest = conf.readEntry("MimeTypeOut");

		if (fi->desc.isEmpty())
			fi->desc = fi->name;
		if (fi->name.isEmpty())
		{
			delete fi;
			fi = NULL;
		}
		return fi;
	}
	return NULL;
}

void KMFilterManager::loadFilters()
{
	if (m_filters.count() == 0)
	{
		QDir	d(locate("data","kdeprint/filters/"));
		QStringList	files = d.entryList("*.desktop",QDir::Files,QDir::Unsorted);
		for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		{
			FilterInfo	*fi = load(d.absFilePath(*it));
			if (fi)
			{
				m_filters.append(fi);
				for (QStringList::ConstIterator mit=fi->mimeSrc.begin(); mit!=fi->mimeSrc.end(); ++mit)
					insertToDict(fi, *mit);
				m_flist << fi->name << fi->desc;
			}
		}

	}
}

void KMFilterManager::insertToDict(FilterInfo *fi, const QString& mime)
{
	QPtrList<FilterInfo>	*l = m_filterdict.find(mime);
	if (!l)
	{
		l = new QPtrList<FilterInfo>;
		l->setAutoDelete(false);
		m_filterdict.insert(mime, l);
	}
	l->append(fi);
}

bool KMFilterManager::checkFilter(const QString& filtername)
{
	QString	path = locate("data", QString::fromLatin1("kdeprint/filters/%1.desktop").arg(filtername));
	if (!path.isEmpty())
	{
		KSimpleConfig	conf(path);
		conf.setGroup("KDE Print Filter Entry");
		return KdeprintChecker::check(&conf);
	}
	return false;
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

QStringList KMFilterManager::autoFilter(const QString& mimesrc, const QString& mimedest)
{
	QStringList	chain;
	int		score(0);

	if (m_filters.count() == 0)
		loadFilters();

	QPtrList<FilterInfo>	*l = m_filterdict.find(mimesrc);
	if (l)
	{
		for (l->first(); l->current(); l->next())
		{
			// direct filter: shortest path => return immediately
			if (l->current()->mimeDest == mimedest)
			{
				chain = QStringList(l->current()->name);
				break;
			}
			// non direct filter: find the shortest way between
			// its output and mimedest (do not consider cyling filters)
			else if (l->current()->mimeDest != mimesrc)
			{
				QStringList	subchain = autoFilter(l->current()->mimeDest, mimedest);
				// If chain length is 0, then there's no possible filter between those 2
				// mime types. Just discard it. If the subchain contains also the current
				// considered filter, then discard it: it denotes of a cycle in filter
				// chain.
				if (subchain.count() > 0 && subchain.findIndex(l->current()->name) == -1)
				{
					subchain.prepend(l->current()->name);
					if (subchain.count() < score || score == 0)
					{
						chain = subchain;
						score = subchain.count();
					}
				}
			}
		}
	}
	// At this point, either we have the shortest path, or empty
	// list if nothing could be found
	return chain;
}
