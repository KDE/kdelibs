#include "qtest_kde.h"

#include "geotest.h"
#include "geo.h"

QTEST_KDEMAIN(GeoTest, NoGUI)

void GeoTest::emptyConstructor()
{
  KABC::Geo geo;

  QCOMPARE( geo.asString(), QString() );
}

void GeoTest::constructor()
{
  KABC::Geo geo( 1.2, 3.4 );

  QVERIFY( (float)geo.latitude() == (float)1.2 );
  QVERIFY( (float)geo.longitude() == (float)3.4 );
}

void GeoTest::isValid()
{
  KABC::Geo geo;

  QVERIFY( !geo.isValid() );

  geo.setLatitude( 23 );

  QVERIFY( !geo.isValid() );

  geo.setLongitude( 45 );

  QVERIFY( geo.isValid() );
}

void GeoTest::setData()
{
  KABC::Geo geo;

  geo.setLatitude( 22.5 );
  geo.setLongitude( 45.1 );

  QVERIFY( (float)geo.latitude() == (float)22.5 );
  QVERIFY( (float)geo.longitude() == (float)45.1 );
}

void GeoTest::equals()
{
  KABC::Geo geo1( 22.5, 33.7 );
  KABC::Geo geo2( 22.5, 33.7 );

  QVERIFY( geo1 == geo2 );
}

void GeoTest::differs()
{
  KABC::Geo geo1( 22.5, 33.7 );
  KABC::Geo geo2( 22.5, 33.6 );

  QVERIFY( geo1 != geo2 );
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

  QVERIFY( geo1 == geo2 );
}

#include "geotest.moc"
