/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kstandarddirs.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kprotocolmanager.h>

#include "kprotocolinfofactory.h"

#include "ksycoca.h"
#include "ksycocadict.h"

KProtocolInfoFactory* KProtocolInfoFactory::_self = 0;

KProtocolInfoFactory::KProtocolInfoFactory() : KSycocaFactory( KST_KProtocolInfoFactory )
{
  _self = this;
}

KProtocolInfoFactory::~KProtocolInfoFactory()
{
  _self = 0;
}


KProtocolInfo*
KProtocolInfoFactory::createEntry(int offset)
{
   KProtocolInfo *info = 0;
   KSycocaType type;
   QDataStream *str = KSycoca::self()->findEntry(offset, type);
   switch (type)
   {
     case KST_KProtocolInfo:
       info = new KProtocolInfo(*str, offset);
       break;
     default:
       return 0;
   }
   if (!info->isValid())
   {
      delete info;
      info = 0;
   }
   return info;
}


QStringList KProtocolInfoFactory::protocols()
{
  QStringList res;

  KSycocaEntry::List list = allEntries();
  for( KSycocaEntry::List::Iterator it = list.begin();
       it != list.end();
       ++it)
  {
     KSycocaEntry *entry = (*it).data();
     KProtocolInfo *info = static_cast<KProtocolInfo *>(entry);

     res.append( info->name() );
  }

  return res;
}

KProtocolInfo *
KProtocolInfoFactory::findProtocol(const QString &protocol)
{
  if (!m_sycocaDict) return 0; // Error!

  if (protocol == m_lastProtocol) return m_lastInfo;

  int offset;

  offset = m_sycocaDict->find_string( protocol );

  if (!offset) return 0; // Not found;

  KProtocolInfo *info = createEntry(offset);

  if (info && (info->name() != protocol))
  {
     // No it wasn't...
     delete info;
     info = 0; // Not found
  }
  m_lastProtocol = protocol;
  m_lastInfo = info;
  return info;
}

KProtocolInfo *
KProtocolInfoFactory::findProtocol(const KURL &url)
{
  QString dummy;
  QString protocol = KProtocolManager::slaveProtocol(url, dummy);
  return findProtocol(protocol);
}

void KProtocolInfoFactory::virtual_hook( int id, void* data )
{ KSycocaFactory::virtual_hook( id, data ); }

