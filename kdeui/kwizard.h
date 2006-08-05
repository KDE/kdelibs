/* This file is part of the KDE libraries
    Copyright (C) 2006 Olivier Goffart <ogoffart at kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef KWIZARD_H
#define KWIZARD_H

#include <kpagedialog.h>

class KWizardPrivate;

/**
 * This class provides a framework for wizard dialogs.
 * 
 * A wizard is a dialog that consists of a sequence of pages. 
 * A wizard's purpose is to walk the user through a process step by step. 
 * Wizards are useful for complex or infrequently occurring tasks that people may find difficult to learn or do.
 * 
 * KWizard provides page titles and displays Next, Back, Finish, Cancel, and Help push buttons, 
 * as appropriate to the current position in the page sequence. 
 * The Finish Button has the code KDialog::User1 , The Next button is KDialog::User2 and the Back
 * button is KDialog::User3
 * The help button may be hidden using KDialog::showButton( Help , false )
 * 
 * Create and populate dialog pages that inherit from QWidget and add them to the wizard using addPage(). 
 * 
 * the function next() and back() are virtual you can inherit them if you want to override the defaults action 
 * of the next and back buttons.
 * 
 * @author Olivier Goffart <ogoffart at kde.org>
 */
class KWizard : public KPageDialog
{
    Q_OBJECT
    public:
        /**
         * Construct a new wizard with @p parent as parent.
         * @param parent is the parent of the widget.
         */
        KWizard(QWidget *parent=0L, Qt::WFlags flags=0);
        virtual ~KWizard();
        
        /**
         * Specify if the content of the page is valid, and if the next button may be enabled on this page.
         * By default all pages are valid
         * 
         * this will disable or enable the next button on the specified page
         * 
         * @param page the page on which the next button will be enabled/disable
         * @param enable if true the next button will be enabled, if false it will be disabled
         */
        void setValid( KPageWidgetItem* page, bool enable );
        
        /**
         * return if a page is valid
         * @see setValid
         */
        bool isValid( KPageWidgetItem *page );
        
        /**
         * Specify weither a page is appropriate.
         * 
         * A page is not appropriate if it should not be showed because the contents of others page make it unapropriate.
         * 
         * A page which is not apropriate will not be shown.
         * 
         * The last page should always be appropriate
         */
        void setAppropriate(KPageWidgetItem *page, bool appropriate);

        /**
         * returns what has been set in setAppropriate
         */
        bool isAppropriate(KPageWidgetItem *page);

 
    public slots:
        /**
         * Called when the user clicks the Back button.
         * 
         * this function will show the preceding relevant page in the sequence.
         * Do nothing if this is the current page is the first page
         */
        virtual void back();
        /**
         * Called when the user clicks the Next/Finish button.
         * 
         * this function will show the next relevant page in the sequence.
         * If the current page is the last page, it will call accept()
         */
        virtual void next();
        
    private slots:
        void slotCurrentPageChanged();
    protected:
        /**
         * Constructor that call the  KPageDialog (KPageWidget *widget, QWidget *parent, Qt::WFlags flags) constructor
         */
        KWizard (KPageWidget *widget, QWidget *parent=0, Qt::WFlags flags=0);

        virtual void showEvent ( QShowEvent * event );
    private:
        KWizardPrivate * const d;
        void init();
        
};


#endif
