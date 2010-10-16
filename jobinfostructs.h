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

#include "peer.h"

struct CollectionCreateInfo
{
   const QString &m_label;
   bool  m_locked;
   const Peer* m_peer;
   
   CollectionCreateInfo( const QString& label ) : 
      m_label( label ), m_locked(false), m_peer(0)
   {
   }
};

struct CollectionDeleteInfo
{
   /**
     * The peer that requested the delete operation
     */
   const Peer* m_peer;
};

struct ItemDeleteInfo
{
   /**
    * The peer that initiated the delete operation
    */
   const Peer *m_peer;
   ItemDeleteInfo( const Peer* peer ) : m_peer( peer )
   {}
};



#endif // JOBINFOSTRUCTS_H
