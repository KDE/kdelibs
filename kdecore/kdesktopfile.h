/* This file is part of the KDE libraries
   Copyright (c) 1999 Pietro Iglio <iglio@kde.org>

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

#ifndef _KDESKTOPFILE_H
#define _KDESKTOPFILE_H

#include "kconfig.h"

/** 
 * KDE Desktop File Management
 *
 * @author Pietro Iglio <iglio@kde.org>
 * @version $Id$
 * @see KConfigBase KConfig
 * @short KDE Desktop File Management class
 */
class KDesktopFile : public KConfig
{
  Q_OBJECT 

public:
  /** 
   * Construct a KDesktopFile object and make it either read-write
   * or read-only.  
   *
   * @param pFileName The file used for saving the data. The
   *                  full path must be specified.
   * @param bReadOnly Whether the object should be read-only.
   * @param resType   allows you to change what sort of resource
   *                  to search for if pFileName is not absolute.  For
   *                  instance, you might want to specify "config".
   */
  KDesktopFile( const QString &pFileName, bool bReadOnly = false,
		const QString &resType = "apps");

  /** 
   * Destructor. 
   *
   * Writes back any dirty configuration entries.
   */
  virtual ~KDesktopFile();

  /**
   * Returns true if the file appears to be a desktop file.
   * The check is performed looking at the file extension (the file is not
   * open).
   * Currently, valid extensions are ".kdelnk" and ".desktop".
   * 
   */
  static bool isDesktopFile(const QString& path);

  /**
   * Returns the value of the "Type=" entry.
   */
  QString readType();

  /**
   * Returns the value of the "Icon=" entry.
   */
  QString readIcon();

  /**
   * Returns the value of the "MiniIcon=" entry.
   */
  QString readMiniIcon();

  /**
   * Returns the value of the "Name=" entry.
   */
  QString readName();

  /**
   * Returns the value of the "Comment=" entry.
   */
  QString readComment();

  /**
   * Returns the value of the "Path=" entry.
   */
  QString readPath();

  /**
   * Returns the value of the "URL=" entry.
   */
  QString readURL();

  /**
   * Returns true if there is an entry "Type=Link".
   * The link points to the "URL=" entry.
   */
  bool hasLinkType();

  /**
   * Returns true if there is an entry "Type=Application".
   */
  bool hasApplicationType();

  /**
   * Returns true if there is an entry "Type=MimeType".
   */
  bool hasMimeTypeType(); // funny name :)

  /**
   * Returns true if there is an entry "Type=FSDev".
   */
  bool hasDeviceType();

private:

  // copy-construction and assignment are not allowed
  KDesktopFile( const KDesktopFile& );
  KDesktopFile& operator= ( const KDesktopFile& );

};

  
#endif

