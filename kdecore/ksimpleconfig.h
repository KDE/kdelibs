/* This file is part of the KDE libraries
   Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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

#ifndef _KSIMPLECONFIG_H
#define _KSIMPLECONFIG_H

#include "kconfig.h"

/** 
 * KDE Configuration entries
 *
 * This is a trivial extension of KConfig for applications that need
 * only one configuration file and no default system.  It also adds
 * the ability to delete keys and/or groups.  Lastly, when the data in
 * memory is written back out at sync() time, it is not merged with
 * what is on disk -- whatever is in memory simply replaces what is on
 * disk entirely.
 *
 * @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
 * @version $Id$
 * @see KApplication::getConfig KConfigBase KConfig
 * @short KDE Configuration Management class with deletion ability 
 */
class KSimpleConfig : public KConfig
{
  Q_OBJECT 

public:
  /** 
   * Construct a KSimpleConfig object and make it either read-write
   * or read-only.  
   *
   * @param pFileName The file used for saving the config data. Either
   *                  a full path can be specified or just the filename. 
   *                  If only a flename is specified, the default 
   *                  directory for "config" files is used.
   * @param bReadOnly Whether the object should be read-only.
   */
  KSimpleConfig( const QString& pFileName, bool bReadOnly = false );

  /** 
   * Destructor. 
   *
   * Writes back any dirty configuration entries.
   */
  virtual ~KSimpleConfig();

  /**
   * Delete a configuration entry.
   *
   * @param pKey The key of the entry to delete
   * @param bLocalized Whether the localized or the non-localized key should
   *                    be deleted
   * @return The old value of that key.
   */
  QString deleteEntry( const QString& pKey, bool bLocalized );

  /**
   * Delete a configuration entry group
   *
   * If the group is not empty and bDeep is false, nothing gets
   * deleted and false is returned.
   * If this group is the current group and it is deleted, the
   * current group is undefined and should be set with setGroup()
   * before the next operation on the configuration object.
   *
   * @param pGroup The name of the group
   * @param bDeep Whether non-empty groups should be completely
   *        deleted (including their entries).
   * @return If the group does not exist or is not empty and bDeep is
   *         false, deleteGroup returns false.
   */
  bool deleteGroup( const QString& pGroup, bool bDeep = true );

  virtual void sync();

private:

  // copy-construction and assignment are not allowed
  KSimpleConfig( const KSimpleConfig& );
  KSimpleConfig& operator= ( const KSimpleConfig& rConfig );

};
  
#endif




