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

#ifndef _KPRINTER_H_
#define _KPRINTER_H_

#include <ksharedptr.h>

#include <qstring.h>
#include <qstringlist.h>

class KPrinterPrivate;

class KPrinter : public KShared
{
   typedef KSharedPtr<KPrinter> Ptr;
   friend Ptr;
public:   
   /**
    * Create a default printer
    */
   KPrinter();

   /**
    * Create the printer named @p name
    */
   KPrinter(const QString &name);
   
   /**
    * Copy constructor
    */
   KPrinter(const KPrinter &printer);

   /**
    * Returns whether this is a valid printer.
    */
   bool isValid();


   /**
    * Return a list of all available printers.
    */
   static QStringList allPrinters();

public:
   // APS specific functions

   // We return a pointer so that we don't need to include aps.h
   /**
    * Internal
    * Access function to Aps structure.
    */
   void * defaultAttributes();
   
private:
   /**
    * Destructor
    */
   ~KPrinter();

   KPrinterPrivate *d;   
};

#endif
