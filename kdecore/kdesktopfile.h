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

class KDesktopFilePrivate;

/** 
 * KDE Desktop File Management.
 *
 * @author Pietro Iglio <iglio@kde.org>
 * @version $Id$
 * @see  KConfigBase  KConfig
 * @short KDE Desktop File Management class
 */
class KDesktopFile : public KConfig
{
  Q_OBJECT

public:
  /**
   * Constructs a KDesktopFile object and make it either read-write
   * or read-only.
   *
   * @param fileName  The name or path of the desktop file. If it
   *                  is not absolute, it will be located
   *                  using the resource type @p resType.
   * @param readOnly  Whether the object should be read-only.
   * @param resType   Allows you to change what sort of resource
   *                  to search for if @p fileName is not absolute.  For
   *                  instance, you might want to specify "config".
   */
  KDesktopFile( const QString &fileName, bool readOnly = false,
		const char * resType = "apps");

  /**
   * Destructs the KDesktopFile object.
   *
   * Writes back any dirty configuration entries.
   */
  virtual ~KDesktopFile();

  /**
   * Checks whether this is really a desktop file.
   *
   * The check is performed looking at the file extension (the file is not
   * opened).
   * Currently, valid extensions are ".kdelnk" and ".desktop".
   * @return @p true if the file appears to be a desktop file.
   */
  static bool isDesktopFile(const QString& path);

  /**
   * Returns the value of the "Type=" entry.
   */
  QString readType() const;

  /**
   * Returns the value of the "Icon=" entry.
   */
  QString readIcon() const;

  /**
   * Returns the value of the "Name=" entry.
   */
  QString readName() const;

  /**
   * Returns the value of the "Comment=" entry.
   */
  QString readComment() const;

  /**
   * Returns the value of the "GenericName=" entry.
   */
  QString readGenericName() const;

  /**
   * Returns the value of the "Path=" entry.
   * @deprecated
   */
  QString readPath() const;

  /**
   * Returns the value of the "Dev=" entry.
   */
  QString readDevice() const;

  /**
   * Returns the value of the "URL=" entry.
   */
  QString readURL() const;

  /**
   * Returns a list of the "Actions=" entries.
   */
  QStringList readActions() const;

  /**
   * Sets the desktop action group.
   */
  void setActionGroup(const QString &group);

  /**
   * Returns true if the action group exists, false otherwise
   */
  bool hasActionGroup(const QString &group) const;

  /**
   * Checks whether there is a "Type=Link" entry.
   *
   * The link points to the "URL=" entry.
   */
  bool hasLinkType() const;

  /**
   * Checks whether there is an entry "Type=Application".
   */
  bool hasApplicationType() const;

  /**
   * Checks whether there is an entry "Type=MimeType".
   */
  bool hasMimeTypeType() const; // funny name :)

  /**
   * Checks whether there is an entry "Type=FSDev".
   */
  bool hasDeviceType() const;

  /**
   * Checks whether the TryExec field contains a binary
   * which is found on the local system.
   */
  bool tryExec() const;

  /**
   * @return The filename as passed to the constructor.
   */
  QString fileName() const;

  /**
   * @return The resource type as passed to the constructor.
   */
  QString resource() const;

  /**
   * @return The value of the "DocPath=" entry.
   */
  QString readDocPath() const;

  QStringList sortOrder() const;


#ifdef KDE_NO_COMPAT
private:
#endif
  /**
   * @deprecated Use @see fileName() instead.
   */
    QString filename() const { return fileName(); };

private:

  // copy-construction and assignment are not allowed
  KDesktopFile( const KDesktopFile& );
  KDesktopFile& operator= ( const KDesktopFile& );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KDesktopFilePrivate *d;
};


#endif

