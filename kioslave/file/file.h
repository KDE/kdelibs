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
#define __file_h__

#include "kioslave_file_export.h"
#include <kio/global.h>
#include <kio/slavebase.h>

#include <QtCore/QObject>
#include <QtCore/QHash>


#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>

class KIOSLAVE_FILE_EXPORT FileProtocol : public QObject, public KIO::SlaveBase
{
  Q_OBJECT
public:
  FileProtocol( const QByteArray &pool, const QByteArray &app);
  virtual ~FileProtocol();

  virtual void get( const KUrl& url );
  virtual void put( const KUrl& url, int _mode,
		    KIO::JobFlags _flags );
  virtual void copy( const KUrl &src, const KUrl &dest,
                     int mode, KIO::JobFlags flags );
  virtual void rename( const KUrl &src, const KUrl &dest,
                       KIO::JobFlags flags );
  virtual void symlink( const QString &target, const KUrl &dest,
                        KIO::JobFlags flags );

  virtual void stat( const KUrl& url );
  virtual void listDir( const KUrl& url );
  virtual void mkdir( const KUrl& url, int permissions );
  virtual void chmod( const KUrl& url, int permissions );
  virtual void chown( const KUrl& url, const QString& owner, const QString& group );
  virtual void setModificationTime( const KUrl& url, const QDateTime& mtime );
  virtual void del( const KUrl& url, bool isfile);
  virtual void open( const KUrl &url, QIODevice::OpenMode mode );
  virtual void read( KIO::filesize_t size );
  virtual void write( const QByteArray &data );
  virtual void seek( KIO::filesize_t offset );
  virtual void close();

  /**
   * Special commands supported by this slave:
   * 1 - mount
   * 2 - unmount
   */
  virtual void special( const QByteArray &data );
  void unmount( const QString& point );
  void mount( bool _ro, const char *_fstype, const QString& dev, const QString& point );
  bool pumount( const QString &point );
  bool pmount( const QString &dev );

private:
  bool createUDSEntry( const QString & filename, const QByteArray & path, KIO::UDSEntry & entry,
                       short int details, bool withACL );
  int setACL( const char *path, mode_t perm, bool _directoryDefault );

  QString getUserName( uid_t uid ) const;
  QString getGroupName( gid_t gid ) const;

    bool deleteRecursive(const QString& path);

private:
  mutable QHash<uid_t, QString> mUsercache;
  mutable QHash<gid_t, QString> mGroupcache;
  int openFd;
  QString openPath;
};

#endif
