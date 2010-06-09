/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
  This file is part of the KDE libraries
  Copyright 1999 Waldo Bastian <bastian@kde.org>
  Copyright 2006 Allen Winter <winter@kde.org>
  Copyright 2006 Gregory S. Hayes <syncomm@kde.org>
  Copyright 2006 Jaison Lee <lee.jaison@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "ksavefile.h"

#include <config.h>

#include <QtCore/QDir>
#include <QProcess>
#include <QTemporaryFile>

#include <kconfig.h>
#include <kde_file.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>
#include <kcomponentdata.h>

#include <stdlib.h>
#include <errno.h>

class KSaveFile::Private
{
public:
    QString realFileName; //The name of the end-result file
    QString tempFileName; //The name of the temp file we are using
    
    QFile::FileError error;
    QString errorString;
    bool wasFinalized;
    KComponentData componentData;

    Private(const KComponentData &c)
        : componentData(c)
    {
        error = QFile::NoError;
        wasFinalized = false;
    }
};

KSaveFile::KSaveFile()
 : d(new Private(KGlobal::mainComponent()))
{
}

KSaveFile::KSaveFile(const QString &filename, const KComponentData &componentData)
 : d(new Private(componentData))
{
    KSaveFile::setFileName(filename);
}

KSaveFile::~KSaveFile()
{
    if (!d->wasFinalized)
        finalize();

    delete d;
}

bool KSaveFile::open(OpenMode flags)
{
    if ( d->realFileName.isNull() ) {
        d->error=QFile::OpenError;
        d->errorString=i18n("No target filename has been given.");
        return false;
    }

    if ( !d->tempFileName.isNull() ) {
#if 0 // do not set an error here, this open() fails, but the file itself is without errors
        d->error=QFile::OpenError;
        d->errorString=i18n("Already opened.");
#endif
        return false;
    }

    // we only check here if the directory can be written to
    // the actual filename isn't written to, but replaced later
    // with the contents of our tempfile
    if (!KStandardDirs::checkAccess(d->realFileName, W_OK)) {
        d->error=QFile::PermissionsError;
        d->errorString=i18n("Insufficient permissions in target directory.");
        return false;
    }

    //Create our temporary file
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    tempFile.setFileTemplate(d->realFileName + "XXXXXX.new");
    if (!tempFile.open()) {
        d->error=QFile::OpenError;
        d->errorString=i18n("Unable to open temporary file.");
        return false;
    }
    
    // if we're overwriting an existing file, ensure temp file's
    // permissions are the same as existing file so the existing
    // file's permissions are preserved. this will succeed completely
    // only if we are the same owner and group - or allmighty root.
    QFileInfo fi ( d->realFileName );
    if (fi.exists()) {
        //Qt apparently has no way to change owner/group of file :(
        if (fchown(tempFile.handle(), fi.ownerId(), fi.groupId())) {
            // failed to set user and group => try to restore group only.
            fchown(tempFile.handle(), -1, fi.groupId());
        }

        tempFile.setPermissions(fi.permissions());
    }
    else {
        mode_t umsk = KGlobal::umask();
        fchmod(tempFile.handle(), 0666&(~umsk));
    }

    //Open oursleves with the temporary file
    QFile::setFileName(tempFile.fileName());
    if (!QFile::open(flags)) {
        tempFile.setAutoRemove(true);
        return false;
    }

    d->tempFileName = tempFile.fileName();
    d->error=QFile::NoError;
    d->errorString.clear();
    return true;
}

void KSaveFile::setFileName(const QString &filename)
{
    d->realFileName = filename;
    
    // make absolute if needed
    if ( QDir::isRelativePath( filename ) ) {
        d->realFileName = QDir::current().absoluteFilePath( filename );
    }

    // follow symbolic link, if any
    d->realFileName = KStandardDirs::realFilePath( d->realFileName );

    return;
}

QFile::FileError KSaveFile::error() const
{
    if ( d->error != QFile::NoError ) {
        return d->error;
    } else {
        return QFile::error();
    }
}

QString KSaveFile::errorString() const
{
    if ( !d->errorString.isEmpty() ) {
        return d->errorString;
    } else {
        return QFile::errorString();
    }
}

QString KSaveFile::fileName() const
{
    return d->realFileName;
}

void KSaveFile::abort()
{
    close();
    QFile::remove(d->tempFileName); //non-static QFile::remove() does not work. 
    d->wasFinalized = true;
}

#ifdef HAVE_FDATASYNC
#  define FDATASYNC fdatasync
#else
#  define FDATASYNC fsync
#endif

bool KSaveFile::finalize()
{
    bool success = false;

    if ( !d->wasFinalized ) {

#ifdef Q_OS_UNIX
        static int extraSync = -1;
        if (extraSync < 0)
            extraSync = getenv("KDE_EXTRA_FSYNC") != 0 ? 1 : 0;
        if (extraSync) {
            if (flush()) {
                forever {
                    if (!FDATASYNC(handle()))
                        break;
                    if (errno != EINTR) {
                        d->error = QFile::WriteError;
                        d->errorString = i18n("Synchronization to disk failed");
                        break;
                    }
                }
            }
        }
#endif

        close();
        
        if( error() != NoError ) {
            QFile::remove(d->tempFileName);
        }
        //Qt does not allow us to atomically overwrite an existing file,
        //so if the target file already exists, there is no way to change it
        //to the temp file without creating a small race condition. So we use
        //the standard rename call instead, which will do the copy without the
        //race condition.
        else if (0 == KDE::rename(d->tempFileName,d->realFileName)) {
            d->error=QFile::NoError;
            d->errorString.clear();
            success = true;
        } else {
            d->error=QFile::OpenError;
            d->errorString=i18n("Error during rename.");
            QFile::remove(d->tempFileName);
        }

        d->wasFinalized = true;
    }

    return success;
}

#undef FDATASYNC

bool KSaveFile::backupFile( const QString& qFilename, const QString& backupDir )
{
    // get backup type from config, by default use "simple"
    // get extension from config, by default use "~"
    // get max number of backups from config, by default set to 10

    KConfigGroup g(KGlobal::config(), "Backups"); // look in the Backups section
    QString type = g.readEntry( "Type", "simple" );
    QString extension = g.readEntry( "Extension", "~" );
    QString message = g.readEntry( "Message", "Automated KDE Commit" );
    int maxnum = g.readEntry( "MaxBackups", 10 );
    if ( type.toLower() == "numbered" ) {
        return( numberedBackupFile( qFilename, backupDir, extension, maxnum ) );
    } else if ( type.toLower() == "rcs" ) {
        return( rcsBackupFile( qFilename, backupDir, message ) );
    } else {
        return( simpleBackupFile( qFilename, backupDir, extension ) );
    }
}

bool KSaveFile::simpleBackupFile( const QString& qFilename,
                                  const QString& backupDir,
                                  const QString& backupExtension )
{
    QString backupFileName = qFilename + backupExtension;
    
    if ( !backupDir.isEmpty() ) {
        QFileInfo fileInfo ( qFilename );
        backupFileName = backupDir + '/' + fileInfo.fileName() + backupExtension;
    }

//    kDebug(180) << "KSaveFile copying " << qFilename << " to " << backupFileName;
    QFile::remove(backupFileName);
    return QFile::copy(qFilename, backupFileName);
}

bool KSaveFile::rcsBackupFile( const QString& qFilename,
                               const QString& backupDir,
                               const QString& backupMessage )
{
    QFileInfo fileInfo ( qFilename );
    
    QString qBackupFilename;
    if ( backupDir.isEmpty() ) {
        qBackupFilename = qFilename;
    } else {
        qBackupFilename = backupDir + fileInfo.fileName();
    }
    qBackupFilename += QString::fromLatin1( ",v" );

    // If backupDir is specified, copy qFilename to the
    // backupDir and perform the commit there, unlinking
    // backupDir/qFilename when finished.
    if ( !backupDir.isEmpty() )
    {
        if ( !QFile::copy(qFilename, backupDir + fileInfo.fileName()) ) {
            return false;
        }
        fileInfo.setFile(backupDir + '/' + fileInfo.fileName());
    }
    
    QString cipath = KStandardDirs::findExe("ci");
    QString copath = KStandardDirs::findExe("co");
    QString rcspath = KStandardDirs::findExe("rcs");
    if ( cipath.isEmpty() || copath.isEmpty() || rcspath.isEmpty() )
        return false;
    
    // Check in the file unlocked with 'ci'
    QProcess ci;
    if ( !backupDir.isEmpty() )
        ci.setWorkingDirectory( backupDir );
    ci.start( cipath, QStringList() << "-u" << fileInfo.filePath() );
    if ( !ci.waitForStarted() )
        return false;
    ci.write( backupMessage.toLatin1() );
    ci.write(".");
    ci.closeWriteChannel();
    if( !ci.waitForFinished() )
        return false;

    // Use 'rcs' to unset strict locking
    QProcess rcs;
    if ( !backupDir.isEmpty() )
        rcs.setWorkingDirectory( backupDir );
    rcs.start( rcspath, QStringList() << "-U" << qBackupFilename );
    if ( !rcs.waitForFinished() )
        return false;

    // Use 'co' to checkout the current revision and restore permissions
    QProcess co;
    if ( !backupDir.isEmpty() )
        co.setWorkingDirectory( backupDir );
    co.start( copath, QStringList() << qBackupFilename );
    if ( !co.waitForFinished() )
        return false;

    if ( !backupDir.isEmpty() ) {
        return QFile::remove( fileInfo.filePath() );
    } else {
        return true;
    }
}

bool KSaveFile::numberedBackupFile( const QString& qFilename,
                                    const QString& backupDir,
                                    const QString& backupExtension,
                                    const uint maxBackups )
{
    QFileInfo fileInfo ( qFilename );
    
    // The backup file name template.
    QString sTemplate;
    if ( backupDir.isEmpty() ) {
        sTemplate = qFilename + ".%1" + backupExtension;
    } else {
        sTemplate = backupDir + '/' + fileInfo.fileName() + ".%1" + backupExtension;
    }

    // First, search backupDir for numbered backup files to remove.
    // Remove all with number 'maxBackups' and greater.
    QDir d = backupDir.isEmpty() ? fileInfo.dir() : backupDir;
    d.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    const QStringList nameFilters = QStringList( fileInfo.fileName() + ".*" + backupExtension );
    d.setNameFilters( nameFilters );
    d.setSorting( QDir::Name );

    uint maxBackupFound = 0;
    foreach ( const QFileInfo &fi, d.entryInfoList() ) {
        if ( fi.fileName().endsWith( backupExtension ) ) {
            // sTemp holds the file name, without the ending backupExtension
            QString sTemp = fi.fileName();
            sTemp.truncate( fi.fileName().length()-backupExtension.length() );
            // compute the backup number
            int idex = sTemp.lastIndexOf( '.' );
            if ( idex > 0 ) {
                bool ok;
                uint num = sTemp.mid( idex+1 ).toUInt( &ok );
                if ( ok ) {
                    if ( num >= maxBackups ) {
                        QFile::remove( fi.filePath() );
                    } else {
                        maxBackupFound = qMax( maxBackupFound, num );
                    }
                }
            }
        }
    }

    // Next, rename max-1 to max, max-2 to max-1, etc.
    QString to=sTemplate.arg( maxBackupFound+1 );
    for ( int i=maxBackupFound; i>0; i-- ) {
        QString from = sTemplate.arg( i );
//        kDebug(180) << "KSaveFile renaming " << from << " to " << to;
        QFile::rename( from, to );
        to = from;
    }

    // Finally create most recent backup by copying the file to backup number 1.
//    kDebug(180) << "KSaveFile copying " << qFilename << " to " << sTemplate.arg(1);
    return QFile::copy(qFilename, sTemplate.arg(1));
}

