/*
   Copyright (C) 2000-2002 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>
   Copyright (C) 2000-2002 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __file_h__
#define __file_h__ "$Id: file.h 591636 2006-10-02 21:47:38Z adridg $"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>

#include <qobject.h>
#include <qintdict.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kio/global.h>
#include <kio/slavebase.h>

// Note that this header file is installed, so think twice
// before breaking binary compatibility (read: it is forbidden :)

class FileProtocol : public QObject, public KIO::SlaveBase
{
  Q_OBJECT
public:
  FileProtocol( const QCString &pool, const QCString &app);
  virtual ~FileProtocol() { }

  virtual void get( const KURL& url );
  virtual void put( const KURL& url, int permissions,
		    bool overwrite, bool resume );
  virtual void copy( const KURL &src, const KURL &dest,
                     int permissions, bool overwrite );
  virtual void rename( const KURL &src, const KURL &dest,
                       bool overwrite );
  virtual void symlink( const QString &target, const KURL &dest,
                        bool overwrite );

  virtual void stat( const KURL& url );
  virtual void listDir( const KURL& url );
  virtual void mkdir( const KURL& url, int permissions );
  virtual void chmod( const KURL& url, int permissions );
  virtual void del( const KURL& url, bool isfile);

  /**
   * Special commands supported by this slave:
   * 1 - mount
   * 2 - unmount
   * 3 - shred
   */
  virtual void special( const QByteArray &data);
  void unmount( const QString& point );
  void mount( bool _ro, const char *_fstype, const QString& dev, const QString& point );
  bool pumount( const QString &point );
  bool pmount( const QString &dev );

protected slots:
  void slotProcessedSize( KIO::filesize_t _bytes );
  void slotInfoMessage( const QString & msg );

protected:

  bool createUDSEntry( const QString & filename, const QCString & path, KIO::UDSEntry & entry, 
                       short int details, bool withACL );
  int setACL( const char *path, mode_t perm, bool _directoryDefault );
  
  QString getUserName( uid_t uid );
  QString getGroupName( gid_t gid );

  QIntDict<QString> usercache;      // maps long ==> QString *
  QIntDict<QString> groupcache;

  class FileProtocolPrivate;
  FileProtocolPrivate *d;
};

#endif
