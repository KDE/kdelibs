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

#ifndef _KPRINTJOB_H_
#define _KPRINTJOB_H_

#include <kpapersize.h>
#include <kprintertray.h>

class KPrinter;
class KPrintJobPrivate;

class KPrintJob
{
public:   
   /**
    * Create a new print job.
    */
   KPrintJob();
   
   /**
    * Destructor
    */
   ~KPrintJob();
   
   /**
    * Set the printer to use for this job.
    */
   void setPrinter(KPrinter::Ptr printer);
   
   /**
    * Returns printer to use for this job.
    */
   const KPrinter::Ptr printer() const;
   
   /**
    * Set the paper size to use for this job.
    */
   void setPaperSize( KPaperSize::Ptr paperSize);
   
   /**
    * Returns the paper size to use for this job.
    */
   const KPaperSize::Ptr paperSize() const;

   /**
    * Returns a list of all possible paper sizes for this job.
    */
   KPaperSize::List allPaperSizes() const;   

   /**
    * Set the input tray to use for this job
    */
   void setInputTray( KPrinterTray::Ptr inputTray);
   
   /**
    * Returns the input tray to use for this job.
    */
   const KPrinterTray::Ptr inputTray() const;

   /**
    * Returns a list of all possible input trays for this job.
    */
   KPrinterTray::List allInputTrays() const;   

   /**
    * Set the output tray to use for this job
    */
   void setOutputTray( KPrinterTray::Ptr outputTray);
   
   /**
    * Returns the output tray to use for this job.
    */
   const KPrinterTray::Ptr outputTray() const;

   /**
    * Returns a list of all possible output trays for this job.
    */
   KPrinterTray::List allOutputTrays() const;   

   enum Orientation { Portrait, Landscape };
  
   /**
    * Set the orientation of the paper
    */
   void setOrientation( Orientation );
   
   /**
    * Returns the orientation of the paper
    */
   Orientation orientation() const;
   
private:
   KPrintJobPrivate *d;   
};

#endif
