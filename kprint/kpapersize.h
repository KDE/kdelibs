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

#ifndef _KPAPERSIZE_H_
#define _KPAPERSIZE_H_

#include <qvaluelist.h>
#include <ksharedptr.h>

class KPrinter;
class KPaperSizePrivate;

class KPaperSize : public KShared
{
public:   
   typedef KSharedPtr<KPaperSize> Ptr;
   typedef QValueList<Ptr> List;
   friend Ptr;

   /**
    * Default constructor.
    */
   KPaperSize();

   /**
    * The name of this paper size.
    */
   QString name() const;
   
   /**
    * The default paper-size for printer.
    */
   static KPaperSize *defaultPaperSize(KPrinter *printer);

   /**
    * All possible paper sizes for printer. 
    */
   static List allPaperSizes(KPrinter *printer);

   /**
    * List of well-known paper sizes. 
    */
   static List wellKnownPaperSizes();
   
private:
   /**
    * Construct paper size from Aps_PageSize structure.
    */
   KPaperSize(void *_apsPageSize);

   /**
    * Destructor
    */
   ~KPaperSize();

   KPaperSizePrivate *d;   
};

#endif
