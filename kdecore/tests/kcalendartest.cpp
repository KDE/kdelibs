#include "qtest_kde.h"

#include "kcalendarsystem.h"
#include "kglobal.h"

#include "kcalendartest.h"
#include "kcalendartest.moc"
#include "klocale.h"

QTEST_KDEMAIN_CORE(KCalendarTest)

void KCalendarTest::testTypes()
{
    const KCalendarSystem *calendar;
    calendar = KCalendarSystem::create( QString("gregorian") );
    QCOMPARE( calendar->calendarType(), QString("gregorian") );
    calendar = KCalendarSystem::create( QString("hebrew") );
    QCOMPARE( calendar->calendarType(), QString("hebrew") );
    calendar = KCalendarSystem::create( QString("hijri") );
    QCOMPARE( calendar->calendarType(), QString("hijri") );
    calendar = KCalendarSystem::create( QString("jalali") );
    QCOMPARE( calendar->calendarType(), QString("jalali") );
    calendar = KCalendarSystem::create( QString("invalid") );
    QCOMPARE( calendar->calendarType(), QString("gregorian") );
    calendar = KCalendarSystem::create( QString() );
    QCOMPARE( calendar->calendarType(), QString("gregorian") );

    QStringList lst = KCalendarSystem::calendarSystems();
    QVERIFY( lst.contains("gregorian") );
    QVERIFY( lst.contains("hebrew") );
    QVERIFY( lst.contains("hijri") );
    QVERIFY( lst.contains("jalali") );

    QCOMPARE( KCalendarSystem::calendarLabel("gregorian"), QString("Gregorian") );
    QCOMPARE( KCalendarSystem::calendarLabel("hebrew"),    QString("Hebrew") );
    QCOMPARE( KCalendarSystem::calendarLabel("hijri"),     QString("Hijri") );
    QCOMPARE( KCalendarSystem::calendarLabel("jalali"),    QString("Jalali") );
}

void KCalendarTest::testLocale()
{
    KGlobal::locale()->setCalendar("gregorian");
    const KCalendarSystem *calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("gregorian") );
    KGlobal::locale()->setCalendar("hebrew");
    calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("hebrew") );
    KGlobal::locale()->setCalendar("hijri");
    calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("hijri") );
    KGlobal::locale()->setCalendar("jalali");
    calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("jalali") );
}

void KCalendarTest::testGregorian()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));
    QDate testDate( 2005, 9, 10 );

    QCOMPARE( calendar->dayOfYear( testDate ), 253 );

    QVERIFY( calendar->setYMD( testDate, 2000, 3, 1 ) );
    QCOMPARE( calendar->year( testDate ), 2000 );
    QCOMPARE( calendar->month( testDate ), 3 );
    QCOMPARE( calendar->day( testDate ), 1 );
    QCOMPARE( calendar->daysInYear( testDate ), 366 );

    QDate newDate = calendar->addYears( testDate, 4);
    QCOMPARE( newDate.year(), 2004 );
    QCOMPARE( calendar->daysInYear( newDate ), 366 );

    newDate = calendar->addMonths( testDate, -4 );
    QCOMPARE( newDate.year(), 1999 );
    QCOMPARE( newDate.month(), 11 );
    QCOMPARE( newDate.day(), 1 );

    newDate = calendar->addDays( newDate, 20 );
    QCOMPARE( newDate.year(), 1999 );
    QCOMPARE( newDate.month(), 11 );
    QCOMPARE( newDate.day(), 21 );
}

void KCalendarTest::testHijri()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "hijri" ));
    QDate testDate( 2005, 9, 10 );

    QCOMPARE( calendar->daysInYear(testDate), 355 );
    QCOMPARE( calendar->daysInMonth(testDate), 29 );
    QCOMPARE( calendar->weeksInYear(testDate.year()), 50 );
    QCOMPARE( calendar->weekNumber(testDate), 31 );
    QCOMPARE( calendar->dayOfYear(testDate), 213 );

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
}


void KCalendarTest::testGregorianBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString("gregorian"));

    QCOMPARE( calendar->calendarType(), QString("gregorian") );
    QCOMPARE( KCalendarSystem::calendarLabel( QString("gregorian") ), QString("Gregorian") );

    QEXPECT_FAIL("", "QDate is Julian, get right date", Continue);
    QCOMPARE( calendar->epoch(), QDate( 1, 1, 1 ) );
    QCOMPARE( calendar->earliestValidDate(), QDate( -4713, 1, 2 ) );
    QCOMPARE( calendar->latestValidDate(), QDate( 9999, 12, 31 ) );

    testValid( calendar, 10000, 13, 32, QDate( -5000, 1, 1 ) );

    QCOMPARE( calendar->isLeapYear( 2007 ), false );
    QCOMPARE( calendar->isLeapYear( 2008 ), true );
    QCOMPARE( calendar->isLeapYear( 1900 ), false );
    QCOMPARE( calendar->isLeapYear( 2000 ), true );
    QCOMPARE( calendar->isLeapYear( QDate( 2007, 1, 1 ) ), false );
    QCOMPARE( calendar->isLeapYear( QDate( 2008, 1, 1 ) ), true );

    QCOMPARE( calendar->daysInWeek( QDate( 2007, 1, 1 ) ), 7 );
    QCOMPARE( calendar->monthsInYear( QDate( 2007, 1, 1 ) ), 12 );

    testYear(  calendar, QDate( 2007, 7, 9 ), 2007, QString("07"), QString("2007") );
    testMonth( calendar, QDate( 2007, 7, 9 ),    7, QString("7"),  QString("07") );
    testDay(   calendar, QDate( 2007, 7, 9 ),    9, QString("9"),  QString("09") );

    testWeekDayName( calendar, 6, QDate( 2007, 7, 28 ),
                     QString("Sat"), QString("Saturday") );
    testMonthName( calendar, 12, 2007, QDate( 2007, 12, 20 ),
                   QString("Dec"), QString("December"),
                   QString("of Dec"), QString("of December") );

    QCOMPARE( calendar->monthsInYear( QDate( 2007, 1, 1 ) ), 12 );

    QCOMPARE( calendar->weekStartDay(), 1 );
    QCOMPARE( calendar->weekDayOfPray(), 7 );

    QCOMPARE( calendar->isProleptic(), false );
    QCOMPARE( calendar->isLunar(), false );
    QCOMPARE( calendar->isLunisolar(), false );
    QCOMPARE( calendar->isSolar(), true );
}

void KCalendarTest::testGregorianYmd()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString("gregorian"));
    testYmd( calendar, 2007, 1, 1, QDate( 2007, 1, 1 ).toJulianDay() );
}


// Test Hijri Calendar System

void KCalendarTest::testHijriBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString("hijri"));

    QCOMPARE( calendar->calendarType(), QString("hijri") );
    QCOMPARE( KCalendarSystem::calendarLabel( QString("hijri") ), QString("Hijri") );

    QCOMPARE( calendar->epoch(), QDate( 622, 7, 16 ) );
    QCOMPARE( calendar->earliestValidDate(), QDate( 622, 7, 16 ) );
    QCOMPARE( calendar->latestValidDate(), QDate( 10323, 10, 21) );

    // Restore after Hijri converted
    //testValid( calendar, 10000, 13, 31, QDate( 1, 1, 1 ) );

    QCOMPARE( calendar->isLeapYear( 1427 ), false );
    QCOMPARE( calendar->isLeapYear( 1428 ), true );
    QCOMPARE( calendar->isLeapYear( QDate( 2007, 1, 1 ) ), false );
    QCOMPARE( calendar->isLeapYear( QDate( 2008, 1, 1 ) ), true );

    QCOMPARE( calendar->daysInWeek( QDate( 2007, 1, 1 ) ), 7 );
    QCOMPARE( calendar->monthsInYear( QDate( 2007, 1, 1 ) ), 12 );

    testYear(  calendar, QDate( 2005, 9, 10 ), 1426, QString("26"), QString("1426") );
    testMonth( calendar, QDate( 2005, 9, 10 ),    8, QString("8"),  QString("08") );
    testDay(   calendar, QDate( 2005, 9, 10 ),    6, QString("6"),  QString("06") );

    testWeekDayName( calendar, 6, QDate( 2005, 9, 10 ),
                     QString("Sab"), QString("Yaum al-Sabt") );
    testMonthName( calendar, 12, 1428, QDate( 2007, 12, 20 ),
                   QString("Hijjah"), QString("Thu al-Hijjah"),
                   QString("of Hijjah"), QString("of Thu al-Hijjah") );

    QCOMPARE( calendar->monthsInYear( QDate( 2005, 9, 10 ) ), 12 );

    QCOMPARE( calendar->weekStartDay(), 1 );
    QCOMPARE( calendar->weekDayOfPray(), 5 );

    QCOMPARE( calendar->isProleptic(), false );
    QCOMPARE( calendar->isLunar(), true );
    QCOMPARE( calendar->isLunisolar(), false );
    QCOMPARE( calendar->isSolar(), false );
}

void KCalendarTest::testHijriYmd()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString("hijri"));
    testYmd( calendar, 1426, 8, 6, QDate( 2005, 9, 10 ).toJulianDay() );
}


// Jalali Calendar System

void KCalendarTest::testJalaliBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "jalali" ));

    QCOMPARE( calendar->calendarType(), QString("jalali") );
    QCOMPARE( KCalendarSystem::calendarLabel( QString("jalali") ), QString("Jalali") );

    QCOMPARE( calendar->epoch(), QDate( 622, 3, 19 ) );
    QCOMPARE( calendar->earliestValidDate(), QDate( 622, 3, 19 ) );
    QCOMPARE( calendar->latestValidDate(), QDate( 10621, 3, 17 ) );

    testValid( calendar, 10000, 13, 32, QDate( 1, 1, 1 ) );

    QCOMPARE( calendar->isLeapYear( 1386 ), false );
    QCOMPARE( calendar->isLeapYear( 1387 ), true );
    QCOMPARE( calendar->isLeapYear( QDate( 2008, 1, 1 ) ), false );
    QEXPECT_FAIL("", "Not working right, 2009-01-01 should be 1387, verify", Continue);
    QCOMPARE( calendar->isLeapYear( QDate( 2009, 1, 1 ) ), true );

    QCOMPARE( calendar->daysInWeek( QDate( 2007, 1, 1 ) ), 7 );
    QCOMPARE( calendar->monthsInYear( QDate( 2007, 1, 1 ) ), 12 );

    testYear(  calendar, QDate( 2005, 8, 31 ), 1384, QString("84"), QString("1384") );
    testMonth( calendar, QDate( 2005, 8, 31 ),    6, QString("6"),  QString("06") );
    testDay(   calendar, QDate( 2005, 8, 31 ),    9, QString("9"),  QString("09") );

    testWeekDayName( calendar, 3, QDate( 2005, 8, 31 ),
                     QString("4sh"), QString("Chahar shanbe") );
    testMonthName( calendar, 6, 1384, QDate( 2005, 8, 31 ),
                   QString("Sha"), QString("Shahrivar"),
                   QString("of Sha"), QString("of Shahrivar") );

    QCOMPARE( calendar->monthsInYear( QDate( 2005, 8, 31 ) ), 12 );

    QCOMPARE( calendar->weekStartDay(), 1 );
    QCOMPARE( calendar->weekDayOfPray(), 5 );

    QCOMPARE( calendar->isProleptic(), false );
    QCOMPARE( calendar->isLunar(), false );
    QCOMPARE( calendar->isLunisolar(), false );
    QCOMPARE( calendar->isSolar(), true );
}

void KCalendarTest::testJalaliYmd()
{
    const KCalendarSystem *calendar = KCalendarSystem::create( QString("jalali") );
    testYmd( calendar, 1384, 6, 9, QDate( 2005, 8, 31 ).toJulianDay() );
}

// generic test functions, call from calendar system specific ones

// Simply tests valid ranges of ymd values, testYmd covers all other dates
void KCalendarTest::testValid( const KCalendarSystem *calendar, int highInvalidYear,
                               int highInvalidMonth, int highInvalidDay, QDate invalidDate )
{
    // min/max year
    QCOMPARE( calendar->isValid( 0, 1, 1 ), false );
    QCOMPARE( calendar->isValid( highInvalidYear, 1, 1 ), false );

    // min/max month
    QCOMPARE( calendar->isValid( 1, 0, 1 ), false );
    QCOMPARE( calendar->isValid( 1, highInvalidMonth, 1 ), false );

    // min/max day
    QCOMPARE( calendar->isValid( 1, 1, 0 ), false );
    QCOMPARE( calendar->isValid( 1, 1, highInvalidDay ), false );

    QCOMPARE( calendar->isValid( 1, 1,  1 ), true );

    QCOMPARE( calendar->isValid( invalidDate ), false );
    QCOMPARE( calendar->isValid( QDate( 2000, 1, 1 ) ), true );
}

void KCalendarTest::testYmd( const KCalendarSystem *calendar, int y, int m, int d, int jd )
{
    QDate testDate1 = QDate::fromJulianDay( jd );
    QCOMPARE( calendar->year(  testDate1 ), y );
    QCOMPARE( calendar->month( testDate1 ), m );
    QCOMPARE( calendar->day(   testDate1 ), d );

    QDate testDate2;
    QVERIFY( calendar->setYMD( testDate2, y, m, d ) );
    QCOMPARE( testDate1, testDate2 );
}

void KCalendarTest::testYear( const KCalendarSystem *calendar, QDate date,
                              int year, QString shortString, QString longString )
{
    QCOMPARE( calendar->year( date ), year );
    QCOMPARE( calendar->yearString( date, KCalendarSystem::ShortFormat ), shortString );
    QCOMPARE( calendar->yearString( date, KCalendarSystem::LongFormat ), longString );
    int i;
    QCOMPARE( calendar->yearStringToInteger( longString, i ), year );
    QCOMPARE( i, longString.length() );
}

void KCalendarTest::testMonth( const KCalendarSystem *calendar, QDate date, 
                               int month, QString shortString, QString longString )
{
    QCOMPARE( calendar->month( date ), month );
    QCOMPARE( calendar->monthString( date, KCalendarSystem::ShortFormat ), shortString );
    QCOMPARE( calendar->monthString( date, KCalendarSystem::LongFormat ), longString );
    int i;
    QCOMPARE( calendar->monthStringToInteger( longString, i ), month );
    QCOMPARE( i, longString.length() );
}

void KCalendarTest::testDay( const KCalendarSystem *calendar, QDate date, 
                               int day, QString shortString, QString longString )
{
    QCOMPARE( calendar->day( date ), day );
    QCOMPARE( calendar->dayString( date, KCalendarSystem::ShortFormat ), shortString );
    QCOMPARE( calendar->dayString( date, KCalendarSystem::LongFormat ), longString );
    int i;
    QCOMPARE( calendar->dayStringToInteger( longString, i ), day );
    QCOMPARE( i, longString.length() );
}

// Pass in the week day number, the equivalent QDate, and the short and long day names
//  e.g. testWeekDayName( cal, 6, QDate(y,m,d), QString("Monday"), QString("Mon") )

void KCalendarTest::testWeekDayName( const KCalendarSystem *calendar, int weekDay, QDate date, 
                                     QString shortName, QString longName )
{
    QCOMPARE( calendar->weekDayName( weekDay, KCalendarSystem::ShortDayName ), shortName );
    QCOMPARE( calendar->weekDayName( weekDay, KCalendarSystem::LongDayName ), longName );
    QCOMPARE( calendar->weekDayName( weekDay ), longName );

    // Test week day < min
    QCOMPARE( calendar->weekDayName( 0, KCalendarSystem::ShortDayName ), QString() );
    QCOMPARE( calendar->weekDayName( 0, KCalendarSystem::LongDayName ), QString() );

    // Test week day > max
    QCOMPARE( calendar->weekDayName( calendar->daysInWeek( date ) + 1, KCalendarSystem::ShortDayName ), QString() );
    QCOMPARE( calendar->weekDayName( calendar->daysInWeek( date ) + 1, KCalendarSystem::LongDayName ), QString() );

    // QDate parameter tests, so always in valid range
    QCOMPARE( calendar->weekDayName( date, KCalendarSystem::ShortDayName ), shortName );
    QCOMPARE( calendar->weekDayName( date, KCalendarSystem::LongDayName ), longName );
    QCOMPARE( calendar->weekDayName( date ), longName );
}

void KCalendarTest::testMonthName( const KCalendarSystem *calendar, int month, int year, QDate date,
                                   QString shortName, QString longName,
                                   QString shortNamePossessive, QString longNamePossessive )
{
    QCOMPARE( calendar->monthName( month, year, KCalendarSystem::ShortName ), shortName );
    QCOMPARE( calendar->monthName( month, year, KCalendarSystem::LongName ), longName );
    QCOMPARE( calendar->monthName( month, year, KCalendarSystem::ShortNamePossessive ),
              shortNamePossessive );
    QCOMPARE( calendar->monthName( month, year, KCalendarSystem::LongNamePossessive ),
              longNamePossessive );
    QCOMPARE( calendar->monthName( month, year ), longName );

    // Test month < min
    QCOMPARE( calendar->monthName( 0, year, KCalendarSystem::ShortName ), QString() );
    QCOMPARE( calendar->monthName( 0, year, KCalendarSystem::LongName ), QString() );
    QCOMPARE( calendar->monthName( 0, year, KCalendarSystem::ShortName ), QString() );
    QCOMPARE( calendar->monthName( 0, year, KCalendarSystem::LongName ), QString() );

    // Test month > max
    QCOMPARE( calendar->monthName( calendar->monthsInYear( date ) + 1, year,
                                   KCalendarSystem::ShortName ), QString() );
    QCOMPARE( calendar->monthName( calendar->monthsInYear( date ) + 1, year,
                                   KCalendarSystem::LongName ), QString() );
    QCOMPARE( calendar->monthName( calendar->monthsInYear( date ) + 1, year,
                                   KCalendarSystem::ShortName ), QString() );
    QCOMPARE( calendar->monthName( calendar->monthsInYear( date ) + 1, year,
                                   KCalendarSystem::LongName ), QString() );

    // QDate parameter tests, so always in valid range
    QCOMPARE( calendar->monthName( date, KCalendarSystem::ShortName ), shortName );
    QCOMPARE( calendar->monthName( date, KCalendarSystem::LongName ), longName );
    QCOMPARE( calendar->monthName( date, KCalendarSystem::ShortNamePossessive ),
              shortNamePossessive );
    QCOMPARE( calendar->monthName( date, KCalendarSystem::LongNamePossessive ),
              longNamePossessive );
    QCOMPARE( calendar->monthName( date ), longName );
}


// Tests to compare new base methods are equal to QDate for Gregorian case


void KCalendarTest::testQDateYearMonthDay()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));

    testQDateYMD( calendar, 2000,  1,  1 );
    testQDateYMD( calendar, 2000,  2, 28 );
    testQDateYMD( calendar, 2000,  2, 29 );
    testQDateYMD( calendar, 2000, 6, 15 );
    testQDateYMD( calendar, 2000, 12, 31 );

    testQDateYMD( calendar, 9999,  1,  1 );
    testQDateYMD( calendar, 9999,  6, 15 );
    testQDateYMD( calendar, 9999, 12, 31 );

    testQDateYMD( calendar,    1,  1,  1 );
    testQDateYMD( calendar,    1,  6, 15 );
    testQDateYMD( calendar,    1, 12, 31 );

    testQDateYMD( calendar, -4713,  1,  2 );
    testQDateYMD( calendar, -4713,  6, 15 );
    testQDateYMD( calendar, -4713, 12, 31 );
}

void KCalendarTest::testQDateYMD( const KCalendarSystem *calendar, int y, int m, int d )
{
    QDate testDate;

    calendar->setDate( testDate, y, m, d );
    QCOMPARE( calendar->year( testDate ),  testDate.year() );
    QCOMPARE( calendar->month( testDate ), testDate.month() );
    QCOMPARE( calendar->day( testDate ),   testDate.day() );
}

void KCalendarTest::testQDateAddYears()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));

    QDate testDate;

    calendar->setDate( testDate, 2000,  1,  1 );
    QCOMPARE( calendar->addYears( testDate, -1 ),  testDate.addYears( -1 ) );
    QCOMPARE( calendar->addYears( testDate,  1 ),  testDate.addYears(  1 ) );

    calendar->setDate( testDate, 2000,  2, 29 );
    QCOMPARE( calendar->addYears( testDate,  1 ),  testDate.addYears(  1 ) );

    calendar->setDate( testDate, -2000,  1,  1 );
    QCOMPARE( calendar->addYears( testDate, -1 ),  testDate.addYears( -1 ) );
    QCOMPARE( calendar->addYears( testDate,  1 ),  testDate.addYears(  1 ) );
}

void KCalendarTest::testQDateAddMonths()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));

    QDate testDate;

    calendar->setDate( testDate, 2000,  1,  1 );
    QCOMPARE( calendar->addMonths( testDate, -1 ),  testDate.addMonths( -1 ) );
    QCOMPARE( calendar->addMonths( testDate,  1 ),  testDate.addMonths(  1 ) );

    calendar->setDate( testDate, 2000,  3,  1 );
    QCOMPARE( calendar->addMonths( testDate, -1 ),  testDate.addMonths( -1 ) );
    QCOMPARE( calendar->addMonths( testDate,  1 ),  testDate.addMonths(  1 ) );

    calendar->setDate( testDate, 2000, 12,  1 );
    QCOMPARE( calendar->addMonths( testDate, -1 ),  testDate.addMonths( -1 ) );
    QCOMPARE( calendar->addMonths( testDate,  1 ),  testDate.addMonths(  1 ) );

    calendar->setDate( testDate, 2000,  1,  1 );
    QCOMPARE( calendar->addMonths( testDate, -30 ),  testDate.addMonths( -30 ) );
    QCOMPARE( calendar->addMonths( testDate,  30 ),  testDate.addMonths(  30 ) );

    calendar->setDate( testDate, -2000,  1,  1 );
    QCOMPARE( calendar->addMonths( testDate, -1 ),  testDate.addMonths( -1 ) );
    QCOMPARE( calendar->addMonths( testDate,  1 ),  testDate.addMonths(  1 ) );

    calendar->setDate( testDate, -2000,  3,  1 );
    QCOMPARE( calendar->addMonths( testDate, -1 ),  testDate.addMonths( -1 ) );
    QCOMPARE( calendar->addMonths( testDate,  1 ),  testDate.addMonths(  1 ) );

    calendar->setDate( testDate, -2000, 12,  1 );
    QCOMPARE( calendar->addMonths( testDate, -1 ),  testDate.addMonths( -1 ) );
    QCOMPARE( calendar->addMonths( testDate,  1 ),  testDate.addMonths(  1 ) );

    calendar->setDate( testDate, -2000,  1,  1 );
    QCOMPARE( calendar->addMonths( testDate, -30 ),  testDate.addMonths( -30 ) );
    QCOMPARE( calendar->addMonths( testDate,  30 ),  testDate.addMonths(  30 ) );
}

void KCalendarTest::testQDateAddDays()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));

    QDate testDate;

    calendar->setDate( testDate, 2000,  1,  1 );
    QCOMPARE( calendar->addDays( testDate, -1 ),  testDate.addDays( -1 ) );
    QCOMPARE( calendar->addDays( testDate,  1 ),  testDate.addDays(  1 ) );

    calendar->setDate( testDate, -2000,  1,  1 );
    QCOMPARE( calendar->addDays( testDate, -1 ),  testDate.addDays( -1 ) );
    QCOMPARE( calendar->addDays( testDate,  1 ),  testDate.addDays(  1 ) );
}

void KCalendarTest::testQDateDaysInYear()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));

    QDate testDate;

    calendar->setDate( testDate, 1900, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
    calendar->setDate( testDate, 1999, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
    calendar->setDate( testDate, 2000, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
    calendar->setDate( testDate, 2001, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
    calendar->setDate( testDate, 2002, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
    calendar->setDate( testDate, 2003, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
    calendar->setDate( testDate, 2004, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
    calendar->setDate( testDate, 2005, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );

    calendar->setDate( testDate, -4700, 1, 1 );
    QEXPECT_FAIL("", "Returns 365 instead of 366", Continue);
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
    calendar->setDate( testDate, -4000, 1, 1 );
    QEXPECT_FAIL("", "Returns 365 instead of 366", Continue);
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );

    calendar->setDate( testDate, 1, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );

    calendar->setDate( testDate, 9996, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
    calendar->setDate( testDate, 9999, 1, 1 );
    QCOMPARE( calendar->daysInYear( testDate ),  testDate.daysInYear() );
}


void KCalendarTest::testQDateDaysInMonth()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));

    QDate testDate;

    // Test all months
    calendar->setDate( testDate, 2000,  1, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000,  2, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000,  3, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000,  4, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000,  5, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000,  6, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000,  7, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000,  8, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000,  9, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000, 10, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000, 11, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2000, 12, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );

    // Test Feb in leap and normal years
    calendar->setDate( testDate, 2000, 2, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
    calendar->setDate( testDate, 2001, 2, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );

    // Test max date
    calendar->setDate( testDate, 9999, 12, 1 );
    QCOMPARE( calendar->daysInMonth( testDate ), testDate.daysInMonth() );
}

void KCalendarTest::testQDateDayOfYear()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));

    QDate testDate;

    calendar->setDate( testDate, 2000,  1,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, 2000,  2, 29 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, 2000,  6,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, 2000, 12,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, 2000, 12, 31 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );

    calendar->setDate( testDate, -2000,  1,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -2000,  2, 29 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -2000,  6,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -2000, 12,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -2000, 12, 31 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );

    calendar->setDate( testDate, -4713,  1,  2 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -4713,  2, 29 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -4713,  6,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -4713, 12,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -4713, 12, 31 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );

    calendar->setDate( testDate, 9999,  1,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, 9999,  2, 29 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, 9999,  6,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, 9999, 12,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, 9999, 12, 31 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );

}

void KCalendarTest::testQDateDayOfWeek()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));

    QDate testDate;

    calendar->setDate( testDate, 2000,  1,  1 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 2000,  1,  2 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 2000,  1,  3 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 2000,  1,  4 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 2000,  1,  5 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 2000,  1,  6 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 2000,  1,  7 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );

    calendar->setDate( testDate, -4713,  1,  2 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4713,  1,  3 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4713,  1,  4 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4713,  1,  5 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4713,  1,  6 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4713,  1,  7 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4713,  1,  8 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );

    calendar->setDate( testDate, 9999,  1,  1 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 9999,  1,  2 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 9999,  1,  3 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 9999,  1,  4 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 9999,  1,  5 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 9999,  1,  6 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, 9999,  1,  7 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
}

// Don'r really need this as Gregorian currently uses QDate directly
void KCalendarTest::testQDateIsLeapYear()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "gregorian" ));

    QDate testDate;

    calendar->setDate( testDate, 2000,  1,  1 );
    QCOMPARE( calendar->isLeapYear( 2000 ),     testDate.isLeapYear( 2000 ) );
    QCOMPARE( calendar->isLeapYear( testDate ), testDate.isLeapYear( 2000 ) );
    calendar->setDate( testDate, 2001,  1,  1 );
    QCOMPARE( calendar->isLeapYear( 2001 ),     testDate.isLeapYear( 2001 ) );
    QCOMPARE( calendar->isLeapYear( testDate ), testDate.isLeapYear( 2001 ) );
    calendar->setDate( testDate, 2004,  1,  1 );
    QCOMPARE( calendar->isLeapYear( 2004 ),     testDate.isLeapYear( 2004 ) );
    QCOMPARE( calendar->isLeapYear( testDate ), testDate.isLeapYear( 2004 ) );

    calendar->setDate( testDate, 1900,  1,  1 );
    QCOMPARE( calendar->isLeapYear( 1900 ),     testDate.isLeapYear( 1900 ) );
    QCOMPARE( calendar->isLeapYear( testDate ), testDate.isLeapYear( 1900 ) );
    calendar->setDate( testDate, 1901,  1,  1 );
    QCOMPARE( calendar->isLeapYear( 1901 ),     testDate.isLeapYear( 1901 ) );
    QCOMPARE( calendar->isLeapYear( testDate ), testDate.isLeapYear( 1901 ) );
    calendar->setDate( testDate, 1904,  1,  1 );
    QCOMPARE( calendar->isLeapYear( 1904 ),     testDate.isLeapYear( 1904 ) );
    QCOMPARE( calendar->isLeapYear( testDate ), testDate.isLeapYear( 1904 ) );

    calendar->setDate( testDate, -2000,  1,  1 );
    QCOMPARE( calendar->isLeapYear( -2000 ),    testDate.isLeapYear( -2000 ) );
    QCOMPARE( calendar->isLeapYear( testDate ), testDate.isLeapYear( -2000 ) );
    calendar->setDate( testDate, -2001,  1,  1 );
    QCOMPARE( calendar->isLeapYear( -2001 ),    testDate.isLeapYear( -2001 ) );
    QCOMPARE( calendar->isLeapYear( testDate ), testDate.isLeapYear( -2001 ) );
    calendar->setDate( testDate, -2004,  1,  1 );
    QCOMPARE( calendar->isLeapYear( -2004 ),    testDate.isLeapYear( -2004 ) );
    QCOMPARE( calendar->isLeapYear( testDate ), testDate.isLeapYear( -2004 ) );
}


// Temporary tests to compare existing code to replacement code


void KCalendarTest::testJalaliCompare()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "jalali" ));

    testCompare( calendar, 2000, 1, 1 );
}

void KCalendarTest::testCompare( const KCalendarSystem *calendar, int year, int month, int day )
{
    QDate dateOld, dateNew;
    calendar->setYMD( dateOld, year, month, day );
    calendar->setDate( dateNew, year, month, day );
    QCOMPARE( dateOld, dateNew );
    QCOMPARE( calendar->daysInYear( dateNew ), dateNew.daysInYear() );
}
