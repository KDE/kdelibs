/* This file is part of the KDE libraries
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
//
// $Log$
// Revision 1.6.4.1  1999/02/14 02:06:06  granroth
// Converted a lot of 'const char*' to 'QString'.  This compiles... but
// it's entirely possible that nothing will run linked to it :-P
//
// Revision 1.6  1998/01/18 14:39:02  kulow
// reverted the changes, Jacek commited.
// Only the RCS comments were affected, but to keep them consistent, I
// thought, it's better to revert them.
// I checked twice, that only comments are affected ;)
//
// Revision 1.4  1998/01/15 13:22:31  kalle
// Read-only mode for KSimpleConfig
//
// Revision 1.3  1997/10/16 11:15:02  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.2  1997/10/08 19:28:53  kalle
// KSimpleConfig implemented
//
// Revision 1.1  1997/10/04 19:51:07  kalle
// new KConfig
//

#ifndef _KSIMPLECONFIG_H
#define _KSIMPLECONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kconfigbase.h>

/** 
* KDE Configuration entries
*
* This is a trivial implementation of KConfigBase for applications
* that need only one configuration file and no default system.
*
* @author Kalle Dalheimer (kalle@kde.org)
* @version $Id$
* @see KApplication::getConfig KConfigBase KConfig
* @short KDE Configuration Management class
*/
class KSimpleConfig : public KConfigBase
{
  Q_OBJECT 

  // copy-construction and assignment are not allowed
  KSimpleConfig( const KSimpleConfig& );
  KSimpleConfig& operator= ( const KSimpleConfig& rConfig );

protected:
  /** Open all appropriate configuration files and pass them on to
	* parseOneConfigFile()
	*/
  virtual void parseConfigFiles();

public:
  /** 
	* Construct a read-write KSimpleConfig object. 
	*
	* @param pfile The file used for saving the config data.
	*/
  KSimpleConfig( const QString& pFile );

  /**
	* Construct a KSimpleConfig object and make it either read-write
	* or read-only.  
	*
	* @param pFile The file uses for saving the config data.
	*        bReadOnly Whether the object should be read-only.
	*/
  KSimpleConfig( const QString& pFile, bool bReadOnly );

  /** 
	* Destructor. 
	*
	* Writes back any dirty configuration entries.
	*/
  virtual ~KSimpleConfig();

  
  /**
	* Returns true if the object is read-only
	*
	*/
  bool isReadOnly() const { return data()->bReadOnly; }

  /**
   * Write back the cache.
   *
   */
  virtual void sync();

  /** Write back the configuration data.
	*/
  bool writeConfigFile( QFile& rFile, bool bGlobal = false );

  /**
	* Delete a configuration entry.
	*
	* @param pKey The key of the entry to delete
	* @param bLocalized Whether the localized or the non-localized key should
	*                    be deleted
	* @return The old value of that key.
	*/
  const QString deleteEntry( const QString& pKey, bool bLocalized );

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
	* deleted (including their entries)
	* @return If the group does not exist or is not empty and bDeep is
	* false, deleteGroup returns false.
	*/
  bool deleteGroup( const QString& pGroup, bool bDeep = true );
};
  

 
#endif
