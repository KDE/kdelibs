/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (C) 1997-1999 Matthias Kalle Dalheimer (kalle@kde.org)

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

// $Id$

#include <config.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include <qfileinfo.h>

#include <kapplication.h>
#include "kconfigbackend.h"

#include "kconfig.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include <qtimer.h>

KConfig::KConfig( const QString& fileName,
                 bool bReadOnly, bool bUseKderc, const char *resType )
  : KConfigBase(), bGroupImmutable(false)
{
  // set the object's read-only status.
  setReadOnly(bReadOnly);

  // for right now we will hardcode that we are using the INI
  // back end driver.  In the future this should be converted over to
  // a object factory of some sorts.
  KConfigINIBackEnd *aBackEnd = new KConfigINIBackEnd(this,
						      fileName,
                                                      resType,
						      bUseKderc);
  // set the object's back end pointer to this new backend
  backEnd = aBackEnd;

  // read initial information off disk
  reparseConfiguration();

  // we let KStandardDirs add custom user config files. It will do
  // this only once. So only the first call ever to this constructor
  // will anything else than return here We have to reparse here as
  // configuration files may appear after customized directories have
  // been added. and the info they contain needs to be inserted into the
  // config object.
  // Since this makes only sense for config directories, addCustomized
  // returns true only if new config directories appeared.
  if (KGlobal::dirs()->addCustomized(this))
      reparseConfiguration();
}

KConfig::~KConfig()
{
  sync();

  delete backEnd;
}

void KConfig::rollback(bool bDeep)
{
  KConfigBase::rollback(bDeep);

  if (!bDeep)
    return; // object's bDeep flag is set in KConfigBase method

  // clear any dirty flags that entries might have set
  for (KEntryMapIterator aIt = aEntryMap.begin();
       aIt != aEntryMap.end(); ++aIt)
    (*aIt).bDirty = false;
}

QStringList KConfig::groupList() const
{
  QStringList retList;

  KEntryMapConstIterator aIt;
  for (aIt = aEntryMap.begin(); aIt != aEntryMap.end(); ++aIt)
    if (aIt.key().mKey.isEmpty())
      retList.append(QString::fromUtf8(aIt.key().mGroup));

  return retList;
}

bool KConfig::hasKey(const QString &key) const
{
   return KConfig::hasKey(key.utf8().data());
}

bool KConfig::hasKey(const char *pKey) const
{
  KEntryKey aEntryKey(mGroup, 0);
  aEntryKey.c_key = pKey;

  KEntryMapConstIterator aIt;

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    aIt = aEntryMap.find(aEntryKey);
    if (aIt != aEntryMap.end() && !(*aIt).mValue.isNull() && !(*aIt).bDeleted )
      return true;
    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  aIt = aEntryMap.find(aEntryKey);
  return  (aIt != aEntryMap.end() && !(*aIt).mValue.isNull() && !(*aIt).bDeleted );
}

QMap<QString, QString> KConfig::entryMap(const QString &pGroup) const
{
  QCString pGroup_utf = pGroup.utf8();
  QMap<QString, QString> tmpMap;
  KEntryMapConstIterator aIt;
  KEntry aEntry;
  KEntryKey groupKey( pGroup_utf, 0 );

  aIt = aEntryMap.find(groupKey);
  if (aIt == aEntryMap.end())
     return tmpMap;
  ++aIt; // advance past special group entry marker
  for (; aIt.key().mGroup == pGroup_utf && aIt != aEntryMap.end(); ++aIt)
  {
    // Leave the default values out && leave deleted entries out
    if (!aIt.key().bDefault && !(*aIt).bDeleted) 
      tmpMap.insert(QString::fromUtf8(aIt.key().mKey), QString::fromUtf8((*aIt).mValue.data(), (*aIt).mValue.length()));
  }

  return tmpMap;
}

void KConfig::reparseConfiguration()
{
  aEntryMap.clear();

  // add the "default group" marker to the map
  KEntryKey groupKey("<default>", 0);
  aEntryMap.insert(groupKey, KEntry());

  parseConfigFiles();
}

KEntryMap KConfig::internalEntryMap(const QString &pGroup) const
{
  QCString pGroup_utf = pGroup.utf8();
  KEntry aEntry;
  KEntryMapConstIterator aIt;
  KEntryKey aKey(pGroup_utf, 0);
  KEntryMap tmpEntryMap;

  aIt = aEntryMap.find(aKey);
  if (aIt == aEntryMap.end()) {
    // the special group key is not in the map,
    // so it must be an invalid group.  Return
    // an empty map.
    return tmpEntryMap;
  }
  // we now have a pointer to the nodes we want to copy.
  for (; aIt.key().mGroup == pGroup_utf && aIt != aEntryMap.end(); ++aIt)
  {
    tmpEntryMap.insert(aIt.key(), *aIt);
  }

  return tmpEntryMap;
}

void KConfig::putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup)
{
  // check to see if the special group key is present,
  // and if not, put it in.
  if (_checkGroup) 
  {
    KEntryKey groupKey( _key.mGroup, 0);
    KEntry &entry = aEntryMap[groupKey];
    bGroupImmutable = entry.bImmutable;
  }
  if (bGroupImmutable)
    return;

  // now either add or replace the data
  KEntry &entry = aEntryMap[_key];
  if (entry.bImmutable)
    return;

  entry = _data;

  if (_key.bDefault)
  {
     // We have added the data as default value, 
     // add it as normal value as well.
     KEntryKey key(_key);
     key.bDefault = false;
     aEntryMap[key] = _data;
  }
}

KEntry KConfig::lookupData(const KEntryKey &_key) const
{
  KEntryMapConstIterator aIt;

  aIt = aEntryMap.find(_key);
  if (aIt != aEntryMap.end())
  {
    const KEntry &entry = *aIt;
    if (entry.bDeleted)
       return KEntry();
    else 
       return entry;
  }
  else {
    return KEntry();
  }
}

bool KConfig::hasGroup(const QString &group) const
{
  return KConfig::hasGroup( group.utf8());
}

bool KConfig::hasGroup(const char *_pGroup) const
{
  return KConfig::hasGroup( QCString(_pGroup));
}

bool KConfig::hasGroup(const QCString &group) const
{
  KEntryKey groupKey( group, 0);
  return aEntryMap.contains(groupKey);
}


#include "kconfig.moc"
