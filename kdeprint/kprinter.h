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

/**
 * A @ref QPrinter wrapper to access protected functions from @ref KPrinter. For internal use only.
 * Do not use.
 *
 * @short A @ref QPrinter wrapper to access protected functions from @ref KPrinter
 */
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

/**
 * This class is the main interface to access the KDE print framework. It allows KDE
 * applications to easily access the print system, through an interface that is compatible
 * with @ref QPrinter. So that the porting of an existing application should be as easy as
 * replacing any occurence of @ref QPrinter by KPrinter. However applications that explicitely
 * used the @ref QPrintDialog class must be changed to the standard KPrinter way of accessing
 * the print dialog.
 *
 * Basically, a KDE application will use the KPrinter object as a paint device, and will draw
 * on it using @ref QPainter. In a standard application, the use of the KPrinter class will
 * look like this:
 * <pre>
 * #include <kprinter.h>
 * #include <qpainter.h>
 *
 * void MyClass::doPrint()
 * {
 *   KPrinter printer;
 *
 *   // do some printer initialization
 *   printer.setFullPage( true );
 *
 *   // initialize the printer using the print dialog
 *   if ( printer.setup( this ) )
 *   {
 *     // create a painter to paint on the printer object
 *     QPainter painter;
 *
 *     // start painting
 *     painter.begin( &printer );
 *     <do something>
 *     // stop painting, this will automatically send the print data to the printer
 *     painter.end();
 *   }
 * }
 * </pre>
 *
 * The KPrinter interface also allows some more advanced possibilities, like the customization
 * of the print dialog to integrate application specific print options. This is done by subclassing
 * @ref KPrintDialogPage and adding the page using @ref addDialogPage().
 *
 * @short The central interface class to access the KDE print system.
 * @author Michael Goffioul
 * @see QPrinter, KPrintDialogPage
 */
class KPrinter : public QPaintDevice
{
friend class KPrinterWrapper;
friend class KPrinterImpl;
public:
	// Print global settings (set via static functions)
	/**
	 * Defines the standard pages available for the print dialog:
	 *  @li @p CopiesPage: page and copies selection (included by default)
	 *  @li @p Custom: unused
	 *
	 * @see addStandardPage(), removeStandardPage()
	 */
	enum StandardPageType { CopiesPage = 0x01, Custom = 0x10 };
	/**
	 * Defines wether the application can perform page selection itself or not.
	 * Some print systems (like CUPS) can do page selection, in this case the
	 * page selection will be enabled in the print dialog, otherwise it will be
	 * disabled. However some applications (word processor) can do page selection
	 * themselves, then it will be enabled in the print dialog, whatever the
	 * print system used. But such an application has to tell kdeprint about its
	 * feature:
	 *  @li @p ApplicationSide: the application will do page selection
	 *  @li @p SystemSide: page selection will be left to the print system, if available (default)
	 *
	 * @see setPageSelection(), pageSelection()
	 */
	enum PageSelectionType { ApplicationSide = 0x01, SystemSide = 0x02 };
	/**
	 * Defines the type of the application, this affects the GUI of the print dialog:
	 *  @li @p Dialog: print dialog used in an application (default)
	 *  @li @p StandAlone: print dialog used as a standalone widget
	 *
	 * @internal
	 * @see setApplicationType(), applicationType()
	 */
	enum ApplicationType { Dialog = 0x00, StandAlone = 0x01 };

	// QPrinter extension
	/**
	 * Defines the page set to print:
	 *  @li @p AllPages: all pages
	 *  @li @p OddPages: only odd pages
	 *  @li @p EvenPages: only even pages
	 *
	 * @see pageSet()
	 */
	enum PageSetType { AllPages = 0x00, OddPages = 0x01, EvenPages = 0x02 };
	/**
	 * Defines the collate property of the printer (if supported by the print system):
	 *  @li @p Collate: copies collated (1-2-3-..., 1-2-3-...)
	 *  @li @p Uncollate: copies uncollated (1-1-..., 2-2-..., 3-3-...)
	 *
	 * @see setCollate(), collate()
	 */
	enum CollateType { Collate = 0x00, Uncollate = 0x01 };

	// from QPrinter class
	/**
	 * Defines the color mode of the printer
	 * @see QPrinter::ColorMode
	 */
	enum ColorMode { GrayScale = QPrinter::GrayScale, Color = QPrinter::Color };
	/**
	 * Defines the orientation of the paper
	 * @see QPrinter::Orientation
	 */
	enum Orientation { Portrait = QPrinter::Portrait, Landscape = QPrinter::Landscape };
	/**
	 * Defines the page order of the print job
	 * @see QPrinter::PageOrder
	 */
	enum PageOrder { FirstPageFirst = QPrinter::FirstPageFirst, LastPageFirst = QPrinter::LastPageFirst };
	/**
	 * Defines the paper size to use
	 * @see QPrinter::PageSize
	 */
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
	/**
	 * Contructor. This also restores the state from a previous KPrinter object created within
	 * the same application run.
	 */
	KPrinter();
	/**
	 * Destructor. This also saves the current KPrinter state for future printing
	 */
	~KPrinter();

	// QPrinter interface (+ some extensions)
	/**
	 * See @ref QPrinter::newPage()
	 */
	bool newPage();
	/**
	 * See @ref QPrinter::abort()
	 */
	bool abort();
	/**
	 * See @ref QPrinter::aborted()
	 */
	bool aborted() const;
	/**
	 * See @ref QPrinter::outputToFile()
	 */
	bool outputToFile() const;
	/**
	 * See @ref QPrinter::setOutputToFile()
	 */
	void setOutputToFile(bool);
	/**
	 * See @ref QPrinter::outputFileName()
	 */
	QString outputFileName() const;
	/**
	 * See @ref QPrinter::setOutputFileName()
	 */
	void setOutputFileName(const QString&);
	/**
	 * See @ref QPrinter::docName()
	 */
	QString docName() const;
	/**
	 * See @ref QPrinter::setDocName()
	 */
	void setDocName(const QString&);
	/**
	 * See @ref QPrinter::creator()
	 */
	QString creator() const;
	/**
	 * See @ref QPrinter::setCreator()
	 */
	void setCreator(const QString&);
	/**
	 * See @ref QPrinter::fullPage()
	 */
	bool fullPage() const;
	/**
	 * See @ref QPrinter::setFullPage()
	 */
	void setFullPage(bool);
	/**
	 * See @ref QPrinter::colorMode()
	 */
	ColorMode colorMode() const;
	/**
	 * See @ref QPrinter::setColorMode()
	 */
	void setColorMode(ColorMode);
	/**
	 * See @ref QPrinter::numCopies()
	 */
	int numCopies() const;
	/**
	 * See @ref QPrinter::setNumCopies()
	 */
	void setNumCopies(int n);
	/**
	 * See @ref QPrinter::orientation()
	 */
	Orientation orientation() const;
	/**
	 * See @ref QPrinter::setOrientation()
	 */
	void setOrientation(Orientation);
	/**
	 * See @ref QPrinter::pageOrder()
	 */
	PageOrder pageOrder() const;
	/**
	 * See @ref QPrinter::setPageOrder()
	 */
	void setPageOrder(PageOrder);
	/**
	 * Get the collate status of the current KPrinter
	 * @returns the collate status
	 */
	CollateType collate() const;
	/**
	 * Set the collate status for the current KPrinter
	 * @param _type the collate type
	 */
	void setCollate(CollateType);
	/**
	 * See @ref QPrinter::minPage()
	 */
	int minPage() const;
	/**
	 * See @ref QPrinter::maxPage()
	 */
	int maxPage() const;
	/**
	 * See @ref QPrinter::setMinMax()
	 */
	void setMinMax(int, int);
	/**
	 * Get the first page to be printed. This function is deprecated and applications
	 * should use @ref pageList() instead, which takes into account all options: collate,
	 * page order, ranges, page set, ...
	 *
	 * @see pageList()
	 */
	int fromPage() const;
	/**
	 * Get the last page to be printed. This function is deprecated and applications
	 * should use @ref pageList() instead, which takes into account all options: collate,
	 * page order, ranges, page set, ...
	 *
	 * @see pageList()
	 */
	int toPage() const;
	/**
	 * Set the first and last page to be printed. See @ref QPrinter::setFromTo()
	 */
	void setFromTo(int, int);
	/**
	 * See @ref QPrinter::pageSize()
	 */
	PageSize pageSize() const;
	/**
	 * See @ref QPrinter::setPageSize()
	 */
	void setPageSize(PageSize);
	/**
	 * See @ref QPrinter::printerName()
	 */
	QString printerName() const;
	/**
	 * See @ref QPrinter::setPrinterName()
	 */
	void setPrinterName(const QString&);
	/**
	 * See @ref QPrinter::printProgram(). Unused
	 */
	QString printProgram() const;
	/**
	 * See @ref QPrinter::SetPrintProgram(). Unused
	 */
	void setPrintProgram(const QString&);
	/**
	 * See @ref QPrinter::printerSelectionOption(). Unused.
	 */
	QString printerSelectionOption() const;
	/**
	 * See @ref QPrinter::setPrinterSelectionOption(). Unused
	 */
	void setPrinterSelectionOption(const QString&);
	/**
	 * Get the current page number.
	 * @see setCurrentPage()
	 */
	int currentPage() const;
	/**
	 * Set the current page number. This page number will be used if the user
	 * selected "current page" in the print dialog. This option will only be
	 * enabled if the application does page selection itself and the application
	 * has called @ref setCurrentPage() before opening the print dialog:
	 * <pre>
	 * MyClass::MyClass()
	 * {
	 *   // declares my application able to do page selection
	 *   KPrinter::setPageSelection( KPrinter::ApplicationSide );
	 * }
	 *
	 * void MyClass::doPrint()
	 * {
	 *   KPrinter printer;
	 *
	 *   printer.setCurrentPage( mycurrentpage );
	 *   if ( printer.setup( this ) )
	 *   {
	 *     QValueList<int> pages = printer.pageList();
	 *     // print the pages
	 *     ...
	 *   }
	 * }
	 * </pre>
	 * @param p the current page number (starting from 1)
	 */
	void setCurrentPage(int p = 0);
	/**
	 * Get the page set of the current KPrinter object.
	 * @returns the page set
	 */
	PageSetType pageSet() const;
	/**
	 * Setup the KPrinter object using the print dialog, returns true if the user clicked OK.
	 * @param parent the parent widget to use for the print dialog
	 * @returns boolean value corresponding to the button clicked by the user
	 */
	bool setup(QWidget *parent = 0);
	/**
	 * See @ref QPrinter::margins()
	 */
	QSize margins() const;
	/**
	 * Not used yet.
	 */
	void setMargins(QSize m);
	/**
	 * Get the real page size in points. Some print system (like CUPS) allows to customize the
	 * page size through printer description files (PPD). The size returned corresponds to this
	 * size.
	 * @returns the page size
	 * @see setRealPageSize
	 */
	QSize realPageSize() const;
	/**
	 * Set the real page size. For internal use only.
	 * @param p the page size
	 * @see realPageSize()
	 */
	void setRealPageSize(QSize p);
	/**
	 * Get the page list to be printed, correpsonding to the options selected by the user. This
	 * takes into account collate, page order, page set, ranges, ... This is useful when the
	 * application does page selection itself.
	 * @returns the page list to be printed
	 * @see setCurrentPage()
	 */
	QValueList<int> pageList() const;

	/**
	 * For internal use only.
	 */
	KPrinterImpl* implementation() const;
	/**
	 * Print the files given in argument. This will first filter the files accorsing to the filtering
	 * options selected by the user in the print dialog, then send the filtered files to the printer
	 * with the print options selected. This function is called automatically when calling
	 * @ref QPainter::end() for a painter object constructed on KPrinter. In normal use, you don't need
	 * this use this fucntion explicitely.
	 */
	bool printFiles(const QStringList& files, bool removeafter = false);

	/**
	 * Add a customized page to the print dialog. The pages will appear in a tab widget in the
	 * bottom half of the dialog, along with the standard "Copies" page. The page must be created
	 * and added each time you want open a print dialog with @ref setup(). If you correctly
	 * reimplemented @ref KPrintDialogPage::setOptions(), the settings will be restored from call
	 * to call, you don't have to worry about state saving/restoration.
	 * @param _page the page to add
	 * @see KPrintDialogPage::setOptions()
	 */
	static void addDialogPage(KPrintDialogPage*);
	/**
	 * Set the page selection mode of the application. If needed, call this method somewhere at
	 * the beginning of your code. The default value is @p SystemSide.
	 * @param _mode the mode for the application
	 * @see pageSelection()
	 */
	static void setPageSelection(PageSelectionType);
	/**
	 * Get the page selection mode of the current application.
	 * @returns the page selection mode
	 * @see setPageSelection()
	 */
	static PageSelectionType pageSelection();
	/**
	 * Set the application type concerning the print dialog. You probably don't want to use it.
	 * For internal use only.
	 * @param _type the type for this application
	 * @see applicationType()
	 */
	static void setApplicationType(ApplicationType);
	/**
	 * Get the application type concerning the print dialog. For internal uase only.
	 * @returns the type for the current application
	 * @see setApplicationType()
	 */
	static ApplicationType applicationType();
	/**
	 * Add a standard page to the print dialog. This is not useful yet as there's only one
	 * standard page defines @p CopiesPage.
	 * @param p the page identifier
	 * @see StandardPageType
	 */
	static void addStandardPage(int p);
	/**
	 * Remove a standard page from the print dialog. If your application doesn't want a
	 * standard page in the dialog, simply call this method with the correct identifier.
	 * By default, the print dialog includes the @p CopiesPage page.
	 * @param p the page identifier
	 * @see StandardPageType
	 */
	static void removeStandardPage(int p);

	/**
	 * The KPrinter object stores all its settings in an internal @ref QMap structure on @ref
	 * QString. This allows to store any property. This method allows an application to access
	 * any print option from the KPrinter object, using the option name. For example if your
	 * application add a customized page to the print dialog, this page will saves its settings
	 * into the KPrinter object using this QMap<QString,QString> structure. After showing the
	 * print dialog, your application can then access these options using this method. The
	 * custom option name should follow the form "kde-appname-optionname".
	 * <pre>
	 * void MyClass::doPrint()
	 * {
	 *   KPrinter prt;
	 *
	 *   // add my custom page
	 *   prt.addDialogPage( new MyDialogPage() );
	 *
	 *   // open print dialog
	 *   if ( prt.setup( this ) )
	 *   {
	 *      QString fntname = prt.option( "kde-myapp-fontname" );
	 *      ...
	 *      do_something;
	 *      ...
	 *   }
	 * }
	 * </pre>
	 * @param key the option name (key)
	 * @returns the option value correponding to the key, or QString::null
	 * @see KPrintDialogPage, setOption, options(), setOptions()
	 */
	const QString& option(const QString& key) const;
	/**
	 * Add or modify an option in the KPrinter object. You probably don't need to use this function
	 * explicitely. This will be done implicitely for example when reimplementing @ref
	 * KPrintDialogPage::getOptions().
	 * @param key the option name
	 * @param value the option value
	 * @see option(), KPrintDialogPage
	 */
	void setOption(const QString& key, const QString& value);
	/**
	 * Get the complete set of print options from the KPrinter object. For internale use.
	 * @returns the option set as a QMap object
	 */
	const QMap<QString,QString>& options() const;
	/**
	 * Set the option set in one operation. This method has some side effects like merging
	 * the internal map with the one given in argument, but only for option in the form
	 * "kde-...". For internal use only.
	 * @param opts the option set to be merged in the KPrinter object
	 */
	void setOptions(const QMap<QString,QString>& opts);
	/**
	 * For internal use only. Does a similar job as @ref setOption(), except that all possible
	 * internal printers are initialized with the option if it has the form "kde-...".
	 * @param opts the option set
	 * @see setOptions()
	 */
	void initOptions(const QMap<QString,QString>& opts);

	/**
	 * Get the search name of the printer selected by the user. Each printer is identified by
	 * a unique name. This method is mainly used for state restoration. For internal use.
	 * @returns the unique printer search name
	 * @see setSearchName
	 */
	QString searchName() const;
	/**
	 * Set the search name of the KPrinter object. For internal use.
	 * @param the unique printer search name
	 * @see searchName()
	 */
	void setSearchName(const QString& n);
	/**
	 * Retrieve the last error messgae issued by the print system. Unimplemented yet.
	 * @returns the last error message
	 */
	QString errorMessage() const;
	/**
	 * Set the last error message. For internal use.
	 * @param msg the error message
	 */
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
