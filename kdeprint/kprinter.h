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

#ifndef KPRINTER_H
#define KPRINTER_H

#include <qpaintdevice.h>
#include <qprinter.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

//**************************************************************************************
// KPrinterWrapper class
//**************************************************************************************

class KPrinterWrapper : public QPrinter
{
friend class KPrinter;
public:
	KPrinterWrapper(KPrinter*);
	~KPrinterWrapper();
protected:
	virtual bool cmd(int, QPainter*, QPDevCmdParam*);
	virtual int metric(int) const;
	int qprinterMetric(int) const;
private:
	KPrinter	*m_printer;
};

//**************************************************************************************

class KPrinterImpl;
class KPrintDialogPage;

class KPrinter : public QPaintDevice
{
friend class KPrinterWrapper;
friend class KPrinterImpl;
public:
	// Print global settings (set via static functions)
	enum StandardPageType { CopiesPage = 0x01, Custom = 0x10 };
	enum PageSelectionType { ApplicationSide = 0x01, SystemSide = 0x02 };
	enum ApplicationType { Dialog = 0x00, StandAlone = 0x01 };

	// QPrinter extension
	enum PageSetType { AllPages = 0x00, OddPages = 0x01, EvenPages = 0x02 };
	enum CollateType { Collate = 0x00, Uncollate = 0x01 };

	// from QPrinter class
	enum ColorMode { GrayScale = QPrinter::GrayScale, Color = QPrinter::Color };
	enum Orientation { Portrait = QPrinter::Portrait, Landscape = QPrinter::Landscape };
	enum PageOrder { FirstPageFirst = QPrinter::FirstPageFirst, LastPageFirst = QPrinter::LastPageFirst };
	enum PageSize
	{
		A4 = QPrinter::A4,
		B5 = QPrinter::B5,
		Letter = QPrinter::Letter,
		Legal = QPrinter::Legal,
		Executive = QPrinter::Executive,
		A0 = QPrinter::A0,
		A1 = QPrinter::A1,
		A2 = QPrinter::A2,
		A3 = QPrinter::A3,
		A5 = QPrinter::A5,
		A6 = QPrinter::A6,
		A7 = QPrinter::A7,
		A8 = QPrinter::A8,
		A9 = QPrinter::A9,
		B0 = QPrinter::B0,
		B1 = QPrinter::B1,
		B10 = QPrinter::B10,
		B2 = QPrinter::B2,
		B3 = QPrinter::B3,
		B4 = QPrinter::B4,
		B6 = QPrinter::B6,
		B7 = QPrinter::B7,
		B8 = QPrinter::B8,
		B9 = QPrinter::B9,
		C5E = QPrinter::C5E,
		Comm10E = QPrinter::Comm10E,
		DLE = QPrinter::DLE,
		Folio = QPrinter::Folio,
		Ledger = QPrinter::Ledger,
		Tabloid = QPrinter::Tabloid,
		NPageSize = QPrinter::NPageSize
	};

	// constructors / destructor
	KPrinter();
	~KPrinter();

	// QPrinter interface (+ some extensions)
	bool newPage();
	bool abort();
	bool aborted() const;
	bool outputToFile() const;
	void setOutputToFile(bool);
	QString outputFileName() const;
	void setOutputFileName(const QString&);
	QString docName() const;
	void setDocName(const QString&);
	QString creator() const;
	void setCreator(const QString&);
	bool fullPage() const;
	void setFullPage(bool);
	ColorMode colorMode() const;
	void setColorMode(ColorMode);
	int numCopies() const;
	void setNumCopies(int n);
	Orientation orientation() const;
	void setOrientation(Orientation);
	PageOrder pageOrder() const;
	void setPageOrder(PageOrder);
	CollateType collate() const;
	void setCollate(CollateType);
	int minPage() const;
	int maxPage() const;
	void setMinMax(int, int);
	int fromPage() const;
	int toPage() const;
	void setFromTo(int, int);
	PageSize pageSize() const;
	void setPageSize(PageSize);
	QString printerName() const;
	void setPrinterName(const QString&);
	QString printProgram() const;
	void setPrintProgram(const QString&);
	QString printerSelectionOption() const;
	void setPrinterSelectionOption(const QString&);
	int currentPage() const;
	void setCurrentPage(int p = 0);
	PageSetType pageSet() const;
	bool setup(QWidget *parent = 0);
	QSize margins() const;
	void setMargins(QSize m);
	QSize realPageSize() const;
	void setRealPageSize(QSize p);
	QValueList<int> pageList() const;

	KPrinterImpl* implementation() const;
	bool printFiles(const QStringList& files, bool removeafter = false);

	static void addDialogPage(KPrintDialogPage*);
	static void setPageSelection(PageSelectionType);
	static PageSelectionType pageSelection();
	static void setApplicationType(ApplicationType);
	static ApplicationType applicationType();
	static void addStandardPage(int p);
	static void removeStandardPage(int p);

	const QString& option(const QString& key) const;
	void setOption(const QString& key, const QString& value);
	const QMap<QString,QString>& options() const;
	void setOptions(const QMap<QString,QString>& opts);
	void initOptions(const QMap<QString,QString>& opts);

	QString searchName() const;
	void setSearchName(const QString& n);
	QString errorMessage() const;
	void setErrorMessage(const QString& msg);

protected:
	virtual bool cmd(int, QPainter*, QPDevCmdParam*);
	virtual int metric(int) const;
	void translateQtOptions();
	void loadSettings();
	void saveSettings();
	void preparePrinting();
	void finishPrinting();

protected:
	KPrinterImpl		*m_impl;
	KPrinterWrapper		*m_wrapper;
	QMap<QString,QString>	m_options;
	QString			m_tmpbuffer;
	QString			m_printername;
	QString			m_searchname;
	QSize			m_margins;
	QSize			m_pagesize;
	QString			m_errormsg;
	bool			m_ready;
};

//**************************************************************************************

// Utility functions

KPrinter::PageSize pageNameToPageSize(const QString& name);
const char* pageSizeToPageName(KPrinter::PageSize s);
QSize rangeToSize(const QString& s);

#endif
