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

#include <klocale.h>

#include "kprinter.h"
#include "kpapersize.h"

#include <stdio.h>

struct knownFormatType { 
   const char *name;
   const char *id;
};

static knownFormatType knownFormats[] = 
{
 { I18N_NOOP("A4"), "A4" },
 { I18N_NOOP("B5"), "B5" },
 { I18N_NOOP("Letter"), "letter" },
 { I18N_NOOP("Legal"), "legal" },
 { 0, 0 }
};



class KPaperSizePrivate
{
public:
   QString name;
   QCString id;
};

KPaperSize::KPaperSize()
{
  d = new KPaperSizePrivate;
}

KPaperSize::KPaperSize(void *_apsPageSize)
{
  Aps_PageSize *apsPageSize = (Aps_PageSize *) _apsPageSize;
  d = new KPaperSizePrivate;
  d->name = QString::fromUtf8(apsPageSize->translatedName);
  d->id = apsPageSize->id;
}

KPaperSize::~KPaperSize()
{
   delete d; d = 0;
}

QString
KPaperSize::name() const
{
   return QString("%1 (%2)\n").arg(d->name).arg(d->id);
}

KPaperSize *
KPaperSize::defaultPaperSize(KPrinter *printer)
{
   // Not Yet implemented
   return 0;
}

KPaperSize::List
KPaperSize::allPaperSizes(KPrinter *printer)
{
   // Not yet implemented
   List result;

fprintf(stderr, "printer = %p, valid is %s\n", printer,
	printer ? (printer->isValid() ? "true" : "false") : "N.A.");

   if (!printer || !printer->isValid())
      return result;

   
   Aps_JobAttrHandle jobAttr = (Aps_JobAttrHandle *) printer->defaultAttributes();

   int numPageSizes;
   Aps_PageSize **apsPageSizes;

   int resultCode = Aps_AttrQuickGetPageSizeOptions(jobAttr, &apsPageSizes, &numPageSizes);

fprintf(stderr, "GetPageSizeOptions returns %d\n", resultCode);
   char errorStr[1024];
   Aps_GetResultText(resultCode, errorStr, 1024);
fprintf(stderr, "GetPageSizeOptions returns: %s\n", errorStr);

   if (resultCode != APS_SUCCESS) 
      return wellKnownPaperSizes(); // Return a default list of sizes
//      return result;

   for(int i=0; i < numPageSizes; ++i)
   {
      KPaperSize::Ptr paperSize;
      paperSize = new KPaperSize(apsPageSizes[i]);
      result.append(paperSize);
   }
   Aps_ReleaseBuffer(apsPageSizes);

   return result;
}

KPaperSize::List
KPaperSize::wellKnownPaperSizes()
{
   // Not yet implemented
   List result;

   knownFormatType *knownFormat = knownFormats;
   while (knownFormat->name && knownFormat->id)
   {
      KPaperSize::Ptr paperSize = new KPaperSize();
      paperSize->d->name = i18n(knownFormat->name);
      paperSize->d->id = knownFormat->id;
      result.append(paperSize);
      knownFormat++;
   }
 
   return result;
}
