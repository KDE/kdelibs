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
#ifndef __kbuildsycoca_h__
#define __kbuildsycoca_h__ 

#include <qobject.h>
#include <qstring.h>
#include <qdict.h>
#include <ksycoca.h>
#include <ksycocatype.h>
#include <ksycocaentry.h>
#include <sys/stat.h>

class QDataStream;

// No need for this in libkio - apps only get readonly access
class KBuildSycoca : public KSycoca
{
   Q_OBJECT
public:
   typedef QValueList<KSycocaEntry::List> KSycocaEntryListList;
public:
   KBuildSycoca();
   virtual ~KBuildSycoca();

   /**
    * Recreate the database file
    */
   void recreate(KSycocaEntryListList *, QDict<Q_UINT32> *);

protected:

   /**
    * Look up gnome mimetypes.
    */
   void processGnomeVfs();

   /**
    * Build the whole system cache, from .desktop files
    */
   bool build(KSycocaEntryListList *, QDict<Q_UINT32> *);
   
   /**
    * Save the ksycoca file
    */
   void save();

   /**
    * Clear the factories
    */
   void clear();

   /**
    * @internal
    * @return true if building (i.e. if a KBuildSycoca);
    */
   virtual bool isBuilding() { return true; }
};

#endif
