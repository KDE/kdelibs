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

#include <klockfile.h>
#include <kconfigbase.h>
#include <kconfiggroup.h>
#include <kconfigdata.h>
#include <kglobal.h>

/**
* Access KDE Configuration entries.
*
* This class implements KDE's default configuration system.
*
* @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
* @see KGlobal::config(), KConfigBase, KConfigGroup
* @short KDE Configuration Management class
*/
class KDECORE_EXPORT KConfig : public KConfigBase
{
public:
    enum OpenFlags {
        NoGlobals = 0x01,
        OnlyLocal = 0x02,
        IncludeGlobals= 0x04
    };

  /**
   * Constructs a KConfig object.
   *
   * @param fileName A file to parse in addition to the
   *        system-wide file(s).  If it is not provided, only global
   *        KDE configuration data will be read (depending on the value of
   *        @p flags).
   * @param flags determines how the configuration file is created and accessed. Passing in
   *        IncludeGlobals will result in entries in kdeglobals being parsed and included in the
   *        merged settings
   */
  explicit KConfig( const QString& fileName = QString(),
                    OpenFlags flags = IncludeGlobals );

  /**
   * Constructs a KConfig object.
   *
   * @param resType the place to look in (config, data, etc) See KStandardDirs.
   * @param fileName A file to parse in addition to the
   *        system-wide file(s).  If it is not provided, only global
   *        KDE configuration data will be read (depending on the value of
   *        @p flags).
   * @param flags determines how the configuration file is created and accessed. Passing in
   *        IncludeGlobals will result in entries in kdeglobals being parsed and included in the
   *        merged settings
   */
  explicit KConfig( const char *resType,
                    const QString& fileName,
                    OpenFlags flags = IncludeGlobals );

  /**
   * Constructs a KConfig object.
   *
   * @param componentData The KComponentData object of your component. If you don't know what this
   *        is you likely want to use the above constructor.
   * @param fileName A file to parse in addition to the
   *        system-wide file(s).  If it is not provided, only global
   *        KDE configuration data will be read (depending on the value of
   *        @p flags).
   * @param flags determines how the configuration file is created and accessed. Passing in
   *        IncludeGlobals will result in entries in kdeglobals being parsed and included in the
   *        merged settings
   * @param resType the place to look in (config, data, etc) See KStandardDirs.
   */
  explicit KConfig(const KComponentData &componentData, const QString &fileName = QString(),
                   OpenFlags flags = IncludeGlobals, const char *resType="config");

  explicit KConfig(KConfigBackEnd *backEnd);

  /**
   * Destructs the KConfig object.
   *
   * Writes back any dirty configuration entries, and destroys
   * dynamically created objects.
   */
  virtual ~KConfig();

  /**
   * Gets the extraConfigFiles in the merge stack.
   */
  QStringList extraConfigFiles() const;

  /**
   * Sets the merge stack to the list of files. The stack is last in first out with
   * the top of the stack being the most specific config file.
   * @param files A list of extra config files containing the full path of the
   * local config file to remove from the stack.
   */
  void setExtraConfigFiles(const QStringList &files);

  /**
   * Remove all files from merge stack. This does not include the local file that
   * was specified in the constructor.
   */
  void removeAllExtraConfigFiles();

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
  void setForceGlobal( bool force );

  /**
   * Returns true if all entries are being written into kdeglobals.
   * @return true if all entries are being written into kdeglobals
   * @see setForceGlobal
   */
  bool forceGlobal() const;

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
   */
  KLockFile::Ptr lockFile( bool bGlobal=false );

  KDE_DEPRECATED QString group() const;
  KConfigGroup group( const char* s );
  KConfigGroup group( const QByteArray &b);
  KConfigGroup group( const QString &str);
  const KConfigGroup group( const QByteArray &arr) const;

protected:
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

private:
  KConfig( const QString& fileName, bool fal);

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
  virtual KEntryMap internalEntryMap() const;

  /** Virtual hook, used to add new "virtual" functions while maintaining
      binary compatibility. Unused in this class.
  */
  virtual void virtual_hook( int id, void* data );
private:
  class Private;
  Private * const d;
};

#endif
