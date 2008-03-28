/*
 * kdiskfreespace.cpp
 *
 * Copyright (c) 1999 Michael Kropfberger <michael.kropfberger@gmx.net>
 * Copyright 2007 David Faure <faure@kde.org>
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
#include <kprocess.h>
#include <kmountpoint.h>
#include <kio/global.h>
#include <config-kfile.h>

#ifndef Q_OS_WIN
#define DF_COMMAND    "df"
#define DF_ARGS       "-k"
#define NO_FS_TYPE    true

#define BLANK ' '
#define FULL_PERCENT 95.0

class KDiskFreeSpace::Private
{
public:
    Private(KDiskFreeSpace *parent)
        : m_parent(parent)
    {}

    void _k_dfDone();

    KDiskFreeSpace   *m_parent;
    KProcess         *dfProc;
    QString           m_mountPoint;
};

/***************************************************************************
  * constructor
**/
KDiskFreeSpace::KDiskFreeSpace(QObject *parent)
    : QObject(parent), d(new Private(this))
{
    d->dfProc = new KProcess(); Q_CHECK_PTR(d->dfProc);
    // we want to parse stdout and to see error messages
    d->dfProc->setOutputChannelMode(KProcess::MergedChannels);
    d->dfProc->setEnv("LANGUAGE", "C");
    connect(d->dfProc,SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(_k_dfDone()));
}


/***************************************************************************
  * destructor
**/
KDiskFreeSpace::~KDiskFreeSpace()
{
    delete d->dfProc;
    delete d;
}

/***************************************************************************
  * reads the df-commands results
**/
bool KDiskFreeSpace::readDF( const QString & mountPoint )
{
    if (d->dfProc->state() == QProcess::Running)
        return false;
    d->m_mountPoint = mountPoint;
    (*d->dfProc) << QString::fromLocal8Bit(DF_COMMAND) << QString::fromLocal8Bit(DF_ARGS);
    d->dfProc->start();
    return true;
}


/***************************************************************************
  * is called, when the df-command has finished
**/
void KDiskFreeSpace::Private::_k_dfDone()
{
    QByteArray allStdOut = dfProc->readAll();
    QTextStream t(&allStdOut, QIODevice::ReadOnly);
    QString s = t.readLine();
    if ( (s.isEmpty()) || ( !s.startsWith( QLatin1String("Filesystem") ) ) )
        kError() << "Error running df command... got [" << s << "]" << endl;

  while ( !t.atEnd() ) {
    s = t.readLine();
    s = s.simplified();
    if ( !s.isEmpty() ) {
      //kDebug(kfile_area) << "GOT: [" << s << "]";

      if (s.indexOf(BLANK)<0) {      // devicename was too long, rest in next line
          if ( !t.atEnd() ) {       // just appends the next line
            s=s.append(t.readLine());
            s=s.simplified();
            //kDebug(kfile_area) << "SPECIAL GOT: [" << s << "]";
          }//if silly linefeed
      }

      //kDebug(kfile_area) << "[" << s << "]";

      //QString deviceName = s.left(s.find(BLANK));
      s=s.remove(0,s.indexOf(BLANK)+1 );
      //kDebug(kfile_area) << "    DeviceName:    [" << deviceName << "]";

      if (!NO_FS_TYPE)
          s=s.remove(0,s.indexOf(BLANK)+1 ); // eat fs type

      QString u=s.left(s.indexOf(BLANK));
      quint64 kibSize = u.toULongLong();
      s=s.remove(0,s.indexOf(BLANK)+1 );
      //kDebug(kfile_area) << "    Size:       [" << kibSize << "]";

      u=s.left(s.indexOf(BLANK));
      quint64 kibUsed = u.toULongLong();
      s=s.remove(0,s.indexOf(BLANK)+1 );
      //kDebug(kfile_area) << "    Used:       [" << kibUsed << "]";

      u=s.left(s.indexOf(BLANK));
      quint64 kibAvail = u.toULongLong();
      s=s.remove(0,s.indexOf(BLANK)+1 );
      //kDebug(kfile_area) << "    Avail:       [" << kibAvail << "]";


      s=s.remove(0,s.indexOf(BLANK)+1 );  // delete the capacity 94%
      QString mountPoint = s.trimmed();
      //kDebug(kfile_area) << "    MountPoint:       [" << mountPoint << "]";

      if ( mountPoint == m_mountPoint )
      {
        //kDebug(kfile_area) << "Found mount point. Emitting";
        emit m_parent->foundMountPoint( mountPoint, kibSize, kibUsed, kibAvail );
      }
    }//if not header
  }//while further lines available

  emit m_parent->done();
  m_parent->deleteLater();
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

class KDiskFreeSpace::Private
{
    public:
        Private(){}
        void _k_dfDone();
};

KDiskFreeSpace::KDiskFreeSpace(QObject *parent)
    : QObject(parent), d(0)
{
}

KDiskFreeSpace::~KDiskFreeSpace()
{
}

bool KDiskFreeSpace::readDF( const QString & mountPoint )
{
    quint64 availUser, total, avail;
    bool bRet = false;
    QFileInfo fi(mountPoint);
    QString dir = QDir::toNativeSeparators(fi.absoluteDir().canonicalPath());

    if(GetDiskFreeSpaceExW((LPCWSTR)dir.utf16(),
                           (PULARGE_INTEGER)&availUser,
                           (PULARGE_INTEGER)&total,
                           (PULARGE_INTEGER)&avail) != 0) {
        availUser = availUser / 1024;
        total = total / 1024;
        avail = avail / 1024;
        emit foundMountPoint( mountPoint, total, total-avail, avail );
        bRet = true;
    }
    emit done();
    deleteLater();
    return bRet;
}

KDiskFreeSpace *KDiskFreeSpace::findUsageInfo( const QString & path )
{
    KDiskFreeSpace * job = new KDiskFreeSpace;
    job->readDF( path );
    return job;
}

void KDiskFreeSpace::Private::_k_dfDone()
{
}

#endif

#include "kdiskfreespace.moc"

