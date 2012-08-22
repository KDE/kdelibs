/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Joseph Wenninger <jowenn@kde.org>
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
 **/

#include "ktempdir.h"
#include <config-kde4support.h>
#include <sys/types.h>

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#ifdef HAVE_TEST
#include <test.h>
#endif
#if HAVE_PATHS_H
#include <paths.h>
#endif

#include <QtCore/QDir>
#include <QtCore/QDebug>

#include "krandom.h"
#include <QCoreApplication>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <QtCore/QVarLengthArray>
#include <windows.h>
#include <shellapi.h>
extern QString mkdtemp_QString (const QString &_template);
#endif

#ifdef _WIN32_WCE
#include <shellapi.h>
#endif

static int s_umask;

// Read umask before any threads are created to avoid race conditions
static int kStoreUmask()
{
    mode_t tmp = 0;
    s_umask = umask(tmp);
    return umask(s_umask);
}

Q_CONSTRUCTOR_FUNCTION(kStoreUmask)

class KTempDir::Private
{
public:
    int error;
    QString tmpName;
    bool exists;
    bool autoRemove;

    Private()
    {
        autoRemove = true;
        exists = false;
        error=0;
    }
};

KTempDir::KTempDir(const QString &directoryPrefix, int mode) : d(new Private)
{
    (void) create( directoryPrefix.isEmpty() ? QDir::tempPath() + QLatin1Char('/') + QCoreApplication::applicationName() : directoryPrefix , mode);
}

bool KTempDir::create(const QString &directoryPrefix, int mode)
{
   (void) KRandom::random();

#ifdef Q_OS_WIN
   const QString nme = directoryPrefix + QLatin1String("XXXXXX");
   const QString realName = mkdtemp_QString(nme);
   if(realName.isEmpty())
   {
       qWarning() << "KTempDir: Error trying to create " << nme
		  << ": " << ::strerror(errno);
       d->error = errno;
       d->tmpName.clear();
       return false;
   }

   // got a return value != 0
   d->tmpName = realName + QLatin1Char('/');
   qDebug() << "KTempDir: Temporary directory created :" << d->tmpName;
   KDE::chmod(nme, mode&(~s_umask));

   // Success!
   d->exists = true;
#else
   QByteArray nme = QFile::encodeName(directoryPrefix) + "XXXXXX";
   char *realName;
   if((realName=mkdtemp(nme.data())) == 0)
   {
       // Recreate it for the warning, mkdtemps emptied it
       nme = QFile::encodeName(directoryPrefix) + "XXXXXX";
       qWarning() << "KTempDir: Error trying to create " << nme.constData()
                  << ": " << ::strerror(errno);
       d->error = errno;
       d->tmpName.clear();
       return false;
   }

   // got a return value != 0
   QByteArray realNameStr(realName);
   d->tmpName = QFile::decodeName(realNameStr.constData())+QLatin1Char('/');
   qDebug() << "KTempDir: Temporary directory created :" << d->tmpName;

   if(chmod(nme.data(), mode&(~s_umask)) < 0) {
       qWarning() << "KTempDir: Unable to change permissions on" << d->tmpName
                  << ":" << ::strerror(errno);
       d->error = errno;
       d->tmpName.clear();
       (void) ::rmdir(realName); // Cleanup created directory
       return false;
   }

   // Success!
   d->exists = true;

   // Set uid/gid (necessary for SUID programs)
   if(chown(nme.data(), getuid(), getgid()) < 0) {
       // Just warn, but don't failover yet
       qWarning() << "KTempDir: Unable to change owner on" << d->tmpName
                  << ":" << ::strerror(errno);
   }

#endif
   return true;
}

KTempDir::~KTempDir()
{
    if (d->autoRemove) {
        unlink();
    }

    delete d;
}

int KTempDir::status() const
{
    return d->error;
}

QString KTempDir::name() const
{
    return d->tmpName;
}

bool KTempDir::exists() const
{
    return d->exists;
}

void KTempDir::setAutoRemove(bool autoRemove)
{
    d->autoRemove = autoRemove;
}

bool KTempDir::autoRemove() const
{
    return d->autoRemove;
}

void KTempDir::unlink()
{
    if (!d->exists) return;
    if (KTempDir::removeDir(d->tmpName))
        d->error=0;
    else
        d->error=errno;
    d->exists=false;
}

#ifndef Q_OS_WIN
// Auxiliary recursive function for removeDirs
static bool rmtree(const QString& name)
{
    if (QFileInfo(name).isDir()) {
        Q_FOREACH(const QFileInfo & entry, QDir(name).entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot)) {
            if( ! rmtree(entry.absoluteFilePath()))
                return false;
        }
    }
    return remove(name.toLocal8Bit().data()) != -1;
}
#endif

bool KTempDir::removeDir( const QString& path )
{
    //kDebug(180) << path;
    if ( !QFile::exists( path ) )
        return true; // The goal is that there is no directory

#ifdef Q_OS_WIN
    QVarLengthArray<WCHAR, MAX_PATH> name;
    name.resize( path.length() + 2 ); // double null terminated!
    memcpy( name.data(), path.utf16(), path.length() * sizeof(WCHAR) );
    name[path.length()     ] = 0;
    name[path.length() + 1 ] = 0;
    if(path.endsWith(QLatin1Char('/')) || path.endsWith(QLatin1Char('\\')))
      name[path.length() - 1 ] = 0;
    SHFILEOPSTRUCTW fileOp;
    memset(&fileOp, 0, sizeof(SHFILEOPSTRUCTW) );
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = (LPCWSTR)name.constData();
    fileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
#ifdef _WIN32_WCE
    // FOF_NOERRORUI is not defined in wince
#else
    fileOp.fFlags |= FOF_NOERRORUI;
#endif
    errno = SHFileOperationW( &fileOp );
    return (errno == 0);
#else
    return rmtree( path );
#endif
}

