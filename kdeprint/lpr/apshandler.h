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

#ifndef APSHANDLER_H
#define APSHANDLER_H

#include "lprhandler.h"

#include <qmap.h>

class ApsHandler : public LprHandler
{
public:
	ApsHandler(KMManager*);

	bool validate(PrintcapEntry*);
	KMPrinter* createPrinter(PrintcapEntry*);
	bool completePrinter(KMPrinter*, PrintcapEntry*, bool = true);
	DrMain* loadDriver(KMPrinter*, PrintcapEntry*, bool = false);
	DrMain* loadDbDriver(const QString&);
	void reset();
	PrintcapEntry* createEntry(KMPrinter*);
	bool savePrinterDriver(KMPrinter*, PrintcapEntry*, DrMain*, bool* = 0);
	bool removePrinter(KMPrinter*, PrintcapEntry*);
	QString printOptions(KPrinter*);

protected:
	QString driverDirInternal();

private:
	QMap<QString,QString> loadResources(PrintcapEntry*);
	QMap<QString,QString> loadVarFile(const QString&);
	QString sysconfDir();
	QString shareDir();
	DrMain* loadApsDriver(bool = false);

private:
	int	m_counter;
};

#endif
