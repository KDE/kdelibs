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

#ifndef KMLRMANAGER_H
#define KMLRMANAGER_H

#include "kmmanager.h"

#include <qdict.h>
#include <qptrlist.h>
#include <qdatetime.h>
#include <kurl.h>

class LprHandler;
class PrintcapEntry;
class LpcHelper;

class KMLprManager : public KMManager
{
public:
	KMLprManager(QObject *parent = 0, const char *name = 0);

	bool completePrinter(KMPrinter*);
	bool completePrinterShort(KMPrinter*);
	bool enablePrinter(KMPrinter*);
	bool disablePrinter(KMPrinter*);
	bool savePrinterDriver(KMPrinter*, DrMain*);
	DrMain* loadPrinterDriver(KMPrinter*, bool = false);
	DrMain* loadFileDriver(const QString&);
	bool createPrinter(KMPrinter*);
	bool removePrinter(KMPrinter*);
	
	QString driverDbCreationProgram();
	QString driverDirectory();
	
	LpcHelper* lpcHelper()	{ return m_lpchelper; }

protected:
	void listPrinters();
	void initHandlers();
	void insertHandler(LprHandler*);
	PrintcapEntry* findEntry(KMPrinter*);
	LprHandler* findHandler(KMPrinter*);
	void checkPrinterState(KMPrinter*);
	bool savePrintcapFile();

private:
	QDict<LprHandler>	m_handlers;
	QPtrList<LprHandler>    m_handlerlist;
	QDict<PrintcapEntry>	m_entries;
	QDateTime		m_updtime;
	LpcHelper		*m_lpchelper;
};

#endif
