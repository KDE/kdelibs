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

#include <kapp.h>
#include "kconfigbackend.h"

#include "ksimpleconfig.h"
#include "ksimpleconfig.moc"

KSimpleConfig::KSimpleConfig(const QString &pFileName, bool bReadOnly)
  : KConfig(pFileName, QString::null, bReadOnly, false)
{
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
    QString retValue = aIt->aValue;
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
      // we want to remove the group and all entries in the group
      for (; aIt.key().group == pGroup && aIt != aEntryMap.end(); ++aIt)
	aEntryMap.remove(aIt);
      // now remove the special group entry
      aEntryMap.remove(groupKey);
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

