/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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
#include "kbuildsycoca.h"
#include "ksycocatype.h"
#include "ksycocafactory.h"

#include <qdatastream.h>
#include <qfile.h>

#include <assert.h>
#include <kglobal.h>
#include <kstddirs.h>

KBuildSycoca::KBuildSycoca()
{
}
   
KBuildSycoca::~KBuildSycoca()
{
}

void
KBuildSycoca::addFactory( KSycocaFactory *factory)
{
   assert(m_lstFactories);
   m_lstFactories->append(factory);
   // TODO borrow code from KRegistry for parsing files
}

void 
KBuildSycoca::save()
{
   // Write header (#pass 1)
   str->device()->at(0);

   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      Q_INT32 aId;
      Q_INT32 aOffset;
      aId = factory->factoryId();
      aOffset = factory->offset();
      (*str) << aId;
      (*str) << aOffset;
   }
   (*str) << (Q_INT32) 0; // No more factories.

   // Write factory data....
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      factory->save(*str);
   }
   int endOfData = str->device()->at();

   // Write header (#pass 2)
   str->device()->at(0);

   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      Q_INT32 aId;
      Q_INT32 aOffset;
      aId = factory->factoryId();
      aOffset = factory->offset();
      (*str) << aId;
      (*str) << aOffset;
   }
   (*str) << (Q_INT32) 0; // No more factories.

   // Jump to end of database
   str->device()->at(endOfData);
}

