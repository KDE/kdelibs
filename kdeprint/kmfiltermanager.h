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

#ifndef KMFILTERMANAGER_H
#define KMFILTERMANAGER_H

#include <qobject.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qdict.h>

class KPrintFilter;

class KMFilterManager : public QObject
{
public:
	KMFilterManager(QObject *parent = 0, const char *name = 0);
	~KMFilterManager();

	KPrintFilter* filter(const QString& idname);
	QStringList filterList();
	int insertFilter(QStringList& list, const QString& filter, bool defaultToStart = true);
	bool checkFilter(const QString& filtername);
	QStringList autoFilter(const QString& mimesrc, const QString& mimedest);

private:
	struct FilterInfo;

protected:
	void loadFilters();
	FilterInfo* load(const QString& filename);
	void insertToDict(FilterInfo *fi, const QString& mime);

private:
	struct FilterInfo;
	QStringList		m_flist;
	QList<FilterInfo>	m_filters;
	QDict< QList<FilterInfo> >	m_filterdict;
};

#endif
