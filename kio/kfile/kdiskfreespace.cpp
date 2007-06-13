/*
 * kdiskfreespace.cpp
 *
 * Copyright (c) 1999 Michael Kropfberger <michael.kropfberger@gmx.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdiskfreespace.h"
#include <QtCore/QFile>
#include <QtCore/QTextIStream>

#include <kdebug.h>
#include <kmountpoint.h>
#include <kio/global.h>
#include <config-kfile.h>

#include "kdiskfreespace.moc"

#ifndef Q_OS_WIN
#include <k3process.h>
#define DF_COMMAND    "df"
#define DF_ARGS       "-k"
#define NO_FS_TYPE    true

#define BLANK ' '
#define FULL_PERCENT 95.0

/***************************************************************************
  * constructor
**/
KDiskFreeSpace::KDiskFreeSpace(QObject *parent)
    : QObject(parent)
{
    dfProc = new K3Process(); Q_CHECK_PTR(dfProc);
    dfProc->setEnvironment("LANGUAGE", "C");
    connect( dfProc, SIGNAL(receivedStdout(K3Process *, char *, int) ),
             this, SLOT (receivedDFStdErrOut(K3Process *, char *, int)) );
    connect(dfProc,SIGNAL(processExited(K3Process *) ),
            this, SLOT(dfDone() ) );

    readingDFStdErrOut=false;
}


/***************************************************************************
  * destructor
**/
KDiskFreeSpace::~KDiskFreeSpace()
{
    delete dfProc;
}

/***************************************************************************
  * is called, when the df-command writes on StdOut
**/
void KDiskFreeSpace::receivedDFStdErrOut(K3Process *, char *data, int len)
{
  QByteArray tmp(data,len+1);  // adds a zero-byte
  dfStringErrOut.append(tmp);
}

/***************************************************************************
  * reads the df-commands results
**/
int KDiskFreeSpace::readDF( const QString & mountPoint )
{
  if (readingDFStdErrOut || dfProc->isRunning())
    return -1;
  m_mountPoint = mountPoint;
  dfStringErrOut=""; // yet no data received
  dfProc->clearArguments();
  (*dfProc) << QString::fromLocal8Bit(DF_COMMAND) << QString::fromLocal8Bit(DF_ARGS);
  if (!dfProc->start( K3Process::NotifyOnExit, K3Process::AllOutput ))
     kError() << "could not execute ["<< DF_COMMAND << "]" << endl;
  return 1;
}


/***************************************************************************
  * is called, when the df-command has finished
**/
void KDiskFreeSpace::dfDone()
{
  readingDFStdErrOut=true;

  QTextStream t (dfStringErrOut, QIODevice::ReadOnly);
  QString s=t.readLine();
  if ( (s.isEmpty()) || ( !s.startsWith( QLatin1String("Filesystem") ) ) )
    kError() << "Error running df command... got [" << s << "]" << endl;
  while ( !t.atEnd() ) {
    QString u,v;
    s=t.readLine();
    s=s.simplified();
    if ( !s.isEmpty() ) {
      //kDebug(kfile_area) << "GOT: [" << s << "]" << endl;

      if (s.indexOf(BLANK)<0)      // devicename was too long, rest in next line
	if ( !t.atEnd() ) {       // just appends the next line
            v=t.readLine();
            s=s.append(v);
            s=s.simplified();
            //kDebug(kfile_area) << "SPECIAL GOT: [" << s << "]" << endl;
	 }//if silly linefeed

      //kDebug(kfile_area) << "[" << s << "]" << endl;

      //QString deviceName = s.left(s.find(BLANK));
      s=s.remove(0,s.indexOf(BLANK)+1 );
      //kDebug(kfile_area) << "    DeviceName:    [" << deviceName << "]" << endl;

      if (!NO_FS_TYPE)
          s=s.remove(0,s.indexOf(BLANK)+1 ); // eat fs type

      u=s.left(s.indexOf(BLANK));
      unsigned long kBSize = u.toULong();
      s=s.remove(0,s.indexOf(BLANK)+1 );
      //kDebug(kfile_area) << "    Size:       [" << kBSize << "]" << endl;

      u=s.left(s.indexOf(BLANK));
      unsigned long kBUsed = u.toULong();
      s=s.remove(0,s.indexOf(BLANK)+1 );
      //kDebug(kfile_area) << "    Used:       [" << kBUsed << "]" << endl;

      u=s.left(s.indexOf(BLANK));
      unsigned long kBAvail = u.toULong();
      s=s.remove(0,s.indexOf(BLANK)+1 );
      //kDebug(kfile_area) << "    Avail:       [" << kBAvail << "]" << endl;


      s=s.remove(0,s.indexOf(BLANK)+1 );  // delete the capacity 94%
      QString mountPoint = s.trimmed();
      //kDebug(kfile_area) << "    MountPoint:       [" << mountPoint << "]" << endl;

      if ( mountPoint == m_mountPoint )
      {
        //kDebug(kfile_area) << "Found mount point. Emitting" << endl;
        emit foundMountPoint( mountPoint, kBSize, kBUsed, kBAvail );
        emit foundMountPoint( kBSize, kBUsed, kBAvail, mountPoint ); // sic!
      }
    }//if not header
  }//while further lines available

  readingDFStdErrOut=false;
  emit done();
  deleteLater();
}

KDiskFreeSpace * KDiskFreeSpace::findUsageInfo( const QString & path )
{
    KDiskFreeSpace * job = new KDiskFreeSpace;
    KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath( path );
    job->readDF( mp ? mp->mountPoint() : QString() );
    return job;
}

#else
// windows
#include <QtCore/QDir>
#include <windows.h>

KDiskFreeSpace::KDiskFreeSpace(QObject *parent)
    : QObject(parent)
{
}

KDiskFreeSpace::~KDiskFreeSpace()
{
}

int KDiskFreeSpace::readDF( const QString & mountPoint )
{
    quint64 availUser, total, avail;
    int iRet = -1;
    QFileInfo fi(mountPoint);
    QString dir = QDir::toNativeSeparators(fi.absoluteDir().canonicalPath());

    if(GetDiskFreeSpaceEx((LPCWSTR)dir.utf16(),
                          (PULARGE_INTEGER)&availUser,
                          (PULARGE_INTEGER)&total,
                          (PULARGE_INTEGER)&avail) != 0) {
        availUser = availUser / 1024;
        total = total / 1024;
        avail = avail / 1024;
        emit foundMountPoint( mountPoint, total, total-avail, avail );
        emit foundMountPoint( total, total-avail, avail, mountPoint ); // sic!
        iRet = 1;
    }
    emit done();
    return iRet;
}

KDiskFreeSpace *KDiskFreeSpace::findUsageInfo( const QString & path )
{
    KDiskFreeSpace * job = new KDiskFreeSpace;
    job->readDF( path );
    return job;
}

// can't convince moc to ignore them on win32 only :(
void KDiskFreeSpace::receivedDFStdErrOut(K3Process *, char *, int)
{
}

void KDiskFreeSpace::dfDone()
{
}

#endif
