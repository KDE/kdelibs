/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __auto_mount_h__
#define __auto_mount_h__

#include <qobject.h>
#include <qstring.h>

namespace KIO {
class Job;
}

/**
 * This class implements synchronous mounting of devices,
 * as well as showing a file-manager window after mounting a device, optionally.
 * It is a wrapper around the asychronous @ref KIO::special() call for mount,
 * used by @ref KMimeType.
 *
 * @short This class implements synchronous mounting of devices.
 */
class KAutoMount : public QObject
{
  Q_OBJECT
  friend class gcc_gives_a_warning_without_this;
public:
  /**
   * Mounts a device.
   * @param readonly if true, the device is mounted read-only
   * @param format the file system (e.g. vfat, ext2...) [optional, fstab is used otherwise]
   * @param device the path to the device (e.g. /dev/fd0)
   * @param mountpoint the directory where to mount the device [optional, fstab is used otherwise]
   * @param desktopFile the file the user clicked on - to notify KDirWatch of the fact that
   * it should emit fileDirty for it (to have the icon change)
   * @param show_filemanager_window if true, a file-manager window for that mountpoint is shown after
   * the mount, if successful.
   */
  KAutoMount( bool readonly, const QString& format, const QString& device, const QString& mountpoint,
              const QString & desktopFile, bool show_filemanager_window = true );

signals:
  /** Emitted when the directory has been mounted */
  void finished();
  /** Emitted in case the directory could not been mounted */
  void error();

protected slots:
  void slotResult( KIO::Job * );

protected:
  QString m_strDevice;
  bool m_bShowFilemanagerWindow;
  QString m_desktopFile;
private:
  /** KAutoMount deletes itself. Don't delete it manually. */
  ~KAutoMount() {}
  class KAutoMountPrivate* d;
};

/**
 * This class implements synchronous unmounting of devices,
 * It is a wrapper around the asychronous @ref KIO::special() call for unmount,
 * used by @ref KMimeType.
 *
 * @short This class implements synchronous unmounting of devices,
 */
class KAutoUnmount : public QObject
{
  Q_OBJECT
  friend class gcc_gives_a_warning_without_this;
public:
  /**
   * Unmounts a device.
   * @param mountpoint the mount point - KAutoUnmount finds the device from that
   * @param desktopFile the file the user clicked on - to notify KDirWatch of the fact that
   * it should emit fileDirty for it (to have the icon change)
   */
  KAutoUnmount( const QString & mountpoint, const QString & desktopFile );

signals:
  /** Emitted when the directory has been unmounted */
  void finished();
  /** Emitted in case the directory could not been unmounted */
  void error();

protected slots:
  void slotResult( KIO::Job * );
private:
  QString m_desktopFile;
  QString m_mountpoint;
private:
  /** KAutoUnmount deletes itself. Don't delete it manually. */
  ~KAutoUnmount() {}
  class KAutoUnmountPrivate* d;
};

#endif
