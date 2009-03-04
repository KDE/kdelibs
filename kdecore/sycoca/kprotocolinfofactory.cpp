/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kprotocolinfofactory.h"

#include <kstandarddirs.h>
#include <kdebug.h>
#include <ksycoca.h>
#include <ksycocadict.h>

K_GLOBAL_STATIC(KSycocaFactorySingleton<KProtocolInfoFactory>, kProtocolInfoFactoryInstance)

KProtocolInfoFactory::KProtocolInfoFactory() : KSycocaFactory( KST_KProtocolInfoFactory )
{
    kProtocolInfoFactoryInstance->instanceCreated(this);
}

KProtocolInfoFactory::~KProtocolInfoFactory()
{
    if (kProtocolInfoFactoryInstance.exists())
        kProtocolInfoFactoryInstance->instanceDestroyed(this);
}

KProtocolInfo*
KProtocolInfoFactory::createEntry(int offset) const
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


QStringList KProtocolInfoFactory::protocols() const
{
    QStringList res;
    const KSycocaEntry::List list = allEntries();
    for( KSycocaEntry::List::const_iterator it = list.begin();
         it != list.end();
         ++it) {
        const KSycocaEntry *entry = (*it).data();
        const KProtocolInfo *info = static_cast<const KProtocolInfo *>(entry);
        res.append( info->name() );
    }
    return res;
}

KProtocolInfo::List KProtocolInfoFactory::allProtocols() const
{
    KProtocolInfo::List result;
    const KSycocaEntry::List list = allEntries();
    for( KSycocaEntry::List::ConstIterator it = list.begin();
         it != list.end();
         ++it) {
        if ((*it)->isType(KST_KProtocolInfo)) {
            result.append(KProtocolInfo::Ptr::staticCast(*it));
        }
    }
    return result;
}

KProtocolInfo::Ptr KProtocolInfoFactory::findProtocol(const QString &protocol)
{
  if (!sycocaDict()) return KProtocolInfo::Ptr(); // Error!

  QMap<QString,KProtocolInfo::Ptr>::iterator it = m_cache.find(protocol);
  if (it != m_cache.end())
     return *it;

  int offset;

  offset = sycocaDict()->find_string( protocol );

  if (!offset) return KProtocolInfo::Ptr(); // Not found;

  KProtocolInfo::Ptr info(createEntry(offset));

  if (info && (info->name() != protocol))
  {
     // No it wasn't...
     return KProtocolInfo::Ptr(); // Not found
  }
  m_cache.insert(protocol,info);
  return info;
}

void KProtocolInfoFactory::virtual_hook( int id, void* data )
{ KSycocaFactory::virtual_hook( id, data ); }

KProtocolInfoFactory* KProtocolInfoFactory::self()
{
    return kProtocolInfoFactoryInstance->self();
}
