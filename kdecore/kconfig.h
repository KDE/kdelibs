/* 
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (C) 1997 Matthias Kalle Dalheimer <kalle@kde.org>
   
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

#ifndef _KCONFIG_H
#define _KCONFIG_H

#include <qtimer.h>
#include <qdatetime.h>

#include "kconfigbase.h"

/** 
* KDE Configuration entries
*
* This class implements KDE's default configuration system.
*
* @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
* @version $Id$
* @see KApplication::getConfig KConfigBase KSimpleConfig
* @short KDE Configuration Management  class
*/
class KConfig : public KConfigBase
{
  Q_OBJECT

public:
  /** 
   * Construct a KConfig object. 
   *
   * @param pGlobalFileName A file to parse in addition to the
   *        system-wide file(s).
   * @param pLocalFileName An additional (usually user-specific)
   *        file to parse; has priority over pGlobalFileName.
   * @param bReadOnly Set the config object's read-only status.
   */
  KConfig( const QString& pGlobalFileName = QString::null, 
	   const QString& pLocalFileName = QString::null,
	   bool bReadOnly = false);

  /** 
   * Destructor. 
   *
   * Writes back any dirty configuration entries, and destroys
   * dynamically created objects.
   */
  virtual ~KConfig();

  /**
   * clears all entries out of the dirtyEntryMap, so the
   * values will not be written to disk on a later call to
   * sync().
   *
   * @param bDeep if true, the dirty map is actually emptied.
   *        otherwise, the config object's global dirty flag is set to
   *        false, but the dirty entries remain in the dirty entry
   *        map.
   *
   * @see KConfigBase::rollback 
   */
  virtual void rollback(bool bDeep = true);

  /**
   * Returns true if the specified group is known about.
   *
   * @param _pGroup the group to search for.
   * @returns Whether or not the group exists.
   */
  virtual bool hasGroup(const QString &_pGroup) const;

  /**
   * Returns a list of groups that are known about.
   * @returns the list of groups.
   */
  virtual QStringList groupList() const;
  
  /*
   * Check if the key has an entry in the currently active group. Use
   * this to determine if a key is not specified for the current group
   * (hasKey returns false). Keys with null data are considered
   * nonexistent.
   *
   * @param pKey the key to search for.
   */
  virtual bool hasKey(const QString &pKey) const;

  /**
   * Return a map (tree) of entries for all entries in a particular
   * group.  Only the actual entry string is returned, none of the
   * other internal data should be included.
   *
   * @param pGroup A group to get keys from.
   * @return A map of entries in the group specified, indexed by key.
   *         The returned map may be empty if the group is not found.
   */
  virtual QMap<QString, QString> entryMap(const QString &pGroup) const;

  /**
   * clears all internal data structures and then rereads
   * configuration information from disk.
   */
  virtual void reparseConfiguration(void);

protected:

  /**
   * Returns an map (tree) of the entries in the specified group.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param pGroup the group to provide a KEntryMap for.
   * @return The map of the entries in the group.
   * @internal
   */
  virtual KEntryMap internalEntryMap(const QString &pGroup) const;

  /**
   * Returns a copy of the internal map used to hold all entries.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param pGroup the group to provide a KEntryMap for.
   * @return The map of the entries in the group.
   * @internal
   */
   virtual KEntryMap internalEntryMap() const { return aEntryMap; }

  /**
   * Insert a key,value pair into the internal storage mechanism of
   * the configuration object.
   *
   * @param _key The key to insert.  It contains information both on
   *        the group of the key and the key itself. If the key already
   *        exists, the old value will be replaced.
   * @param _data the KEntry that is to be stored.
   */ 
  virtual void putData(const KEntryKey &_key, const KEntry &_data);

  /**
   * Look up an entry in the config object's internal structure.
   *
   * @param _key The key to look up  It contains information both on
   *        the group of the key and the entry's key itself.
   * @return the KEntry value (data) found for the key.  KEntry.aValue
   * will be the null string if nothing was located.
   */
  virtual KEntry lookupData(const KEntryKey &_key) const;

  /**
   * Contains all key,value entries, as well as some "special"
   * keys which indicate the start of a group of entries.  These
   * special keys will have the .key portion of their KEntryKey
   * set to QString::null.
   */
  KEntryMap aEntryMap;

private:
  /**
   * indicates whether the internal data cache is full or empty.
   * If it is empty, the config files on disk will need to be
   * reopened and read before any reading can take place.
   */
  bool isCached;

  /**
   * timer which is periodically triggered to flush out the
   * data cache.
   */
  QTimer *cacheTimer;

  /**
   * The last time a value was either read or written.  Used to
   * determine a cache time threshold.  Because lookupData() is
   * called for reads and writes, we only need to update the time
   * there; we can ignore putData().
   *
   * @see #lookupData, #putData
   */
  QTime lastIoOp;
  
  /**
   * time between flush attempts.  We initialize this to 30 seconds.
   */
  int flushInterval;

  /**
   * copy-construction and assignment are not allowed
   */
  KConfig( const KConfig& );
  /**
   * copy-construction and assignment are not allowed
   */
  KConfig& operator= ( const KConfig& rConfig );

  /**
   * Checks whether the cache is loaded from disk, and loads it if
   * needed.  Not actually a const function, so we need to do some
   * internal funny business to get around constness, but needed to be
   * const because of where it is called from (functions that really
   * *should* remain const).
   */
  void cacheCheck() const;

private slots:
  /**
   * attempts to flush the entry cache. Will set isCached
   * variable to the appropriate value upon completion.
   */
  void flushCache();

};

inline bool KConfig::hasGroup(const QString &_pGroup) const
{
  cacheCheck();

  KEntryKey groupKey = { _pGroup, QString::null };
  return aEntryMap.contains(groupKey);
}

inline bool KConfig::hasKey(const QString &pKey) const
{
  KEntryKey aEntryKey;

  cacheCheck();

  if (!locale().isNull()) {
    // try the localized key first
    QString aKey = pKey + "[";
    aKey += locale();
    aKey += "]";
    
    aEntryKey.group = group();
    aEntryKey.key = aKey;
    
    if (aEntryMap.contains(aEntryKey))
      return true;
  }

  // try the non-localized version
  aEntryKey.group = group();
  aEntryKey.key = pKey;
  return aEntryMap.contains(aEntryKey);
}

inline void KConfig::putData(const KEntryKey &_key, const KEntry &_data)
{
  cacheCheck();

  // check to see if the special group key is present,
  // and if not, put it in.
  if (!hasGroup(_key.group)) {
    KEntryKey groupKey = { _key.group, QString::null };
    aEntryMap.insert(groupKey, KEntry());
  }

  // now either add or replace the data
  KEntryMapIterator aIt = aEntryMap.find(_key);
  if (aIt != aEntryMap.end())
    aEntryMap.replace(_key, _data);
  else
    aEntryMap.insert(_key, _data);
}
  
inline KEntry KConfig::lookupData(const KEntryKey &_key) const
{
  cacheCheck();

  KEntryMapConstIterator aIt;

  aIt = aEntryMap.find(_key);
  if (aIt != aEntryMap.end())
    return *aIt;
  else {
    return KEntry();
  }
}

inline void KConfig::cacheCheck() const
{
  KConfig *that = (KConfig *)this;
  that->lastIoOp = QTime::currentTime();
  if (!isCached) {
    that->reparseConfiguration();
  }
}

#endif
