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

#ifndef KPRINTDIALOGPAGE_H
#define KPRINTDIALOGPAGE_H

#include <qwidget.h>
#include <qmap.h>

class KMPrinter;
class DrMain;

/**
 * This class is intended to be used as base class for customized print dialog page. One of
 * the feature of the KDE print framework is to allow to customize the print dialog to
 * add some application specific print options. This is done by subclassing KPrintDialogPage
 * and reimplementing the 3 virtual functions @ref getOptions, @ref setOptions and @ref
 * isValid(). The print options will be stored in the KPrinter object, and will be
 * accessible via @ref KPrinter::option(). The option name should follow the form
 * "kde-appname-optionname" for internal reasons.
 *
 * <pre>
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
 * </pre>
 *
 * @short Base class for customized print dialog pages.
 * @see KPrinter
 */
class KPrintDialogPage : public QWidget
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
	virtual bool isValid(QString&);
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
};

#endif
