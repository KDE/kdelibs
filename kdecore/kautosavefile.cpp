/*  This file is part of the KDE libraries
    Copyright (c) 2006 Jacob R Rideout <kde@jacobrideout.net>

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

#ifndef QT_NO_CAST_FROM_ASCII
#define QT_NO_CAST_FROM_ASCII
#endif
#ifndef QT_NO_CAST_TO_ASCII
#define QT_NO_CAST_TO_ASCII
#endif

#include <QtCore/QLatin1Char>
#include <QtCore/QCoreApplication>
#include "kautosavefile.h"
#include "klockfile.h"
#include "krandom.h"
#include "kglobal.h"
#include "kstandarddirs.h"

class KAutoSaveFilePrivate
{
public:
    KAutoSaveFilePrivate() :
            managedFile(),
            lock ()
    {}

    QString tempFileName();
    KUrl managedFile;
    KLockFile::Ptr lock ;
};

QString KAutoSaveFilePrivate::tempFileName()
{
    QString name( managedFile.fileName() );
    name.replace( QLatin1Char('/'), QLatin1Char('_') );
    name+= KRandom::randomString( 8 );
    return name;
}

KAutoSaveFile::KAutoSaveFile(const KUrl &filename, QObject *parent) :
        QFile(parent),
        d(new KAutoSaveFilePrivate)
{
    setManagedFile(filename);
    KGlobal::dirs()->addResourceType("stale",QString::fromLatin1("data/stalefiles"));

}

KAutoSaveFile::KAutoSaveFile(QObject *parent) :
        QFile(parent),
        d(new KAutoSaveFilePrivate)
{
    KGlobal::dirs()->addResourceType("stale",QString::fromLatin1("data/stalefiles"));
}

KAutoSaveFile::~KAutoSaveFile()
{
    delete d;
}

KUrl KAutoSaveFile::managedFile() const
{
    return d->managedFile;
}

void KAutoSaveFile::setManagedFile(const KUrl &filename)
{
    d->managedFile = filename;
}

void KAutoSaveFile::releaseLock()
{
    d->lock.clear();
    remove();
}

bool KAutoSaveFile::open(OpenMode openmode)
{
    if ( d->managedFile == KUrl() )
        return false;

    QString tempFile =  KStandardDirs::locateLocal( "stale",
                        QCoreApplication::instance()->applicationName()+QChar::fromLatin1('/')+d->tempFileName() );

    d->lock = new KLockFile(tempFile);
    d->lock ->setStaleTime(3600); // HARDCODE

    if ( d->lock ->isLocked() )
        return false;

    setFileName(tempFile);
    if ( QFile::open(openmode) )
    {
        d->lock ->lock ();
    }
    return false;
}

QList<KAutoSaveFile *> KAutoSaveFile::staleFiles(const KUrl &filename)
{
    KGlobal::dirs()->addResourceType("stale",QString::fromLatin1("data/stalefiles"));

    QString url;

    /*

    // Do we want to keep track of all the details of a file to
    // ensure that we get the correct one? If so, we need manifest
    // file, that links urls to temp file names.

    // this get the url, without password, for security reasons
    if (filename.hasPass() && filename.hasUser())
    {  
        KUrl tempUrl(filename);
        tempUrl.setPass("");
        url = tempUrl.url();
    }    
    else
        url = filename.url();
    */

    url = filename.fileName();

    // get stale files
    QStringList files;
    if ( !url.isEmpty() )
        files = KGlobal::dirs()->findAllResources( "stale", url+QChar::fromLatin1('*') , true, false );

    QList<KAutoSaveFile *> list;
    QString file;
    KAutoSaveFile * asFile;

    // contruct a KAutoSaveFile for each stale file
    foreach(file, files)
    {
        asFile = new KAutoSaveFile(filename);
        asFile->setFileName(file);
        list.append(asFile);
    }

    return list;
}

QList<KAutoSaveFile *> KAutoSaveFile::allStaleFiles(const QString &applicationName)
{
    KGlobal::dirs()->addResourceType("stale",QString::fromLatin1("data/stalefiles"));

    QString appName(applicationName);
    if ( appName.isEmpty() )
        appName = QCoreApplication::instance()->applicationName();

    // get stale files
    QStringList files = KGlobal::dirs()->findAllResources( "stale", appName+QLatin1String("/*"), false, false );

    QList<KAutoSaveFile *> list;
    QString file;
    KAutoSaveFile * asFile;

    // contruct a KAutoSaveFile for each stale file
    foreach(file, files)
    {
        QString name(file);
        file.chop(8);
        asFile = new KAutoSaveFile(name);
        asFile->setFileName(file);
        list.append(asFile);
    }

    return list;
}

#include "kautosavefile.moc"
