/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#ifndef __KNOTEBOOK_H
#define __KNOTEBOOK_H

#define KNOTEBOOK_VERSION_MAJOR 0
#define KNOTEBOOK_VERSION_MINOR 20
#define KNOTEBOOK_VERSION (KNOTEBOOK_VERSION_MAJOR * 10) + KNOTEBOOK_VERSION_MINOR

#ifndef KDE_KDIRECTIONBUTTON
#define KDE_KDIRECTIONBUTTON
#endif
#ifndef KDE_KWIZARD
#define KDE_KWIZARD
#endif
#ifndef KDE_KTABBAR
#define KDE_KTABBAR
#endif

#include <ktabbar.h>
#include <kwizard.h>

struct KNoteBookProtected;

/**
* KNoteBook is a tabbed dialog where each tab can have multiple pages.
* The widget has the advantages of both KTabCtl and QTabDialog plus some more.
*
* Features:
*
* - Multiple pages per tab with previous/next button and optional arrow buttons in the title..
*
* - An optional popup menu which takes you to a specific page in a specific section (tab).
*
* - Easy additions of Ok, Cancel, Default and Help buttons.
*
* - Automatic resizing/repainting/repositioning of buttons. Not like QTabDialog where
* there is always an Ok button.
*
* - Scrollable tab bar.
*
* - Can be used as both modal/non-modal dialog and a child widget.
*
* @short KNoteBook
* @author Thomas Tanghus <tanghus@earthling.net>
* @version 0.3
*/
class KNoteBook : public KDialog
{
        Q_OBJECT
public:
/**
* Constructs a KNoteBook.
*
* A modal notebook can be created like this:
* <pre>
* KNoteBook *nb = new KNoteBook(this, "notebook", true);
* connect(nb, SIGNAL(okclicked()), SLOT(nb->hide()));
* connect(nb, SIGNAL(okclicked()), SLOT(readNewInput()));
* </pre>
*
* A very simple program where the main window is a KNoteBook with one tab
* and one page inserted could look like this:
* <pre>
* int main(int argc, char **argv)
* {
*   KApplication a(argc,argv,"knotebooktest");  // create an application object
*   KNoteBook *nb = new KNoteBook(); // create the notebook
*   nb->setCancelButton();           // add a Cancel button and connect it to the quit() slot
*   QObject::connect( nb, SIGNAL(cancelclicked()), &a, SLOT(quit()) );
* 
*   QTab *tab = new QTab;            // create a QTab to hold the tab data
*   tab->label = "A tab";
*   tab->enabled = true;
*   nb->addTab( tab );
*   QLabel *l = new QLabel(nb);
*   l->setAlignment(AlignCenter);
*   l->setText("This is a QLabel as a page");
*   l->setMinimumSize(400, 300);
*   KWizardPage *p = new KWizardPage;
*   p->w = l;
*   p->title.setNum("A page title");
*   p->enabled = true;
*   nb->addPage( p );
* 
*   a.setMainWidget(nb);
*   nb->adjustSize();
*   nb->show();
*   return a.exec();
* }
</pre>
* @see QDialog::QDialog
*
* @param parent The parent of the notebook.
*
* @param name The internal name.
*
* @param modal If modal is true the notebook wil become a modal dialog.
*
* @param f Window flags.
*/
        KNoteBook(QWidget *parent = 0, const char *name = 0, bool modal = false, WFlags f = 0);
/**
* Destructor
*/
        ~KNoteBook();
/**
* Adds a new tab to the notebook and creates a new @ref KWizard to hold the pages.
* The tabs are numbered from 0 to n where 0 is the tab first added and n is the
* tab last added. if you add 4 tabs the last tab will be number 3. 
* Subsequent calls to @ref addPage will add pages to this tab  until a new call to addTab().
*
* @param tab The QTab containing the tab data.
*
* @param p The optional KWizardPage containing the page data.If 'p' is not 0 it
* will be added as a new page.
* This is equivalent to @ref addPage (p);
* @return Returns the id of the new tab.
*/
        int addTab(QTab *tab, KWizardPage *p = 0L);
/**
* Adds a new page to the last added tab.
* The pages are numbered from 0 to n where 1 is the page first added and n is the
* page last added. Subsequent calls to @ref addPage will add pages to the last added
* tab until a new call to addTab().
*
* @param p The KWizardPage containing the page data..
*
* @return Returns the id of the new page. The id is relative to the KWizard, NOT
* the KNoteBook.
*/
        int addPage(KWizardPage *p);
/**
* Turn to another tab. This method calls @ref showSection to show the corresponding KWizard.
*
* @param tab The tab to turn to
*/
        void gotoTab(int tab);

        QSize sizeHint() { return minimumSize(); };
        void adjustSize() { resize(sizeHint()); };

/**
* Adds a Cancel button to the bottom of the dialog. The text will be a translated
* version of the string '&Cancel' thereby giving it the shortcut key 'c'.
* If any buttons are added a space will be created at the bottom of the dialog
* to fit the buttons. When clicked the button will emit the @ref cancelclicked signal.
* @see KLocale#translate
*/
        void setCancelButton();
/**
* Adds a Cancel button to the bottom of the dialog.
* @param text A user supplied text to write to the button.
*/
        void setCancelButton(const char *text);
/**
* Adds a Default button to the bottom of the dialog. The text will be a translated
* version of the string '&Default' thereby giving it the shortcut key 'd'.
* If any buttons are added a space will be created at the bottom of the dialog
* to fit the buttons. When clicked the button will emit the @ref defaultclicked signal.
* @see KLocal::translate
*/
        void setDefaultButton();
/**
* Adds a Default button to the bottom of the dialog.
* @param text A user supplied text to write to the button.
*/
        void setDefaultButton(const char *text);
/**
* Adds a Help button to the bottom right of the dialog. The text will be a translated
* version of the string '&Help' thereby giving it the shortcut key 'h'.
* If any buttons are added a space will be created at the bottom of the dialog
* to fit the buttons. When clicked the button will emit the @ref helpclicked signal.
*/
        void setHelpButton();
/**
* Adds a Help button to the bottom of the dialog. This button will generate the
* signal @ref helpclicked where the int is the page to which the help was requested.
* @param text A user supplied text to write to the button.
*/
        void setHelpButton(const char *);
/**
* Adds an Ok button to the bottom right of the dialog. The text will be a translated
* version of the string '&Ok' thereby giving it the shortcut key 'o'.
* If any buttons are added a space will be created at the bottom of the dialog
* to fit the buttons. When clicked the button will emit the @ref okclicked signal.
*/
        void setOkButton();
/**
* Adds an Ok button to the bottom of the dialog. This button will generate the
* signal @ref okclicked where the int is the page to which the help was requested.
* @param text A user supplied text to write to the button.
*/
        void setOkButton(const char *);
/**
* Get Ok button.
* @return Returns the Ok buttonwidget or 0L if no button is added.
*/
        QButton *getOkButton();
/**
* Get Cancel button.
* @return Returns the Cancel buttonwidget or 0L if no button is added.
*/
        QButton *getCancelButton();
/**
* Get Default button.
* @return Returns the Default buttonwidget or 0L if no button is added.
*/
        QButton *getDefaultButton();
/**
* Get Help button.
* @return Returns the Help buttonwidget or 0L if no button is added.
*/
        QButton *getHelpButton();
/**
* Let direction buttons reflect page.
* @param state If state is true the direction buttons (Previous and Next) will have the
* title of the corresponding page.
* @see #directionsReflectsPage
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
* @param state If state is true a menu containing the pages in the wizard
* will popup when the user RMB-clicks on the page-title.
*/
        void setEnablePopupMenu(bool state);
/**
* Returns whether the menu is enabled or not.
* @return 'true' if the menu is enabled, otherwise 'false'.
*/
        bool enablePopupMenu();
/**
* Get the popup menu.
* @return Returns the handle of the popup menu.
*/
        QPopupMenu *getMenu();
/**
* En/Disable the arrowbuttons.
* @param state If state is true two arrows will appear to the right of the title.
* @see #enableArrowButtons
* @see KWizard#setEnableArrowButtons.
*/
        void setEnableArrowButtons(bool state);
/**
* @return Returns whether the arrow buttons are enabled or not.
* @see #setEnableArrowButtons
* @see KWizard#enableArrowButtons.
*/
        bool enableArrowButtons();
/**
* Returns the handle of the tab bar.
*/
        KTabBar *getTabBar();
/**
* Returns the number of tabs in the notebook.
*/
        int numTabs();
/**
* En/Disable a tab in the notebook. If a tab is disabled it is not selectable
* from the tab bar. If the user reaches a disabled tab by traversing through
* the pages the notebook will jump to the next enabled tab.
*/
        void setTabEnabled(int tab, bool state);
/**
* @return Returns whether the tab is enabled or not.
* @see #setTabEnabled
*/
        bool isTabEnabled(int tab);
/**
* En/Disable a page in a section (tab) in the notebook
* @see KWizard#setPageEnabled
*/
        void setPageEnabled(int tab, int page, bool state);
/**
* @return Returns whether a page in a section (tab) is enabled or not.
* @see #setTabEnabled
*/
        bool isPageEnabled(int tab, int page);

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
*/
        void defaultclicked(int);
/**
* This signal is emitted when the user clicks on the Help button.
* The int is the page which was showing when help was requested.
* @see #setHelpButton
*/
        void helpclicked(int);

protected slots:
/**
* Called by @ref gotoTab to show the appropriate KWizard.
*/
        void showSection(int);
/**
* If the menu is enabled by @ref enablePopupMenu this method will let the menu
* popup at 'pos'.
* @internal
*/
        void popupMenu(QPoint pos);
/**
* @internal
*/
        void menuChoice(int);
/**
* @internal
*/
        void menuChoiceRedirect(int);
/**
* @internal
*/
        void directionButton(bool, bool);
/**
* @internal
*/
        void okClicked();
/**
* @internal
*/
        void cancelClicked();
/**
* @internal
*/
        void defaultClicked();
/**
* @internal
*/
        void helpClicked();
/**
* @internal
*/
        void tabScroll( ArrowType );

protected:
/**
* @internal
*/
        void init();
/**
* @internal
*/
        void resizeEvent(QResizeEvent *);
/**
* @internal
*/
        void paintEvent(QPaintEvent *);
/*
 @internal

        bool eventFilter( QObject *, QEvent * );
*/
/**
* @internal
*/
        void setSizes();
/**
* @internal
*/
        QSize childSize();

/**
* @internal
*/
        KNoteBookProtected *pnote;
/**
* @internal
*/
        QList<KWizard> *sections;

};

#endif //  __KNOTEBOOK_H



