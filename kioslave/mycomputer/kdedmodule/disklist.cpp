/*
 * disklist.cpp
 *
 * $Id$
 *
 * Copyright (c) 1999 Michael Kropfberger <michael.kropfberger@gmx.net>
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

#include <config.h>

#include <math.h>
#include <stdlib.h>

#include <kapplication.h>

#include "disklist.h"

#define BLANK ' '
#define DELIMITER '#'
#define FULL_PERCENT 95.0

/***************************************************************************
  * constructor
**/
DiskList::DiskList(QObject *parent, const char *name)
    : QObject(parent,name)
{
  /*
#ifdef _OS_LINUX_
  kdDebug() << "_OS_LINUX_" << endl;
#endif
#ifdef _OS_FREEBSD_
  kdDebug() << "_OS_FREEBSD_" << endl;
#endif
#ifdef _OS_SUN_
  kdDebug() << "_OS_SUN_" << endl;
#endif
#ifdef _OS_SOLARIS_
  kdDebug() << "_OS_SOLARIS_" << endl;
#endif
#ifdef _OS_HPUX_
  kdDebug() << "_OS_HPUX_" << endl;
#endif
#ifdef _OS_UNIX_
  kdDebug() << "_OS_UNIX_" << endl;
#endif
  */

if (NO_FS_TYPE)
  kdDebug() << "df gives no FS_TYPE" << endl;

   disks = new Disks;
   disks->setAutoDelete(TRUE);

   // BackgroundProcesses ****************************************
      dfProc = new KProcess(); Q_CHECK_PTR(dfProc);
       connect( dfProc, SIGNAL(receivedStdout(KProcess *, char *, int) ),
           this, SLOT (receivedDFStdErrOut(KProcess *, char *, int)) );
       //connect( dfProc, SIGNAL(receivedStderr(KProcess *, char *, int) ),
       //  this, SLOT (receivedDFStdErrOut(KProcess *, char *, int)) );
       connect(dfProc,SIGNAL(processExited(KProcess *) ),
              this, SLOT(dfDone() ) );

       readingDFStdErrOut=FALSE;
       config = kapp->config();
       loadSettings();
       //readFSTAB();
       //readDF();
};


/***************************************************************************
  * destructor
**/
DiskList::~DiskList()
{
};


/***************************************************************************
  * saves the KConfig for special mount/umount scripts
**/
void DiskList::applySettings()
{
  QString oldgroup=config->group();
  config->setGroup("DiskList");
  QString key;
  DiskEntry *disk;
  for (disk=disks->first();disk!=0;disk=disks->next()) {
   key.sprintf("Mount%s%s%s%s",SEPARATOR,disk->deviceName().latin1()
                              ,SEPARATOR,disk->mountPoint().latin1());
   config->writeEntry(key,disk->mountCommand());

   key.sprintf("Umount%s%s%s%s",SEPARATOR,disk->deviceName().latin1()
                              ,SEPARATOR,disk->mountPoint().latin1());
   config->writeEntry(key,disk->umountCommand());

   key.sprintf("Icon%s%s%s%s",SEPARATOR,disk->deviceName().latin1()
                              ,SEPARATOR,disk->mountPoint().latin1());
   config->writeEntry(key,disk->realIconName());
 }
 config->sync();
 config->setGroup(oldgroup);
}


/***************************************************************************
  * reads the KConfig for special mount/umount scripts
**/
void DiskList::loadSettings()
{
  config->setGroup("DiskList");
  QString key;
  DiskEntry *disk;
  for (disk=disks->first();disk!=0;disk=disks->next()) {
    key.sprintf("Mount%s%s%s%s",SEPARATOR,disk->deviceName().latin1()
		,SEPARATOR,disk->mountPoint().latin1());
    disk->setMountCommand(config->readEntry(key,""));

    key.sprintf("Umount%s%s%s%s",SEPARATOR,disk->deviceName().latin1()
		,SEPARATOR,disk->mountPoint().latin1());
    disk->setUmountCommand(config->readEntry(key,""));

    key.sprintf("Icon%s%s%s%s",SEPARATOR,disk->deviceName().latin1()
		,SEPARATOR,disk->mountPoint().latin1());
    QString icon=config->readEntry(key,"");
    if (!icon.isEmpty()) disk->setIconName(icon);
 }
}

/***************************************************************************
  * tries to figure out the possibly mounted fs
**/
int DiskList::readFSTAB()
{
  if (readingDFStdErrOut || dfProc->isRunning()) return -1;

QFile f(FSTAB);
  if ( f.open(IO_ReadOnly) ) {
    QTextStream t (&f);
    QString s;
    DiskEntry *disk;

    //disks->clear(); // ############

    while (! t.eof()) {
      s=t.readLine();
      s=s.simplifyWhiteSpace();
      if ( (!s.isEmpty() ) && (s.find(DELIMITER)!=0) ) {
               // not empty or commented out by '#'
	//	kdDebug() << "GOT: [" << s << "]" << endl;
	disk = new DiskEntry();// Q_CHECK_PTR(disk);
        disk->setMounted(FALSE);
        disk->setDeviceName(s.left(s.find(BLANK)) );
            s=s.remove(0,s.find(BLANK)+1 );
	    //  kdDebug() << "    deviceName:    [" << disk->deviceName() << "]" << endl;
#ifdef _OS_SOLARIS_
            //device to fsck
            s=s.remove(0,s.find(BLANK)+1 );
#endif
         disk->setMountPoint(s.left(s.find(BLANK)) );
            s=s.remove(0,s.find(BLANK)+1 );
	    //kdDebug() << "    MountPoint:    [" << disk->mountPoint() << "]" << endl;
	    //kdDebug() << "    Icon:          [" << disk->iconName() << "]" << endl;
         disk->setFsType(s.left(s.find(BLANK)) );
            s=s.remove(0,s.find(BLANK)+1 );
	    //kdDebug() << "    FS-Type:       [" << disk->fsType() << "]" << endl;
         disk->setMountOptions(s.left(s.find(BLANK)) );
            s=s.remove(0,s.find(BLANK)+1 );
	    //kdDebug() << "    Mount-Options: [" << disk->mountOptions() << "]" << endl;
         if ( (disk->deviceName() != "none")
	      && (disk->fsType() != "swap")
	      && (disk->mountPoint() != "/dev/swap")
	      && (disk->mountPoint() != "/dev/pts")
	      && (disk->mountPoint().find("/proc") == -1 ) )
	   replaceDeviceEntry(disk);
         else
           delete disk;

      } //if not empty
    } //while
    f.close();
  } //if f.open

  loadSettings(); //to get the mountCommands

  //  kdDebug() << "DiskList::readFSTAB DONE" << endl;
  return 1;
}


/***************************************************************************
  * is called, when the df-command writes on StdOut or StdErr
**/
void DiskList::receivedDFStdErrOut(KProcess *, char *data, int len )
{

  /* ATTENTION: StdERR no longer connected to this...
   * Do we really need StdErr?? on HP-UX there was eg. a line
   * df: /home_tu1/ijzerman/floppy: Stale NFS file handle
   * but this shouldn't cause a real problem
   */

  QString tmp = QString(data) + QString("\0");  // adds a zero-byte
  tmp.truncate(len);

  dfStringErrOut.append(tmp);
}

/***************************************************************************
  * reads the df-commands results
**/
int DiskList::readDF()
{
  if (readingDFStdErrOut || dfProc->isRunning()) return -1;
  setenv("LANG", "en_US", 1);
  setenv("LC_ALL", "en_US", 1);
  setenv("LC_MESSAGES", "en_US", 1);
  setenv("LC_TYPE", "en_US", 1);
  setenv("LANGUAGE", "en_US", 1);
  dfStringErrOut=""; // yet no data received
  dfProc->clearArguments();
  (*dfProc) << DF_COMMAND << DF_ARGS;
  if (!dfProc->start( KProcess::NotifyOnExit, KProcess::AllOutput ))
    qFatal(i18n("could not execute [%s]").local8Bit().data(), DF_COMMAND);
  return 1;
}


/***************************************************************************
  * is called, when the df-command has finished
**/
void DiskList::dfDone()
{
  readingDFStdErrOut=TRUE;
  for ( DiskEntry *disk=disks->first(); disk != 0; disk=disks->next() )
    disk->setMounted(FALSE);  // set all devs unmounted

  QTextStream t (dfStringErrOut, IO_ReadOnly);
  QString s=t.readLine();
  if ( ( s.isEmpty() ) || ( s.left(10) != "Filesystem" ) )
    qFatal("Error running df command... got [%s]",s.latin1());
  while ( !t.atEnd() ) {
    QString u,v;
    DiskEntry *disk;
    s=t.readLine();
    s=s.simplifyWhiteSpace();
    if ( !s.isEmpty() ) {
      disk = new DiskEntry(); Q_CHECK_PTR(disk);

      if (s.find(BLANK)<0)      // devicename was too long, rest in next line
	if ( !t.eof() ) {       // just appends the next line
            v=t.readLine();
            s=s.append(v.latin1() );
            s=s.simplifyWhiteSpace();
	    //kdDebug() << "SPECIAL GOT: [" << s << "]" << endl;
	 }//if silly linefeed

      //kdDebug() << "EFFECTIVELY GOT " << s.length() << " chars: [" << s << "]" << endl;

      disk->setDeviceName(s.left(s.find(BLANK)) );
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug() << "    DeviceName:    [" << disk->deviceName() << "]" << endl;

      if (NO_FS_TYPE) {
	//kdDebug() << "THERE IS NO FS_TYPE_FIELD!" << endl;
         disk->setFsType("?");
      } else {
         disk->setFsType(s.left(s.find(BLANK)) );
         s=s.remove(0,s.find(BLANK)+1 );
      };
      //kdDebug() << "    FS-Type:       [" << disk->fsType() << "]" << endl;
      //kdDebug() << "    Icon:          [" << disk->iconName() << "]" << endl;

      u=s.left(s.find(BLANK));
      disk->setKBSize(u.toInt() );
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug() << "    Size:       [" << disk->kBSize() << "]" << endl;

      u=s.left(s.find(BLANK));
      disk->setKBUsed(u.toInt() );
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug() << "    Used:       [" << disk->kBUsed() << "]" << endl;

      u=s.left(s.find(BLANK));
      disk->setKBAvail(u.toInt() );
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug() << "    Avail:       [" << disk->kBAvail() << "]" << endl;


      s=s.remove(0,s.find(BLANK)+1 );  // delete the capacity 94%
      disk->setMountPoint(s.left(s.find(BLANK)) );
      s=s.remove(0,s.find(BLANK)+1 );
      //kdDebug() << "    MountPoint:       [" << disk->mountPoint() << "]" << endl;

      if ( (disk->kBSize() > 0)
	   && (disk->deviceName() != "none")
	   && (disk->fsType() != "swap")
	   && (disk->mountPoint() != "/dev/swap")
	   && (disk->mountPoint() != "/dev/pts")
	   && (disk->mountPoint().find("/proc") == -1 ) ) {
        disk->setMounted(TRUE);    // its now mounted (df lists only mounted)
	replaceDeviceEntry(disk);
      } else
	delete disk;

    }//if not header
  }//while further lines available

  readingDFStdErrOut=FALSE;
  loadSettings(); //to get the mountCommands
  emit readDFDone();
}


/***************************************************************************
  * updates or creates a new DiskEntry in the KDFList and TabListBox
**/
void DiskList::replaceDeviceEntry(DiskEntry *disk)
{
  //
  // The 'disks' may already already contain the 'disk'. If it do
  // we will replace some data. Otherwise 'disk' will be added to the list
  //

  //
  // 1999-27-11 Espen Sand:
  // I can't get find() to work. The Disks::compareItems(..) is
  // never called.
  //
  //int pos=disks->find(disk);

  int pos = -1;
  for( u_int i=0; i<disks->count(); i++ )
  {
    DiskEntry *item = disks->at(i);
    int res = disk->deviceName().compare( item->deviceName() );
    if( res == 0 )
    {
      res = disk->mountPoint().compare( item->mountPoint() );
    }
    if( res == 0 )
    {
      pos = i;
      break;
    }
  }

  if ((pos == -1) && (disk->mounted()) )
    // no matching entry found for mounted disk
    if ((disk->fsType() == "?") || (disk->fsType() == "cachefs")) {
      //search for fitting cachefs-entry in static /etc/vfstab-data
      DiskEntry* olddisk = disks->first();
      QString odiskName;
      while (olddisk != 0) {
        int p;
        // cachefs deviceNames have no / behind the host-column
	// eg. /cache/cache/.cfs_mnt_points/srv:_home_jesus
	//                                      ^    ^
        odiskName = olddisk->deviceName().copy();
        int ci=odiskName.find(':'); // goto host-column
        while ((ci =odiskName.find('/',ci)) > 0) {
           odiskName.replace(ci,1,"_");
        }//while
        // check if there is something that is exactly the tail
	// eg. [srv:/tmp3] is exact tail of [/cache/.cfs_mnt_points/srv:_tmp3]
        if ( ( (p=disk->deviceName().findRev(odiskName
	            ,disk->deviceName().length()) )
                != -1)
	      && (p + odiskName.length()
	          == disk->deviceName().length()) )
        {
             pos = disks->at(); //store the actual position
             disk->setDeviceName(olddisk->deviceName());
             olddisk=0;
	} else
          olddisk=disks->next();
      }// while
    }// if fsType == "?" or "cachefs"


#ifdef NO_FS_TYPE
  if (pos != -1) {
     DiskEntry * olddisk = disks->at(pos);
     if (olddisk)
        disk->setFsType(olddisk->fsType());
  }
#endif

  if (pos != -1) {  // replace
      DiskEntry * olddisk = disks->at(pos);
      if ( (-1!=olddisk->mountOptions().find("user")) &&
           (-1==disk->mountOptions().find("user")) ) {
          // add "user" option to new diskEntry
          QString s=disk->mountOptions();
          if (s.length()>0) s.append(",");
          s.append("user");
          disk->setMountOptions(s);
       }
      disk->setMountCommand(olddisk->mountCommand());
      disk->setUmountCommand(olddisk->umountCommand());

      //FStab after an older DF ... needed for critFull
      //so the DF-KBUsed survive a FStab lookup...
      //but also an unmounted disk may then have a kbused set...
      if ( (olddisk->mounted()) && (!disk->mounted()) ) {
         disk->setKBSize(olddisk->kBSize());
         disk->setKBUsed(olddisk->kBUsed());
         disk->setKBAvail(olddisk->kBAvail());
      }
          if ( (olddisk->percentFull() != -1) &&
               (olddisk->percentFull() <  FULL_PERCENT) &&
                  (disk->percentFull() >= FULL_PERCENT) ) {
	    kdDebug() << "Device " << disk->deviceName()
		      << " is critFull! " << olddisk->percentFull()
		      << "--" << disk->percentFull() << endl;
            emit criticallyFull(disk);
	  }
      disks->remove(pos); // really deletes old one
      disks->insert(pos,disk);
  } else {
    disks->append(disk);
  }//if

}

#include "disklist.moc"






