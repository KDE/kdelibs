#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qfile.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "addressbook.h"
#include "binaryformat.h"
#include "vcardformat.h"

#include "resourcefile.h"

using namespace KABC;

ResourceFile::ResourceFile( AddressBook *addressBook, const KConfig *config )
    : Resource( addressBook )      
{
	QString fileName = config->readEntry( "FileName" );
	uint type = config->readNumEntry( "FileFormat", FORMAT_VCARD );

	Format *format = 0;
        switch ( type ) {
	    case FORMAT_VCARD:
		if ( fileName.isEmpty() )
		    fileName = locateLocal( "data", "kabc/std.vcf" );
		format = new VCardFormat;
		break;
	    case FORMAT_BINARY:
		if ( fileName.isEmpty() )
		    fileName = locateLocal( "data", "kabc/std.bin" );
		format = new BinaryFormat;
		break;
	    default:
		kdDebug( 5700 ) << "ResourceFile: no valid format type." << endl;
	}

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
    mFormat = new VCardFormat();
  } else {
    mFormat = format;
  }

  mFileCheckTimer = new QTimer( this );

  setFileName( filename );

  connect( mFileCheckTimer, SIGNAL( timeout() ), SLOT( checkFile() ) );
}

Ticket *ResourceFile::requestSaveTicket()
{
  kdDebug(5700) << "ResourceFile::requestSaveTicket()" << endl;

  if ( !addressBook() ) return 0;

  if ( !lock( mFileName ) ) {
    kdDebug(5700) << "ResourceFile::requestSaveTicket(): Can't lock file '"
                  << mFileName << "'" << endl;
    return 0;
  }
  return createTicket( this );
}


bool ResourceFile::open()
{
  return true;
}

void ResourceFile::close()
{
}

bool ResourceFile::load()
{
  kdDebug(5700) << "ResourceFile::load(): '" << mFileName << "'" << endl;

  return mFormat->load( addressBook(), this, mFileName );
}

bool ResourceFile::save( Ticket *ticket )
{
  kdDebug(5700) << "ResourceFile::save()" << endl;
  
  bool success = mFormat->save( addressBook(), this, mFileName );

  delete ticket;
  unlock( mFileName );

  return success;
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
  mFileName = fileName;

  struct stat s;
  int result = stat( QFile::encodeName( mFileName ), &s );
  if ( result == 0 ) {
    mChangeTime  = s.st_ctime;
  }

  mFileCheckTimer->start( 500 );
}

QString ResourceFile::fileName() const
{
  return mFileName;
}

void ResourceFile::checkFile()
{
  struct stat s;
  int result = stat( QFile::encodeName( mFileName ), &s );

#if 0
  kdDebug(5700) << "AddressBook::checkFile() result: " << result
            << " new ctime: " << s.st_ctime
            << " old ctime: " << mChangeTime
            << endl;
#endif

  if ( result == 0 && ( mChangeTime != s.st_ctime ) ) {
    mChangeTime  = s.st_ctime;
    load();
    addressBook()->emitAddressBookChanged();
  }
}

QString ResourceFile::identifier() const
{
    return fileName();
}

void ResourceFile::removeAddressee( const Addressee& addr )
{
    mFormat->removeAddressee( addr );
}

#include "resourcefile.moc"
