/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qprinter.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>

#include <klocale.h>
#include <klistview.h>
#include <klineedit.h>
#include <knuminput.h>

#include <kprinter.h>

#include "kprintdialog.h"

class KPrintDialogPrivate
{
public:
   QPrinter *printer;
};

KPrintDialog::KPrintDialog(QWidget *parent, bool modal)
  : KDialogBase(Tabbed, i18n("Print"), User1 | Cancel, User1,
		parent, "kprintdialog", modal, false, i18n("&Print"))
{
   d = new KPrintDialogPrivate();
   d->printer = 0;

   addGeneralPage();
   addPaperPage();
   addAdvancedPage();
}

void KPrintDialog::addGeneralPage()
{
   QFrame *page = addPage( i18n("&General"));
   QGridLayout *pageLayout = new QGridLayout( page, 3, 3);

   QButtonGroup *printDest;
   printDest = new QButtonGroup(page);
   printDest->setTitle(i18n("Print destination"));
   QGridLayout *printDestLayout;
   printDestLayout = new QGridLayout( printDest, 5, 3, marginHint(), spacingHint());
   printDestLayout->addColSpacing(0, 20);
   printDestLayout->addRowSpacing(0, 10+marginHint()); // Make room for the printDest title!

   QRadioButton *printToPrinter;
   printToPrinter = new QRadioButton(i18n("Print to p&rinter:"), printDest);
   printDestLayout->addMultiCellWidget( printToPrinter, 1,1,0,1 );

   KListView *listView = new KListView(printDest);   
   listView->addColumn(i18n("Printer"));
   listView->addColumn(i18n("Description"));
   QStringList printers = KPrinter::allPrinters();
   for(QStringList::ConstIterator it = printers.begin();
       it != printers.end();
       ++it)
   {
      (void) new QListViewItem( listView, *it, i18n("..."));
   }
   listView->setFixedHeight(100);
   printDestLayout->addWidget( listView, 2, 1);
   printDestLayout->setRowStretch(2, 1);

   QRadioButton *printToFile;
   printToPrinter = new QRadioButton(i18n("Print to &file:"), printDest);
   printDestLayout->addMultiCellWidget(printToPrinter, 3,3, 0, 1);

   KLineEdit *fileSelection;
   fileSelection = new KLineEdit( printDest);
   printDestLayout->addWidget( fileSelection, 4, 1);

   QPushButton *execBrowse;
   execBrowse = new QPushButton( i18n("&Browse..."), printDest );
   printDestLayout->addWidget( execBrowse, 4, 2);

   pageLayout->addWidget(printDest, 0, 0);

   QButtonGroup *options;
   options = new QButtonGroup(page);
   options->setTitle(i18n("Options"));
   QGridLayout *optionsLayout;
   optionsLayout = new QGridLayout( options, 2, 3, marginHint(), spacingHint());
   optionsLayout->addRowSpacing(0, 10+marginHint()); // Make room for the printDest title!

   KNumInput *nrOfCopies;
   nrOfCopies = new KIntNumInput( options);
   nrOfCopies->setLabel( i18n("&Number of copies:"), AlignHCenter | AlignVCenter);
   optionsLayout->addWidget(nrOfCopies, 1, 0);

   pageLayout->addWidget(options, 1, 0);

   pageLayout->addWidget(new QWidget(page), 2, 0);
   pageLayout->setRowStretch(2, 1);

}

void KPrintDialog::addPaperPage()
{
   QFrame *page = addPage( i18n("P&aper"));

}

void KPrintDialog::addAdvancedPage()
{
   QFrame *page = addPage( i18n("&Advanced"));

}

KPrintDialog::~KPrintDialog()
{
   delete d; d = 0;
}


#include "kprintdialog.moc"
