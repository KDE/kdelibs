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
#include "resourcedirconfig.h"

#include "resourcedir.h"

using namespace KABC;

extern "C"
{
  ResourceConfigWidget *config_widget( QWidget *parent ) {
    return new ResourceDirConfig( parent, "ResourceDirConfig" );
  }

  Resource *resource( AddressBook *ab, const KConfig *config ) {
    return new ResourceDir( ab, config );
  }
}


ResourceDir::ResourceDir( AddressBook *addressBook, const KConfig *config )
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
      kdDebug( 5700 ) << "ResourceDir: no valid format type." << endl;
 }

  init( path, format );
}

ResourceDir::ResourceDir( AddressBook *addressBook, const QString &path,
                            Format *format ) :
  Resource( addressBook )
{
  init( path, format );
}

ResourceDir::~ResourceDir()
{
  delete mFormat;
}

void ResourceDir::init( const QString &path, Format *format )
{
  if ( !format ) {
    mFormat = new VCardFormat();
  } else {
    mFormat = format;
  }

  connect( &mDirWatch, SIGNAL( dirty(const QString&) ), SLOT( pathChanged() ) );
  connect( &mDirWatch, SIGNAL( created(const QString&) ), SLOT( pathChanged() ) );
  connect( &mDirWatch, SIGNAL( deleted(const QString&) ), SLOT( pathChanged() ) );

  setPath( path );
}

Ticket *ResourceDir::requestSaveTicket()
{
  kdDebug(5700) << "ResourceDir::requestSaveTicket()" << endl;

  if ( !addressBook() ) return 0;

  if ( !lock( mPath ) ) {
    kdDebug(5700) << "ResourceDir::requestSaveTicket(): Can't lock path '"
                  << mPath << "'" << endl;
    return 0;
  }
  return createTicket( this );
}


bool ResourceDir::open()
{
  QDir dir( mPath );
  if ( !dir.exists() ) // no directory available
    return dir.mkdir( dir.path() );

  QString testName = dir.entryList( QDir::Files )[0];
  if ( testName.isNull() || testName.isEmpty() ) // no file in directory
    return true;

  QFile file( mPath + "/" + testName );
  if ( !file.open( IO_ReadWrite ) ) {
    addressBook()->error( i18n( "Can't create file '%1'." ).arg( file.name() ) );
    return false;
  }

  if ( file.size() == 0 )
    return true;

  bool ok = mFormat->checkFormat( &file );
  file.close();

  return ok;
}

void ResourceDir::close()
{
}

bool ResourceDir::load()
{
  kdDebug(5700) << "ResourceDir::load(): '" << mPath << "'" << endl;

  QDir dir( mPath );
  QStringList files = dir.entryList();

  QStringList::Iterator it;
  bool ok = true;
  for ( it = files.begin(); it != files.end(); ++it ) {
    QFile file( mPath + "/" + (*it) );

    if ( !file.open( IO_ReadOnly ) ) {
      addressBook()->error( i18n( "Unable to open file '%1' for reading" ).arg( file.name() ) );
      ok = false;
      continue;
    }

    if ( !mFormat->load( addressBook(), this, &file ) )
      ok = false;

    file.close();
  }

  return ok;
}

bool ResourceDir::save( Ticket *ticket )
{
  kdDebug(5700) << "ResourceDir::save(): '" << mPath << "'" << endl;
  
  AddressBook::Iterator it;
  bool ok = true;

  for ( it = addressBook()->begin(); it != addressBook()->end(); ++it ) {
    if ( (*it).resource() != this || !(*it).changed() )
      continue;

    QFile file( mPath + "/" + (*it).uid() );
    if ( !file.open( IO_ReadWrite ) ) {
      addressBook()->error( i18n( "Unable to open file '%1' for writing" ).arg( file.name() ) );
      continue;
    }

    bool success = mFormat->save( *it, &file );

    if ( !success ) {
      ok = false;
      file.close();
      QFile::remove( file.name() );
      addressBook()->error( i18n( "Unable to save file '%1'." ).arg( file.name() ) );
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

bool ResourceDir::lock( const QString &path )
{
  kdDebug(5700) << "ResourceDir::lock()" << endl;

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

void ResourceDir::unlock( const QString &path )
{
  QString p = path;
  p.replace( QRegExp( "/" ), "_" );

  QString lockName = locate( "data", "kabc/lock/" + p + ".lock" );
  ::unlink( QFile::encodeName( lockName ) );
  QFile::remove( mLockUniqueName );
  addressBook()->emitAddressBookUnlocked();
}

void ResourceDir::setPath( const QString &path )
{
  mDirWatch.stopScan();
  mDirWatch.removeDir( mPath );

  mPath = path;
  mDirWatch.addDir( mPath, true );
  mDirWatch.startScan();
}

QString ResourceDir::path() const
{
  return mPath;
}

void ResourceDir::pathChanged()
{
  load();
  addressBook()->emitAddressBookChanged();
}

QString ResourceDir::identifier() const
{
    return path();
}

void ResourceDir::removeAddressee( const Addressee& addr )
{
    QFile::remove( mPath + "/" + addr.uid() );
}

#include "resourcedir.moc"
