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

#include <stdio.h>
#include <klocale.h>

#include "aps.h"

#include "kprintertray.h"
#include "kprinter.h"

struct knownPrinterTraysType { 
   const char *id;
   const char *name;
};

static knownPrinterTraysType knownPrinterTrays[] = 
{
 { "lower", I18N_NOOP("Lower Tray") },
 { "middle", I18N_NOOP("Middle Tray") },
 { "upper", I18N_NOOP("Upper Tray") },
 { "rear", I18N_NOOP("Rear Tray") },
 { "enveloppe", I18N_NOOP("Envelope Tray") },
 { "cassette", I18N_NOOP("Default") },
 { "largecapacity", I18N_NOOP("Large Capacity Tray") },
 { "anysmallformat", I18N_NOOP("Small Formats Tray") },
 { "anylargeformat", I18N_NOOP("Large Formats Tray") },
 { "manual", I18N_NOOP("Manual Feed") },
 { 0, 0 }
};

KPrinterTray::KPrinterTray(const QCString &id)
  : m_id(id.lower())
{
   knownPrinterTraysType *tray = knownPrinterTrays;
   while (tray->name)
   {
      if (m_id == tray->id)
         break;
      tray++;
   }
   if (tray->name)
      m_name = i18n(tray->name);
   else
      m_name = i18n(m_id);
}

KPrinterTray::~KPrinterTray()
{
}

QString
KPrinterTray::name() const
{
   return m_name;
}

QCString
KPrinterTray::id() const
{
   return m_id;
}

KPrinterTray::List
KPrinterTray::allInputTrays(KPrinter *printer)
{
   List result;

fprintf(stderr, "printer = %p, valid is %s\n", printer,
	printer ? (printer->isValid() ? "true" : "false") : "N.A.");

   if (!printer || !printer->isValid())
      return result;

   
   Aps_JobAttrHandle jobAttr = (Aps_JobAttrHandle *) printer->defaultAttributes();

   int numInputSlots;
   char **inputSlots;

   int resultCode = Aps_AttrQuickGetInputSlotOptions(jobAttr, &inputSlots, &numInputSlots);

fprintf(stderr, "GetInputSlotOptions returns %d\n", resultCode);
   char errorStr[1024];
   Aps_GetResultText(resultCode, errorStr, 1024);
fprintf(stderr, "GetInputSlotOptions returns: %s\n", errorStr);

   if (resultCode != APS_SUCCESS) 
   {
      result.append( new KPrinterTray("Cassette")); // Default
      return result;
   }  
   for(int i=0; i < numInputSlots; i++)
   {
      KPrinterTray::Ptr tray;
      tray = new KPrinterTray(inputSlots[i]);
      result.append(tray);
   }
   Aps_ReleaseBuffer(inputSlots);
   return result;
}

KPrinterTray::List
KPrinterTray::allOutputTrays(KPrinter *printer)
{
   List result;

fprintf(stderr, "printer = %p, valid is %s\n", printer,
	printer ? (printer->isValid() ? "true" : "false") : "N.A.");

   if (!printer || !printer->isValid())
      return result;

   
   Aps_JobAttrHandle jobAttr = (Aps_JobAttrHandle *) printer->defaultAttributes();

   int numInputSlots;
   char **inputSlots;

   int resultCode = Aps_AttrQuickGetOutputBinOptions(jobAttr, &inputSlots, &numInputSlots);

fprintf(stderr, "GetInputSlotOptions returns %d\n", resultCode);
   char errorStr[1024];
   Aps_GetResultText(resultCode, errorStr, 1024);
fprintf(stderr, "GetInputSlotOptions returns: %s\n", errorStr);

   if (resultCode != APS_SUCCESS) 
   {
      result.append( new KPrinterTray("Upper")); // Default
      return result;
   }  
   for(int i=0; i < numInputSlots; i++)
   {
      KPrinterTray::Ptr tray;
      tray = new KPrinterTray(inputSlots[i]);
      result.append(tray);
   }
   Aps_ReleaseBuffer(inputSlots);
   return result;
}

