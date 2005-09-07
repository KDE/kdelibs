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

#ifndef KMDRIVERDB_H
#define KMDRIVERDB_H

#include <qobject.h>
#include <q3ptrlist.h>
#include <q3dict.h>
#include <qstring.h>

#include "kmdbentry.h"

class KMDBCreator;

class KMDriverDB : public QObject
{
	Q_OBJECT
public:
	static KMDriverDB* self();

	KMDriverDB(QObject *parent = 0);
	~KMDriverDB();

	void init(QWidget *parent = 0);
	KMDBEntryList* findEntry(const QString& manu, const QString& model);
	KMDBEntryList* findPnpEntry(const QString& manu, const QString& model);
	Q3Dict<KMDBEntryList>* findModels(const QString& manu);
	const Q3Dict< Q3Dict<KMDBEntryList> >& manufacturers() const	{ return m_entries; }

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
	Q3Dict< Q3Dict<KMDBEntryList> >	m_entries;
	Q3Dict< Q3Dict<KMDBEntryList> >	m_pnpentries;

	static KMDriverDB	*m_self;
};

#endif
