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
#include <kcombobox.h>
#include <klineedit.h>
#include <knuminput.h>

#include <kprinter.h>
#include <kprintjob.h>

#include "kprintdialog.h"

class KPrintDialogPrivate
{
public:
   QPrinter *printer;
   KPrintJob *job;

   // Widgets
   KListView *printerList;
   KComboBox *paperSizes;
   KComboBox *paperOrientation;
};

KPrintDialog::KPrintDialog(QWidget *parent, KPrintJob *job, bool modal)
  : KDialogBase(Tabbed, i18n("Print"), User1 | Cancel, User1,
		parent, "kprintdialog", modal, false, i18n("&Print"))
{
   d = new KPrintDialogPrivate();
   d->printer = 0;
   d->printerList = 0;
   if (job)
      d->job = job;
   else
      d->job = new KPrintJob();

   addGeneralPage();
   addPaperPage();
   addAdvancedPage();
   setAvailablePrinters();
   setPrinterSettings();
}

void KPrintDialog::addGeneralPage()
{
   QFrame *page = addPage( i18n("&General"));
   QGridLayout *pageLayout = new QGridLayout( page, 3, 3);

   int row = 0;

   QButtonGroup *printDest;
   printDest = new QButtonGroup(page);
   printDest->setTitle(i18n("Print destination"));
   QGridLayout *printDestLayout;
   printDestLayout = new QGridLayout( printDest, 5, 3, marginHint(), spacingHint());
   printDestLayout->addColSpacing(0, 20);
   printDestLayout->addRowSpacing(row, 10+marginHint()); // Make room for the printDest title!

   row++;

   QRadioButton *printToPrinter;
   printToPrinter = new QRadioButton(i18n("Print to p&rinter:"), printDest);
   printDestLayout->addMultiCellWidget( printToPrinter, row, row,0,1 );

   row++;

   d->printerList = new KListView(printDest);   
   d->printerList->addColumn(i18n("Printer"));
   d->printerList->addColumn(i18n("Description"));
   d->printerList->setFixedHeight(100);
   printDestLayout->addWidget( d->printerList, row, 1);
   printDestLayout->setRowStretch(row, 1);

   row++;

   QRadioButton *printToFile;
   printToPrinter = new QRadioButton(i18n("Print to &file:"), printDest);
   printDestLayout->addMultiCellWidget(printToPrinter, row, row, 0, 1);

   row++;

   KLineEdit *fileSelection;
   fileSelection = new KLineEdit( printDest);
   printDestLayout->addWidget( fileSelection, row, 1);

   QPushButton *execBrowse;
   execBrowse = new QPushButton( i18n("&Browse..."), printDest );
   printDestLayout->addWidget( execBrowse, row, 2);

   pageLayout->addWidget(printDest, 0, 0);

   row = 0; // New group... new rows

   QButtonGroup *options;
   options = new QButtonGroup(page);
   options->setTitle(i18n("Options"));
   QGridLayout *optionsLayout;
   optionsLayout = new QGridLayout( options, 2, 3, marginHint(), spacingHint());
   optionsLayout->addRowSpacing(row, 10+marginHint()); // Make room for the printDest title!

   row++;

   KNumInput *nrOfCopies;
   nrOfCopies = new KIntNumInput( options);
   nrOfCopies->setLabel( i18n("&Number of copies:"), AlignHCenter | AlignVCenter);
   optionsLayout->addWidget(nrOfCopies, row, 0);

   pageLayout->addWidget(options, 1, 0);

   pageLayout->setRowStretch(2, 1);// Keep empty space at bottom

}

void KPrintDialog::addPaperPage()
{
   QFrame *page = addPage( i18n("P&aper"));
   QGridLayout *pageLayout = new QGridLayout( page, 3, 3, 0, spacingHint());

   QLabel *label;
   int row = 0;

   label = new QLabel(i18n("Paper &orientation:"), page);
   label->setAlignment(AlignLeft | AlignTop | ShowPrefix);
   pageLayout->addWidget(label, row, 0);

   d->paperOrientation = new KComboBox(page);
   label->setBuddy(d->paperOrientation);
   d->paperOrientation->insertItem(i18n("Portrait"));
   d->paperOrientation->insertItem(i18n("Landscape"));
   d->paperOrientation->setMinimumSize(d->paperOrientation->minimumSizeHint());
   pageLayout->addWidget(d->paperOrientation, row, 1);

   row++;

   label = new QLabel(i18n("Paper &size:"), page);
   label->setAlignment(AlignLeft | AlignTop | ShowPrefix);
   pageLayout->addWidget(label, row, 0);

   d->paperSizes = new KComboBox(page);
   label->setBuddy(d->paperSizes);
   d->paperSizes->insertItem(i18n("Dummy Paper Format"));
   d->paperSizes->setMinimumSize(d->paperSizes->minimumSizeHint());
   pageLayout->addWidget(d->paperSizes, row, 1);

   row++;   

   pageLayout->setRowStretch(row, 1); // Keep empty space at bottom 
}

void KPrintDialog::addAdvancedPage()
{
   QFrame *page = addPage( i18n("&Advanced"));

}

void KPrintDialog::setAvailablePrinters()
{
   d->printerList->clear();
   QStringList printers = KPrinter::allPrinters();
   for(QStringList::ConstIterator it = printers.begin();
       it != printers.end();
       ++it)
   {
      (void) new QListViewItem( d->printerList, *it, i18n("..."));
   }
}

void KPrintDialog::setPrinterSettings()
{
   d->paperSizes->clear();
   KPaperSize::List sizes = d->job->allPaperSizes();
   for(KPaperSize::List::ConstIterator it = sizes.begin();
       it != sizes.end();
       ++it)
   {
      d->paperSizes->insertItem((*it)->name());
   }
}

KPrintDialog::~KPrintDialog()
{
   delete d; d = 0;
}


#include "kprintdialog.moc"
