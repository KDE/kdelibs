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

#ifndef _KPAGESIZE_H_
#define _KPAGESIZE_H_

#include <qvaluelist.h>
#include <ksharedptr.h>

class KPrinter;
class KPageSizePrivate;

class KPageSize : public KShared
{
public:   
   typedef KSharedPtr<KPageSize> Ptr;
   typedef QValueList<Ptr> List;
   friend Ptr;

   /**
    * Default constructor.
    */
   KPageSize();

   /**
    * The name of this page size.
    */
   QString name() const;
   
   /**
    * The default page-size for printer.
    */
   static KPageSize *defaultPageSize(KPrinter *printer);

   /**
    * All possible page sizes for printer. 
    */
   static List allPageSizes(KPrinter *printer);
   
private:
   /**
    * Destructor
    */
   ~KPageSize();

   KPageSizePrivate *d;   
};

#endif
