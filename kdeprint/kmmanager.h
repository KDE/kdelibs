/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id:  $
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

#ifndef KMMANAGER_H
#define KMMANAGER_H

#include "kmprinter.h"

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>

class DrMain;
class KMDBEntry;
class QWidget;

class KMManager : public QObject
{
friend class KMVirtualManager;

public:
	KMManager(QObject *parent = 0, const char *name = 0);
	virtual ~KMManager();

	// error management functions
	QString errorMsg() const		{ return m_errormsg; }
	void setErrorMsg(const QString& s)	{ m_errormsg = s; }

	// printer management functions
	virtual bool createPrinter(KMPrinter *p);
	virtual bool removePrinter(KMPrinter *p);
	virtual bool enablePrinter(KMPrinter *p);
	virtual bool disablePrinter(KMPrinter *p);
	virtual bool completePrinter(KMPrinter *p);
	virtual bool completePrinterShort(KMPrinter *p);
	virtual bool setDefaultPrinter(KMPrinter *p);
	virtual bool testPrinter(KMPrinter *p);
	bool modifyPrinter(KMPrinter *oldp, KMPrinter *newp);
	bool removePrinter(const QString& name);
	bool enablePrinter(const QString& name);
	bool disablePrinter(const QString& name);
	bool completePrinter(const QString& name);
	bool setDefaultPrinter(const QString& name);

	// printer listing functions
	KMPrinter* findPrinter(const QString& name);
	QList<KMPrinter>* printerList(bool reload = true);

	// driver DB functions
	virtual QString driverDbCreationProgram();
	virtual QString driverDirectory();

	// driver functions
	virtual DrMain* loadPrinterDriver(KMPrinter *p, bool config = false);
	virtual DrMain* loadDbDriver(KMDBEntry *entry);
	virtual DrMain* loadFileDriver(const QString& filename);
	virtual bool savePrinterDriver(KMPrinter *p, DrMain *d);

	// configuration functions
	virtual bool configure(QWidget *parent = 0);

protected:
	// the real printer listing job is done here
	virtual void listPrinters();

	// utility functions
	void addPrinter(KMPrinter *p);	// in any case, the pointer given MUST not be used after
					// calling this function. Useful when listing printers.
	void setHardDefault(KMPrinter*);
	void setSoftDefault(KMPrinter*);
	// this function uncompress the given file (or does nothing
	// if the file is not compressed). Returns wether the file was
	// compressed or not.
	bool uncompressFile(const QString& srcname, QString& destname);
	bool notImplemented();

protected:
	QString			m_errormsg;
	KMPrinterList		m_printers;
};

#endif
