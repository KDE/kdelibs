/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
  Copyright (c) 2006 Allen Winter <winter@kde.org>
  Copyright (c) 2006 Gregory S. Hayes <syncomm@kde.org>

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

#undef QT3_SUPPORT
#include <config.h>

#include <sys/types.h>
#include <errno.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_TEST
#include <test.h>
#endif

#include <qdir.h>
#include <qstring.h>
#include <QProcess>

#include <kde_file.h>
#include "kapplication.h"
#include "ksavefile.h"
#include "kstandarddirs.h"
#include "ktempfile.h"
#include "kconfig.h"

class KSaveFile::Private
{
public:
   QString _fileName;
#define mFileName d->_fileName
   KTempFile *_tempFile;
#define mTempFile d->_tempFile
   ~Private() { delete _tempFile; }
};

KSaveFile::KSaveFile(const QString &filename, int mode)
 : d(new Private)
{
   mTempFile = new KTempFile(true);

   // follow symbolic link, if any
   QString real_filename = KStandardDirs::realFilePath(filename);

   // we only check here if the directory can be written to
   // the actual filename isn't written to, but replaced later
   // with the contents of our tempfile
   if (!checkAccess(real_filename, W_OK))
   {
      mTempFile->setError(EACCES);
      return;
   }

   if (mTempFile->create(real_filename, QLatin1String(".new"), mode))
   {
      mFileName = real_filename; // Set filename upon success

      // if we're overwriting an existing file, ensure temp file's
      // permissions are the same as existing file so the existing
      // file's permissions are preserved
      KDE_struct_stat stat_buf;
      if (KDE_stat(QFile::encodeName(real_filename), &stat_buf)==0)
      {
         // But only if we own the existing file
         if (stat_buf.st_uid == getuid())
         {
            bool changePermission = true;
            if (stat_buf.st_gid != getgid())
      {
               if (fchown(mTempFile->handle(), (uid_t) -1, stat_buf.st_gid) != 0)
               {
                  // Use standard permission if we can't set the group
                  changePermission = false;
               }
            }
            if (changePermission)
               fchmod(mTempFile->handle(), stat_buf.st_mode);
         }
      }
   }
}

KSaveFile::~KSaveFile()
{
   if (mTempFile->isOpen())
      close(); // Close if we were still open

   delete d;
}

QString
KSaveFile::name() const
{
   return mFileName;
}

void
KSaveFile::abort()
{
   mTempFile->close();
   mTempFile->unlink();
}

bool
KSaveFile::close()
{
   if (mTempFile->name().isEmpty() || mTempFile->handle()==-1)
      return false; // Save was aborted already
   if (!mTempFile->sync())
   {
      abort();
      return false;
   }
   if (mTempFile->close())
   {
      if (0==KDE_rename(QFile::encodeName(mTempFile->name()),
                        QFile::encodeName(mFileName)))
         return true; // Success!
      mTempFile->setError(errno);
   }
   // Something went wrong, make sure to delete the interim file.
   mTempFile->unlink();
   return false;
}

static int
write_all(int fd, const char *buf, size_t len)
{
   while (len > 0)
   {
      int written = write(fd, buf, len);
      if (written < 0)
      {
          if (errno == EINTR)
             continue;
          return -1;
      }
      buf += written;
      len -= written;
   }
   return 0;
}

static bool
copy_all( int fd, KDE_struct_stat *buff, QString newName )
{
   QByteArray cNewName = QFile::encodeName( newName );
   const char *copyname = cNewName.data();
   int permissions = buff->st_mode & 07777;

   if ( KDE_stat( copyname, buff ) == 0 )
   {
      if ( unlink( copyname ) != 0 )
      {
         ::close( fd );
         return false;
      }
   }

   mode_t old_umask = umask( 0 );
   int fd2 = KDE_open( copyname,
                       O_WRONLY | O_CREAT | O_EXCL, permissions | S_IWUSR );
   umask( old_umask );

   if ( fd2 < 0 )
   {
      ::close( fd );
      return false;
   }

    char buffer[ 32*1024 ];

    while( 1 )
    {
       int n = ::read( fd, buffer, 32*1024 );
       if ( n == -1 )
       {
          if ( errno == EINTR )
              continue;
          ::close( fd );
          ::close( fd2 );
          return false;
       }
       if ( n == 0 )
          break; // Finished

       if ( write_all( fd2, buffer, n ) )
       {
          ::close( fd );
          ::close( fd2 );
          return false;
       }
    }

    ::close( fd );

    if ( ::close( fd2 ) )
        return false;

    // successfully created copy file
    return true;
}

bool KSaveFile::backupFile( const QString& qFilename, const QString& backupDir )
{
    // get backup type from config, by default use "simple"
    // get extension from config, by default use "~"
    // get max number of backups from config, by default set to 10

    KConfigGroup g( KGlobal::config(), "Backups" ); // look in the Backups section
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
   QByteArray cFilename = QFile::encodeName( qFilename );
   const char *filename = cFilename.data();

   int fd = KDE_open( filename, O_RDONLY );
   if ( fd < 0 )
      return false;

   KDE_struct_stat buff;
   if ( KDE_fstat( fd, &buff ) < 0 )
   {
      ::close( fd );
      return false;
   }

   QString sBackup;
   if ( backupDir.isEmpty() )
       sBackup = qFilename;
   else
   {
       QString nameOnly;
       int slash = qFilename.lastIndexOf('/');
       if (slash < 0)
	   nameOnly = qFilename;
       else
	   nameOnly = qFilename.mid(slash + 1);
       sBackup = backupDir;
       if ( backupDir[backupDir.length()-1] != '/' )
           sBackup += '/';
       sBackup += nameOnly;
   }
   sBackup += backupExtension;

   return( copy_all( fd, &buff, sBackup ) );
}

bool KSaveFile::rcsBackupFile( const QString& qFilename,
                               const QString& backupDir,
                               const QString& backupMessage )
{
    QString qBackupFilename = qFilename;
    qBackupFilename += QString::fromLatin1( ",v" );

    // If backupDir is specified, copy qFilename to the
    // backupDir and perform the commit there, unlinking
    // backupDir/qFilename when finished.
    if ( !backupDir.isEmpty() ) 
    {
        QString sBackup = backupDir + "/" + qFilename;
        QByteArray cFilename = QFile::encodeName( qFilename );
        const char *filename = cFilename.data();
        int fd = KDE_open( filename, O_RDONLY );
        if ( fd < 0 )
            return false;

        KDE_struct_stat buff;
        if ( KDE_fstat( fd, &buff ) < 0 )
        {
            ::close( fd );
            return false;
        }
        if( !copy_all( fd, &buff, sBackup ) )
            return false;
    }

    // Check in the file unlocked with 'ci'
    QProcess ci;
    if ( !backupDir.isEmpty() )
        ci.setWorkingDirectory( backupDir );
    ci.start( "ci", QStringList() << "-u" << qFilename );
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
    rcs.start( "rcs", QStringList() << "-U" << qBackupFilename );
    if ( !rcs.waitForFinished() )
        return false;

    // Use 'co' to checkout the current revision and restore permissions
    QProcess co;
    if ( !backupDir.isEmpty() )
        co.setWorkingDirectory( backupDir );
    co.start( "co", QStringList() << qBackupFilename );
    if ( !co.waitForFinished() )
        return false;

    if ( !backupDir.isEmpty() ) {
        return( QFile::remove( backupDir + '/' + qFilename ) );
    } else {
        return true;
    }
}

bool KSaveFile::numberedBackupFile( const QString& qFilename,
                                    const QString& backupDir,
                                    const QString& backupExtension,
                                    const uint maxBackups )
{
   QByteArray cFilename = QFile::encodeName( qFilename );
   const char *filename = cFilename.data();

   int fd = KDE_open( filename, O_RDONLY );
   if ( fd < 0 )
      return false;

   KDE_struct_stat buff;
   if ( KDE_fstat( fd, &buff) < 0 )
   {
      ::close( fd );
      return false;
   }

   QString sBackup;
   if ( backupDir.isEmpty() )
       sBackup = qFilename;
   else
   {
       QString nameOnly;
       int slash = qFilename.lastIndexOf( '/' );
       if ( slash < 0 )
	   nameOnly = qFilename;
       else
	   nameOnly = qFilename.mid( slash + 1 );
       sBackup = backupDir;
       if ( backupDir[backupDir.length()-1] != '/' )
           sBackup += '/';
       sBackup += nameOnly;
   }

   // The backup file name template.
   QString sTemplate = sBackup + "." + "%1" + backupExtension;

   // First, search backupDir for numbered backup files to remove.
   // Remove all with number 'maxBackups' and greater.
   QDir d( backupDir );
   d.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
   QStringList nameFilters = QStringList( sTemplate.arg( "*" ) );
   d.setNameFilters( nameFilters );
   d.setSorting( QDir::Name );

   QFileInfoList list = d.entryInfoList();

   uint maxBackupFound = 0;
   for ( int i=0; i<list.size(); ++i ) {
       QFileInfo fi = list.at( i );
       if ( fi.fileName().endsWith( backupExtension ) ) {
           // sTemp holds the file name, without the ending backupExtension
           QString sTemp = fi.fileName();
           sTemp.truncate( fi.fileName().length()-backupExtension.length() );
           // compute the backup number
           int idex = sTemp.lastIndexOf( "." );
           if ( idex > 0 ) {
               bool ok;
               uint num = sTemp.mid( idex+1 ).toUInt( &ok );
               if ( ok ) {
                   if ( num >= maxBackups ) {
                       unlink( QFile::encodeName( fi.fileName() ) );
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
       KDE_rename( QFile::encodeName( from ), QFile::encodeName( to ) );
       to = from;
   }

   // Finally create most recent backup by copying the file to backup number 1.
   return( copy_all( fd, &buff, sTemplate.arg( 1 ) ) );
}

int KSaveFile::status() const
{
   return mTempFile->status();
}

int KSaveFile::handle() const
{
   return mTempFile->handle();
}

FILE* KSaveFile::fstream()
{
   return mTempFile->fstream();
}

QFile* KSaveFile::file()
{
   return mTempFile->file();
}

QTextStream* KSaveFile::textStream()
{
   return mTempFile->textStream();
}

QDataStream* KSaveFile::dataStream()
{
   return mTempFile->dataStream();
}

#undef mFileName
#undef mTempFile
