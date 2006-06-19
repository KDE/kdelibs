/*
    This file is part of libkabc.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2006 Tom Abers <tomalbers@kde.nl>

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

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kio/scheduler.h>
#include <klocale.h>
#include <ksavefile.h>
#include <kstandarddirs.h>

#include "formatfactory.h"
#include "resourcefileconfig.h"
#include "stdaddressbook.h"
#include "lock.h"

#include "resourcefile.h"

using namespace KABC;

ResourceFile::ResourceFile( const KConfig *config )
  : Resource( config ), mFormat( 0 ),
    mAsynchronous( false )
{
  QString fileName, formatName;

  if ( config ) {
    fileName = config->readPathEntry( "FileName", StdAddressBook::fileName() );
    formatName = config->readEntry( "FileFormat", "vcard" );
  } else {
    fileName = StdAddressBook::fileName();
    formatName = "vcard";
  }

  init( fileName, formatName );
}

ResourceFile::ResourceFile( const QString &fileName,
                            const QString &formatName )
  : Resource( 0 ), mFormat( 0 ),
    mAsynchronous( false )
{
  init( fileName, formatName );
}

void ResourceFile::init( const QString &fileName, const QString &formatName )
{
  mFormatName = formatName;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );

  if ( !mFormat ) {
    mFormatName = "vcard";
    mFormat = factory->format( mFormatName );
  }

  connect( &mDirWatch, SIGNAL( dirty(const QString&) ), SLOT( fileChanged() ) );
  connect( &mDirWatch, SIGNAL( created(const QString&) ), SLOT( fileChanged() ) );
  connect( &mDirWatch, SIGNAL( deleted(const QString&) ), SLOT( fileChanged() ) );

  setFileName( fileName );

  mLock = 0;
}

ResourceFile::~ResourceFile()
{
  delete mFormat;
  mFormat = 0;
}

void ResourceFile::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  if ( mFileName == StdAddressBook::fileName() )
    config->deleteEntry( "FileName" );
  else
    config->writePathEntry( "FileName", mFileName );

  config->writeEntry( "FileFormat", mFormatName );
}

Ticket *ResourceFile::requestSaveTicket()
{
  kdDebug(5700) << "ResourceFile::requestSaveTicket()" << endl;

  if ( !addressBook() ) return 0;

  delete mLock;
  mLock = new Lock( mFileName );

  if ( mLock->lock() ) {
    addressBook()->emitAddressBookLocked();
  } else {
    addressBook()->error( mLock->error() );
    kdDebug(5700) << "ResourceFile::requestSaveTicket(): Unable to lock file '"
                  << mFileName << "': " << mLock->error() << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceFile::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;

  delete mLock;
  mLock = 0;

  addressBook()->emitAddressBookUnlocked();
}

bool ResourceFile::doOpen()
{
  QFile file( mFileName );

  if ( !file.exists() ) {
    // try to create the file
    bool ok = file.open( IO_WriteOnly );
    if ( ok )
      file.close();

    return ok;
  } else {
    QFileInfo fileInfo( mFileName );
    if ( readOnly() || !fileInfo.isWritable() ) {
      if ( !file.open( IO_ReadOnly ) )
        return false;
    } else {
      if ( !file.open( IO_ReadWrite ) )
        return false;
    }

    if ( file.size() == 0 ) {
      file.close();
      kdDebug() << "File size is zero. Evaluating backups" << endl;
      for (int i=0; i!=20; i++)
      {
        QFile backup( mFileName + "__" + QString::number(i) );
        kdDebug() << "Evaluating" << backup.name() << " size: " << backup.size() << endl;
        if ( backup.size() != 0 )
        {
          kdDebug() << "Restoring backup " << i << endl;
          const QString src = mFileName + "__" + QString::number(i);
          const QString dest = mFileName;

          // remove dest
          QFile::remove( dest );

          // copy src to dest
          if ( backup.open( IO_ReadOnly ) ) {
            const QByteArray data = backup.readAll();

            QFile out( dest );
            if ( out.open( IO_WriteOnly ) ) {
              out.writeBlock( data );
              out.close();
            }

            backup.close();
          }
          return true; 
        }
      }
      return true;
    }

    bool ok = mFormat->checkFormat( &file );
    file.close();

    return ok;
  }
}

void ResourceFile::doClose()
{
}

bool ResourceFile::load()
{
  kdDebug(5700) << "ResourceFile::load(): '" << mFileName << "'" << endl;

  mAsynchronous = false;

  QFile file( mFileName );
  if ( !file.open( IO_ReadOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mFileName ) );
    return false;
  }

  clear();

  return mFormat->loadAll( addressBook(), this, &file );
}

bool ResourceFile::asyncLoad()
{
  kdDebug(5700) << "ResourceFile::asyncLoad()" << endl;

  mAsynchronous = true;

  bool ok = load();

  if ( !ok )
    emitLoadingError();
  else
    emitLoadingFinished();

  return true;
}

bool ResourceFile::save( Ticket * )
{
  kdDebug(5700) << "ResourceFile::save()" << endl;

  // Only do the logrotate dance when the __0 file is not 0 bytes.
  QFile file( mFileName + "__0" );
  if ( file.size() != 0 ) {
    const QString last = mFileName + "__20";
    kdDebug() << "deleting " << last << endl;

    QFile::remove( last );

    for (int i=19; i>=0; i--)
    {
      const QString src = mFileName + "__" + QString::number(i);
      const QString dest = mFileName + "__" + QString::number(i+1);
      kdDebug() << "moving " << src << " -> " << dest << endl;

      // copy src to dest
      QFile in( src );
      if ( in.open( IO_ReadOnly ) ) {
        const QByteArray data = in.readAll();

        QFile out( dest );
        if ( out.open( IO_WriteOnly ) ) {
          out.writeBlock( data );
          out.close();
        }

        in.close();
      }

      // remove src
      QFile::remove( src );
    }
  } else
    kdDebug() << "Not starting logrotate __0 is 0 bytes." << endl;

  QString extension = "__0";
  (void) KSaveFile::backupFile( mFileName, QString::null /*directory*/,
                                extension );

  mDirWatch.stopScan();

  KSaveFile saveFile( mFileName );
  bool ok = false;

  if ( saveFile.status() == 0 && saveFile.file() ) {
    mFormat->saveAll( addressBook(), this, saveFile.file() );
    ok = saveFile.close();
  }

  if ( !ok ) {
    saveFile.abort();
    addressBook()->error( i18n( "Unable to save file '%1'." ).arg( mFileName ) );
  }

  mDirWatch.startScan();

  return ok;
}

bool ResourceFile::asyncSave( Ticket *ticket )
{
  kdDebug(5700) << "ResourceFile::asyncSave()" << endl;

  bool ok = save( ticket );

  if ( !ok )
    QTimer::singleShot( 0, this, SLOT( emitSavingError() ) );
  else
    QTimer::singleShot( 0, this, SLOT( emitSavingFinished() ) );

  return ok;
}

void ResourceFile::setFileName( const QString &fileName )
{
  mDirWatch.stopScan();
  if ( mDirWatch.contains( mFileName ) )
    mDirWatch.removeFile( mFileName );

  mFileName = fileName;

  mDirWatch.addFile( mFileName );
  mDirWatch.startScan();
}

QString ResourceFile::fileName() const
{
  return mFileName;
}

void ResourceFile::setFormat( const QString &format )
{
  mFormatName = format;
  delete mFormat;

  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( mFormatName );
}

QString ResourceFile::format() const
{
  return mFormatName;
}

void ResourceFile::fileChanged()
{
    kdDebug(5700) << "ResourceFile::fileChanged(): " << mFileName << endl;

  if ( !addressBook() )
    return;

  if ( mAsynchronous )
    asyncLoad();
  else {
    load();
    kdDebug() << "addressBookChanged() " << endl;
    addressBook()->emitAddressBookChanged();
  }
}

void ResourceFile::removeAddressee( const Addressee &addr )
{
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/photos/" ) + addr.uid() ) );
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/logos/" ) + addr.uid() ) );
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/sounds/" ) + addr.uid() ) );

  mAddrMap.erase( addr.uid() );
}

void ResourceFile::emitSavingFinished()
{
  emit savingFinished( this );
}

void ResourceFile::emitSavingError()
{
  emit savingError( this, i18n( "Unable to save file '%1'." ).arg( mFileName ) );
}

void ResourceFile::emitLoadingFinished()
{
  emit loadingFinished( this );
}

void ResourceFile::emitLoadingError()
{
  emit loadingError( this, i18n( "Problems during parsing file '%1'." ).arg( mFileName ) );
}

#include "resourcefile.moc"
