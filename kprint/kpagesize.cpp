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

#include "kpagesize.h"

class KPageSizePrivate
{
public:
   QString name;
};

KPageSize::KPageSize()
{
  d = new KPageSizePrivate;
}

KPageSize::~KPageSize()
{
   delete d; d = 0;
}

QString
KPageSize::name() const
{
   return d->name;
}

KPageSize *
KPageSize::defaultPageSize(KPrinter *printer)
{
   // Not Yet implemented
   return 0;
}

KPageSize::List
KPageSize::allPageSizes(KPrinter *printer)
{
   // Not yet implemented
   List result;

   KPageSize::Ptr pageSize;
   pageSize = new KPageSize();
   pageSize->d->name = QString::fromLatin1("A4");
   result.append(pageSize);
   pageSize = new KPageSize();
   pageSize->d->name = QString::fromLatin1("B5");
   result.append(pageSize);

   return result;
}
