#include "QtTest/qttest_kde.h"

#include "geotest.h"
#include "geo.h"

QTTEST_KDEMAIN(GeoTest, NoGUI)

void GeoTest::emptyConstructor()
{
  KABC::Geo geo;

  COMPARE( geo.asString(), QString() );
}

void GeoTest::constructor()
{
  KABC::Geo geo( 1.2, 3.4 );

  VERIFY( (float)geo.latitude() == (float)1.2 );
  VERIFY( (float)geo.longitude() == (float)3.4 );
}

void GeoTest::isValid()
{
  KABC::Geo geo;

  VERIFY( !geo.isValid() );

  geo.setLatitude( 23 );

  VERIFY( !geo.isValid() );

  geo.setLongitude( 45 );

  VERIFY( geo.isValid() );
}

void GeoTest::setData()
{
  KABC::Geo geo;

  geo.setLatitude( 22.5 );
  geo.setLongitude( 45.1 );

  VERIFY( (float)geo.latitude() == (float)22.5 );
  VERIFY( (float)geo.longitude() == (float)45.1 );
}

void GeoTest::equals()
{
  KABC::Geo geo1( 22.5, 33.7 );
  KABC::Geo geo2( 22.5, 33.7 );

  VERIFY( geo1 == geo2 );
}

void GeoTest::differs()
{
  KABC::Geo geo1( 22.5, 33.7 );
  KABC::Geo geo2( 22.5, 33.6 );

  VERIFY( geo1 != geo2 );
}

void GeoTest::serialization()
{
  KABC::Geo geo1( 22.5, 33.7 );
  QByteArray data;

  QDataStream s( &data, QIODevice::WriteOnly );
  s << geo1;

  KABC::Geo geo2;
  QDataStream t( &data, QIODevice::ReadOnly );
  t >> geo2;

  VERIFY( geo1 == geo2 );
}

#include "geotest.moc"
