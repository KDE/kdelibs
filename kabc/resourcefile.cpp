#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qregexp.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "addressbook.h"
#include "binaryformat.h"
#include "vcardformat.h"
#include "stdaddressbook.h"

#include "resourcefile.h"

using namespace KABC;

ResourceFile::ResourceFile( AddressBook *addressBook, const KConfig *config )
    : Resource( addressBook )
{
  QString path = config->readEntry( "FilePath" );
  uint type = config->readNumEntry( "FileFormat", Format::VCard );

  Format *format = 0;
  switch ( type ) {
    case Format::VCard:
      if ( path.isEmpty() )
        path = StdAddressBook::fileName();
      format = new VCardFormat;
      break;
    case Format::Binary:
      if ( path.isEmpty() )
        path = locateLocal( "data", "kabc/std.bin" );
      format = new BinaryFormat;
      break;
    default:
      kdDebug( 5700 ) << "ResourceFile: no valid format type." << endl;
 }

  init( path, format );
}

ResourceFile::ResourceFile( AddressBook *addressBook, const QString &path,
                            Format *format ) :
  Resource( addressBook )
{
  init( path, format );
}

ResourceFile::~ResourceFile()
{
  delete mFormat;
}

void ResourceFile::init( const QString &path, Format *format )
{
  if ( !format ) {
    mFormat = new VCardFormat();
  } else {
    mFormat = format;
  }

  connect( &mDirWatch, SIGNAL( dirty() ), SLOT( pathChanged() ) );

  setPath( path );
}

Ticket *ResourceFile::requestSaveTicket()
{
  kdDebug(5700) << "ResourceFile::requestSaveTicket()" << endl;

  if ( !addressBook() ) return 0;

  if ( !lock( mPath ) ) {
    kdDebug(5700) << "ResourceFile::requestSaveTicket(): Can't lock path '"
                  << mPath << "'" << endl;
    return 0;
  }
  return createTicket( this );
}


bool ResourceFile::open()
{
  QDir dir( mPath );
  if ( !dir.exists() ) // no directory available
    return dir.mkdir( dir.path() );

  QString testName = dir.entryList( QDir::Files )[0];
  if ( testName.isNull() || testName.isEmpty() ) // no file in directory
    return true;

  QFile file( mPath + "/" + testName );
  if ( !file.open( IO_ReadWrite ) )
    return false;

  bool ok = mFormat->checkFormat( &file );
  file.close();

  return ok;
}

void ResourceFile::close()
{
}

bool ResourceFile::load()
{
  kdDebug(5700) << "ResourceFile::load(): '" << mPath << "'" << endl;

  QDir dir( mPath );
  QStringList files = dir.entryList();

  QStringList::Iterator it;
  bool ok = true;
  for ( it = files.begin(); it != files.end(); ++it ) {
    QFile file( mPath + "/" + (*it) );

    if ( !file.open( IO_ReadOnly ) ) {
      addressBook()->error( QString( i18n( "Can't load file '%1' for reading" ) ).arg( file.name() ) );
      ok = false;
      continue;
    }

    if ( !mFormat->load( addressBook(), this, &file ) )
      ok = false;

    file.close();
  }

  return ok;
}

bool ResourceFile::save( Ticket *ticket )
{
  kdDebug(5700) << "ResourceFile::save(): '" << mPath << "'" << endl;
  
  AddressBook::Iterator it;
  bool ok = true;

  for ( it = addressBook()->begin(); it != addressBook()->end(); ++it ) {
    if ( (*it).resource() != this || !(*it).changed() )
      continue;

    QFile file( mPath + "/" + (*it).uid() );
    if ( !file.open( IO_ReadWrite ) ) {
      addressBook()->error( QString( i18n( "Can't load file '%1' for saving." ) ).arg( file.name() ) );
      continue;
    }

    bool success = mFormat->save( &(*it), &file );

    if ( !success ) {
      ok = false;
      file.close();
      QFile::remove( file.name() );
      addressBook()->error( QString( i18n( "Can't save file '%1'." ) ).arg( file.name() ) );
      continue;
    }

    // mark as unchanged
    (*it).setChanged( false );

    file.close();
  }

  delete ticket;
  unlock( mPath );

  return ok;
}

bool ResourceFile::lock( const QString &path )
{
  kdDebug(5700) << "ResourceFile::lock()" << endl;

  QString p = path;
  p.replace( QRegExp("/"), "_" );

  QString lockName = locateLocal( "data", "kabc/lock/" + p + ".lock" );
  kdDebug(5700) << "-- lock name: " << lockName << endl;

  if ( QFile::exists( lockName ) ) return false;

  QString lockUniqueName;
  lockUniqueName = p + kapp->randomString( 8 );
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

void ResourceFile::unlock( const QString &path )
{
  QString p = path;
  p.replace( QRegExp( "/" ), "_" );

  QString lockName = locate( "data", "kabc/lock/" + p + ".lock" );
  ::unlink( QFile::encodeName( lockName ) );
  QFile::remove( mLockUniqueName );
  addressBook()->emitAddressBookUnlocked();
}

void ResourceFile::setPath( const QString &path )
{
  mDirWatch.stopScan();
  mDirWatch.removeDir( mPath );

  mPath = path;
  mDirWatch.addDir( mPath, true );
  mDirWatch.startScan();
}

QString ResourceFile::path() const
{
  return mPath;
}

void ResourceFile::pathChanged()
{
  load();
  addressBook()->emitAddressBookChanged();
}

QString ResourceFile::identifier() const
{
    return path();
}

void ResourceFile::removeAddressee( const Addressee& addr )
{
    QFile::remove( mPath + "/" + addr.uid() );
}

#include "resourcefile.moc"
