#include "qtest_kde.h"
#include "krfcdatetest.h"
#include "krfcdatetest.moc"

QTEST_KDEMAIN(KRFCDateTest, NoGUI)

#include "k3rfcdate.h"

void KRFCDateTest::test()
{
  // valid RFC dates

  // From http://www.w3.org/TR/NOTE-datetime
  time_t ref = K3RFCDate::parseDateISO8601("1994-11-05T13:15:30Z");

  QCOMPARE(K3RFCDate::parseDateISO8601("1994-11-05T08:15:30-05:00"),   ref);
  QCOMPARE(K3RFCDate::parseDateISO8601("1994-11-05T18:15:30+05:00"),   ref);
  QCOMPARE(K3RFCDate::parseDate("Thu Nov 5 1994 18:15:30 GMT+0500"),   ref);
  QCOMPARE(K3RFCDate::parseDate("Thu Nov 5 1994 18:15:30 GMT+05:00"),  ref);
  QCOMPARE(K3RFCDate::parseDate("Wednesday, 05-Nov-94 13:15:30 GMT"),  ref);
  QCOMPARE(K3RFCDate::parseDate("Wed, 05-Nov-1994 13:15:30 GMT"),      ref);
  QCOMPARE(K3RFCDate::parseDate("Wed, 05-November-1994 13:15:30 GMT"), ref);

  // invalid dates

  ref = 0;

  // pass RFC date to ISO parser
  QCOMPARE(K3RFCDate::parseDateISO8601("Thu, 01 Jan 2004 19:48:21 GMT"), ref); 
  // pass ISO date to RFC parser
  QCOMPARE(K3RFCDate::parseDate("1994-01-01T12:00:00"), ref);

  // empty/null strings
  QCOMPARE(K3RFCDate::parseDateISO8601(QString()), ref);
  QCOMPARE(K3RFCDate::parseDateISO8601(""), ref);
  QCOMPARE(K3RFCDate::parseDate(QString()), ref);
  QCOMPARE(K3RFCDate::parseDate(""), ref);


  // valid ISO dates
  
  ref = K3RFCDate::parseDateISO8601("1994-01-01T12:00:00");

  QCOMPARE(K3RFCDate::parseDateISO8601("1994"),       ref);
  QCOMPARE(K3RFCDate::parseDateISO8601("1994-01"),    ref);
  QCOMPARE(K3RFCDate::parseDateISO8601("1994-01-01"), ref);

}

void KRFCDateTest::testRFC2822()
{
	QDateTime local = QDateTime::currentDateTime();
	QDateTime utc = local.toUTC();

	QCOMPARE(QString(K3RFCDate::rfc2822DateString(utc.toTime_t(), 0)),
		utc.toString("ddd, dd MMM yyyy hh:mm:ss +0000"));

	int offset = K3RFCDate::localUTCOffset();
	int z = qAbs(offset);
	QCOMPARE(QString(K3RFCDate::rfc2822DateString(local.toTime_t(), offset)),
		local.toString("ddd, dd MMM yyyy hh:mm:ss %1%2%3")
		       .arg(offset < 0? '-' : '+')
		       .arg(int(z/60%24), 2, 10, QLatin1Char('0'))
		       .arg(int(z%60), 2, 10, QLatin1Char('0')));

	QCOMPARE(K3RFCDate::rfc2822DateString(local.toTime_t(), offset),
		K3RFCDate::rfc2822DateString(local.toTime_t()));
}

