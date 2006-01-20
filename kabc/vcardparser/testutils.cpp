#include <QFile>

#include <vcardparser.h>
#include <kabc/addressee.h>

using namespace KABC;

Addressee vcard1()
{
  Addressee addr;

  addr.setName( "Frank Dawson" );
  addr.setOrganization( "Lotus Development Corporation" );
  addr.setUrl( KUrl( "http://home.earthlink.net/~fdawson") );
  addr.insertEmail( "fdawson@earthlink.net" );
  addr.insertEmail( "Frank_Dawson@Lotus.com", true );
  addr.insertPhoneNumber( PhoneNumber("+1-919-676-9515",PhoneNumber::Voice|PhoneNumber::Msg
                                      |PhoneNumber::Work ) );
  addr.insertPhoneNumber( PhoneNumber("+1-919-676-9564",PhoneNumber::Fax |PhoneNumber::Work ));
  Address a( Address::Work | Address::Postal | Address::Parcel );
  a.setStreet( "6544 Battleford Drive" );
  a.setLocality( "Raleigh" );
  a.setRegion( "NC" );
  a.setPostalCode( "27613-3502" );
  a.setCountry( "U.S.A." );
  addr.insertAddress( a );
  return addr;
}

Addressee vcard2()
{
  Addressee addr;

  addr.setName( "Tim Howes" );
  addr.setOrganization( "Netscape Communications Corp." );
  addr.insertEmail( "howes@netscape.com" );
  addr.insertPhoneNumber( PhoneNumber("+1-415-937-3419",PhoneNumber::Voice|PhoneNumber::Msg
                                      |PhoneNumber::Work) );
  addr.insertPhoneNumber( PhoneNumber("+1-415-528-4164",PhoneNumber::Fax |PhoneNumber::Work) );
  Address a( Address::Work );
  a.setStreet( "501 E. Middlefield Rd." );
  a.setLocality( "Mountain View" );
  a.setRegion( "CA" );
  a.setPostalCode( "94043" );
  a.setCountry( "U.S.A." );
  addr.insertAddress( a );
  return addr;
}

Addressee vcard3()
{
  Addressee addr;

  addr.setName( "ian geiser" );
  addr.setOrganization( "Source eXtreme" );
  addr.insertEmail( "geiseri@yahoo.com" );
  addr.setTitle( "VP of Engineering" );
  return addr;
}


QByteArray vcardAsText( const QString& location )
{
  QByteArray text;

  QFile file( location );
  if ( file.open( QIODevice::ReadOnly ) ) {
    text = file.readAll();
    file.close();
  }

  return text;
}

Addressee::List vCardsAsAddresseeList()
{
  Addressee::List l;

  l.append( vcard1() );
  l.append( vcard2() );
  l.append( vcard3() );

  return l;
}

QString vCardsAsText()
{
  QByteArray vcards = vcardAsText( "tests/vcard1.vcf" );
  vcards += vcardAsText( "tests/vcard2.vcf" );
  vcards += vcardAsText( "tests/vcard3.vcf" );

  return vcards;
}
