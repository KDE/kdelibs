#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qfile.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "formatfactory.h"
#include "resourcefileconfig.h"
#include "stdaddressbook.h"

#include "resourcefile.h"

using namespace KABC;

extern "C"
{
  KRES::ResourceConfigWidget *config_widget( QWidget *parent ) {
    KGlobal::locale()->insertCatalogue( "kabc_file" );
    return new ResourceFileConfig( parent, "ResourceFileConfig" );
  }

  Resource *resource( const KConfig *config ) {
    KGlobal::locale()->insertCatalogue( "kabc_file" );
    return new ResourceFile( config );
  }
}

ResourceFile::ResourceFile( const KConfig *config )
    : Resource( config ), mFormat( 0 )
{
  QString fileName;

  if ( config ) {
    fileName = config->readEntry( "FileName", StdAddressBook::fileName() );
    mFormatName = config->readEntry( "FileFormat", "vcard" );
  } else {
    fileName = StdAddressBook::fileName();
    mFormatName = "vcard";
  }

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
}

ResourceFile::~ResourceFile()
{
  delete mFormat;
  mFormat = 0;
}

void ResourceFile::writeConfig( KConfig *config )
{
  Resource::writeConfig( config );

  config->writeEntry( "FileName", mFileName );
  config->writeEntry( "FileFormat", mFormatName );
}

Ticket *ResourceFile::requestSaveTicket()
{
  kdDebug(5700) << "ResourceFile::requestSaveTicket()" << endl;

  if ( !addressBook() ) return 0;

  if ( !lock( mFileName ) ) {
    kdDebug(5700) << "ResourceFile::requestSaveTicket(): Unable to lock file '"
                  << mFileName << "'" << endl;
    return 0;
  }
  return createTicket( this );
}


bool ResourceFile::doOpen()
{
  QFile file( mFileName );

  if ( !file.exists() )
    return true;

  if ( !file.open( IO_ReadOnly ) )
    return true;

  if ( file.size() == 0 )
    return true;

  bool ok = mFormat->checkFormat( &file );
  file.close();

  return ok;
}

void ResourceFile::doClose()
{
}

bool ResourceFile::load()
{
  kdDebug(5700) << "ResourceFile::load(): '" << mFileName << "'" << endl;

  QFile file( mFileName );
  if ( !file.open( IO_ReadOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mFileName ) );
    return false;
  }

  return mFormat->loadAll( addressBook(), this, &file );
}

bool ResourceFile::save( Ticket *ticket )
{
  kdDebug(5700) << "ResourceFile::save()" << endl;

  QFile file( mFileName );
  file.open( IO_ReadOnly );

  QByteArray array = file.readAll();
  file.close();

  // create backup file
  QFile backupFile( QString( "%1_%2" ).arg( mFileName )
                                      .arg( QDate::currentDate().dayOfWeek() ) );
  if ( backupFile.open( IO_WriteOnly ) ) {
    backupFile.writeBlock( array );
    backupFile.close();
  }

  if ( !file.open( IO_WriteOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mFileName ) );
    return false;
  }
  
  mFormat->saveAll( addressBook(), this, &file );
  file.close();

  delete ticket;
  unlock( mFileName );

  return true;
}

bool ResourceFile::lock( const QString &fileName )
{
  kdDebug(5700) << "ResourceFile::lock()" << endl;

  QString fn = fileName;
  fn.replace( QRegExp("/"), "_" );

  QString lockName = locateLocal( "data", "kabc/lock/" + fn + ".lock" );
  kdDebug(5700) << "-- lock name: " << lockName << endl;

  if (QFile::exists( lockName )) return false;

  QString lockUniqueName;
  lockUniqueName = fn + kapp->randomString( 8 );
  mLockUniqueName = locateLocal( "data", "kabc/lock/" + lockUniqueName );
  kdDebug(5700) << "-- lock unique name: " << mLockUniqueName << endl;

  // Create unique file
  QFile file( mLockUniqueName );
  file.open( IO_WriteOnly );
  file.close();

  // Create lock file
  int result = ::link( QFile::encodeName( mLockUniqueName ),
                       QFile::encodeName( lockName ) );

  if ( result == 0 ) {
    addressBook()->emitAddressBookLocked();
    return true;
  }

  // TODO: check stat

  return false;
}

void ResourceFile::unlock( const QString &fileName )
{
  QString fn = fileName;
  fn.replace( QRegExp( "/" ), "_" );

  QString lockName = locate( "data", "kabc/lock/" + fn + ".lock" );
  ::unlink( QFile::encodeName( lockName ) );
  QFile::remove( mLockUniqueName );
  addressBook()->emitAddressBookUnlocked();
}

void ResourceFile::setFileName( const QString &fileName )
{
  mDirWatch.stopScan();
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
  if ( mFormat )
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
  load();
  addressBook()->emitAddressBookChanged();
}

void ResourceFile::removeAddressee( const Addressee &addr )
{
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/photos/" ) + addr.uid() ) );
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/logos/" ) + addr.uid() ) );
  QFile::remove( QFile::encodeName( locateLocal( "data", "kabc/sounds/" ) + addr.uid() ) );
}

void ResourceFile::cleanUp()
{
  unlock( mFileName );
}

#include "resourcefile.moc"
