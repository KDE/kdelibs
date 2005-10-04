/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Portions copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KCONFIGBACKEND_H
#define _KCONFIGBACKEND_H

#include <kconfigbase.h>
#include <klockfile.h>
#include "kdelibs_export.h"
#include <qbytearray.h> // TODO: move data to Private class and use a forward declaration

class QFile;
class KConfigBackEndPrivate;

/**
 * Abstract base class for KDE configuration file loading/saving.
 *
 * This class forms the base for all classes that implement some
 * manner of loading/saving to configuration files.  It is an
 * abstract base class, meaning that you cannot directly instantiate
 * objects of this class. As of right now, the only back end available
 * is one to read/write to INI-style files, but in the future, other
 * formats may be available, such as XML or a database.
 *
 * @author Preston Brown <pbrown@kde.org>,
 *         Matthias Kalle Dalheimer <kalle@kde.org>
 * @short KDE Configuration file loading/saving abstract base class
 */
class KDECORE_EXPORT KConfigBackEnd
{
  friend class KConfig;
  friend class KSharedConfig;
public:
  /**
   * Constructs a configuration back end.
   *
   * @param _config Specifies the configuration object which values
   *        will be passed to as they are read, or from where values
   *        to be written to will be obtained from.
   * @param _fileName The name of the file in which config
   *        data is stored.  All registered configuration directories
   *        will be looked in in order of decreasing relevance.
   * @param _resType the resource type of the fileName specified, _if_
   *        it is not an absolute path (otherwise this parameter is ignored).
   * @param _useKDEGlobals If true, the user's system-wide kdeglobals file
   *        will be imported into the config object.  If false, only
   *        the filename specified will be dealt with.
   */
  KConfigBackEnd(KConfigBase *_config, const QString &_fileName,
		 const char * _resType, bool _useKDEGlobals);

  /**
   * Destructs the configuration backend.
   */
  virtual ~KConfigBackEnd();

  /**
   * Parses all configuration files for a configuration object.  This
   * method must be reimplemented by the derived classes.
   *
   * @returns Whether or not parsing was successful.
   */
  virtual bool parseConfigFiles() = 0;

  /**
   * Writes configuration data to file(s).  This method must be
   * reimplemented by the derived classes.
   *
   * @param bMerge Specifies whether the old config file already
   *        on disk should be merged in with the data in memory.  If true,
   *        data is read off the disk and merged.  If false, the on-disk
   *        file is removed and only in-memory data is written out.
   */
  virtual void sync(bool bMerge = true) = 0;

  /**
   * Changes the filenames associated with this back end.  You should
   * probably reparse your config info after doing this.
   *
   * @param _fileName the new filename to use
   * @param _resType the resource type of the fileName specified, _if_
   *        it is not an absolute path (otherwise this parameter is ignored).
   * @param _useKDEGlobals specifies whether or not to also parse the
   *        global KDE configuration files.
   */
  void changeFileName(const QString &_fileName, const char * _resType,
		      bool _useKDEGlobals);

  /**
   * Returns the state of the app-config object.
   *
   * @see KConfig::getConfigState
   */
  virtual KConfigBase::ConfigState getConfigState() const
    { return mConfigState; }

  /**
   * Returns the filename as passed to the constructor.
   * @return the filename as passed to the constructor.
   */
  QString fileName() const { return mfileName; }

  /**
   * Returns the resource type as passed to the constructor.
   * @return the resource type as passed to the constructor.
   */
  const char * resource() const { return resType; }

  /**
   * Set the locale string that defines the current language.
   * @param _localeString the identifier of the language
   * @see KLocale
   */
  void setLocaleString(const QByteArray &_localeString) { localeString = _localeString; }

  /**
   * Set the file mode for newly created files.
   * @param mode the filemode (as in chmod)
   */
  void setFileWriteMode(int mode);

  /**
   * Check whether the config files are writable.
   * @param warnUser Warn the user if the configuration files are not writable.
   * @return Indicates that all of the configuration files used are writable.
   * @since 3.2
   */
  bool checkConfigFilesWritable(bool warnUser);

  /**
   * Returns a lock file object for the configuration file
   * @param bGlobal If true, returns a lock file object for kdeglobals
   * @since 3.3
   */
  KLockFile::Ptr lockFile( bool bGlobal = false );

protected:
  KConfigBase *pConfig;

  QString mfileName;
  QByteArray resType;
  bool useKDEGlobals : 1;
  bool bFileImmutable : 1;
  QByteArray localeString;
  QString mLocalFileName;
  QString mGlobalFileName;
  KConfigBase::ConfigState mConfigState;
  int mFileMode;

protected:
  virtual void virtual_hook( int id, void* data );
protected:
  class KConfigBackEndPrivate;
  KConfigBackEndPrivate *d;
};


/**
 * Class for KDE INI-style configuration file loading/saving.
 *
 * @author Preston Brown <pbrown@kde.org>,
 *         Matthias Kalle Dalheimer <kalle@kde.org>
 */
class KDECORE_EXPORT KConfigINIBackEnd : public KConfigBackEnd
{

public:
  /**
   * Constructs an ini-style configuration back end.
   *
   * @param _config Specifies the configuration object which values
   *        will be passed to as they are read, or from where values
   *        to be written to will be obtained from.
   * @param _fileName The name of the file in which config
   *        data is stored.  All registered configuration directories
   *        will be looked in in order of decreasing relevance.
   * @param _resType the resource type of the fileName specified, _if_
   *        it is not an absolute path (otherwise this parameter is ignored).
   * @param _useKDEGlobals If true, the user's system-wide kdeglobals file
   *        will be imported into the config object.  If false, only
   *        the filename specified will be dealt with.
   */
  KConfigINIBackEnd(KConfigBase *_config, const QString &_fileName,
		    const char * _resType, bool _useKDEGlobals = true)
    : KConfigBackEnd(_config, _fileName, _resType, _useKDEGlobals) {}

  /**
   * Destructs the configuration backend.
   */
  virtual ~KConfigINIBackEnd() {}

  /**
   * Parses all INI-style configuration files for a config object.
   *
   * @returns Whether or not parsing was successful.
   */
  bool parseConfigFiles();

  /**
   * Writes configuration data to file(s).
   * @param bMerge Specifies whether the old config file already
   *        on disk should be merged in with the data in memory.  If true,
   *        data is read off the disk and merged.  If false, the on-disk
   *        file is removed and only in-memory data is written out.
   */
  virtual void sync(bool bMerge = true);

protected:
  /**
   * Parses one configuration file.
   *
   * @param rFile The configuration file to parse
   * @param pWriteBackMap If specified, points to a KEntryMap where
   *        the data read from the file should be stored, instead of
   *        inserting them directly into the configuration object.
   *        Use this area as a "scratchpad" when you need to know what is
   *        on disk but don't want to effect the configuration object.
   * @param bGlobal Specifies whether entries should be marked as
   *        belonging to the global KDE configuration file rather
   *        than the application-specific KDE configuration file(s).
   * @param bDefault Specifies whether entries should be marked as
   *        being default values.
   */
  void parseSingleConfigFile(QFile& rFile, KEntryMap *pWriteBackMap = 0L,
			     bool bGlobal = false, bool bDefault = false);

  /**
   * Writes configuration file back.
   *
   * @param filename The name of the file to write.
   * @param bGlobal Specifies whether to write only entries which
   *        are marked as belonging to the global KDE config file.
   *        If this is false, it skips those entries.
   * @param bMerge Specifies whether the old config file already
   *        on disk should be merged in with the data in memory.  If true,
   *        data is read off the disk and merged.  If false, the on-disk
   *        file is removed and only in-memory data is written out.
   * @return Whether some entries are left to be written to other
   *         files.
   */
  bool writeConfigFile(const QString &filename, bool bGlobal = false, bool bMerge = true);

  /** Get the entry map.
   *
   * @param map the entries will be stored in this object.
   * @param bGlobal Specifies whether to get only entries which
   *        are marked as belonging to the global KDE config file.
   *        If this is false, it skips those entries.
   * @param mergeFile if not null, the dirty entries for this file will
   * be merged.
   *
   * @return Whether there will be some entries left for writing to other
   * files.
   */
  bool getEntryMap(KEntryMap &map, bool bGlobal, QFile *mergeFile);

  /** Write the entries in @e aTempMap to the file stream.*/
  void writeEntries(FILE *pStream, const KEntryMap &aTempMap);

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KConfigINIBackEndPrivate;
  KConfigINIBackEndPrivate *not_d;
};

#endif
