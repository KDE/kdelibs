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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCONFIG_H
#define KCONFIG_H

#include <kconfigbase.h>
#include <kconfigdata.h>
#include <klockfile.h>

/**
* Access KDE Configuration entries.
*
* This class implements KDE's default configuration system.
*
* @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
* @see KGlobal::config(), KConfigBase, KSimpleConfig
* @short KDE Configuration Management class
*/
class KDECORE_EXPORT KConfig : public KConfigBase
{
  Q_OBJECT

public:

  /**
   * Constructs a KConfig object.
   *
   * @param fileName A file to parse in addition to the
   *        system-wide file(s).  If it is not provided, only global
   *        KDE configuration data will be read (depending on the value of
   *        @p bUseKDEGlobals).
   * @param bReadOnly Set the config object's read-only status. Note that the
   *        object will automatically become read-only if either the user does not have
   *        write permission to @p fileName or if no file was specified.
   * @param bUseKDEGlobals Toggle reading the global KDE configuration file.
   * @param resType the place to look in (config, data, etc) See KStandardDirs.
   */
  KConfig( const QString& fileName = QString::null,
          bool bReadOnly = false, bool bUseKDEGlobals = true, const char *resType="config");

  KConfig(KConfigBackEnd *backEnd, bool bReadOnly = false);

  /**
   * Destructs the KConfig object.
   *
   * Writes back any dirty configuration entries, and destroys
   * dynamically created objects.
   */
  virtual ~KConfig();

  /**
   * Clears all entries out of the @p dirtyEntryMap, so the
   * values will not be written to disk on a later call to
   * sync().
   *
   * @param bDeep If true, the dirty map is actually emptied.
   *        otherwise, the config object's global dirty flag is set to
   *        false, but the dirty entries remain in the dirty entry
   *        map.
   *
   * @see KConfigBase::rollback
   */
  virtual void rollback(bool bDeep = true);


  /**
   * Returns a list of groups that are known.
   * @return a list of of groups
   */
  virtual QStringList groupList() const;

  /**
   * Returns a map (tree) of entries for all entries in a particular
   * group.
   *
   *  Only the actual entry string is returned, none of the
   * other internal data should be included.
   *
   * @param pGroup A group to get keys from.
   * @return A map of entries in the group specified, indexed by key.
   *         The returned map may be empty if the group is not found.
   */
  virtual QMap<QString, QString> entryMap(const QString &pGroup) const;

  /**
   * Clears all internal data structures and then reread
   * configuration information from disk.
   */
  virtual void reparseConfiguration();

  /**
   * Set the file mode for newly created files.
   *
   * @param mode the mode for new files as described in chmod(2)
   * @see man:chmod(2) for a description of @p mode
   */
  void setFileWriteMode(int mode);

  /**
   * Forces all following write-operations being performed on kdeglobals,
   * independent of the bGlobal flag in writeEntry().
   * @param force true to force writing in kdeglobals
   * @see forceGlobal
   */
  void setForceGlobal( bool force ) { bForceGlobal = force; }

  /**
   * Returns true if all entries are being written into kdeglobals.
   * @return true if all entries are being written into kdeglobals
   * @see setForceGlobal
   */
  bool forceGlobal() const { return bForceGlobal; }

  /**
   * Checks whether the config file contains the update @p id
   * as contained in @p updateFile. If not, it runs kconf_update
   * to update the config file.
   *
   * If you install config update files with critical fixes
   * you may wish to use this method to verify that a critical
   * update has indeed been performed to catch the case where
   * a user restores an old config file from backup that has
   * not been updated yet.
   * @param id the update to check
   * @param updateFile the file containing the update
   * @since 3.1
   */
  void checkUpdate(const QString &id, const QString &updateFile);

  /**
   * Copies all entries from this config object to a new config
   * object that will save itself to @p file.
   *
   * Actual saving to @p file happens when the returned object is
   * destructed or when sync() is called upon it.
   *
   * @param file the new config object will save itself to.
   * @param config optional config object to reuse
   * @since 3.2
   */
  KConfig* copyTo(const QString &file, KConfig *config=0) const;

  /**
   * Returns a lock file object for the configuration file or 0 if
   * the backend does not support locking.
   * @param bGlobal if true, return the lock file for the global config file
   *
   * NOTE: KConfig::sync() requires a lock on both the normal and global
   * config file. When calling KConfig::sync() while having a lock on the
   * global config file, the normal config file MUST be locked AS WELL and the 
   * normal config file MUST be locked BEFORE the global config file!
   * Otherwise there is a risk of deadlock.
   * @since 3.3
   */
  KLockFile::Ptr lockFile( bool bGlobal=false );

protected:

  /**
   * Returns true if the specified group is known.
   *
   * @param group The group to search for.
   * @returns true if the group exists.
   */
  virtual bool internalHasGroup(const QByteArray &group) const;

  /**
   * @internal
   * Returns a map (tree) of the entries in the specified group.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param pGroup the group to provide a KEntryMap for.
   * @return The map of the entries in the group.
   */
  virtual KEntryMap internalEntryMap(const QString &pGroup) const;

  /**
   * @internal
   * Returns a copy of the internal map used to hold all entries.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @return The map of the entries in the group.
   */
   virtual KEntryMap internalEntryMap() const { return aEntryMap; }

  /**
   * Inserts a (key, value) pair into the internal storage mechanism of
   * the configuration object.
   *
   * @param _key The key to insert.  It contains information both on
   *        the group of the key and the key itself. If the key already
   *        exists, the old value will be replaced.
   * @param _data the KEntry that is to be stored.
   * @param _checkGroup When false, assume that the group already exists.
   */
  virtual void putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup=true);

  /**
   * Looks up an entry in the config object's internal structure.
   *
   * @param _key The key to look up  It contains information both on
   *        the group of the key and the entry's key itself.
   * @return the KEntry value (data) found for the key.  KEntry.aValue
   * will be the null string if nothing was located.
   */
  virtual KEntry lookupData(const KEntryKey &_key) const;

  /**
   * Contains all key,value entries, as well as some "special"
   * keys which indicate the start of a group of entries.
   *
   * These special keys will have the .key portion of their KEntryKey
   * set to QString::null.
   */
  KEntryMap aEntryMap;

private:
  /**
   * @internal
   * copy-construction and assignment are not allowed
   */
  KConfig( const KConfig& );
  /**
   * @internal
   * copy-construction and assignment are not allowed
   */
  KConfig& operator= ( const KConfig& rConfig );

private:
  bool bGroupImmutable : 1; // Current group is immutable.
  bool bFileImmutable  : 1; // Current file is immutable.
  bool bForceGlobal    : 1; // Apply everything to kdeglobals.
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class Private;
  Private *d;
};

/**
 * KConfig variant using shared memory
 *
 * KSharedConfig provides a reference counted, shared memory variant
 * of KConfig.
 */
class KDECORE_EXPORT KSharedConfig : public KConfig, public KShared
{
public:
  typedef KSharedPtr<KSharedConfig> Ptr;

public:  
  /**
   * Returns a ref-counted pointer to a shared read-write config object.
   * @param fileName the name of the file to use for the configuration
   * @param immutable if true, force the config object to be read-only.
   * @param bUseKDEGlobals Toggle reading the global KDE configuration file.
   */
  static KSharedConfig::Ptr openConfig(const QString& fileName, bool immutable = false,
    bool bUseKDEGlobals = true);

private:
  KSharedConfig( const QString& fileName, bool immutable, bool useKDEGlobals );
  ~KSharedConfig();

  static QList<KSharedConfig*> *s_list;
};

#endif
