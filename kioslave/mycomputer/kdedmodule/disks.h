/*
 * disks.h
 *
 * Copyright (c) 1998 Michael Kropfberger <michael.kropfberger@gmx.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifndef __DISKS_H__
#define __DISKS_H__

#include <qobject.h>
#include <qstring.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qfile.h>

#include <kio/global.h>
#include <kprogress.h>
#include <kprocess.h>
#include <klocale.h>

class DiskEntry : public QObject
{
  Q_OBJECT
public:
  DiskEntry(QObject *parent=0, const char *name=0);
  DiskEntry(const QString & deviceName, QObject *parent=0, const char *name=0);
  ~DiskEntry();
  QString lastSysError() {return sysStringErrOut; };
  QString deviceName() const { return device; };
  QString mountPoint() const { return mountedOn; };
  QString mountOptions() const { return options; };
  /**
   * sets the used mountCommand for the actual DiskEntry.
   * @param mntcmd   is a string containing the executable file and
   *                 special codes which will be filled in when used: <BR>
   *                 %m : mountpoint <BR>
   *                 %d : deviceName <BR>
   *                 %t : filesystem type <BR>
   *                 %o : mount options <BR>
   *                 all this information is gained from the objects' data
   *                 if no mountCommand is set it defaults to "mount %d"
   **/
  QString mountCommand() const { return mntcmd; };
  /**
   * sets the used umountCommand for the actual DiskEntry.
   * @param mntcmd   is a string containing the executable file and
   *                 special codes which will be filled in when used: <BR>
   *                 %m : mountpoint <BR>
   *                 %d : deviceName <BR>
   *                 all this information is gained from the objects' data
   *                 if no umountCommand is set it defaults to "umount %d"
   **/
  QString umountCommand() const { return umntcmd; };
  QString fsType() const { return type; };
  bool mounted() const { return isMounted; };
  int kBSize() const { return size; };

  QString discType();  

  QString iconName();
  QString realIconName() { return icoName; };
  QString prettyKBSize() const { return KIO::convertSizeFromKB(size); };
  int kBUsed() const { return used; };
  QString prettyKBUsed() const { return KIO::convertSizeFromKB(used); };
  int kBAvail() const  { return avail; };
  QString prettyKBAvail() const { return KIO::convertSizeFromKB(avail); };
  float percentFull() const;

signals:
  void sysCallError(DiskEntry *disk, int err_no);
  void deviceNameChanged();
  void mountPointChanged();
  void mountOptionsChanged();
  void fsTypeChanged();
  void mountedChanged();
  void kBSizeChanged();
  void kBUsedChanged();
  void kBAvailChanged();
  void iconNameChanged();

public slots:

  int toggleMount();
  int mount();
  int umount();
  int remount();
  void setMountCommand(const QString & mnt);
  void setUmountCommand(const QString & umnt);
  void setDeviceName(const QString & deviceName);
  void setMountPoint(const QString & mountPoint);
  void setIconName(const QString & iconName);
  void setMountOptions(const QString & mountOptions);
  void setFsType(const QString & fsType);
  void setMounted(bool nowMounted);
  void setKBSize(int kb_size);
  void setKBUsed(int kb_used);
  void setKBAvail(int kb_avail);
  QString guessIconName();

private slots:
   void receivedSysStdErrOut(KProcess *, char *data, int len);

private:
  void init();
  int sysCall(const QString & command);
  QString prettyPrint(int kBValue) const;

  KShellProcess     *sysProc;
  QString           sysStringErrOut;
  bool              readingSysStdErrOut;

  QString     device,
              type,
              mountedOn,
              options,
              icoName,
              mntcmd,
              umntcmd;

  int         size,
              used,
              avail;       // ATTENTION: used+avail != size (clustersize!)

  bool        isMounted,
              iconSetByUser;
};

#endif
