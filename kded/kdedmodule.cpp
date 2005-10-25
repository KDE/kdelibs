/*
   This file is part of the KDE libraries

   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#include <qtimer.h>

#include "kded.h"
#include "kdedmodule.h"
#include "kconfigdata.h"

#if 0 // KDED_OBJECTS
typedef QMap<KEntryKey, KSharedPtr<KShared> > KDEDObjectMap;
#endif

class KDEDModulePrivate
{
public:
#if 0 // KDED_OBJECTS
  KDEDObjectMap *objMap;
  int timeout;
  QTimer timer;
#endif
};

KDEDModule::KDEDModule(const DCOPCString &name) : QObject(), DCOPObject(name)
{
   d = new KDEDModulePrivate;
#if 0 // KDED_OBJECTS
   d->objMap = 0;
   d->timeout = 0;
   d->timer.setSingleShot( true );
   connect(&(d->timer), SIGNAL(timeout()), this, SLOT(idle()));
#endif
}

KDEDModule::~KDEDModule()
{
   emit moduleDeleted(this);
   delete d; d = 0;
}

#if 0 // see header (grep keyword: KDED_OBJECTS)
void KDEDModule::setIdleTimeout(int secs)
{
   d->timeout = secs*1000;
}

void KDEDModule::resetIdle()
{
   d->timer.stop();
   if (!d->objMap || d->objMap->isEmpty())
      d->timer.start(d->timeout);
}

void KDEDModule::insert(const DCOPCString &app, const DCOPCString &key, KShared *obj)
{
   if (!d->objMap)
      d->objMap = new KDEDObjectMap;

   // appKey acts as a placeholder
   KEntryKey appKey(app, 0);
   d->objMap->insert(appKey, 0);

   KEntryKey indexKey(app, key);

   // Prevent deletion in case the same object is inserted again.
   KSharedPtr<KShared> _obj = obj;

   d->objMap->insert(indexKey, _obj);
   resetIdle();
}

KShared * KDEDModule::find(const DCOPCString &app, const DCOPCString &key)
{
   if (!d->objMap)
      return 0;
   KEntryKey indexKey(app, key);

   KDEDObjectMap::ConstIterator it = d->objMap->find(indexKey);
   if (it == d->objMap->end())
      return 0;

   return it.value().get();
}

void KDEDModule::remove(const DCOPCString &app, const DCOPCString &key)
{
   if (!d->objMap)
      return;
   KEntryKey indexKey(app, key);

   d->objMap->remove(indexKey);
   resetIdle();
}

void KDEDModule::removeAll(const DCOPCString &app)
{
   if (!d->objMap)
      return;

   KEntryKey indexKey(app, 0);
   // Search for placeholder.

   KDEDObjectMap::Iterator it = d->objMap->find(indexKey);
   while (it != d->objMap->end())
   {
      if (it.key().mGroup != app)
         break; // All keys for this app have been removed.
      it = d->objMap->erase(it);
   }
   resetIdle();
}
#endif

bool KDEDModule::isWindowRegistered(long windowId) const
{
   return Kded::self()->isWindowRegistered(windowId);
}
#include "kdedmodule.moc"
