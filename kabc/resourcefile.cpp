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

#include "addressbook.h"
#include "stdaddressbook.h"

#include "formatfactory.h"

#include "resourceconfigwidget.h"
#include "resourcefile.h"
#include "resourcefileconfig.h"

using namespace KABC;

ResourceFile::ResourceFile( AddressBook *addressBook, const KConfig *config )
    : Resource( addressBook )
{
  QString fileName = config->readEntry( "FileName" );
  QString type = config->readEntry( "FileFormat" );

  FormatFactory *factory = FormatFactory::self();
  Format *format = factory->format( type );

  init( fileName, format );
}

ResourceFile::ResourceFile( AddressBook *addressBook, const QString &filename,
                            Format *format ) :
  Resource( addressBook )
{
  init( filename, format );
}

ResourceFile::~ResourceFile()
{
  delete mFormat;
}

void ResourceFile::init( const QString &filename, Format *format )
{
  if ( !format ) {
    FormatFactory *factory = FormatFactory::self();
    mFormat = factory->format( "vcard" );
  } else {
    mFormat = format;
  }

  connect( &mDirWatch, SIGNAL( dirty(const QString&) ), SLOT( fileChanged() ) );
  connect( &mDirWatch, SIGNAL( created(const QString&) ), SLOT( fileChanged() ) );
  connect( &mDirWatch, SIGNAL( deleted(const QString&) ), SLOT( fileChanged() ) );

  setFileName( filename );
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


bool ResourceFile::open()
{
  QFile file( mFileName );

  if ( !file.open( IO_ReadOnly ) )
    return true;

  if ( file.size() == 0 )
    return true;

  bool ok = mFormat->checkFormat( &file );
  file.close();

  return ok;
}

void ResourceFile::close()
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
  if ( !file.open( IO_WriteOnly ) ) {
    addressBook()->error( i18n( "Unable to open file '%1'." ).arg( mFileName ) );
    return false;
  }
  
  mFormat->saveAll( addressBook(), this, &file );

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
  lockUniqueName = fn + kapp->randomString(8);
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

void ResourceFile::fileChanged()
{
  load();
  addressBook()->emitAddressBookChanged();
}

QString ResourceFile::identifier() const
{
  return fileName();
}

void ResourceFile::removeAddressee( const Addressee& )
{
  // this function is only used by record-based resources
}

#include "resourcefile.moc"
