/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#ifndef KMDRIVERDB_H
#define KMDRIVERDB_H

#include <qobject.h>
#include <qptrlist.h>
#include <qdict.h>
#include <qstring.h>

#include "kmdbentry.h"

class KMDBCreator;

class KMDriverDB : public QObject
{
	Q_OBJECT
public:
	static KMDriverDB* self();

	KMDriverDB(QObject *parent = 0, const char *name = 0);
	~KMDriverDB();

	void init(QWidget *parent = 0);
	KMDBEntryList* findEntry(const QString& manu, const QString& model);
	KMDBEntryList* findPnpEntry(const QString& manu, const QString& model);
	QDict<KMDBEntryList>* findModels(const QString& manu);
	const QDict< QDict<KMDBEntryList> >& manufacturers() const	{ return m_entries; }

protected:
	void loadDbFile();
	void insertEntry(KMDBEntry *entry);
	QString dbFile();

protected slots:
	void slotDbCreated();

signals:
	void dbLoaded(bool reloaded);
	void error(const QString&);

private:
	KMDBCreator			*m_creator;
	QDict< QDict<KMDBEntryList> >	m_entries;
	QDict< QDict<KMDBEntryList> >	m_pnpentries;

	static KMDriverDB	*m_self;
};

#endif
