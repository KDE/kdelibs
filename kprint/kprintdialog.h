/* 
   This file is part of the KDE libraries
   Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KPRINTDIALOG_H_
#define _KPRINTDIALOG_H_

#include <kdialogbase.h>

class KPrintJob;
class KPrintDialogPrivate;

class KPrintDialog: public KDialogBase
{
   Q_OBJECT
public:   
   KPrintDialog(QWidget *parent=0, KPrintJob *job = 0, bool modal = true);
   ~KPrintDialog();

#if 0
   // This looks like a bad interface.
   void setPrinter( QPrinter * );
   QPrinter *printer() const;
#endif

protected:
   void addGeneralPage();
   void addPaperPage();
   void addAdvancedPage();
   
   /**
    * Fill the list of available printers
    */
   void setAvailablePrinters();
   
   /**
    * Set the settings according to the currently selected printer
    */
   void setPrinterSettings();

protected slots:
   void slotBrowse();
   void slotPrinterSelected();

protected:
   
   KPrintDialogPrivate *d;   
};

#endif
