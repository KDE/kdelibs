/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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

#ifndef KPRINTDIALOGPAGE_H
#define KPRINTDIALOGPAGE_H

#include <qwidget.h>
#include <qmap.h>

#include <kdelibs_export.h>

class KMPrinter;
class DrMain;

/**
 * This class is intended to be used as base class for customized print dialog page. One of
 * the feature of the KDE print framework is to allow to customize the print dialog to
 * add some application specific print options. This is done by subclassing KPrintDialogPage
 * and reimplementing the 3 virtual functions getOptions, setOptions and
 * isValid(). The print options will be stored in the KPrinter object, and will be
 * accessible via KPrinter::option(). The option name should follow the form
 * "kde-appname-optionname" for internal reasons.
 *
 * \code
 * #include <kdeprint/kprintdialogpage.h>
 *
 * class MyDialogPage : public KPrintDialogPage
 * {
 * public:
 *   MyDialogPage( QWidget *parent = 0, const char *name = 0 );
 *
 *   //reimplement virtual functions
 *   void getOptions( QMap<QString,QString>& opts, bool incldef = false );
 *   void setOptions( const QMap<QString,QString>& opts );
 *   bool isValid( QString& msg );
 *
 * private:
 *   QComboBox *m_fontcombo;
 * }
 *
 * MyDialogPage::MyDialogPage( QWidget *parent, const char *name )
 * : KPrintDialogPage( parent, name )
 * {
 *   setTitle( i18n( "My Page" ) );
 * }
 *
 * void MyDialogPage::getOptions( QMap<QString,QString>& opts, bool incldef )
 * {
 *   if ( incldef || m_fontcombo->currentText() != mydefaultvalue )
 *     opts[ "kde-myapp-fontname" ] = m_fontcombo->currentText();
 * }
 *
 * void MyDialogPage::setOptions( const QMap<QString,QString>& opts )
 * {
 *   QString fntname = opts[ "kde-myapp-fontname" ];
 *   m_fontcombo->setEditText( fntname );
 * }
 *
 * bool MyDialogPage::isValid( QString& msg)
 * {
 *   if ( m_fontcombo->currentText().isEmpty() )
 *   {
 *     msg = i18n( "Font name cannot be empty." );
 *     return false;
 *   }
 *   return true;
 * }
 * \endcode
 *
 * @short Base class for customized print dialog pages.
 * @see KPrinter
 */
class KDEPRINT_EXPORT KPrintDialogPage : public QWidget
{
	Q_OBJECT
public:
	/**
	 * Standard constructor.
	 */
	KPrintDialogPage(QWidget *parent = 0, const char *name = 0);
	/**
	 * Modified constructor. For internal use only.
	 */
	KPrintDialogPage(KMPrinter *pr, DrMain *dr = 0, QWidget *parent = 0, const char *name = 0);
	/**
	 * Destructor
	 */
	virtual ~KPrintDialogPage();

	/**
	 * This function is called to fill the structure @p opts with the selected options from this dialog
	 * page. If @p incldef is true, include also options with default values, otherwise discard them.
	 * Reimplement this function in subclasses.
	 * @param opts the option set to fill
	 * @param incldef if true, include also options with default values
	 * @see setOptions()
	 */
	virtual void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	/**
	 * This function is called to update the current page with the options contained in @p opts.
	 * Reimplement it in subclasses.
	 * @param opts the structure containing the options to update the page
	 */
	virtual void setOptions(const QMap<QString,QString>& opts);
	/**
	 * Returns true if options selected in the page are valid (no conflict), false otherwise.
	 * When returning false, @p msg should contain an error message explaining what is wrong
	 * in the selected options.
	 * @param msg should contain an error message when returning false
	 * @returns valid status
	 */
	virtual bool isValid(QString& msg);
	/**
	 * Get the ID of the page. Not used yet.
	 * @returns the page ID
	 * @see setId()
	 */
	int id() const 				{ return m_ID; }
	/**
	 * Set the ID of the page. Not used yet.
	 * @param ID the ID number
	 * @see id()
	 */
	void setId(int ID)			{ m_ID = ID; }
	/**
	 * Get the page title.
	 * @returns the page title
	 * @see setTitle()
	 */
	QString	title() const 			{ return m_title; }
	/**
	 * Set the page title. This title will be used as tab name for this page in the print
	 * dialog.
	 * @param txt the page title
	 * @see title()
	 */
	void setTitle(const QString& txt)	{ m_title = txt; }
	/**
	 * Tell wether or not the page should be disable if a non real printer (special
	 * printer) is selected in the print dialog. Returns false by default. Application
	 * specific pages usually corresponds to printer-independent options, so the
	 * page should be kept enabled whatever the selected printer. The default value
	 * is then correct and your application doesn't to change anything.
	 * @returns true if the page should be disabled for non real printers
	 * @see setOnlyRealPrinters()
	 */
	bool onlyRealPrinters() const	{ return m_onlyreal; }
	/**
	 * Change the page state when a non real printer is selected in the print dialog.
	 * Usually, the default value (false) is OK in most cases and you don't need to
	 * call this function explicitly.
	 * @param on if true, then the page will be disabled if a non real printer is selected
	 * @see onlyRealPrinters()
	 */
	void setOnlyRealPrinters(bool on = true) { m_onlyreal = on; }
	/**
	 * For internal use only.
	 */
	DrMain* driver() 			{ return m_driver; }
	/**
	 * For internal use only
	 */
	KMPrinter* printer()			{ return m_printer; }

protected:
	KMPrinter	*m_printer;
	DrMain		*m_driver;
	int 		m_ID;
	QString		m_title;
	bool		m_onlyreal;
};

#endif
