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

#ifndef KMLPDMANAGER_H
#define KMLPDMANAGER_H

#include "kmmanager.h"
#include <q3dict.h>

class PrintcapEntry;
class PrinttoolEntry;
class GsChecker;

class KMLpdManager : public KMManager
{
public:
	KMLpdManager(QObject *parent = 0, const char *name = 0);
	~KMLpdManager();

	bool completePrinterShort(KMPrinter*);
	bool completePrinter(KMPrinter*);
	bool createPrinter(KMPrinter*);
	bool removePrinter(KMPrinter*);
	bool enablePrinter(KMPrinter*);
	bool disablePrinter(KMPrinter*);

	// Driver DB functions
	QString driverDbCreationProgram();
	QString driverDirectory();

	// Driver loading functions
	DrMain* loadDbDriver(KMDBEntry*);
	DrMain* loadPrinterDriver(KMPrinter *p, bool config = false);
	bool savePrinterDriver(KMPrinter*, DrMain*);
	bool validateDbDriver(KMDBEntry*);

protected:
	void listPrinters();
	bool writePrinters();
	void loadPrintcapFile(const QString& filename);
	bool writePrintcapFile(const QString& filename);
	void loadPrinttoolDb(const QString& filename);
	QMap<QString,QString> loadPrinttoolCfgFile(const QString& filename);
	bool savePrinttoolCfgFile(const QString& templatefile, const QString& dirname, const QMap<QString,QString>& options);
	bool checkGsDriver(const QString& gsdriver);
	bool createSpooldir(PrintcapEntry*);
	bool createPrinttoolEntry(KMPrinter*, PrintcapEntry*);
	PrintcapEntry* findPrintcapEntry(const QString& name);
	PrinttoolEntry* findPrinttoolEntry(const QString& name);
	QString programName(int);
	void checkStatus();
	bool enablePrinter(KMPrinter*, bool);

private:
	Q3Dict<PrintcapEntry>	m_entries;
	Q3Dict<PrinttoolEntry>	m_ptentries;
	GsChecker		*m_gschecker;
};

#endif
