#include "qtest_kde.h"

#include "kcalendarsystemfactory.h"
#include "kcalendarsystem.h"
#include "kglobal.h"

#include "kcalendartest.h"
#include "kcalendartest.moc"
#include "klocale.h"

QTEST_KDEMAIN(KCalendarTest, NoGUI)

void KCalendarTest::listTypes()
{
    QStringList lst = KCalendarSystemFactory::calendarSystems();
    QVERIFY( lst.contains("hijri") );
    QVERIFY( lst.contains("jalali") );
    QVERIFY( lst.contains("hebrew") );
    QVERIFY( lst.contains("gregorian") );
}

void KCalendarTest::testGregorian()
{
    KGlobal::locale()->setCalendar("gregorian");
    const KCalendarSystem *calendar = KGlobal::locale()->calendar();
    QDate testDate( 2005, 9, 10 );

    QCOMPARE( calendar->year(testDate), 2005 );
    QCOMPARE( calendar->yearString(testDate, true), QString("05") );
    QCOMPARE( calendar->yearString(testDate, false), QString("2005") );
    // TODO: int   yearStringToInteger (const QString &sNum, int &iLength) const 
    QCOMPARE( calendar->month(testDate), 9 );
    QCOMPARE( calendar->monthString(testDate, true), QString("9") );
    QCOMPARE( calendar->monthString(testDate, false), QString("09") );
    // TODO: int   monthStringToInteger (const QString &sNum, int &iLength) const 
    QCOMPARE( calendar->day(testDate), 10 );
    QCOMPARE( calendar->dayString(testDate, true), QString("10") );
    QCOMPARE( calendar->dayString(testDate, false), QString("10") );
    //TODO:  int   dayStringToInteger (const QString &sNum, int &iLength) const 
    QCOMPARE( calendar->monthsInYear(testDate), 12 );
    QCOMPARE( calendar->daysInYear(testDate), 365 );
    QCOMPARE( calendar->daysInMonth(testDate), 30 );
    QCOMPARE( calendar->weeksInYear(testDate.year()), 52 );
    QCOMPARE( calendar->weekNumber(testDate), 36 );
    QCOMPARE( calendar->dayOfWeek(testDate), 6 );
    QCOMPARE( calendar->dayOfYear(testDate), 253 );
    QCOMPARE( calendar->monthName(testDate.month(), testDate.year(), true),
	     QString("Sep") );
    QCOMPARE( calendar->monthName(testDate.month(), testDate.year(), false),
	     QString("September") );
    QCOMPARE( calendar->monthName(testDate.month(), testDate.year()),
	     QString("September") );
    QCOMPARE( calendar->monthName(testDate, true), QString("Sep") );
    QCOMPARE( calendar->monthName(testDate, false), QString("September") );
    QCOMPARE( calendar->monthName(testDate), QString("September") );
    QCOMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year(), true),
	     QString("of Sep") );
    QCOMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year(), false),
	     QString("of September") );
    QCOMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year()),
	     QString("of September") );
    QCOMPARE( calendar->monthNamePossessive(testDate, true), QString("of Sep") );
    QCOMPARE( calendar->monthNamePossessive(testDate, false), QString("of September") );
    QCOMPARE( calendar->monthNamePossessive(testDate), QString("of September") );
    QCOMPARE( calendar->weekDayName(6, false), QString("Saturday") );
    QCOMPARE( calendar->weekDayName(6, true), QString("Sat") );
    QCOMPARE( calendar->weekDayName(6), QString("Saturday") );
    QCOMPARE( calendar->weekDayName(testDate), QString("Saturday") );
    QCOMPARE( calendar->weekDayName(testDate, false), QString("Saturday") );
    QCOMPARE( calendar->weekDayName(testDate, true), QString("Sat") );

    QVERIFY( calendar->setYMD( testDate, 2000, 3, 1 ) );
    QCOMPARE( calendar->year(testDate), 2000 );
    QCOMPARE( calendar->month(testDate), 3 );
    QCOMPARE( calendar->day(testDate), 1 );
    QCOMPARE( calendar->daysInYear(testDate), 366 );

    QDate newDate = calendar->addYears(testDate, 4);
    QCOMPARE( newDate.year(), 2004 );
    QCOMPARE( calendar->daysInYear(newDate), 366 );

    newDate = calendar->addMonths( testDate, -4 );
    QCOMPARE( newDate.year(), 1999 );
    QCOMPARE( newDate.month(), 11 );
    QCOMPARE( newDate.day(), 1 );

    newDate = calendar->addDays( newDate, 20 );
    QCOMPARE( newDate.year(), 1999 );
    QCOMPARE( newDate.month(), 11 );
    QCOMPARE( newDate.day(), 21 );

    QCOMPARE( calendar->calendarName(), QString("gregorian") );
    QCOMPARE( calendar->minValidYear(), 1753 );
    QCOMPARE( calendar->maxValidYear(), 8000 );
    QCOMPARE( calendar->isLunar(), false );
    QCOMPARE( calendar->isLunisolar(), false );
    QCOMPARE( calendar->isSolar(), true );
}

void KCalendarTest::testHijri()
{
    KGlobal::locale()->setCalendar("hijri");
    const KCalendarSystem *calendar = KGlobal::locale()->calendar();
    QDate testDate( 2005, 9, 10 );

    QCOMPARE( calendar->year(testDate), 1426 );
    QCOMPARE( calendar->yearString(testDate, true), QString("26") );
    QCOMPARE( calendar->yearString(testDate, false), QString("1426") );
    // TODO: int   yearStringToInteger (const QString &sNum, int &iLength) const 
    QCOMPARE( calendar->month(testDate), 8 );
    QCOMPARE( calendar->monthString(testDate, true), QString("8") );
    QCOMPARE( calendar->monthString(testDate, false), QString("08") );
    // TODO: int   monthStringToInteger (const QString &sNum, int &iLength) const 
    QCOMPARE( calendar->day(testDate), 6 );
    QCOMPARE( calendar->dayString(testDate, true), QString("6") );
    QCOMPARE( calendar->dayString(testDate, false), QString("06") );
    //TODO:  int   dayStringToInteger (const QString &sNum, int &iLength) const 
    QCOMPARE( calendar->monthsInYear(testDate), 12 );
    QCOMPARE( calendar->daysInYear(testDate), 355 );
    QCOMPARE( calendar->daysInMonth(testDate), 29 );
    QCOMPARE( calendar->weeksInYear(testDate.year()), 50 );
    QCOMPARE( calendar->weekNumber(testDate), 31 );
    QCOMPARE( calendar->dayOfWeek(testDate), 6 );
    QCOMPARE( calendar->dayOfYear(testDate), 213 );
    QCOMPARE( calendar->monthName(testDate.month(), testDate.year(), true),
	     QString("Ramadan") );
    QCOMPARE( calendar->monthName(testDate.month(), testDate.year(), false),
	     QString("Ramadan") );
    QCOMPARE( calendar->monthName(testDate.month(), testDate.year()),
	     QString("Ramadan") );
    QCOMPARE( calendar->monthName(testDate, true), QString("Sha`ban") );
    QCOMPARE( calendar->monthName(testDate, false), QString("Sha`ban") );
    QCOMPARE( calendar->monthName(testDate), QString("Sha`ban") );
    QCOMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year(), true),
	     QString("of Ramadan") );
    QCOMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year(), false),
	     QString("of Ramadan") );
    QCOMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year()),
	     QString("of Ramadan") );
    QCOMPARE( calendar->monthNamePossessive(testDate, true), QString("of Sha`ban") );
    QCOMPARE( calendar->monthNamePossessive(testDate, false), QString("of Sha`ban") );
    QCOMPARE( calendar->monthNamePossessive(testDate), QString("of Sha`ban") );
    QCOMPARE( calendar->weekDayName(6, false), QString("Yaum al-Sabt") );
    QCOMPARE( calendar->weekDayName(6, true), QString("Sab") );
    QCOMPARE( calendar->weekDayName(6), QString("Yaum al-Sabt") );
    QCOMPARE( calendar->weekDayName(testDate), QString("Yaum al-Sabt") );
    QCOMPARE( calendar->weekDayName(testDate, false), QString("Yaum al-Sabt") );
    QCOMPARE( calendar->weekDayName(testDate, true), QString("Sab") );

    QVERIFY( calendar->setYMD( testDate, 2000, 3, 1 ) );
    QCOMPARE( calendar->year(testDate), 2000 );
    QCOMPARE( calendar->month(testDate), 3 );
    QCOMPARE( calendar->day(testDate), 1 );

    QDate newDate = calendar->addYears(testDate, 4);
    QCOMPARE( newDate.year(), 2566 );
    QCOMPARE( calendar->daysInYear(newDate), 355 );

    newDate = calendar->addMonths( testDate, -4 );
    QCOMPARE( newDate.year(), 2561 );
    QCOMPARE( newDate.month(), 11 );
    QCOMPARE( newDate.day(), 10 );

    newDate = calendar->addDays( newDate, 20 );
    QCOMPARE( newDate.year(), 2561 );
    QCOMPARE( newDate.month(), 11 );
    QCOMPARE( newDate.day(), 30 );

    QCOMPARE( calendar->calendarName(), QString("hijri") );
    QCOMPARE( calendar->minValidYear(), 1166 );
    QCOMPARE( calendar->maxValidYear(), 7604 );
    QCOMPARE( calendar->isLunar(), true );
    QCOMPARE( calendar->isLunisolar(), false );
    QCOMPARE( calendar->isSolar(), false );
}

