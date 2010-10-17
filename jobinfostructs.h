/*
 * Copyright 2010, Valentin Rusu <kde@rusu.info>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JOBINFOSTRUCTS_H
#define JOBINFOSTRUCTS_H
/**
 * @file jobinfostructs.h
 *
 * This file contain a collection of data objects exchanged by the various
 * asynchronous jobs used by the frontend and backend layer to interact.
 * We opted for structs and not classes as these data objects are not 
 * real full-fledget objets. They only need to transport the various contextual
 * parameters needed for the jobs to perform.
 *
 * All these data objects inherit from a base JobBaseInfo that mainly holds
 * information about the d-bus peer that originated the various operations
 * the jobs are requested to do.
 *
 * Some of the structures do not have any members. That is on purpose, as 
 * defining ans using them would allow easy addition of new data pieces to
 * exchange on the job chain path.
 */

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCrypto/QtCrypto>

#include "peer.h"

/**
 * This is the base class of all the jobs value objects
 * It takes ownership of the Peer pointer and deletes it
 * when it's destructor is called
 */
struct JobBaseInfo
{
   /**
     * The peer that requested the delete operation
     * Beware when storing the pointer to the peer outside
     * this class as it's deleting it when destructed
     */
   const Peer* m_peer;
   JobBaseInfo( const Peer* peer ) : m_peer( peer ) {}
   virtual ~JobBaseInfo()
   {
      if (m_peer != 0)
         delete m_peer, m_peer = 0;
   }
};

class BackendCollection;

struct CollectionCreateInfo : public JobBaseInfo
{
   const QString &m_label;
   bool  m_locked;
   
   CollectionCreateInfo( const QString& label, const Peer* peer ) : 
      JobBaseInfo(peer), m_label( label ), m_locked(false)
   {}
};

struct CollectionDeleteInfo : public JobBaseInfo
{
   mutable BackendCollection *m_collection;
   CollectionDeleteInfo( const Peer* peer, BackendCollection *collection =0) : JobBaseInfo( peer ),
      m_collection(collection)
   {}
};

struct CollectionUnlockInfo : public JobBaseInfo
{
   mutable BackendCollection *m_collection; // collection may be changed on the job path
   CollectionUnlockInfo( const Peer* peer, BackendCollection *collection =0) : JobBaseInfo( peer ),
      m_collection(collection)
   {}
};

class BackendItem;

struct ItemCreateInfo : public JobBaseInfo
{
   const QString &m_label;
   const QMap<QString, QString> &m_attributes;
   const QCA::SecureArray &m_secret;
   bool m_replace;
   bool m_locked;
   /**
    * @param label human-readable label for the new item
    * @param attributes attributes to set for the new item
    * @param secret the secret to store
    * @param locked true if the item should be locked after creation, false else
    * @param replace if true, replace an item with the same attributes if it already exists
    * @param peer the d-bus peer that initiated this operation
    */
   ItemCreateInfo( const QString &label, 
                   const QMap<QString, QString> &attributes,
                   const QCA::SecureArray &secret,
                   bool replace,
                   bool locked,
                   const Peer* peer) :
      JobBaseInfo( peer ),
      m_label(label), m_attributes(attributes), m_secret(secret), 
      m_replace(replace), m_locked(locked)
   {}
};

struct ItemDeleteInfo : public JobBaseInfo
{
   mutable BackendItem* m_item; // item information may be added on the job path so let it be mutable
   ItemDeleteInfo( const Peer* peer, BackendItem *item =0) : JobBaseInfo( peer ), m_item( item )
   {}
};

struct ItemUnlockInfo : public JobBaseInfo
{
   mutable BackendItem *m_item; // item information may be added on the job path so let it be mutable
   ItemUnlockInfo( const Peer *peer, BackendItem *item =0) : JobBaseInfo( peer ), m_item( item ) 
   {}
};

#endif // JOBINFOSTRUCTS_H
