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
// Revision 1.9  1999/04/08 01:39:40  torben
// Torben: Corrected API and docu
//
// Revision 1.8  1999/03/01 23:33:19  kulow
// CVS_SILENT ported to Qt 2.0
//
// Revision 1.7.4.1  1999/02/14 02:05:41  granroth
// Converted a lot of 'const char*' to 'QString'.  This compiles... but
// it's entirely possible that nothing will run linked to it :-P
//
// Revision 1.7  1998/01/18 14:38:48  kulow
// reverted the changes, Jacek commited.
// Only the RCS comments were affected, but to keep them consistent, I
// thought, it's better to revert them.
// I checked twice, that only comments are affected ;)
//
// Revision 1.5  1997/12/12 14:46:01  denis
// Reverting to lower case true/false
//
// Revision 1.4  1997/12/12 14:33:45  denis
// replaced true by TRUE and false by FALSE
//
// Revision 1.3  1997/10/21 20:44:44  kulow
// removed all NULLs and replaced it with 0L or "".
// There are some left in mediatool, but this is not C++
//
// Revision 1.2  1997/10/16 11:14:30  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.1  1997/10/04 19:50:58  kalle
// new KConfig
//

#ifndef _KCONFIG_H
#define _KCONFIG_H

#include <kconfigbase.h>

/** 
* KDE Configuration entries
*
* This class implements KDE's default-based configuration system.
*
* @author Kalle Dalheimer (kalle@kde.org)
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
   * @param pGlobalAppFile A file to parse in addition to the
   *  normally parsed files.  
   * @param pLocalAppFile Another file to parse in addition to the
   * normally parsed files (has priority over pGlobalAppFile 
   */
  KConfig( const QString& pGlobalAppFile = QString::null, 
	   const QString& pLocalAppFile = QString::null );

  /** 
   * Destructor. 
   *
   * Writes back any dirty configuration entries.
   */
  virtual ~KConfig();

  /**
   * Write back the config cache.
   */
  virtual void sync();

protected:
  /**
   * copy-construction and assignment are not allowed
   */
  KConfig( const KConfig& );
  /**
   * copy-construction and assignment are not allowed
   */
  KConfig& operator= ( const KConfig& rConfig );

  /**
   * Open all appropriate configuration files and pass them on to
   * parseOneConfigFile()
   */
  virtual void parseConfigFiles();

  /**
   * Write back the configuration data.
   */
  bool writeConfigFile( QFile& rFile, bool bGlobal = false );
};
  


#endif
