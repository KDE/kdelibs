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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KCONFIGBACKEND_H
#define _KCONFIGBACKEND_H

#include "kconfigdata.h"

class KConfigBase;
class QFile;

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
 * @version $Id$
 * @short KDE Configuration file loading/saving abstract base class 
 */
class KConfigBackEnd
{
public:
  /**
   * Construct a configuration back end.
   *
   * @param _config Specifies the configuration object which values
   *        will be passed to as they are read, or from where values
   *        to be written to will be obtained from.
   * @param _globalFileName The name of the file in which "global" config
   *        data is stored.  This is usually the apps system-wide
   *        config file.
   * @param _localFileName The name of the file in which "local" config
   *        data is stored.  This is ually the user-specific config file
   *        for the application.
   * @param _useKderc If true, the user's system-wide kderc file will be
   *        imported into the config object.  If false, only the filenames
   *        specified will be dealt with.
   */
  KConfigBackEnd(KConfigBase *_config, const QString &_globalFileName,
		 const QString &_localFileName, bool _useKderc);
  
  /**
   * Destructor.
   */
  virtual ~KConfigBackEnd() {};

  /** 
   * Parse all configuration files for a configuration object.  This
   * method must be reimplemented by the derived classes.
   * 
   * @returns Whether or not parsing was successful.  
   */
  virtual bool parseConfigFiles() = 0;

  /**
   * Write configuration data to file(s).  This method must be
   * reimplemented by the derived classes.
   *
   * @param bMerge Specifies whether the old config file already
   *        on disk should be merged in with the data in memory.  If true,
   *        data is read off the disk and merged.  If false, the on-disk
   *        file is removed and only in-memory data is written out.
   */
  virtual void sync(bool bMerge = true) = 0;

  /**
   * change the filenames associated with this back end.  You should
   * probably reparse your config info after doing this.
   */
  void changeFileNames(const QString &_globalFileName, 
		       const QString &_localFileName,
		       bool _useKderc) 
    { aGlobalFileName = _globalFileName; aLocalFileName = _localFileName;
    useKderc = _useKderc; }

protected:
  KConfigBase *pConfig;

  QString aGlobalFileName;
  QString aLocalFileName;
  bool useKderc;
};

/**
 * Class for KDE INI-style configuration file loading/saving.
 *
 * @author Preston Brown <pbrown@kde.org>, 
 *         Matthias Kalle Dalheimer <kalle@kde.org>
 * @version $Id$
 */
class KConfigINIBackEnd : public KConfigBackEnd
{

public:
  /**
   * construct a configuration backend.
   *
   * @param _config Specifies the configuration object which values
   *        will be passed to as they are read, or from where values
   *        to be written to will be obtained from.
   * @param _globalFileName The name of the file in which "global" config
   *        data is stored.  This is usually the apps system-wide
   *        config file.
   * @param _localFileName The anem of the file in which "local" config
   *        data is stored.  This is ually the user-specific config file
   *        for the application.
   * @param _useKderc If true, the user's system-wide kderc file will be
   *        imported into the config object.  If false, only the filenames
   *        specified will be dealt with.
   */
  KConfigINIBackEnd(KConfigBase *_config, const QString &_globalFileName,
		    const QString &_localFileName, bool _useKderc = true)
    : KConfigBackEnd(_config, _globalFileName, _localFileName, _useKderc) {}
  
  /**
   * Destructor.
   */
  virtual ~KConfigINIBackEnd() {};

  /**
   * Parse all INI-style configuration files for a config object.
   *
   * @returns Whether or not parsing was successful.
   */
  bool parseConfigFiles();

  /**
   * write configuration data to file(s). 
   */
  virtual void sync(bool bMerge = true);
  
protected:
  /**
   * Parse one configuration file.
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
   */
  void parseSingleConfigFile(QFile& rFile, KEntryMap *pWriteBackMap = 0L,
			     bool bGlobal = false);

  /**
   * Write configuration file back.
   *
   * @param rFile The name of the file to write.
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
  bool writeConfigFile(QFile& rFile, bool bGlobal = false, bool bMerge = true);
};

#endif
