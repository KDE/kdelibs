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

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCrypto/QtCrypto>

#include "peer.h"

struct JobBaseInfo
{
   /**
     * The peer that requested the delete operation
     */
   const Peer* m_peer;
   JobBaseInfo( const Peer* peer ) : m_peer( peer ) {}
};

class BackendCollection;

struct CollectionCreateInfo : public JobBaseInfo
{
   const QString &m_label;
   bool  m_locked;
   
   CollectionCreateInfo( const QString& label, const Peer* peer ) : 
      JobBaseInfo(peer), m_label( label ), m_locked(false)
   {
   }
};

struct CollectionDeleteInfo : public JobBaseInfo
{
   CollectionDeleteInfo( const Peer* peer ) : JobBaseInfo( peer )
   {}
};

struct CollectionUnlockInfo : public JobBaseInfo
{
   BackendCollection *m_collection;
   CollectionUnlockInfo( const Peer* peer ) : JobBaseInfo( peer )
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
   ItemDeleteInfo( const Peer* peer ) : JobBaseInfo( peer )
   {}
};

struct ItemUnlockInfo : public JobBaseInfo
{
   mutable BackendItem *m_item; // item information may be added on the job path so let it be mutable
   ItemUnlockInfo( const Peer *peer, BackendItem *item =0) : JobBaseInfo( peer ), m_item( item ) 
   {}
};

#endif // JOBINFOSTRUCTS_H
