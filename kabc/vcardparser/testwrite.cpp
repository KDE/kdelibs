/*
    This file is part of libkabc.

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

#include <kabc/addressee.h>
#include <kabc/phonenumber.h>
#include <kabc/address.h>
#include <kabc/key.h>
#include <kabc/picture.h>
#include <kabc/sound.h>
#include <kabc/secrecy.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include <qfile.h>
#include <qtextstream.h>

#include "vcardconverter.h"

int main( int argc, char **argv )
{
  KAboutData aboutData( "testwrite", "vCard test writer", "0.1" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app( false, false );
  

  KABC::Addressee addressee;

  addressee.setNameFromString( "Mr. Tobias Koenig Jr." );
  addressee.setNickName( "tokoe" );
  addressee.setBirthday( QDate( 1982, 7, 19 ) );
  addressee.setMailer( "mutt1.2" );
  addressee.setTimeZone( KABC::TimeZone( +2 ) );

  KABC::Geo geo;
  geo.setLatitude( 30 );
  geo.setLongitude( 51 );
  addressee.setGeo( geo );

  addressee.setTitle( "nerd" );
  addressee.setRole( "Maintainer" );
  addressee.setOrganization( "KDE" );
  addressee.setNote( "nerver\ntouch a running system" );
  addressee.setProductId( "testId" );
  addressee.setRevision( QDateTime::currentDateTime() );
  addressee.setSortString( "koenig" );
  addressee.setUrl( KURL( "http://wgess16.dyndns.org") );
  addressee.setSecrecy(  KABC::Secrecy( KABC::Secrecy::Confidential ) );
/*
  QImage img;
  img.load( "testimg.png", "PNG" );
  KABC::Picture photo;
  photo.setData( img );
  addressee.setPhoto( photo );

  QImage img2;
  img2.load( "testimg.png", "PNG" );
  KABC::Picture logo;
  logo.setData( img2 );
  addressee.setLogo( logo );

  QFile soundFile( "testsound.wav" );
  soundFile.open( IO_ReadOnly );
  QByteArray data = soundFile.readAll();
  soundFile.close();
  KABC::Sound sound;
  sound.setData( data );
  addressee.setSound( sound );
*/
  addressee.insertEmail( "tokoe@kde.org", true );
  addressee.insertEmail( "tokoe82@yahoo.de", true );

  KABC::PhoneNumber phone1( "3541523475", KABC::PhoneNumber::Pref | KABC::PhoneNumber::Home );
  KABC::PhoneNumber phone2( "+46745673475", KABC::PhoneNumber::Work );
  addressee.insertPhoneNumber( phone1 );
  addressee.insertPhoneNumber( phone2 );

  KABC::Key key( "secret key", KABC::Key::X509 );
  addressee.insertKey( key );

  QStringList categories;
  categories << "Friends" << "School" << "KDE";
  addressee.setCategories( categories );

  KABC::Address a( KABC::Address::Work | KABC::Address::Postal | KABC::Address::Parcel );
  a.setStreet( "6544 Battleford Drive" );
  a.setLocality( "Raleigh" );
  a.setRegion( "NC" );
  a.setPostalCode( "27613-3502" );
  a.setCountry( "U.S.A." );
  addressee.insertAddress( a );

  addressee.insertCustom( "1hsdf", "ertuer", "iurt" );
  addressee.insertCustom( "2hsdf", "ertuer", "iurt" );
  addressee.insertCustom( "3hsdf", "ertuer", "iurt" );

  KABC::Addressee::List list;
  for ( int i = 0; i < 1000; ++i ) {
    KABC::Addressee addr = addressee;
    addr.setUid( QString::number( i ) );
    list.append( addr );
  }

  KABC::VCardConverter converter;
  QString txt = converter.createVCards( list );

  QFile file( "out.vcf" );
  file.open( IO_WriteOnly );

  QTextStream s( &file );
  s.setEncoding( QTextStream::UnicodeUTF8 );
  s << txt;
  file.close();

  return 0;
}
