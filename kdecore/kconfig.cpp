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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <config.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "kconfig.h"
#include "kconfigbackend.h"
#include "kglobal.h"
#include "kstandarddirs.h"
#include "kstaticdeleter.h"
#include "ktoolinvocation.h"

#include <qtimer.h>
#include <qfileinfo.h>

KConfig::KConfig( const QString& fileName,
                 bool readOnly, bool bUseKderc, const char *resType )
  : KConfigBase(), bGroupImmutable(false), bFileImmutable(false),
    bForceGlobal(false)
{
  // set the object's read-only status.
  setReadOnly(readOnly);

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

KConfig::KConfig(KConfigBackEnd *aBackEnd, bool readOnly)
    : bGroupImmutable(false), bFileImmutable(false),
    bForceGlobal(false)
{
  setReadOnly(readOnly);
  backEnd = aBackEnd;
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

  KEntryMapConstIterator aIt = aEntryMap.begin();
  KEntryMapConstIterator aEnd = aEntryMap.end();
  for (; aIt != aEnd; ++aIt)
  {
    while(aIt.key().mKey.isEmpty())
    {
      QByteArray _group = aIt.key().mGroup;
      ++aIt;
      while (true)
      {
         if (aIt == aEnd)
            return retList; // done

         if (aIt.key().mKey.isEmpty())
            break; // Group is empty, next group

         if (!aIt.key().bDefault && !(*aIt).bDeleted)
         {
            if (_group != "$Version") // Special case!
               retList.append(QString::fromUtf8(_group));
            break; // Group is non-empty, added, next group
         }
         ++aIt;
      }
    }
  }

  return retList;
}

QMap<QString, QString> KConfig::entryMap(const QString &pGroup) const
{
  QByteArray pGroup_utf = pGroup.toUtf8();
  KEntryKey groupKey( pGroup_utf, 0 );
  QMap<QString, QString> tmpMap;

  KEntryMapConstIterator aIt = aEntryMap.find(groupKey);
  if (aIt == aEntryMap.end())
     return tmpMap;
  ++aIt; // advance past special group entry marker
  for (; aIt != aEntryMap.end() && aIt.key().mGroup == pGroup_utf; ++aIt)
  {
    // Leave the default values out && leave deleted entries out
    if (!aIt.key().bDefault && !(*aIt).bDeleted)
      tmpMap.insert(QString::fromUtf8(aIt.key().mKey), QString::fromUtf8((*aIt).mValue.data(), (*aIt).mValue.length()));
  }

  return tmpMap;
}

void KConfig::reparseConfiguration()
{
  // Don't lose pending changes
  if (!isReadOnly() && backEnd && bDirty)
    backEnd->sync();

  aEntryMap.clear();

  // add the "default group" marker to the map
  KEntryKey groupKey("<default>", 0);
  aEntryMap.insert(groupKey, KEntry());

  bFileImmutable = false;
  parseConfigFiles();
  bFileImmutable = bReadOnly;
}

KEntryMap KConfig::internalEntryMap(const QString &pGroup) const
{
  QByteArray pGroup_utf = pGroup.toUtf8();
  KEntry aEntry;
  KEntryMapConstIterator aIt;
  KEntryKey aKey(pGroup_utf, 0);
  KEntryMap tmpEntryMap;

  aIt = aEntryMap.find(aKey);
  //Copy any matching nodes.
  for (; aIt != aEntryMap.end() && aIt.key().mGroup == pGroup_utf ; ++aIt)
  {
    tmpEntryMap.insert(aIt.key(), *aIt);
  }

  return tmpEntryMap;
}

void KConfig::putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup)
{
  if (bFileImmutable && !_key.bDefault)
    return;

  // check to see if the special group key is present,
  // and if not, put it in.
  if (_checkGroup)
  {
    KEntryKey groupKey( _key.mGroup, 0);
    KEntry &entry = aEntryMap[groupKey];
    bGroupImmutable = entry.bImmutable;
  }
  if (bGroupImmutable && !_key.bDefault)
    return;

  // now either add or replace the data
  KEntry &entry = aEntryMap[_key];
  bool immutable = entry.bImmutable;
  if (immutable && !_key.bDefault)
    return;

  entry = _data;
  entry.bImmutable |= immutable;
  entry.bGlobal |= bForceGlobal; // force to kdeglobals

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
  KEntryMapConstIterator aIt = aEntryMap.find(_key);
  if (aIt != aEntryMap.end())
  {
    if (!aIt->bDeleted) 
       return *aIt;
  }
  
  return KEntry();
}

bool KConfig::internalHasGroup(const QByteArray &_group) const
{
  KEntryKey groupKey( _group, 0);

  KEntryMapConstIterator aIt = aEntryMap.find(groupKey);
  KEntryMapConstIterator aEnd = aEntryMap.end();

  if (aIt == aEnd)
     return false;
  ++aIt;
  for(; (aIt != aEnd); ++aIt)
  {
     if (aIt.key().mKey.isEmpty())
        break;

     if (!aIt.key().bDefault && !(*aIt).bDeleted)
        return true;
  }
  return false;
}

void KConfig::setFileWriteMode(int mode)
{
  backEnd->setFileWriteMode(mode);
}

KLockFile::Ptr KConfig::lockFile(bool bGlobal)
{
  KConfigINIBackEnd *aBackEnd = dynamic_cast<KConfigINIBackEnd*>(backEnd);
  if (!aBackEnd) return 0;
  return aBackEnd->lockFile(bGlobal);
}

void KConfig::checkUpdate(const QString &id, const QString &updateFile)
{
  QString oldGroup = group();
  setGroup("$Version");
  QString cfg_id = updateFile+":"+id;
  QStringList ids = readListEntry("update_info");
  if (!ids.contains(cfg_id))
  {
     QStringList args;
     args << "--check" << updateFile;
     KToolInvocation::kdeinitExecWait("kconf_update", args);
     reparseConfiguration();
  }
  setGroup(oldGroup);
}

KConfig* KConfig::copyTo(const QString &file, KConfig *config) const
{
  if (!config)
     config = new KConfig(QString::null, false, false);
  config->backEnd->changeFileName(file, "config", false);
  config->setReadOnly(false);
  config->bFileImmutable = false;
  config->backEnd->mConfigState = ReadWrite;

  QStringList groups = groupList();
  for(QStringList::ConstIterator it = groups.begin();
      it != groups.end(); ++it)
  {
     QMap<QString, QString> map = entryMap(*it);
     config->setGroup(*it);
     for (QMap<QString,QString>::Iterator it2  = map.begin();
          it2 != map.end(); ++it2)
     {
        config->writeEntry(it2.key(), it2.value());
     }

  }
  return config;
}

void KConfig::virtual_hook( int id, void* data )
{ KConfigBase::virtual_hook( id, data ); }

static KStaticDeleter< QList<KSharedConfig*> > sd;
QList<KSharedConfig*> *KSharedConfig::s_list = 0;

KSharedConfig::Ptr KSharedConfig::openConfig(const QString& fileName, bool immutable, bool useKDEGlobals )
{
  if (s_list)
  {
     for(QList<KSharedConfig*>::ConstIterator it = s_list->begin();
         it != s_list->end(); ++it)
     {
        if ((*it)->backEnd->fileName() == fileName &&
                (*it)->backEnd->bFileImmutable == immutable &&
                (*it)->backEnd->useKDEGlobals == useKDEGlobals )
           return (*it);
     }
  }
  return new KSharedConfig(fileName, immutable, useKDEGlobals);
}

KSharedConfig::KSharedConfig( const QString& fileName, bool readonly, bool usekdeglobals)
 : KConfig(fileName, readonly, usekdeglobals)
{
  if (!s_list)
  {
    sd.setObject(s_list, new QList<KSharedConfig*>);
  }

  s_list->append(this);
}

KSharedConfig::~KSharedConfig()
{
  if ( s_list )
    s_list->removeAll(this);
}

#include "kconfig.moc"
