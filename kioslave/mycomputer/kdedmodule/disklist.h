/*
 * disklist.h
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


#ifndef __DISKLIST_H__
#define __DISKLIST_H__

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
//#include <kcontrol.h>

// defines the os-type
#include <qglobal.h>

#include "disks.h"

#define DF_COMMAND    "df"
// be pessimistic: df -T only works under linux !??
#if defined(_OS_LINUX_)
#define DF_ARGS       "-kT"
#define NO_FS_TYPE    false
#else
#define DF_ARGS       "-k"
#define NO_FS_TYPE    true
#endif		

#ifdef _OS_SOLARIS_
#define CACHEFSTAB "/etc/cachefstab"
#define FSTAB "/etc/vfstab"
#else
#define FSTAB "/etc/fstab"
#endif

#define SEPARATOR "|"

/***************************************************************************/
typedef QPtrList<DiskEntry>		DisksBase;
typedef QPtrListIterator<DiskEntry>	DisksIterator;

/***************************************************************************/
class Disks : public DisksBase
{
 public:
  Disks(bool deepCopies=TRUE) { dc = deepCopies;};
  ~Disks() { clear(); };
private: 
  int compareItems( DiskEntry s1, DiskEntry s2 ) 
  {
    int ret = s1.deviceName().compare(s2.deviceName());
    if( ret == 0 )
    {
      ret = s1.mountPoint().compare(s2.mountPoint());
    }

    kdDebug() << "compareDISKS " << s1.deviceName() << " vs " << s2.deviceName() << " (" << ret << ")" << endl;
    return( ret );
  }

  /*
  int compareItems( DiskEntry* s1, DiskEntry* s2 ) {
	int ret;
	ret = strcmp (static_cast<DiskEntry*>(s1)->deviceName(),
		      static_cast<DiskEntry*>(s2)->deviceName() );
	if (0 == ret)
	   ret = strcmp (static_cast<DiskEntry*>(s1)->mountPoint(),
			 static_cast<DiskEntry*>(s2)->mountPoint());
	return ret;
      };
  */

  bool  dc;
};

/***************************************************************************/
class DiskList : public QObject
{  Q_OBJECT
public:
   DiskList( QObject *parent=0, const char *name=0 );
 ~DiskList();
   int readFSTAB(); 
   int readDF();
   int find(const DiskEntry* disk) {return disks->find(disk);};
   DiskEntry*  at(uint index) {return disks->at(index);};
   DiskEntry* first() {return disks->first();};
   DiskEntry* next() {return disks->next();};
   uint count() { return disks->count(); };

signals:
   void readDFDone();
   void criticallyFull(DiskEntry *disk);

public slots:
  void loadSettings();
  void applySettings();
   
private slots:
   void receivedDFStdErrOut(KProcess *, char *data, int len);
   void dfDone();

private: 
  void replaceDeviceEntry(DiskEntry *disk);

  Disks            *disks;
  KProcess         *dfProc;
  QString           dfStringErrOut;
  bool              readingDFStdErrOut;
  KConfig           *config;
 
};
/***************************************************************************/


#endif
