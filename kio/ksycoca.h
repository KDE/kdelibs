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

#include <qobject.h>
#include "ksycocatype.h"

class QDataStream;
class KSycocaFactory;
class KSycocaFactoryList;

/**
 * @internal
 * Read-only SYstem COnfiguration CAche
 */
class KSycoca : public QObject
{
  Q_OBJECT
protected:
   /**
    * @internal
    * Building database
    */
   KSycoca( bool /* buildDatabase */ );

public:

   /**
    * Read-only database
    */
   KSycoca();

   /**
    * Get or create the only instance of KSycoca (read-only)
    */
   static KSycoca *self();

   virtual ~KSycoca();

   /**
    * @internal - called by factories in read-only mode
    * This is how factories get a stream to an entry
    */
   QDataStream *findEntry(int offset, KSycocaType &type);
   /**
    * @internal - called by factories in read-only mode
    */
   QDataStream *registerFactory( KSycocaFactoryId id);
   /**
    * @internal - returns stream to the header (see DESIGN)
    */
   QDataStream *findHeader();
    
   /**
    * @internal
    * @return true if building (i.e. if a KBuildSycoca);
    */
   virtual bool isBuilding() { return false; }

protected:
   KSycocaFactoryList *m_lstFactories;
   QDataStream *str;
   static KSycoca *_self;
};

#endif
