#include <qfile.h>

#include <kdebug.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kurlrequester.h>

#include "addressbook.h"
#include "formatfactory.h"
#include "resourcenetconfig.h"
#include "stdaddressbook.h"

#include "resourcenet.h"

using namespace KABC;

extern "C"
{
  ResourceConfigWidget *config_widget( QWidget *parent ) {
    KGlobal::locale()->insertCatalogue("kabc_net");
    return new ResourceNetConfig( parent, "ResourceDirConfig" );
  }

  Resource *resource( AddressBook *ab, const KConfig *config ) {
    KGlobal::locale()->insertCatalogue("kabc_net");
    return new ResourceNet( ab, config );
  }
}


ResourceNet::ResourceNet( AddressBook *addressBook, const KConfig *config )
    : Resource( addressBook )
{
  KURL url = config->readEntry( "NetUrl" );
  QString type = config->readEntry( "NetFormat" );

  FormatFactory *factory = FormatFactory::self();
  FormatPlugin *format = factory->format( type );

  init( url, format );
}

ResourceNet::ResourceNet( AddressBook *addressBook, const KURL &url,
                          FormatPlugin *format ) :
  Resource( addressBook )
{
  init( url, format );
}

ResourceNet::~ResourceNet()
{
  delete mFormat;
}

void ResourceNet::init( const KURL &url, FormatPlugin *format )
{
  if ( !format ) {
    FormatFactory *factory = FormatFactory::self();
    mFormat = factory->format( "vcard" );
  } else {
    mFormat = format;
  }

  setUrl( url );
}

Ticket *ResourceNet::requestSaveTicket()
{
  kdDebug(5700) << "ResourceNet::requestSaveTicket()" << endl;

  if ( !addressBook() )
    return 0;

  return createTicket( this );
}


bool ResourceNet::open()
{
  KIO::UDSEntryList entries;
  if ( !KIO::NetAccess::listDir( mUrl, entries ) )
    return false;

  return true;
}

void ResourceNet::close()
{
}

bool ResourceNet::load()
{
  kdDebug(5700) << "ResourceNet::load(): '" << mUrl.url() << "'" << endl;

  KIO::UDSEntryList entries;
  if ( !KIO::NetAccess::listDir( mUrl, entries, false, false ) )
    return false;

  QStringList files = KIO::convertUDSEntryListToFileNames( entries );

  QStringList::Iterator it;
  bool ok = true;
  for ( it = files.begin(); it != files.end(); ++it ) {
    if ( (*it).endsWith( "/" ) ) // is a directory
      continue;

    QString tmpFile;
    if ( KIO::NetAccess::download( mUrl.url() + "/" + (*it), tmpFile ) ) {
      QFile file( tmpFile );
      if ( !file.open( IO_ReadOnly ) ) {
        addressBook()->error( i18n( "Unable to open file '%1' for reading" ).arg( file.name() ) );
        ok = false;
      } else {
        if ( !mFormat->loadAll( addressBook(), this, &file ) )
        ok = false;
      }

      KIO::NetAccess::removeTempFile( tmpFile );
    } else {
      addressBook()->error( i18n( "Unable to open URL '%1' for reading" ).arg( mUrl.url() + "/" + (*it) ) );
      ok = false;
    }
  }

  return ok;
}

bool ResourceNet::save( Ticket *ticket )
{
  kdDebug(5700) << "ResourceNet::save(): '" << mUrl.url() << "'" << endl;

  AddressBook::Iterator it;
  bool ok = true;

  for ( it = addressBook()->begin(); it != addressBook()->end(); ++it ) {
    if ( (*it).resource() != this || !(*it).changed() )
      continue;

    KTempFile tmpFile;
    QFile *file = tmpFile.file();

    mFormat->save( *it, file );

    // mark as unchanged
    (*it).setChanged( false );

    tmpFile.close();

    if ( !KIO::NetAccess::upload( tmpFile.name(), mUrl.url() + "/" + (*it).uid() ) ) {
      addressBook()->error( i18n( "Unable to save URL '%1'" ).arg( mUrl.url() + "/" + (*it).uid() ) );
      ok = false;
    }

    tmpFile.unlink();
  }

  delete ticket;

  return ok;
}

void ResourceNet::setUrl( const KURL &url )
{
  mUrl = url;
}

KURL ResourceNet::url() const
{
  return mUrl;
}

QString ResourceNet::identifier() const
{
    return url().url();
}

void ResourceNet::removeAddressee( const Addressee& addr )
{
  KIO::NetAccess::del( mUrl.url() + "/" + addr.uid() );
}

#include "resourcenet.moc"
