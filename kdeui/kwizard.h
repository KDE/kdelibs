/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#ifndef __KWIZARD_H
#define __KWIZARD_H

#define KWIZARD_VERSION_MAJOR 0
#define KWIZARD_VERSION_MINOR 20
#define KWIZARD_VERSION (KWIZARD_VERSION_MAJOR * 10) + KWIZARD_VERSION_MINOR

#include <qdialog.h>
#include <kapp.h>
#include <klocale.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qaccel.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpen.h>
#include <kdbtn.h>
#include <kseparator.h>

/**
* KDialog inherits QDialog. So far the only difference is that if the dialog is modeless
* and has a parent the default keybindings (escape = reject(), enter = accept() etc.) are
* disabled.
* @short KDialog
* @author Thomas Tanghus <tanghus@earthling.net>
* @version 0.1.1
*/
class KDialog : public QDialog
{
	Q_OBJECT

public:
/**
* Constructor. Takes the same arguments as QDialog.
*/
	KDialog(QWidget *parent = 0, const char *name = 0, bool modal = false, WFlags f = 0);
protected:

/**
* @internal
*/
	virtual void keyPressEvent(QKeyEvent*);
};

struct KWizProtected;

/**
* KWizardPage holds information about the pages in the wizard. Given as
* argument to @ref KWizard#addPage.
*/
struct KWizardPage
{
  QString title;
  QWidget *w;
  bool enabled;
  int id;
};

/**
* KWizard is a multi-purpose dialog. It is very useful for:
*
* - Configuration dialogs where the order in which the entries are filled is important.
*
* - Tutorials. Just create some HTML-widgets and feed them to a KWizard and your done (almost).
*
* - NoteBooks. KWizard is an integrated part of KNoteBook which provides a notebook
*   dialog similar to the ones in OS/2 Warp 4.
*
* - Propably many other things...
*
* The dialog contains:
*
* - A title at the top of the page.
*
* - A separator.
*
* - The userprovided page-widget.
*
* - A separator.
*
* - A label indicating the pagination.
*
* - A Previous and a Next button.
*
* - Optionally Ok, Cancel and Help buttons.
* 
* @short KWizard
* @author Thomas Tanghus <tanghus@earthling.net>
* @version 0.2
*/
class KWizard : public KDialog
{
friend class KNoteBook;
	Q_OBJECT

public:
/**
* Constructor
*/
	KWizard(QWidget *parent = 0, const char *name = 0, bool modal = false, WFlags f = 0);
/**
* Destructor
*/
	~KWizard();

/**
* Adds a page to the wizard.
* The pages are numbered from 0-n where 0 is the page first added and n is the
* page last added.
* @param p KWizardPage containing the page data.
*
* @return Returns the id of the new page.
*/
        int addPage(KWizardPage *p);

/**
* Changes the QWidget on the page with id "id" to "w".
*/
        void setPage(int id, QWidget *w);
/**
* Changes the title on the page with id "id" to "title".
*/
        void setPage(int id, QString title);
/**
* En/disable a specified page. If a page is disable its content will be grayd out
* and it will not receive keyboard input.
* @param page The page to en/disable.
* @param state If 'state' is false the page will be disabled, otherwise it will be enabled.
* @see KNoteBook#setPageEnabled
*/
        void setPageEnabled(int page, bool state);
/**
* Check the state of the page.
* @see #setPageEnabled
* @return Returns true if the page is enabled, otherwise false.
*/
        bool isPageEnabled(int page) { return pages->at(page)->enabled; };

/**
* Adds a Cancel button to the bottom of the dialog.
* The text will be a translated version of the string '&Cancel' thereby
* giving it the shortcut key 'c' if text is null.
* If any buttons are added a space will be created at the bottom of the dialog
* to fit the buttons. When clicked the button will emit the @ref cancelclicked signal.
* @param text A user supplied text to write to the button.
*/
        void setCancelButton(const QString &text = QString::null);

/**
* Adds a Default button to the bottom of the dialog.
* The text will be a translated version of the string '&Default' thereby 
* giving it the shortcut key 'd' if text is null.
* If any buttons are added a space will be created at the bottom of the dialog
* to fit the buttons. When clicked the button will emit the @ref defaultclicked signal.
* @param text A user supplied text to write to the button.
*/
        void setDefaultButton(const QString& text = QString::null);

/**
* Adds a Help button to the bottom of the dialog. This button will generate the
* signal @ref helpclicked where the int is the page to which the help was requested.
* if text is null, the text will be a translated
* version of the string '&Help' thereby giving it the shortcut key 'h'.
* @see #getHelpButton
* @param text A user supplied text to write to the button.
*/
        void setHelpButton(const QString& text = QString::null);

/**
* Adds an Ok button to the bottom of the dialog. This button will generate the
* signal @ref okclicked where the int is the page to which the help was requested.
*  The text will be a translated
* version of the string '&Ok' thereby giving it the shortcut key 'o' if text is null.
* @see #getOkButton
* @param text A user supplied text to write to the button.
*/
        void setOkButton(const QString& text = QString::null);
/**
* @see #setOkButton
* @return Returns the Ok buttonwidget or 0L if no button is added.
*/
        QButton * getOkButton();
/**
* @see #setCancelButton
* @return Returns the Cancel buttonwidget or 0L if no button is added.
*/
        QButton * getCancelButton();
/**
* @see #setDefaultButton
* @return Returns the Default buttonwidget or 0L if no button is added.
*/
        QButton * getDefaultButton();
/**
* @see #setHelpButton
* @return Returns the Help buttonwidget or 0L if no button is added.
*/
        QButton * getHelpButton();
/**
* getNextButton.
* @return Returns the Next buttonwidget.
*/
        QButton * getNextButton();
/**
* getPreviousButton
* @return Returns the Previous buttonwidget.
*/
        QButton * getPreviousButton();
/**
* @see #setEnableArrowButtons
* @return Returns the left arrowbutton.
*/
        KDirectionButton * getLeftArrow();
/**
* @see #setEnableArrowButtons
* @return Returns the right arrowbutton.
*/
        KDirectionButton * getRightArrow();
/**
* Let direction buttons reflect page.
* @see #directionsReflectsPage
* @param state If state is true the direction buttons (Previous and Next) will have the
* title of the corresponding page.
*/
        void setDirectionsReflectsPage(bool state);
/**
* @return Returns whether the direction buttons reflects the title of the corresponding page.
*
* @see #setDirectionsReflectsPage(bool state)
*/
        bool directionsReflectsPage();
/**
* En/Disable the popup menu.
* @see #enablePopupMenu
* @param state If state is true a menu containing the pages in the wizard
* will popup when the user RMB-clicks on the page-title.
*/
        void setEnablePopupMenu(bool state);
/**
* @see #setEnablePopupMenu
* @return Returns 'true' if the popupmenu is enabled.
*/
        bool enablePopupMenu();
/**
* getMenu
* @see #setEnablePopupMenu #enablePopupMenu
* @return Returns the handle of the popup menu.
*/
        QPopupMenu * getMenu();
/**
* En/Disable the arrow buttons at the rightmost of the title.
* @param state If state is true two arrows will appear to the right of the pagination
* allowing the user to change pages by clicking on them.
* @see #enableArrowButtons
*/
        void setEnableArrowButtons(bool state);
/**
* @see #setEnableArrowButtons
* @return Returns 'true' if the arrow buttons are enabled.
*/
        bool enableArrowButtons();
        QSize sizeHint();
        void adjustSize() { resize(sizeHint()); };
/**
* getTitle
* @return Returns the title of a specified page. Used by KNoteBook
* @param page The page to retrieve the title from.
*/

        QString getTitle(int page) { return pages->at(page)->title.data(); };
/**
* numPages
* @return Returns the number of pages in the wizard.
*/
        int numPages();

signals:
/**
* This signal is emitted when the user clicks on the Ok button.
*/
        void okclicked();
/**
* This signal is emitted when the user clicks on the Cancel button.
*/
        void cancelclicked();
/**
* This signal is emitted when the user clicks on the Default button.
* The int is the page which was showing when default settings was requested.
*/
        void defaultclicked(int);
/**
* This signal is emitted when the user clicks on the Help button.
* The int is the page which was showing when help was requested.
* @see #setHelpButton
* <BR>
* @see #getHelpButton
*/
        void helpclicked(int);
/**
* This signal is emitted when a page is selected. The int is the page number
*/
        void selected(int);

/**
* This signal is emitted when the dialog is closed. Mainly usable if the dialog
* is a top level widget.
*/
        void closed();

/**
* This signal is emitted when a popup menu is requested. This happens when the user
* RMB-clicks somewhere in the title (not the titlebar). This is used by KNoteBook.
*/
        void popup(QPoint);
/**
* This signal is used by KNoteBook.
*/
        void nomorepages(bool, bool);

public slots:
/*
* Goto to a specified page.
* @param page The page number to turn to.
*/
        void gotoPage(int page);

protected slots:
        void nextPage();
	void previousPage();
	void okClicked();
	void cancelClicked();
	void defaultClicked();
        void helpClicked();

protected:
        bool eventFilter( QObject *, QEvent * );
        void closeEvent(QCloseEvent *);

/**
* @internal
*/
        QSize pageSize();
/**
* @internal
*/
        void setSizes();
/**
* @internal
*/
        void resizeEvent(QResizeEvent *);
/**
* @internal
*/
        void paintEvent(QPaintEvent *);

        QList<KWizardPage> *pages;
        //QList<QWidget> pages;
        //QStrList titles;
        KWizProtected *pwiz;
};

#endif // __KWIZARD_H


