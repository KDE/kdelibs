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

#include "kconfig.h"
#include "kconfig.moc"

KConfig::KConfig( const QString& pGlobalFileName,
		  const QString& pLocalFileName,
		  bool bReadOnly )
  : KConfigBase(), flushInterval(30)
{
  QString aGlobalFileName, aLocalFileName;

  // set the object's read-only status.
  setReadOnly(bReadOnly);

  if (!pGlobalFileName.isNull()) {
    aGlobalFileName = pGlobalFileName;
    if (!bReadOnly) {
      // the file should exist in any case if the object is not read only.
      QFileInfo info( aGlobalFileName );
      if (!info.exists()) {
	// Can we allow the write? (see above)
	if (checkAccess( aGlobalFileName, W_OK )) {
	  // Yes, write OK, create empty file
	  QFile file( aGlobalFileName );
	  file.open( IO_WriteOnly );
	  file.close();
	}
      }
    }
  }

  if (!pLocalFileName.isNull()) {
    aLocalFileName = pLocalFileName;
    if (!bReadOnly) {
      // the file should exist in any case if the object is not read only.
      QFileInfo info( aLocalFileName );
      if (!info.exists()) {
	// Can we allow the write? (see above)
	if (checkAccess( pLocalFileName, W_OK )) {
	  // Yes, write OK, create empty file
	  QFile file( pLocalFileName );
	  file.open( IO_WriteOnly );
	  // Set uid/gid (necessary for SUID programs)
	  chown(file.name().ascii(), getuid(), getgid());
	  file.close();
	}
      }
    }
  }
  
  // for right now we will hardcode that we are using the INI
  // back end driver.  In the future this should be converted over to
  // a object factory of some sorts.
  KConfigINIBackEnd *aBackEnd = new KConfigINIBackEnd(this,
						      aGlobalFileName,
						      aLocalFileName,
						      true);
  // set the object's back end pointer to this new backend
  backEnd = aBackEnd;

  // add the "default group" marker to the map
  KEntryKey groupKey = { "<default>", QString::null };
  aEntryMap.insert(groupKey, KEntry());

  // need to set this before we actually parse so as to avoid
  // infinite looping when parseConfigFiles calls things like
  // hasGroup, putData, etc. which would then try to load
  // the cache if it isCached was false.
  isCached = true;
  parseConfigFiles();

  // cache flushing setup
  cacheTimer = new QTimer(this, "cacheTimer");
  connect(cacheTimer, SIGNAL(timeout()), SLOT(flushCache()));
  // initial cache timeout of 30 seconds.  It will auto-adjust.
  cacheTimer->start(flushInterval * 1000);
}

KConfig::~KConfig()
{
  sync();
  
  if (backEnd)
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
    aIt->bDirty = false;
}

QStringList KConfig::groupList() const
{
  QStringList retList;

  cacheCheck();

  KEntryMapConstIterator aIt;
  for (aIt = aEntryMap.begin(); aIt != aEntryMap.end(); ++aIt)
    if (aIt.key().key == QString::null)
      retList.append(aIt.key().group);

  return retList;
}

QMap<QString, QString> KConfig::entryMap(const QString &pGroup) const
{
  QMap<QString, QString> tmpMap;
  KEntryMapConstIterator aIt;
  KEntry aEntry;
  KEntryKey groupKey = { pGroup, QString::null };

  cacheCheck();

  aIt = aEntryMap.find(groupKey);
  for (; aIt.key().group == pGroup && aIt != aEntryMap.end(); ++aIt)
    tmpMap.insert(aIt.key().key, aIt->aValue);

  return tmpMap;
}

void KConfig::reparseConfiguration()
{
  aEntryMap.clear();

  // add the "default group" marker to the map
  KEntryKey groupKey = { "<default>", QString::null };
  aEntryMap.insert(groupKey, KEntry());

  parseConfigFiles();
  isCached = true;
}

KEntryMap KConfig::internalEntryMap(const QString &pGroup) const
{
  KEntry aEntry;
  KEntryMapConstIterator aIt;
  KEntryKey aKey = { pGroup, QString::null };
  KEntryMap tmpEntryMap;

  cacheCheck();

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
    KEntryKey groupKey = { "<default>", QString::null };
    aEntryMap.insert(groupKey, KEntry());
    // reset the interval to 30 second checks
    flushInterval = 30;
  }

  cacheTimer->changeInterval(flushInterval * 1000);
}
