/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "khtml_pagecache.h"

#include <kstaticdeleter.h>

#include <qintdict.h>
#include <qtimer.h>

#include <assert.h>

// We keep 12 pages in memory.
#define EXPIRE_QUEUE_LENGTH 12

class KHTMLPageCacheEntry
{
public:
  KHTMLPageCacheEntry(long id) : m_id(id), m_valid(false) 
   { }

  void addData(const QByteArray &data)
   { m_data.append( data ); }

  void endData()
   { m_valid = true; }

  bool isValid() 
   { return m_valid; }

  KHTMLPageCacheDelivery *fetchData(QObject *recvObj, const char *recvSlot);

  void saveData(QDataStream *str);

private:
  long m_id;
  bool m_valid;
  QValueList<QByteArray> m_data;
};

class KHTMLPageCachePrivate
{
public:
  long newId;
  QIntDict<KHTMLPageCacheEntry> dict;
  QList<KHTMLPageCacheDelivery> delivery;
  QList<KHTMLPageCacheEntry> expireQueue;
  bool deliveryActive;
};


KHTMLPageCacheDelivery *
KHTMLPageCacheEntry::fetchData(QObject *recvObj, const char *recvSlot)
{
  KHTMLPageCacheDelivery *delivery = new KHTMLPageCacheDelivery(m_data);
  recvObj->connect(delivery, SIGNAL(emitData(const QByteArray&)), recvSlot);
  return delivery;
}

void
KHTMLPageCacheEntry::saveData(QDataStream *str)
{
  for(QValueList<QByteArray>::Iterator it = m_data.begin();
      it != m_data.end();
      ++it)
  {
     str->writeRawBytes((*it).data(), (*it).size());
  }
}

static KStaticDeleter<KHTMLPageCache> pageCacheDeleter;

KHTMLPageCache *KHTMLPageCache::_self = 0;

KHTMLPageCache *
KHTMLPageCache::self()
{
  if (!_self)
     _self = pageCacheDeleter.setObject(new KHTMLPageCache);  
  return _self;
}

KHTMLPageCache::KHTMLPageCache()
{
  d = new KHTMLPageCachePrivate;
  d->newId = 1;
  d->deliveryActive = false;
}

KHTMLPageCache::~KHTMLPageCache()
{
  delete d;
}

long
KHTMLPageCache::createCacheEntry()
{
  KHTMLPageCacheEntry *entry = new KHTMLPageCacheEntry(d->newId);
  d->dict.insert(d->newId, entry);   
  d->expireQueue.append(entry);
  if (d->expireQueue.count() > EXPIRE_QUEUE_LENGTH)
  {
     KHTMLPageCacheEntry *entry = d->expireQueue.take(0);
     delete entry;
  }
  return (d->newId++);
}

void
KHTMLPageCache::addData(long id, const QByteArray &data)
{
  KHTMLPageCacheEntry *entry = d->dict.find(id);
  if (entry) 
     entry->addData(data);
}

void
KHTMLPageCache::endData(long id)
{
  KHTMLPageCacheEntry *entry = d->dict.find(id);
  if (entry) 
     entry->endData();
}

void
KHTMLPageCache::cancelEntry(long id)
{
  KHTMLPageCacheEntry *entry = d->dict.take(id);
  if (entry) 
     delete entry;
}

bool
KHTMLPageCache::isValid(long id)
{
  KHTMLPageCacheEntry *entry = d->dict.find(id);
  if (entry) 
     return entry->isValid();
  return false;
}

void
KHTMLPageCache::fetchData(long id, QObject *recvObj, const char *recvSlot)
{
  KHTMLPageCacheEntry *entry = d->dict.find(id);
  if (!entry) return;

  // Make this entry the most recent entry.
  d->expireQueue.removeRef(entry);
  d->expireQueue.append(entry);

  d->delivery.append( entry->fetchData(recvObj, recvSlot) );
  if (!d->deliveryActive)
  {
     d->deliveryActive = true;
     QTimer::singleShot(20, this, SLOT(sendData()));
  }
}

void
KHTMLPageCache::sendData()
{
  if (d->delivery.isEmpty())
  {
     d->deliveryActive = false;
     return;
  }
  KHTMLPageCacheDelivery *delivery = d->delivery.take(0);
  assert(delivery);

  QValueList<QByteArray>::Iterator it = delivery->data.begin();
  delivery->emitData(*it);
  delivery->data.remove(it);  
  if (!delivery->data.isEmpty())
  {
     d->delivery.append( delivery );
  }
  QTimer::singleShot(20, this, SLOT(sendData()));
}

void 
KHTMLPageCache::saveData(long id, QDataStream *str)
{
  KHTMLPageCacheEntry *entry = d->dict.find(id);
  assert(entry);

  entry->saveData(str);
}
