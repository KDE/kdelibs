#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include "format.h"

using namespace KABC;

bool Format::load( AddressBook *ab, Resource *resource, const QString &fileName )
{
  QFile file( fileName );

  if ( !file.open( IO_ReadOnly ) ) return false;

  bool result = load( ab, resource, &file );
  
  file.close();
  
  return result;
}

bool Format::save( AddressBook *ab, Resource *resource,
                   const QString &fileName )
{
  QFile file( fileName );

  if ( !file.open( IO_WriteOnly ) ) {
    kdDebug( 5700 ) << "Format::save(): " << "can't open file '" <<
                       fileName << "'" << endl;
    return false;
  }

  bool result = true;
  AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end() && result; ++it ) {
    if ( (*it).resource() != resource && (*it).resource() != 0 ) continue;

    result = save( *it, &file );
  }

  file.close();
  
  return result;
}

bool Format::checkFormat( const QString &fileName ) const
{
  QFile file( fileName );

  if ( !file.open( IO_ReadOnly ) ) return false;

  bool result = checkFormat( &file );

  file.close();

  return result;
}

QString Format::typeInfo()
{
  return i18n("Undefined Type");
}

void Format::removeAddressee( const Addressee & )
{
  kdDebug() << "Warning! Format::removeAddressee() called." << endl;
}
