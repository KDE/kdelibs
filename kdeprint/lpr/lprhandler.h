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

#ifndef LPRHANDLER_H
#define LPRHANDLER_H

#include <qstring.h>

class PrintcapEntry;
class KMPrinter;
class DrMain;
class KMManager;
class KPrinter;

class LprHandler
{
public:
	LprHandler(const QString& name, KMManager *mgr = 0);
	virtual ~LprHandler();

	virtual bool validate(PrintcapEntry*);
	virtual KMPrinter* createPrinter(PrintcapEntry*);
	virtual bool completePrinter(KMPrinter*, PrintcapEntry*, bool shortmode = true);
	virtual DrMain* loadDriver(KMPrinter*, PrintcapEntry*, bool = false);
	virtual DrMain* loadDbDriver(const QString&);
	virtual bool savePrinterDriver(KMPrinter*, PrintcapEntry*, DrMain*, bool* = 0);
	virtual PrintcapEntry* createEntry(KMPrinter*);
	virtual bool removePrinter(KMPrinter*, PrintcapEntry*);
	virtual QString printOptions(KPrinter*);
	virtual void reset();

	QString name() const;
	KMManager* manager() const;
	QString driverDirectory();

protected:
	DrMain* loadToolDriver(const QString&);
	QString locateDir(const QString& dirname, const QString& paths);
	QString cachedDriverDir() const;
	void setCachedDriverDir(const QString&);
	virtual QString driverDirInternal();

protected:
	QString	m_name;
	KMManager	*m_manager;
	QString	m_cacheddriverdir;
};

inline QString LprHandler::name() const
{ return m_name; }

inline KMManager* LprHandler::manager() const
{ return m_manager; }

inline QString LprHandler::cachedDriverDir() const
{ return m_cacheddriverdir; }

inline void LprHandler::setCachedDriverDir(const QString& s)
{ m_cacheddriverdir = s; }

#endif
