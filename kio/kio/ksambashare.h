/* This file is part of the KDE project
   Copyright (c) 2004 Jan Schaefer <j_schaef@informatik.uni-kl.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef ksambashare_h
#define ksambashare_h

#include <QtCore/QObject>

#include <kio/kio_export.h>

/**
 * Similar functionality like KFileShare, 
 * but works only for Samba and do not need 
 * any suid script.
 * Singleton class, call instance() to get an instance.
 */
class KIO_EXPORT KSambaShare : public QObject 
{
Q_OBJECT
public:
  /**
   * Returns the one and only instance of KSambaShare
   */
  static KSambaShare* instance();

  /**
   * Whether or not the given path is shared by Samba.
   * @param path the path to check if it is shared by Samba.
   * @return whether the given path is shared by Samba.
   */
  bool isDirectoryShared( const QString & path ) const;
  
  /**
   * Returns a list of all directories shared by Samba.
   * The resulting list is not sorted.
   * @return a list of all directories shared by Samba.
   */
  QStringList sharedDirectories() const;
  
  /**
   * KSambaShare destructor. 
   * Do not call!
   * The instance is destroyed automatically!
   */ 
  virtual ~KSambaShare();
  
  /**
   * Returns the path to the used smb.conf file
   * or null if no file was found
   */
  QString smbConfPath() const;
  
Q_SIGNALS:
  /**
   * Emitted when the smb.conf file has changed
   */
  void changed();  
  
private:
  KSambaShare();
  class KSambaSharePrivate;
  KSambaSharePrivate * const d;
  
  Q_PRIVATE_SLOT( d, void _k_slotFileChange(const QString&) )
};

#endif
