/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
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

#ifndef __ksycoca_h__
#define __ksycoca_h__ "$Id$"

#include <ksycocatype.h>

class QDataStream;
class KSycocaFactory;
class KSycocaFactoryList;

class KSycoca
{
public:
   KSycoca(bool buildDatabase=false);

   virtual ~KSycoca();

   void addFactory( KSycocaFactory *);
   void save();

   static QDataStream *findEntry(int offset, KSycocaType &type);
   static QDataStream *registerFactory( KSycocaFactoryId id);

protected:
   QDataStream *_findEntry(int offset, KSycocaType &type);
   QDataStream *_registerFactory( KSycocaFactoryId id);

private:
   KSycocaFactoryList *factories;
   QDataStream *str;
   static KSycoca *self;
};

#endif
