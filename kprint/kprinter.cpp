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

#include "kprinter.h"
#include "aps.h"

#include <stdio.h>

class KPrinterPrivate
{
public:
   KPrinterPrivate() 
	: valid(false) 
	{ }

   ~KPrinterPrivate() 
   { 
     if (valid)
     {
        Aps_ReleaseHandle(aps_printer);
        Aps_ReleaseHandle(aps_jobAttr);
     } 
   }

   KPrinterPrivate(const KPrinterPrivate &d) 
   { 
     valid = d.valid;
     aps_printer = d.aps_printer;
     aps_jobAttr = d.aps_jobAttr;
     if (valid)
     {
        Aps_AddRef(aps_printer);
        Aps_AddRef(aps_jobAttr);
     }
   };

   bool valid;
   Aps_PrinterHandle aps_printer;
   Aps_JobAttrHandle aps_jobAttr;
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

KPrinter::KPrinter()
{
fprintf(stderr, "Opening default printer...");
  d = new KPrinterPrivate;
  if (Aps_OpenDefaultPrinter(&d->aps_printer) == APS_SUCCESS)
  {
fprintf(stderr, "APS_SUCCESS\n");
fprintf(stderr, "Fetching default attributes...");
     if (Aps_PrinterGetDefAttr(d->aps_printer, &d->aps_jobAttr) == APS_SUCCESS)
     {
fprintf(stderr, "APS_SUCCESS\n");
        d->valid = true;
     }
     if (!d->valid)
     {
        Aps_ReleaseHandle(d->aps_printer);
     }
  }
fprintf(stderr, "Done\n");
}

KPrinter::KPrinter(const QString &name)
{
fprintf(stderr, "Opening printer '%s'...", name.latin1());
  d = new KPrinterPrivate;
  if (Aps_OpenPrinter(name.utf8(), &d->aps_printer) == APS_SUCCESS)
  {
fprintf(stderr, "APS_SUCCESS\n");
fprintf(stderr, "Fetching default attributes...");
     if (Aps_PrinterGetDefAttr(d->aps_printer, &d->aps_jobAttr) == APS_SUCCESS)
     {
fprintf(stderr, "APS_SUCCESS\n");
        d->valid = true;
     }
     if (!d->valid)
     {
        Aps_ReleaseHandle(d->aps_printer);
     }
  }
fprintf(stderr, "Done\n");
}

KPrinter::KPrinter(const KPrinter &printer)
 : KShared(printer)
{
  d = new KPrinterPrivate(*printer.d);
}

KPrinter::~KPrinter()
{
   delete d; d = 0;
}

bool
KPrinter::isValid()
{
   return d->valid;
}

void *
KPrinter::defaultAttributes()
{
   return (void *) d->aps_jobAttr;
}

