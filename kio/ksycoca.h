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

#include <dcopobject.h>
#include <qobject.h>
#include "ksycocatype.h"

class QDataStream;
class KSycocaFactory;
class KSycocaFactoryList;

/**
 * Sycoca file version number.
 * If the existing file is outdated, it will not get read
 * but instead we'll ask kded to regenerate a new one...
*/
#define KSYCOCA_VERSION 10

/**
 * @internal
 * Read-only SYstem COnfiguration CAche
 */
class KSycoca : public QObject, public DCOPObject
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
    * slot for receiving kded's signal, when the sycoca file changes
    */
   bool process(const QCString &fun, const QByteArray &data,
                QCString &replyType, QByteArray &replyData);

   /**
    * @internal - called by factories in read-only mode
    * This is how factories get a stream to an entry
    */
   QDataStream *findEntry(int offset, KSycocaType &type);
   /**
    * @internal - called by factories in read-only mode
    */
   QDataStream *findFactory( KSycocaFactoryId id);
   /**
    * @internal - returns stream to the header (see DESIGN)
    */
   QDataStream *findHeader();
    
   /**
    * @internal - add a factory
    */
   void addFactory( KSycocaFactory * );

   /**
    * @internal
    * @return true if building (i.e. if a KBuildSycoca);
    */
   virtual bool isBuilding() { return false; }

protected:
   void checkVersion();
   KSycocaFactoryList *m_lstFactories;
   QDataStream *str;
   static KSycoca *_self;
};

#endif
