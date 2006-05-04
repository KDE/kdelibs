
#include "kasciitest.h"
#include "qtest_kde.h"

#include <kascii.h>

QTEST_KDEMAIN( KAsciiTest, NoGUI )

void KAsciiTest::testkasciistricmp()
{
  QVERIFY( kasciistricmp( "test", "test" ) == 0 );
  QVERIFY( kasciistricmp( "test", "Test" ) == 0 );
  QVERIFY( kasciistricmp( "TeSt", "tEst" ) == 0 );

  QVERIFY( kasciistricmp( 0, 0 ) == 0 );
  QVERIFY( kasciistricmp( "", "" ) == 0 );
  QVERIFY( kasciistricmp( 0, "" ) < 0 );
  QVERIFY( kasciistricmp( "", 0 ) > 0 );

  QVERIFY( kasciistricmp( "", "foo" ) < 0 );
  QVERIFY( kasciistricmp( "foo", "" ) > 0 );

  QVERIFY( kasciistricmp( "test", "testtest" ) < 0 );
  QVERIFY( kasciistricmp( "testtest", "test" ) > 0 );

  QVERIFY( kasciistricmp( "a", "b" ) < 0 );
  QVERIFY( kasciistricmp( "b", "a" ) > 0 );
  QVERIFY( kasciistricmp( "A", "b" ) < 0 );
  QVERIFY( kasciistricmp( "b", "A" ) > 0 );
  QVERIFY( kasciistricmp( "a", "B" ) < 0 );
  QVERIFY( kasciistricmp( "B", "a" ) > 0 );
  QVERIFY( kasciistricmp( "A", "B" ) < 0 );
  QVERIFY( kasciistricmp( "B", "A" ) > 0 );
}

#include "kasciitest.moc"
