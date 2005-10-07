#include "QtTest/qttest_kde.h"

#include "kcalendarsystemfactory.h"
#include "kcalendarsystem.h"
#include "kglobal.h"

#include "kcalendartest.h"
#include "kcalendartest.moc"
#include "klocale.h"

QTTEST_KDEMAIN(KCalendarTest, NoGUI)

void KCalendarTest::listTypes()
{
    QStringList lst = KCalendarSystemFactory::calendarSystems();
    VERIFY( lst.contains("hijri") );
    VERIFY( lst.contains("jalali") );
    VERIFY( lst.contains("hebrew") );
    VERIFY( lst.contains("gregorian") );
}

void KCalendarTest::testGregorian()
{
    KGlobal::locale()->setCalendar("gregorian");
    const KCalendarSystem *calendar = KGlobal::locale()->calendar();
    QDate testDate( 2005, 9, 10 );

    COMPARE( calendar->year(testDate), 2005 );
    COMPARE( calendar->yearString(testDate, true), QString("05") );
    COMPARE( calendar->yearString(testDate, false), QString("2005") );
    // TODO: int   yearStringToInteger (const QString &sNum, int &iLength) const 
    COMPARE( calendar->month(testDate), 9 );
    COMPARE( calendar->monthString(testDate, true), QString("9") );
    COMPARE( calendar->monthString(testDate, false), QString("09") );
    // TODO: int   monthStringToInteger (const QString &sNum, int &iLength) const 
    COMPARE( calendar->day(testDate), 10 );
    COMPARE( calendar->dayString(testDate, true), QString("10") );
    COMPARE( calendar->dayString(testDate, false), QString("10") );
    //TODO:  int   dayStringToInteger (const QString &sNum, int &iLength) const 
    COMPARE( calendar->monthsInYear(testDate), 12 );
    COMPARE( calendar->daysInYear(testDate), 365 );
    COMPARE( calendar->daysInMonth(testDate), 30 );
    COMPARE( calendar->weeksInYear(testDate.year()), 52 );
    COMPARE( calendar->weekNumber(testDate), 36 );
    COMPARE( calendar->dayOfWeek(testDate), 6 );
    COMPARE( calendar->dayOfYear(testDate), 253 );
    COMPARE( calendar->monthName(testDate.month(), testDate.year(), true),
	     QString("Sep") );
    COMPARE( calendar->monthName(testDate.month(), testDate.year(), false),
	     QString("September") );
    COMPARE( calendar->monthName(testDate.month(), testDate.year()),
	     QString("September") );
    COMPARE( calendar->monthName(testDate, true), QString("Sep") );
    COMPARE( calendar->monthName(testDate, false), QString("September") );
    COMPARE( calendar->monthName(testDate), QString("September") );
    COMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year(), true),
	     QString("of Sep") );
    COMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year(), false),
	     QString("of September") );
    COMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year()),
	     QString("of September") );
    COMPARE( calendar->monthNamePossessive(testDate, true), QString("of Sep") );
    COMPARE( calendar->monthNamePossessive(testDate, false), QString("of September") );
    COMPARE( calendar->monthNamePossessive(testDate), QString("of September") );
    COMPARE( calendar->weekDayName(6, false), QString("Saturday") );
    COMPARE( calendar->weekDayName(6, true), QString("Sat") );
    COMPARE( calendar->weekDayName(6), QString("Saturday") );
    COMPARE( calendar->weekDayName(testDate), QString("Saturday") );
    COMPARE( calendar->weekDayName(testDate, false), QString("Saturday") );
    COMPARE( calendar->weekDayName(testDate, true), QString("Sat") );

    VERIFY( calendar->setYMD( testDate, 2000, 3, 1 ) );
    COMPARE( calendar->year(testDate), 2000 );
    COMPARE( calendar->month(testDate), 3 );
    COMPARE( calendar->day(testDate), 1 );
    COMPARE( calendar->daysInYear(testDate), 366 );

    QDate newDate = calendar->addYears(testDate, 4);
    COMPARE( newDate.year(), 2004 );
    COMPARE( calendar->daysInYear(newDate), 366 );

    newDate = calendar->addMonths( testDate, -4 );
    COMPARE( newDate.year(), 1999 );
    COMPARE( newDate.month(), 11 );
    COMPARE( newDate.day(), 1 );

    newDate = calendar->addDays( newDate, 20 );
    COMPARE( newDate.year(), 1999 );
    COMPARE( newDate.month(), 11 );
    COMPARE( newDate.day(), 21 );

    COMPARE( calendar->calendarName(), QString("gregorian") );
    COMPARE( calendar->minValidYear(), 1753 );
    COMPARE( calendar->maxValidYear(), 8000 );
    COMPARE( calendar->isLunar(), false );
    COMPARE( calendar->isLunisolar(), false );
    COMPARE( calendar->isSolar(), true );
}

void KCalendarTest::testHijri()
{
    KGlobal::locale()->setCalendar("hijri");
    const KCalendarSystem *calendar = KGlobal::locale()->calendar();
    QDate testDate( 2005, 9, 10 );

    COMPARE( calendar->year(testDate), 1426 );
    COMPARE( calendar->yearString(testDate, true), QString("26") );
    COMPARE( calendar->yearString(testDate, false), QString("1426") );
    // TODO: int   yearStringToInteger (const QString &sNum, int &iLength) const 
    COMPARE( calendar->month(testDate), 8 );
    COMPARE( calendar->monthString(testDate, true), QString("8") );
    COMPARE( calendar->monthString(testDate, false), QString("08") );
    // TODO: int   monthStringToInteger (const QString &sNum, int &iLength) const 
    COMPARE( calendar->day(testDate), 6 );
    COMPARE( calendar->dayString(testDate, true), QString("6") );
    COMPARE( calendar->dayString(testDate, false), QString("06") );
    //TODO:  int   dayStringToInteger (const QString &sNum, int &iLength) const 
    COMPARE( calendar->monthsInYear(testDate), 12 );
    COMPARE( calendar->daysInYear(testDate), 355 );
    COMPARE( calendar->daysInMonth(testDate), 29 );
    COMPARE( calendar->weeksInYear(testDate.year()), 50 );
    COMPARE( calendar->weekNumber(testDate), 31 );
    COMPARE( calendar->dayOfWeek(testDate), 6 );
    COMPARE( calendar->dayOfYear(testDate), 213 );
    COMPARE( calendar->monthName(testDate.month(), testDate.year(), true),
	     QString("Ramadan") );
    COMPARE( calendar->monthName(testDate.month(), testDate.year(), false),
	     QString("Ramadan") );
    COMPARE( calendar->monthName(testDate.month(), testDate.year()),
	     QString("Ramadan") );
    COMPARE( calendar->monthName(testDate, true), QString("Sha`ban") );
    COMPARE( calendar->monthName(testDate, false), QString("Sha`ban") );
    COMPARE( calendar->monthName(testDate), QString("Sha`ban") );
    COMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year(), true),
	     QString("of Ramadan") );
    COMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year(), false),
	     QString("of Ramadan") );
    COMPARE( calendar->monthNamePossessive(testDate.month(), testDate.year()),
	     QString("of Ramadan") );
    COMPARE( calendar->monthNamePossessive(testDate, true), QString("of Sha`ban") );
    COMPARE( calendar->monthNamePossessive(testDate, false), QString("of Sha`ban") );
    COMPARE( calendar->monthNamePossessive(testDate), QString("of Sha`ban") );
    COMPARE( calendar->weekDayName(6, false), QString("Yaum al-Sabt") );
    COMPARE( calendar->weekDayName(6, true), QString("Sab") );
    COMPARE( calendar->weekDayName(6), QString("Yaum al-Sabt") );
    COMPARE( calendar->weekDayName(testDate), QString("Yaum al-Sabt") );
    COMPARE( calendar->weekDayName(testDate, false), QString("Yaum al-Sabt") );
    COMPARE( calendar->weekDayName(testDate, true), QString("Sab") );

    VERIFY( calendar->setYMD( testDate, 2000, 3, 1 ) );
    COMPARE( calendar->year(testDate), 2000 );
    COMPARE( calendar->month(testDate), 3 );
    COMPARE( calendar->day(testDate), 1 );

    QDate newDate = calendar->addYears(testDate, 4);
    COMPARE( newDate.year(), 2566 );
    COMPARE( calendar->daysInYear(newDate), 355 );

    newDate = calendar->addMonths( testDate, -4 );
    COMPARE( newDate.year(), 2561 );
    COMPARE( newDate.month(), 11 );
    COMPARE( newDate.day(), 10 );

    newDate = calendar->addDays( newDate, 20 );
    COMPARE( newDate.year(), 2561 );
    COMPARE( newDate.month(), 11 );
    COMPARE( newDate.day(), 30 );

    COMPARE( calendar->calendarName(), QString("hijri") );
    COMPARE( calendar->minValidYear(), 1166 );
    COMPARE( calendar->maxValidYear(), 7604 );
    COMPARE( calendar->isLunar(), true );
    COMPARE( calendar->isLunisolar(), false );
    COMPARE( calendar->isSolar(), false );
}

