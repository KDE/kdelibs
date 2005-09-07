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

#ifndef KMUIMANAGER_H
#define KMUIMANAGER_H

#include <qobject.h>
#include <q3ptrlist.h>

#include "kprinter.h"

class KMPropertyPage;
class KMWizard;
class KPrintDialogPage;
class KPrintDialog;
class KPrinterPropertyDialog;
class KMConfigDialog;
class Q3ListView;

class KDEPRINT_EXPORT KMUiManager : public QObject
{
	Q_OBJECT
public:
	enum PrintDialogFlagType
	{
		Properties     = 0x0001,
		Default        = 0x0002,
		Preview        = 0x0004,
		OutputToFile   = 0x0008,
		PrintCommand   = 0x0020,
		Persistent     = 0x0040,
		PrintDialogAll = 0x001F
	};
	enum CopyFlagType
	{
		Current       = 0x001,
		Range         = 0x002,
		Collate       = 0x004,
		Order         = 0x008,
		PageSet       = 0x010,
		CopyAll       = 0x0FF,
		PSSelect      = Range|Order|PageSet,
		NoAutoCollate = 0x100
	};

	KMUiManager(QObject *parent = 0);
	virtual ~KMUiManager();

	// print management
	virtual void setupPropertyPages(KMPropertyPage*);
	virtual void setupWizard(KMWizard*);
	virtual void setupConfigDialog(KMConfigDialog*);

	// print dialog
	void addPrintDialogPage(KPrintDialogPage *page)		{ m_printdialogpages.append(page); }
	int copyFlags(KPrinter *pr = 0, bool usePlugin = true);
	int dialogFlags();
	void setupPrintDialog(KPrintDialog*);
	virtual void setupPrintDialogPages(Q3PtrList<KPrintDialogPage>*);

	// printer property dialog
	void setupPropertyDialog(KPrinterPropertyDialog*);
	virtual void setupPrinterPropertyDialog(KPrinterPropertyDialog*);

	// page processing capabilities
	int systemPageCap();
	virtual int pluginPageCap();
	int pageCap();

	// job management
	virtual void setupJobViewer(Q3ListView*);

protected:
	int			m_printdialogflags;
	Q3PtrList<KPrintDialogPage>	m_printdialogpages;
};

#endif
