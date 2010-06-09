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

#include "kautosavefile.h"

#include <stdio.h> // for FILENAME_MAX

#include <QtCore/QLatin1Char>
#include <QtCore/QCoreApplication>
#include "klockfile.h"
#include "krandom.h"
#include "kglobal.h"
#include "kstandarddirs.h"

class KAutoSaveFilePrivate
{
public:
    KAutoSaveFilePrivate()
            : lock(0),
              managedFileNameChanged(false)
    {}

    QString tempFileName();
    KUrl managedFile;
    KLockFile::Ptr lock;
    static const int padding;
    bool managedFileNameChanged;
};

const int KAutoSaveFilePrivate::padding = 8;

QString KAutoSaveFilePrivate::tempFileName()
{
    static const int maxNameLength = FILENAME_MAX;

    // Note: we drop any query string and user/pass info
    QString protocol(managedFile.protocol());
    QString path(managedFile.directory());
    QString name(managedFile.fileName());

    // Remove any part of the path to the right if it is longer than the max file size and
    // ensure that the max filesize takes into account the other parts of the tempFileName
    // Subtract 1 for the _ char, 3 for the padding sepperator, 5 is for the .lock
    path = path.left(maxNameLength - padding - name.size() - protocol.size() - 9);

    QString junk = KRandom::randomString(padding);
    // tempName = fileName + junk.trunicated + protocol + _ + path.trunicated + junk
    // This is done so that the separation between the filename and path can be determined
    name += junk.right(3) + protocol + QLatin1Char('_');
    name += path + junk;

    return QString::fromLatin1(KUrl::toPercentEncoding(name));
}

KAutoSaveFile::KAutoSaveFile(const KUrl &filename, QObject *parent)
        : QFile(parent),
          d(new KAutoSaveFilePrivate)
{
    setManagedFile(filename);
    KGlobal::dirs()->addResourceType("stale", 0, QString::fromLatin1("data/stalefiles"));
}

KAutoSaveFile::KAutoSaveFile(QObject *parent)
        : QFile(parent),
          d(new KAutoSaveFilePrivate)
{
    KGlobal::dirs()->addResourceType("stale", 0, QString::fromLatin1("data/stalefiles"));
}

KAutoSaveFile::~KAutoSaveFile()
{
    releaseLock();
    delete d;
}

KUrl KAutoSaveFile::managedFile() const
{
    return d->managedFile;
}

void KAutoSaveFile::setManagedFile(const KUrl &filename)
{
    releaseLock();

    d->managedFile = filename;
    d->managedFileNameChanged = true;
}

void KAutoSaveFile::releaseLock()
{
    if (d->lock && d->lock->isLocked()) {
        d->lock.clear();
        if (!fileName().isEmpty()) {
        remove();
        }
    }
}

bool KAutoSaveFile::open(OpenMode openmode)
{
    if (d->managedFile == KUrl()) {
        return false;
    }

    QString tempFile;
    if (d->managedFileNameChanged) {
        tempFile =  KStandardDirs::locateLocal("stale",
                                               QCoreApplication::instance()->applicationName()
                                               + QChar::fromLatin1('/')
                                               + d->tempFileName()
                                              );
    } else {
        tempFile = fileName();
    }

    d->managedFileNameChanged = false;

    setFileName(tempFile);

    if (QFile::open(openmode)) {

        d->lock = new KLockFile(tempFile + QString::fromLatin1(".lock"));
        if (d->lock->isLocked()) {
            close();
            return false;
        }

        d->lock->setStaleTime(60); // HARDCODE, 1 minute

        if (d->lock->lock(KLockFile::ForceFlag|KLockFile::NoBlockFlag) == KLockFile::LockOK) {
            return true;
        } else {
            close();
        }
    }

    return false;
}

QList<KAutoSaveFile *> KAutoSaveFile::staleFiles(const KUrl &filename, const QString &applicationName)
{
    KGlobal::dirs()->addResourceType("stale", 0, QString::fromLatin1("data/stalefiles"));

    QString appName(applicationName);
    if (appName.isEmpty()) {
        appName = QCoreApplication::instance()->applicationName();
    }

    QString url = filename.fileName();

    if (url.isEmpty()) {
        return QList<KAutoSaveFile *>();
    }

    // get stale files
    const QStringList files = KGlobal::dirs()->findAllResources("stale",
                                                  appName + QChar::fromLatin1('/') +
                                                  url + QChar::fromLatin1('*'),
                                                  KStandardDirs::Recursive);

    QList<KAutoSaveFile *> list;
    KAutoSaveFile * asFile;

    // contruct a KAutoSaveFile for each stale file
    foreach(const QString &file, files) {
        if (file.endsWith(".lock"))
            continue;
        // sets managedFile
        asFile = new KAutoSaveFile(filename);
        asFile->setFileName(file);
        // flags the name, so it isn't regenerated
        asFile->d->managedFileNameChanged = false;
        list.append(asFile);
    }

    return list;
}

QList<KAutoSaveFile *> KAutoSaveFile::allStaleFiles(const QString &applicationName)
{
    KGlobal::dirs()->addResourceType("stale", 0, QString::fromLatin1("data/stalefiles"));

    QString appName(applicationName);
    if (appName.isEmpty()) {
        appName = QCoreApplication::instance()->applicationName();
    }

    // get stale files
    const QStringList files = KGlobal::dirs()->findAllResources("stale", appName + QLatin1String("/*"));

    QList<KAutoSaveFile *> list;

    // contruct a KAutoSaveFile for each stale file
    foreach(QString file, files) { // krazy:exclude=foreach (no const& because modified below)
        if (file.endsWith(".lock"))
            continue;
        const QString sep = file.right(3);
        file.chop(KAutoSaveFilePrivate::padding);

        int sepPos = file.indexOf(sep);
        int pathPos = file.indexOf(QChar::fromLatin1('_'), sepPos);
        KUrl name;
        name.setProtocol(file.mid(sepPos + 3, pathPos - sep.size() - 3));
        name.setPath(KUrl::fromPercentEncoding(file.right(pathPos - 1).toLatin1()));
        name.addPath(KUrl::fromPercentEncoding(file.left(sepPos).toLatin1()));

        // sets managedFile
        KAutoSaveFile* asFile = new KAutoSaveFile(name);
        asFile->setFileName(file);
        // flags the name, so it isn't regenerated
        asFile->d->managedFileNameChanged = false;
        list.append(asFile);
    }

    return list;
}

#include "kautosavefile.moc"
