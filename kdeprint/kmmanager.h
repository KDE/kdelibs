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

#ifndef KMMANAGER_H
#define KMMANAGER_H

#include <kdeprint/kmprinter.h>

#include <qobject.h>
#include <qstring.h>
#include <qptrlist.h>

class DrMain;
class KMDBEntry;
class KMVirtualManager;
class KMSpecialManager;
class QWidget;
class KActionCollection;
class PrinterFilter;

class KMManager : public QObject
{
	Q_OBJECT

friend class KMVirtualManager;
friend class KMSpecialManager;
friend class KMFactory;

public:
	enum PrinterOperations {
		PrinterEnabling  = 0x01,
		PrinterCreation  = 0x02,
		PrinterDefault   = 0x04,
		PrinterTesting   = 0x08,
		PrinterConfigure = 0x10,
		PrinterRemoval   = 0x20,
		PrinterAll       = 0xFF
	};
	enum ServerOperations {
		ServerRestarting = 0x1,
		ServerConfigure  = 0x2,
		ServerAll        = 0xF
	};

	KMManager(QObject *parent = 0, const char *name = 0);
	virtual ~KMManager();

    static KMManager* self();

	// error management functions
	QString errorMsg() const		{ return m_errormsg; }
	void setErrorMsg(const QString& s)	{ m_errormsg = s; }

	// support management ?
	bool hasManagement() const 		{ return m_hasmanagement; }

	// printer management functions
	virtual bool createPrinter(KMPrinter *p);
	virtual bool removePrinter(KMPrinter *p);
	virtual bool enablePrinter(KMPrinter *p, bool on);
	virtual bool startPrinter(KMPrinter *p, bool on);
	virtual bool completePrinter(KMPrinter *p);
	virtual bool completePrinterShort(KMPrinter *p);
	virtual bool setDefaultPrinter(KMPrinter *p);
	virtual bool testPrinter(KMPrinter *p);
	bool upPrinter(KMPrinter *p, bool state);
	bool modifyPrinter(KMPrinter *oldp, KMPrinter *newp);
	bool removePrinter(const QString& name);
	bool enablePrinter(const QString& name, bool state);
	bool startPrinter(const QString& name, bool state);
	bool completePrinter(const QString& name);
	bool setDefaultPrinter(const QString& name);
	int printerOperationMask() const 	{ return m_printeroperationmask; }
	int addPrinterWizard(QWidget *parent = 0);

	// special printer management functions
	bool createSpecialPrinter(KMPrinter *p);
	bool removeSpecialPrinter(KMPrinter *p);

	// printer listing functions
	KMPrinter* findPrinter(const QString& name);
	QPtrList<KMPrinter>* printerList(bool reload = true);
	QPtrList<KMPrinter>* printerListComplete(bool reload = true);
	KMPrinter* defaultPrinter();
	void enableFilter(bool on);
	bool isFilterEnabled() const;

	// driver DB functions
	virtual QString driverDbCreationProgram();
	virtual QString driverDirectory();

	// driver functions
	virtual DrMain* loadPrinterDriver(KMPrinter *p, bool config = false);
	virtual DrMain* loadDbDriver(KMDBEntry *entry);
	virtual DrMain* loadFileDriver(const QString& filename);
	DrMain* loadDriver(KMPrinter *p, bool config = false);
	virtual bool savePrinterDriver(KMPrinter *p, DrMain *d);
	virtual bool validateDbDriver(KMDBEntry *entry);

	// configuration functions
	bool invokeOptionsDialog(QWidget *parent = 0);
	virtual QString stateInformation();

	// server functions
	int serverOperationMask() const 	{ return m_serveroperationmask; }
	virtual bool restartServer();
	virtual bool configureServer(QWidget *parent = 0);
	virtual QStringList detectLocalPrinters();

	// additional actions (for print manager)
	virtual void createPluginActions(KActionCollection*);
	virtual void validatePluginActions(KActionCollection*, KMPrinter*);

	// utility function
	virtual void checkUpdatePossible();

signals:
	void updatePossible( bool );

protected:
	// the real printer listing job is done here
	virtual void listPrinters();

	// utility functions
	void addPrinter(KMPrinter *p);	// in any case, the pointer given MUST not be used after
					// calling this function. Useful when listing printers.
	void setHardDefault(KMPrinter*);
	void setSoftDefault(KMPrinter*);
	KMPrinter* softDefault() const;
	KMPrinter* hardDefault() const;
	// this function uncompress the given file (or does nothing
	// if the file is not compressed). Returns wether the file was
	// compressed or not.
	bool uncompressFile(const QString& srcname, QString& destname);
	bool notImplemented();
	void setHasManagement(bool on)		{ m_hasmanagement = on; }
	void setPrinterOperationMask(int m)	{ m_printeroperationmask = m; }
	void setServerOperationMask(int m)	{ m_serveroperationmask = m; }
	QString testPage();
	void discardAllPrinters(bool);

protected:
	QString			m_errormsg;
	KMPrinterList		m_printers, m_fprinters;	// filtered printers
	bool 			m_hasmanagement;
	int			m_printeroperationmask;
	int 			m_serveroperationmask;
	KMSpecialManager	*m_specialmgr;
	KMVirtualManager	*m_virtualmgr;
	PrinterFilter	*m_printerfilter;
};

#endif
