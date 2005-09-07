#include "QtTest/qttest_kde.h"
#include "krfcdatetest.h"
#include "krfcdatetest.moc"

QTTEST_KDEMAIN(KRFCDateTest, NoGUI)

#include "krfcdate.h"

void KRFCDateTest::test()
{
  // From http://www.w3.org/TR/NOTE-datetime
  time_t ref = KRFCDate::parseDateISO8601("1994-11-05T13:15:30Z");
  
  COMPARE(KRFCDate::parseDateISO8601("1994-11-05T08:15:30-05:00"),   ref);
  COMPARE(KRFCDate::parseDateISO8601("1994-11-05T18:15:30+05:00"),   ref);
  COMPARE(KRFCDate::parseDate("Thu Nov 5 1994 18:15:30 GMT+0500"),   ref);
  COMPARE(KRFCDate::parseDate("Thu Nov 5 1994 18:15:30 GMT+05:00"),  ref);
  COMPARE(KRFCDate::parseDate("Wednesday, 05-Nov-94 13:15:30 GMT"),  ref);
  COMPARE(KRFCDate::parseDate("Wed, 05-Nov-1994 13:15:30 GMT"),      ref);
  COMPARE(KRFCDate::parseDate("Wed, 05-November-1994 13:15:30 GMT"), ref);
  
  ref = KRFCDate::parseDateISO8601("1994-01-01T12:00:00");
  
  COMPARE(KRFCDate::parseDateISO8601("1994"),       ref);
  COMPARE(KRFCDate::parseDateISO8601("1994-01"),    ref);
  COMPARE(KRFCDate::parseDateISO8601("1994-01-01"), ref);
}

