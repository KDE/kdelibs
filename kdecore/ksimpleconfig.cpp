/* This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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

#include "kglobal.h"
#include "kstddirs.h"
#include "kconfigbackend.h"

#include "ksimpleconfig.h"
#include "ksimpleconfig.moc"

KSimpleConfig::KSimpleConfig(const QString &pFileName, bool bReadOnly)
  : KConfig("", bReadOnly, false)
{
  // the difference between KConfig and KSimpleConfig is just that 
  // for KSimpleConfig an absolute filename is guaranteed
  if (!pFileName.isNull() && pFileName[0] != '/') {
     backEnd->changeFileName( KGlobal::dirs()->
	getSaveLocation("config", QString::null, true)+pFileName, false);
  } else {
     backEnd->changeFileName(pFileName, false);
  }
  parseConfigFiles();
}

KSimpleConfig::~KSimpleConfig()
{
  // we need to call the KSimpleConfig version of sync.  Relying on the
  // regular KConfig sync is bad, because the KSimpleConfig sync has
  // different behaviour.  Syncing here will insure that the sync() call
  // in the KConfig destructor doesn't actually do anything.
  sync();
}

QString KSimpleConfig::deleteEntry( const QString& pKey, bool bLocalized )
{
  QString aLocalizedKey = pKey;

  // localize the key, if requested
  if (bLocalized) {
      aLocalizedKey += "[";
      aLocalizedKey += locale();
      aLocalizedKey += "]";
  }

  // retrieve the current entry map for the group specified by pKey
  KEntryKey entryKey = { group(), aLocalizedKey };
  KEntryMapIterator aIt;

  aIt = aEntryMap.find(entryKey);
  if (aIt != aEntryMap.end()) {
    QString retValue = (*aIt).aValue;
    // we found the key, get rid of it
    aEntryMap.remove(aIt);
    return retValue;
  } else
    return QString::null;
}


bool KSimpleConfig::deleteGroup( const QString& pGroup, bool bDeep )
{

  KEntryMapIterator aIt;
  KEntryKey groupKey = { pGroup, QString() };

  aIt = aEntryMap.find(groupKey);
  if (aIt != aEntryMap.end()) {
    // group found, remove it
    if (!bDeep) {
      // just remove the special group entry
      aEntryMap.remove(aIt);
      return false;
    } else {
      // we have to build up a list of things to remove, because if you
      // remove things while traversing a QMap with an iterator,
      // the iterator gets confused.
      QValueList<KEntryKey> keyList;
      // we want to remove the group and all entries in the group
      for (; aIt.key().group == pGroup && aIt != aEntryMap.end(); ++aIt)
	keyList.append(aIt.key());
    
      QValueList<KEntryKey>::Iterator kIt(keyList.begin());
      for (; kIt != keyList.end(); ++kIt)
	aEntryMap.remove(*kIt);

      return true;
    }
  } else
    // no such group
    return false;
}

void KSimpleConfig::sync()
{
   if (isReadOnly())
       return;
   backEnd->sync(false);

   if (isDirty())
     rollback();
}

