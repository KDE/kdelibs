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

#ifndef _KPRINTERTRAY_H_
#define _KPRINTERTRAY_H_

#include <qstring.h>
#include <qvaluelist.h>
#include <ksharedptr.h>

class KPrinter;

class KPrinterTray : public KShared
{
public:   
   typedef KSharedPtr<KPrinterTray> Ptr;
   typedef QValueList<Ptr> List;
   friend Ptr;

   /**
    * Constructor.
    */
   KPrinterTray( const QCString &id);

   /**
    * The translated name of this print tray
    */
   QString name() const;

   /**
    * The id of this print tray
    */
   QCString id() const;   

   /**
    * All possible input trays for printer.
    */
   static List allInputTrays(KPrinter *printer);

   /**
    * All possible output trays for printer.
    */
   static List allOutputTrays(KPrinter *printer);

private:
   /**
    * Destructor
    */
   ~KPrinterTray();

   QString m_name;
   QCString m_id;
};

#endif
