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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KMLRMANAGER_H
#define KMLRMANAGER_H

#include "kmmanager.h"

#include <q3dict.h>
#include <q3ptrlist.h>
#include <qdatetime.h>
#include <kurl.h>

class LprHandler;
class PrintcapEntry;
class LpcHelper;
class KPrinter;

class KMLprManager : public KMManager
{
	Q_OBJECT
public:
	KMLprManager(QObject *parent, const char *name, const QStringList & /*args*/);

	bool completePrinter(KMPrinter*);
	bool completePrinterShort(KMPrinter*);
	bool enablePrinter(KMPrinter*, bool);
	bool startPrinter(KMPrinter*, bool);
	bool savePrinterDriver(KMPrinter*, DrMain*);
	DrMain* loadPrinterDriver(KMPrinter*, bool = false);
	DrMain* loadFileDriver(const QString&);
	bool createPrinter(KMPrinter*);
	bool removePrinter(KMPrinter*);

	QString driverDbCreationProgram();
	QString driverDirectory();

	LpcHelper* lpcHelper()	{ return m_lpchelper; }
	QString printOptions(KPrinter*);

	void createPluginActions(KActionCollection*);
	void validatePluginActions(KActionCollection*, KMPrinter*);
	QString stateInformation();

protected slots:
	void slotEditPrintcap();

protected:
	void listPrinters();
	void initHandlers();
	void insertHandler(LprHandler*);
	PrintcapEntry* findEntry(KMPrinter*);
	LprHandler* findHandler(KMPrinter*);
	void checkPrinterState(KMPrinter*);
	bool savePrintcapFile();

private:
	Q3Dict<LprHandler>	m_handlers;
	Q3PtrList<LprHandler>    m_handlerlist;
	Q3Dict<PrintcapEntry>	m_entries;
	QDateTime		m_updtime;
	LpcHelper		*m_lpchelper;
	KMPrinter		*m_currentprinter;
};

#endif
