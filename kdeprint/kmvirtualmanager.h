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

#ifndef KMVIRTUALMANAGER_H
#define KMVIRTUALMANAGER_H

#include <qobject.h>
#include <q3ptrlist.h>
#include <qdatetime.h>

#include <kdelibs_export.h>

#include "kpreloadobject.h"

class KMPrinter;
class KMManager;
class QWidget;

class KDEPRINT_EXPORT KMVirtualManager : public QObject, public KPReloadObject
{
public:
	KMVirtualManager(KMManager *parent = 0);
	~KMVirtualManager();

        void refresh();
	void reset();
	void virtualList(Q3PtrList<KMPrinter>& list, const QString& prname);
	void triggerSave();

	KMPrinter* findPrinter(const QString& name);
	KMPrinter* findInstance(KMPrinter *p, const QString& name);
	void setDefault(KMPrinter *p, bool save = true);
	QString defaultPrinterName();
	bool isDefault(KMPrinter *p, const QString& name);
	bool testInstance(KMPrinter *p);

	void create(KMPrinter *p, const QString& name);
	void remove(KMPrinter *p, const QString& name);
	void copy(KMPrinter *p, const QString& src, const QString& name);
	void setAsDefault(KMPrinter *p, const QString& name, QWidget *parent = NULL);

protected:
	void loadFile(const QString& filename);
	void saveFile(const QString& filename);
	void addPrinter(KMPrinter *p);
	void checkPrinter(KMPrinter*);
	void reload();
	void configChanged();

private:
	KMManager		*m_manager;
	QDateTime		m_checktime;
        QString                 m_defaultprinter;
};

inline void KMVirtualManager::reset()
{ m_checktime = QDateTime(); }

#endif
