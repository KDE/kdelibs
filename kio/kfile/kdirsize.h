/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KDIRSIZE_H
#define __KDIRSIZE_H

#include <kio/job.h>
#include <kfileitem.h>

/**
 * Computes a directory size (similar to "du", but doesn't give the same results
 * since we simply sum up the dir and file sizes, whereas du speaks disk blocks)
 */
class KIO_EXPORT KDirSize : public KIO::Job
{
  Q_OBJECT
protected:
  KDirSize( const KURL & directory );
  KDirSize( const KFileItemList & lstItems );
  ~KDirSize() {}

public:
  /**
   * @return the size we found
   */
  KIO::filesize_t totalSize() const { return m_totalSize; }

  /**
   * @return the total number of files (counting symlinks to files, sockets
   * and character devices as files) in this directory and all sub-directories
   * @since 3.3
   */
  KIO::filesize_t totalFiles() const { return m_totalFiles; }

  /**
   * @return the total number of sub-directories found (not including the
   * directory the search started from and treating symlinks to directories
   * as directories)
   * @since 3.3
   */
  KIO::filesize_t totalSubdirs() const { return m_totalSubdirs; }

  /**
   * Asynchronous method. Connect to the result signal.
   * This one lists a single directory.
   */
  static KDirSize * dirSizeJob( const KURL & directory );

  /**
   * Asynchronous method. Connect to the result signal.
   * This one lists the items from @p lstItems.
   * The reason we asks for items instead of just urls, is so that
   * we directly know if the item is a file or a directory,
   * and in case of a file, we already have its size.
   */
  static KDirSize * dirSizeJob( const KFileItemList & lstItems );

  /**
   * Synchronous method - you get the result as soon as
   * the call returns.
   */
  static KIO::filesize_t dirSize( const KURL & directory );

protected:
  /**
   * @internal
   */
  void setSync() { m_bAsync = false; }

  void startNextJob( const KURL & url );

protected Q_SLOTS:

  virtual void slotResult( KIO::Job *job );
  void slotEntries( KIO::Job * , const KIO::UDSEntryList &);
  void processList();

private:
  bool m_bAsync;
  KIO::filesize_t m_totalSize;
  KIO::filesize_t m_totalFiles;
  KIO::filesize_t m_totalSubdirs;
  KFileItemList m_lstItems;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KDirSize* d;
};

#endif
