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
   void setPrinter(KPrinter *printer);
   
   /**
    * Returns printer to use for this job.
    */
   const KPrinter *printer() const;
   
   /**
    * Set the paper size to use for this job.
    */
   void setPaperSize( KPaperSize *paperSize);
   
   /**
    * Returns the paper size to use for this job.
    */
   const KPaperSize *paperSize() const;

   /**
    * Returns a list of all possible paper sizes for this job.
    */
   KPaperSize::List allPaperSizes() const;   

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
