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
#include "kpapersize.h"
#include "kprintjob.h"



class KPrintJobPrivate
{
public:
   KPrintJob::Orientation orientation;
   KPrinter::Ptr printer;
   KPaperSize::Ptr paperSize;
};

KPrintJob::KPrintJob()
{
  d = new KPrintJobPrivate;
  d->printer = new KPrinter("ap"); // Default printer
}

KPrintJob::~KPrintJob()
{
   delete d; d = 0;
}

void
KPrintJob::setPrinter( KPrinter *printer)
{
   d->printer = printer;
   d->paperSize = KPaperSize::defaultPaperSize(d->printer);
}

const KPrinter *
KPrintJob::printer() const
{
   return d->printer;
}

void
KPrintJob::setPaperSize( KPaperSize *paperSize)
{
   d->paperSize = paperSize;
}

const KPaperSize *
KPrintJob::paperSize() const 
{
   return d->paperSize;
}

KPaperSize::List
KPrintJob::allPaperSizes() const
{
   return KPaperSize::allPaperSizes(d->printer);
}

void
KPrintJob::setOrientation(Orientation orientation)
{
   d->orientation = orientation;
}

KPrintJob::Orientation
KPrintJob::orientation() const
{
   return d->orientation;
}
