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

#include "aps.h"

#include "kprinter.h"

#include <stdio.h>

class KPrinterPrivate
{
public:
   KPrinterPrivate() { }
   ~KPrinterPrivate() { }
   KPrinterPrivate & operator=(const KPrinterPrivate &d) 
   { 
     return *this;
   };
};

QStringList
KPrinter::allPrinters()
{
   QStringList list;

   int count = -1;
   char **names;
   Aps_Result result = Aps_GetPrinters( &names, &count);
fprintf(stderr, "GetPrinters returns: %d, count = %d\n", result, count);
   if (result == APS_SUCCESS)
   {
      for(int i = 0; i < count; i++)
      {
fprintf(stderr, "Printer: '%s'\n", names[i]);
         // WABA: What is the decoding ??
         QString name = QString::fromLocal8Bit(names[i]);
         list.append(name);
      }
      Aps_ReleaseBuffer(names);
   }
   return list;
}

KPrinter::KPrinter(const QString &name)
{
  d = new KPrinterPrivate;
}

KPrinter::KPrinter(const KPrinter &printer)
{
  d = new KPrinterPrivate;
  *d = *(printer.d);
}

KPrinter::~KPrinter()
{
   delete d; d = 0;
}

KPrinter &
KPrinter::operator=(const KPrinter &printer)
{
   *d = *(printer.d);
   return *this;
}

