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

#ifndef __ksycocafactory_h__
#define __ksycocafactory_h__ "$Id$"

#include "ksycocatype.h"

#include <qlist.h>
class KSycoca;

class KSycocaFactory
{
public:
   virtual KSycocaFactoryId factoryId() const = 0;
 
public:
   /**
    * Create a factory which can be used to lookup from/create a database
    */
   KSycocaFactory() { }

   virtual ~KSycocaFactory() { }

   int offset() { return mOffset; }
   void setOffset(int _offset) { mOffset = _offset; }

   /**
    * Saves all entries it maintains as well as index files
    * for these entries to the stream 'str'.
    */
   virtual void save(QDataStream &str) = 0;

private:
   int mOffset;
};

class KSycocaFactoryList : public QList<KSycocaFactory>
{
public:
   KSycocaFactoryList() { }
};

#endif
