#include "vcardformatplugin2.h"

#include "address.h"
#include "addressee.h"
#include "vcardparser/vcardtool.h"

#include <qfile.h>

using namespace KABC;

extern "C"
{
  FormatPlugin *format()
  {
    return new VCardFormatPlugin2();
  }
}

VCardFormatPlugin2::VCardFormatPlugin2()
{
}

VCardFormatPlugin2::~VCardFormatPlugin2()
{
}

bool VCardFormatPlugin2::load( Addressee &addressee, QFile *file )
{
  QString data;

  QTextStream t( file );
  t.setEncoding( QTextStream::UnicodeUTF8 );
  data = t.read();

  VCardTool tool;
  Addressee::List l = tool.parseVCards( data );

  if ( ! l.first().isEmpty() ) {
    addressee = l.first();
    return true;
  }

  return false;
}

bool VCardFormatPlugin2::loadAll( AddressBook *addressBook, Resource *resource, QFile *file )
{
  QString data;

  QTextStream t( file );
  t.setEncoding( QTextStream::UnicodeUTF8 );
  data = t.read();

  VCardTool tool;

  Addressee::List l = tool.parseVCards( data );

  Addressee::List::iterator itr;

  for ( itr = l.begin(); itr != l.end(); ++itr) {
    Addressee addressee = *itr;
    addressee.setResource( resource );
    addressBook->insertAddressee( addressee );
  }

  return true;
}

void VCardFormatPlugin2::save( const Addressee &addressee, QFile *file )
{
  VCardTool tool;
  Addressee::List vcardlist;


  vcardlist.append( addressee );

  QTextStream t( file );
  t.setEncoding( QTextStream::UnicodeUTF8 );
  t << tool.createVCards( vcardlist );
}

void VCardFormatPlugin2::saveAll( AddressBook *ab, Resource *resource, QFile *file )
{
  VCardTool tool;
  Addressee::List vcardlist;

  AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    if ( (*it).resource() == resource ) {
      (*it).setChanged( false );
      vcardlist.append( *it );
    }
  }

  QTextStream t( file );
  t.setEncoding( QTextStream::UnicodeUTF8 );
  t << tool.createVCards( vcardlist );
}

bool VCardFormatPlugin2::checkFormat( QFile *file ) const
{
  QString line;

  file->readLine( line, 1024 );
  line = line.stripWhiteSpace();
  if ( line == "BEGIN:VCARD" )
    return true;
  else
    return false;
}

