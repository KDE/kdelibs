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
#include <stdlib.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <qfileinfo.h>

#include <kapp.h>
#include "kconfigbackend.h"

#include "kconfig.h"
#include "kglobal.h"
#include "kstddirs.h"
#include <qtimer.h>

KConfig::KConfig( const QString& fileName,
		  bool bReadOnly, bool bUseKderc )
  : KConfigBase(), flushInterval(30)
{
  // set the object's read-only status.
  setReadOnly(bReadOnly);

  // for right now we will hardcode that we are using the INI
  // back end driver.  In the future this should be converted over to
  // a object factory of some sorts.
  KConfigINIBackEnd *aBackEnd = new KConfigINIBackEnd(this,
						      fileName,
						      "config",
						      bUseKderc);
  // set the object's back end pointer to this new backend
  backEnd = aBackEnd;

  // need to set this before we actually parse so as to avoid
  // infinite looping when parseConfigFiles calls things like
  // hasGroup, putData, etc. which would then try to load
  // the cache if it isCached was false.
  isCached = true;

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

  // cache flushing setup
  //  cacheTimer = new QTimer(this, "cacheTimer");
  //  connect(cacheTimer, SIGNAL(timeout()), SLOT(flushCache()));
  // initial cache timeout of 30 seconds.  It will auto-adjust.
  //  cacheTimer->start(flushInterval * 1000);
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

  //  cacheCheck();

  KEntryMapConstIterator aIt;
  for (aIt = aEntryMap.begin(); aIt != aEntryMap.end(); ++aIt)
    if (aIt.key().key == QString::null)
      retList.append(aIt.key().group);

  return retList;
}

bool KConfig::hasKey(const QString &pKey) const
{
  KEntryKey aEntryKey;
  KEntryMapConstIterator aIt;

  //  cacheCheck();

  if (!locale().isNull()) {
    // try the localized key first
    QString aKey = pKey;
    aKey += '[';
    aKey += locale();
    aKey += ']';

    aEntryKey.group = group();
    aEntryKey.key = aKey;

    aIt = aEntryMap.find(aEntryKey);
    if (aIt != aEntryMap.end() && !(*aIt).aValue.isNull())
      return true;
  }

  // try the non-localized version
  aEntryKey.group = group();
  aEntryKey.key = pKey;

  aIt = aEntryMap.find(aEntryKey);
  return  (aIt != aEntryMap.end() && !(*aIt).aValue.isNull());
}

QMap<QString, QString> KConfig::entryMap(const QString &pGroup) const
{
  QMap<QString, QString> tmpMap;
  KEntryMapConstIterator aIt;
  KEntry aEntry;
  KEntryKey groupKey = { pGroup, QString() };

  //  cacheCheck();

  aIt = aEntryMap.find(groupKey);
  ++aIt; // advance past special group entry marker
  for (; aIt.key().group == pGroup && aIt != aEntryMap.end(); ++aIt)
    tmpMap.insert(aIt.key().key, (*aIt).aValue);

  return tmpMap;
}

void KConfig::reparseConfiguration()
{
  // do this right away to avoid infinite loops inside parseConfigFiles()
  // if it chooses to call putData or lookupData or something which will
  // call cacheCheck() --> reparseConfiguration() --> you get it
  //  isCached = true;
  aEntryMap.clear();

  // add the "default group" marker to the map
  KEntryKey groupKey = { "<default>", QString() };
  aEntryMap.insert(groupKey, KEntry());

  parseConfigFiles();
}

KEntryMap KConfig::internalEntryMap(const QString &pGroup) const
{
  KEntry aEntry;
  KEntryMapConstIterator aIt;
  KEntryKey aKey = { pGroup, QString() };
  KEntryMap tmpEntryMap;

  //  cacheCheck();

  aIt = aEntryMap.find(aKey);
  if (aIt == aEntryMap.end()) {
    // the special group key is not in the map,
    // so it must be an invalid group.  Return
    // an empty map.
    return tmpEntryMap;
  }
  // we now have a pointer to the nodes we want to copy.
  for (; aIt.key().group == pGroup && aIt != aEntryMap.end(); ++aIt)
    tmpEntryMap.insert(aIt.key(), *aIt);

  return tmpEntryMap;
}

void KConfig::putData(const KEntryKey &_key, const KEntry &_data)
{
  //  cacheCheck();

  // check to see if the special group key is present,
  // and if not, put it in.
  if (!hasGroup(_key.group)) {
    KEntryKey groupKey = { _key.group, QString() };
    aEntryMap.insert(groupKey, KEntry());
  }

  // now either add or replace the data
  KEntryMapIterator aIt = aEntryMap.find(_key);
  if (aIt != aEntryMap.end())
    aEntryMap.replace(_key, _data);
  else
    aEntryMap.insert(_key, _data);
}

KEntry KConfig::lookupData(const KEntryKey &_key) const
{
  //  cacheCheck();

  KEntryMapConstIterator aIt;

  aIt = aEntryMap.find(_key);
  if (aIt != aEntryMap.end())
    return *aIt;
  else {
    return KEntry();
  }
}

void KConfig::cacheCheck() const
{
  KConfig *that = const_cast<KConfig *>(this);
  that->lastIoOp = QTime::currentTime();
  if (!isCached) {
    that->reparseConfiguration();
  }
}

void KConfig::flushCache()
{
  if (!isCached) {
    // don't need to do anything
    return;
  }

  if (isDirty()) {
    // if the config object has dirty status, we can't flush it
    return;
  }

  // check if time of last I/O operation occured within timeout zone
  if (lastIoOp.addSecs(flushInterval) > QTime::currentTime()) {
    // IO occured within the flush interval.  Increase flush interval
    // and reset timer accordingly.
    flushInterval += (int)(flushInterval * .66);
  } else {
    // no I/O within the timeout period.  Flush the cache.
    isCached = false;
    aEntryMap.clear();
    KEntryKey groupKey = { "<default>", QString() };
    aEntryMap.insert(groupKey, KEntry());
    // reset the interval to 30 second checks
    flushInterval = 30;
  }

  cacheTimer->changeInterval(flushInterval * 1000);
}

#include "kconfig.moc"
