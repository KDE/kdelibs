#include "qtest_kde.h"

#include "kcalendarsystem.h"
#include "kcalendarsystemhebrew.h"
#include "kglobal.h"
#include "kdebug.h"

#include "kcalendartest.h"
#include "kcalendartest.moc"
#include "klocale.h"

QTEST_KDEMAIN_CORE(KCalendarTest)

void KCalendarTest::testTypes()
{
    const KCalendarSystem *calendar;
    calendar = KCalendarSystem::create( QString("gregorian") );
    QCOMPARE( calendar->calendarType(), QString("gregorian") );
    delete calendar;
    calendar = KCalendarSystem::create( QString("gregorian-proleptic") );
    QCOMPARE( calendar->calendarType(), QString("gregorian-proleptic") );
    delete calendar;
    calendar = KCalendarSystem::create( QString("hebrew") );
    QCOMPARE( calendar->calendarType(), QString("hebrew") );
    delete calendar;
    calendar = KCalendarSystem::create( QString("hijri") );
    QCOMPARE( calendar->calendarType(), QString("hijri") );
    delete calendar;
    calendar = KCalendarSystem::create( QString("indian-national") );
    QCOMPARE( calendar->calendarType(), QString("indian-national") );
    delete calendar;
    calendar = KCalendarSystem::create( QString("jalali") );
    QCOMPARE( calendar->calendarType(), QString("jalali") );
    delete calendar;
    calendar = KCalendarSystem::create( QString("julian") );
    QCOMPARE( calendar->calendarType(), QString("julian") );
    delete calendar;
    calendar = KCalendarSystem::create( QString("invalid") );
    QCOMPARE( calendar->calendarType(), QString("gregorian") );
    delete calendar;
    calendar = KCalendarSystem::create( QString() );
    QCOMPARE( calendar->calendarType(), QString("gregorian") );
    delete calendar;

    QStringList lst = KCalendarSystem::calendarSystems();
    QVERIFY( lst.contains("gregorian") );
    QVERIFY( lst.contains("hebrew") );
    QVERIFY( lst.contains("hijri") );
    QVERIFY( lst.contains("indian-national") );
    QVERIFY( lst.contains("jalali") );
    QVERIFY( lst.contains("julian") );

    QCOMPARE( KCalendarSystem::calendarLabel("gregorian"), QString("Gregorian") );
    QCOMPARE( KCalendarSystem::calendarLabel("gregorian-proleptic"), QString("Gregorian (Proleptic)") );
    QCOMPARE( KCalendarSystem::calendarLabel("hebrew"),    QString("Hebrew") );
    QCOMPARE( KCalendarSystem::calendarLabel("hijri"),     QString("Hijri") );
    QCOMPARE( KCalendarSystem::calendarLabel("indian-national"),    QString("Indian National") );
    QCOMPARE( KCalendarSystem::calendarLabel("jalali"),    QString("Jalali") );
    QCOMPARE( KCalendarSystem::calendarLabel("julian"),    QString("Julian") );
}

void KCalendarTest::testLocale()
{
    KGlobal::locale()->setCalendar("gregorian");
    const KCalendarSystem *calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("gregorian") );
    KGlobal::locale()->setCalendar("gregorian-proleptic");
    calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("gregorian-proleptic") );
    KGlobal::locale()->setCalendar("hebrew");
    calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("hebrew") );
    KGlobal::locale()->setCalendar("hijri");
    calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("hijri") );
    KGlobal::locale()->setCalendar("indian-national");
    calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("indian-national") );
    KGlobal::locale()->setCalendar("jalali");
    calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("jalali") );
    KGlobal::locale()->setCalendar("julian");
    calendar = KGlobal::locale()->calendar();
    QCOMPARE( calendar->calendarType(), QString("julian") );
}

void KCalendarTest::testFormatDate()
{
    KLocale *locale = new KLocale( *KGlobal::locale() );
    locale->setDateFormatShort( "%y-%m-%d" );
    const KCalendarSystem *calendar = KCalendarSystem::create( QString( "gregorian" ), locale );

    //Test default settings
    QDate testDate( 2005, 10, 20 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "2005" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "05" ) );
    QCOMPARE( calendar->formatDate( testDate, "%m" ), QString( "10" ) );
    QCOMPARE( calendar->formatDate( testDate, "%n" ), QString( "10" ) );
    QCOMPARE( calendar->formatDate( testDate, "%d" ), QString( "20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%e" ), QString( "20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%B" ), QString( "October" ) );
    QCOMPARE( calendar->formatDate( testDate, "%b" ), QString( "Oct" ) );
    QCOMPARE( calendar->formatDate( testDate, "%h" ), QString( "Oct" ) );
    QCOMPARE( calendar->formatDate( testDate, "%A" ), QString( "Thursday" ) );
    QCOMPARE( calendar->formatDate( testDate, "%a" ), QString( "Thu" ) );
    QCOMPARE( calendar->formatDate( testDate, "%j" ), QString( "293" ) );
    QCOMPARE( calendar->formatDate( testDate, "%V" ), QString( "42" ) );
    QCOMPARE( calendar->formatDate( testDate, "%G" ), QString( "2005" ) );
    QCOMPARE( calendar->formatDate( testDate, "%g" ), QString( "05" ) );
    QCOMPARE( calendar->formatDate( testDate, "%u" ), QString( "4" ) );
    QCOMPARE( calendar->formatDate( testDate, "%D" ), QString( "10/20/05" ) );
    QCOMPARE( calendar->formatDate( testDate, "%F" ), QString( "2005-10-20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%x" ), QString( "05-10-20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%%" ), QString( "%" ) );
    QCOMPARE( calendar->formatDate( testDate, "%t" ), QString( "\t" ) );

    QCOMPARE( calendar->formatDate( testDate, "-%Y-%C-%y-%m-%n-%d-%e-%B-%b-%h-%A-%a-%j-%V-%G-%g-%u:%D:%F:%x:%%-%t-" ),
              QString( "-2005-20-05-10-10-20-20-October-Oct-Oct-Thursday-Thu-293-42-2005-05-4:10/20/05:2005-10-20:05-10-20:%-\t-" ) );

    //Test the minimum widths are respected
    QCOMPARE( calendar->formatDate( testDate, "%1Y" ), QString( "2005" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1C" ), QString( "20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1y" ), QString( "05" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1m" ), QString( "10" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1n" ), QString( "10" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1d" ), QString( "20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1e" ), QString( "20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%01B" ), QString( "October" ) );
    QCOMPARE( calendar->formatDate( testDate, "%01b" ), QString( "Oct" ) );
    QCOMPARE( calendar->formatDate( testDate, "%01h" ), QString( "Oct" ) );
    QCOMPARE( calendar->formatDate( testDate, "%01A" ), QString( "Thursday" ) );
    QCOMPARE( calendar->formatDate( testDate, "%01a" ), QString( "Thu" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1j" ), QString( "293" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1V" ), QString( "42" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1G" ), QString( "2005" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1g" ), QString( "05" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1u" ), QString( "4" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1D" ), QString( "10/20/05" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1F" ), QString( "2005-10-20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1x" ), QString( "05-10-20" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1%" ), QString( "%" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1t" ), QString( "\t" ) );

    testDate.setDate( 2005, 1, 2 );

    //Test various padding options with minimum width
    QCOMPARE( calendar->formatDate( testDate, "%" ), QString( "" ) );
    QCOMPARE( calendar->formatDate( testDate, "%m" ), QString( "01" ) );
    QCOMPARE( calendar->formatDate( testDate, "%n" ), QString( "1" ) );
    QCOMPARE( calendar->formatDate( testDate, "%d" ), QString( "02" ) );
    QCOMPARE( calendar->formatDate( testDate, "%e" ), QString( "2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%j" ), QString( "002" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_m" ), QString( " 1" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_n" ), QString( "1" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_d" ), QString( " 2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_e" ), QString( "2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_j" ), QString( "  2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-m" ), QString( "1" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-n" ), QString( "1" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-d" ), QString( "2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-e" ), QString( "2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-j" ), QString( "2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0m" ), QString( "01" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0n" ), QString( "1" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0d" ), QString( "02" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0e" ), QString( "2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0j" ), QString( "002" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-_j" ), QString( "  2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_-j" ), QString( "2" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-_0j" ), QString( "002" ) );

    //Test various padding options with width override
    QCOMPARE( calendar->formatDate( testDate, "%1m" ), QString( "01" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-05m" ), QString( "00001" ) );
    QCOMPARE( calendar->formatDate( testDate, "%10m" ), QString( "0000000001" ) );
    QCOMPARE( calendar->formatDate( testDate, "%15m" ), QString( "000000000000001" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-5m" ), QString( "1" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_5m" ), QString( "    1" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_15m" ), QString( "              1" ) );
    QCOMPARE( calendar->formatDate( testDate, "%1B" ), QString( "January" ) );
    QCOMPARE( calendar->formatDate( testDate, "%15B" ), QString( "        January" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-15B" ), QString( "January" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_15B" ), QString( "        January" ) );
    QCOMPARE( calendar->formatDate( testDate, "%015B" ), QString( "00000000January" ) );

    //Test week number in previous year
    QCOMPARE( calendar->formatDate( testDate, "%V" ), QString( "53" ) );
    QCOMPARE( calendar->formatDate( testDate, "%G" ), QString( "2004" ) );
    QCOMPARE( calendar->formatDate( testDate, "%g" ), QString( "04" ) );

    //test case overrides
    QCOMPARE( calendar->formatDate( testDate, "%B" ), QString( "January" ) );
    QCOMPARE( calendar->formatDate( testDate, "%^B" ), QString( "JANUARY" ) );
    QCOMPARE( calendar->formatDate( testDate, "%#B" ), QString( "JANUARY" ) );
    QCOMPARE( calendar->formatDate( testDate, "%m" ), QString( "01" ) );
    QCOMPARE( calendar->formatDate( testDate, "%^m" ), QString( "01" ) );
    QCOMPARE( calendar->formatDate( testDate, "%#m" ), QString( "01" ) );

    //Test various year options

    testDate.setDate( 789, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "0789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "07" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "7" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( " 789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( " 7" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( "89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "0789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "07" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "89" ) );

    testDate.setDate( 709, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "0709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "07" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "09" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "7" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( " 709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( " 7" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( " 9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "0709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "07" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "09" ) );

    testDate.setDate( 89, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "0089" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "00" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "0" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( "  89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( " 0" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( "89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "0089" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "00" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "89" ) );

    testDate.setDate( 9, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "0009" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "00" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "09" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "0" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( "   9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( " 0" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( " 9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "0009" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "00" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "09" ) );

    testDate.setDate( -9, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "-0009" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "-00" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "-09" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "-9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "-0" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "-9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( "-   9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( "- 0" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( "- 9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "-0009" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "-00" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "-09" ) );

    testDate.setDate( -89, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "-0089" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "-00" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "-0" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( "-  89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( "- 0" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "-0089" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "-00" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "-89" ) );

    testDate.setDate( -789, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "-0789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "-07" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "-789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "-7" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( "- 789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( "- 7" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "-0789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "-07" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "-89" ) );

    testDate.setDate( -709, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "-0709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "-07" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "-09" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "-709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "-7" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "-9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( "- 709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( "- 7" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( "- 9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "-0709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "-07" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "-09" ) );

    testDate.setDate( -1789, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "-1789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "-17" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "-1789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "-17" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( "-1789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( "-17" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( "-89" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "-1789" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "-17" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "-89" ) );

    testDate.setDate( -1709, 1, 1 );
    QCOMPARE( calendar->formatDate( testDate, "%Y" ), QString( "-1709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%C" ), QString( "-17" ) );
    QCOMPARE( calendar->formatDate( testDate, "%y" ), QString( "-09" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-Y" ), QString( "-1709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-C" ), QString( "-17" ) );
    QCOMPARE( calendar->formatDate( testDate, "%-y" ), QString( "-9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_Y" ), QString( "-1709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_C" ), QString( "-17" ) );
    QCOMPARE( calendar->formatDate( testDate, "%_y" ), QString( "- 9" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0Y" ), QString( "-1709" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0C" ), QString( "-17" ) );
    QCOMPARE( calendar->formatDate( testDate, "%0y" ), QString( "-09" ) );
}

void KCalendarTest::testReadDate()
{
    KLocale *locale = new KLocale( *KGlobal::locale() );
    locale->setDateFormatShort( "%y-%m-%d" );
    const KCalendarSystem *calendar = KCalendarSystem::create( QString( "gregorian" ), locale );
    int currentYear = QDate::currentDate().year();

    //Test default settings
    QDate testDate( 2005, 10, 20 );

    //Test Month and Day
    QCOMPARE( calendar->readDate( "2005-01-01", "%Y-%m-%d" ), QDate( 2005, 1, 1 ) );
    QCOMPARE( calendar->readDate( "05-01-01",   "%y-%m-%d" ), QDate( 2005, 1, 1 ) );
    QCOMPARE( calendar->readDate( "01-01",      "%m-%d" ),    QDate( currentYear, 1, 1 ) );

    //Test Day-of-Year
    QCOMPARE( calendar->readDate( "2005-001", "%Y-%j" ), QDate( 2005, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2005-1",   "%Y-%j" ), QDate( 2005, 1, 1 ) );
    QCOMPARE( calendar->readDate( "001",      "%j" ),    QDate( currentYear, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2005-365", "%Y-%j" ), QDate( 2005, 12, 31 ) );
    QCOMPARE( calendar->readDate( "2000-366", "%Y-%j" ), QDate( 2000, 12, 31 ) );

    QCOMPARE( calendar->readDate( "2005-000", "%Y-%j" ), QDate() );
    QCOMPARE( calendar->readDate( "2005-366", "%Y-%j" ), QDate() );
    QCOMPARE( calendar->readDate( "2000-367", "%Y-%j" ), QDate() );

    //Test Weekday and Month names
    QCOMPARE( calendar->readDate( "Monday 2004-01-05", "%A %Y-%m-%d" ), QDate( 2004,  1,  5 ) );
    QCOMPARE( calendar->readDate( "Mon 2004-01-05",    "%A %Y-%m-%d" ), QDate( 2004,  1,  5 ) );
    QCOMPARE( calendar->readDate( "Monday 2004-01-05", "%a %Y-%m-%d" ), QDate( 2004,  1,  5 ) );
    QCOMPARE( calendar->readDate( "Mon 2004-01-05",    "%a %Y-%m-%d" ), QDate( 2004,  1,  5 ) );

    QCOMPARE( calendar->readDate( "Sunday 2004-01-11", "%A %Y-%m-%d" ), QDate( 2004,  1, 11 ) );
    QCOMPARE( calendar->readDate( "Sun 2004-01-11",    "%A %Y-%m-%d" ), QDate( 2004,  1, 11 ) );
    QCOMPARE( calendar->readDate( "Sunday 2004-01-11", "%a %Y-%m-%d" ), QDate( 2004,  1, 11 ) );
    QCOMPARE( calendar->readDate( "Sun 2004-01-11",    "%a %Y-%m-%d" ), QDate( 2004,  1, 11 ) );

    QCOMPARE( calendar->readDate( "2004 January 01",  "%Y %B %d" ), QDate( 2004,  1,  1 ) );
    QCOMPARE( calendar->readDate( "2004 Jan 01",      "%Y %B %d" ), QDate( 2004,  1,  1 ) );
    QCOMPARE( calendar->readDate( "2004 January 01",  "%Y %b %d" ), QDate( 2004,  1,  1 ) );
    QCOMPARE( calendar->readDate( "2004 Jan 01",      "%Y %b %d" ), QDate( 2004,  1,  1 ) );

    QCOMPARE( calendar->readDate( "2004 December 01", "%Y %B %d" ), QDate( 2004, 12,  1 ) );
    QCOMPARE( calendar->readDate( "2004 Dec 01",      "%Y %B %d" ), QDate( 2004, 12,  1 ) );
    QCOMPARE( calendar->readDate( "2004 December 01", "%Y %b %d" ), QDate( 2004, 12,  1 ) );
    QCOMPARE( calendar->readDate( "2004 Dec 01",      "%Y %b %d" ), QDate( 2004, 12,  1 ) );

    QCOMPARE( calendar->readDate( "Wednesday 2004 December 01", "%A %Y %B %d" ), QDate( 2004, 12,  1 ) );
    QCOMPARE( calendar->readDate( "Wed 2004 Dec 01",            "%A %Y %B %d" ), QDate( 2004, 12,  1 ) );
    QCOMPARE( calendar->readDate( "Wednesday 2004 December 01", "%a %Y %b %d" ), QDate( 2004, 12,  1 ) );
    QCOMPARE( calendar->readDate( "Wed 2004 Dec 01",            "%a %Y %b %d" ), QDate( 2004, 12,  1 ) );

    //Test ISO Week and Day-of-Week
    QCOMPARE( calendar->readDate( "2004-W01-01", "%Y-W%V-%u" ), QDate( 2003, 12, 29 ) );
    QCOMPARE( calendar->readDate( "2004-W01-02", "%Y-W%V-%u" ), QDate( 2003, 12, 30 ) );
    QCOMPARE( calendar->readDate( "2004-W01-03", "%Y-W%V-%u" ), QDate( 2003, 12, 31 ) );
    QCOMPARE( calendar->readDate( "2004-W01-04", "%Y-W%V-%u" ), QDate( 2004, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2004-W01-05", "%Y-W%V-%u" ), QDate( 2004, 1, 2 ) );
    QCOMPARE( calendar->readDate( "2004-W01-06", "%Y-W%V-%u" ), QDate( 2004, 1, 3 ) );
    QCOMPARE( calendar->readDate( "2004-W01-07", "%Y-W%V-%u" ), QDate( 2004, 1, 4 ) );

    QCOMPARE( calendar->readDate( "2004-W24-01", "%Y-W%V-%u" ), QDate( 2004, 6, 7 ) );
    QCOMPARE( calendar->readDate( "2004-W24-02", "%Y-W%V-%u" ), QDate( 2004, 6, 8 ) );
    QCOMPARE( calendar->readDate( "2004-W24-03", "%Y-W%V-%u" ), QDate( 2004, 6, 9 ) );
    QCOMPARE( calendar->readDate( "2004-W24-04", "%Y-W%V-%u" ), QDate( 2004, 6, 10 ) );
    QCOMPARE( calendar->readDate( "2004-W24-05", "%Y-W%V-%u" ), QDate( 2004, 6, 11 ) );
    QCOMPARE( calendar->readDate( "2004-W24-06", "%Y-W%V-%u" ), QDate( 2004, 6, 12 ) );
    QCOMPARE( calendar->readDate( "2004-W24-07", "%Y-W%V-%u" ), QDate( 2004, 6, 13 ) );

    QCOMPARE( calendar->readDate( "2004-W53-01", "%Y-W%V-%u" ), QDate( 2004, 12, 27 ) );
    QCOMPARE( calendar->readDate( "2004-W53-02", "%Y-W%V-%u" ), QDate( 2004, 12, 28 ) );
    QCOMPARE( calendar->readDate( "2004-W53-03", "%Y-W%V-%u" ), QDate( 2004, 12, 29 ) );
    QCOMPARE( calendar->readDate( "2004-W53-04", "%Y-W%V-%u" ), QDate( 2004, 12, 30 ) );
    QCOMPARE( calendar->readDate( "2004-W53-05", "%Y-W%V-%u" ), QDate( 2004, 12, 31 ) );
    QCOMPARE( calendar->readDate( "2004-W53-06", "%Y-W%V-%u" ), QDate( 2005, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2004-W53-07", "%Y-W%V-%u" ), QDate( 2005, 1, 2 ) );

    //Need to fix each year!
    QCOMPARE( calendar->readDate( "W46-05", "W%V-%u" ), QDate( 2009, 11, 13 ) );

    QCOMPARE( calendar->readDate( "2004-W00-01", "%Y-W%V-%u" ), QDate() );
    QCOMPARE( calendar->readDate( "2004-W01-00", "%Y-W%V-%u" ), QDate() );
    QCOMPARE( calendar->readDate( "2004-W01-08", "%Y-W%V-%u" ), QDate() );
    QCOMPARE( calendar->readDate( "2004-W54-01", "%Y-W%V-%u" ), QDate() );

    //Test all 3 options, should go with most accurate
    QCOMPARE( calendar->readDate( "2004-01-01-365-W52-01", "%Y-%m-%d-%j-W%V-%u" ), QDate( 2004, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2004-001-W52-01",       "%Y-%j-W%V-%u" ),       QDate( 2004, 1, 1 ) );

    //Test spaces and literals
    QCOMPARE( calendar->readDate( "2004- 1-1",    "%Y-%m-%d" ), QDate( 2004, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2004-1-1",     "%Y-%m-%e" ), QDate( 2004, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2004 01 01",   "%Y %m %d" ), QDate( 2004, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2004  01  01", "%Y %m %d" ), QDate( 2004, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2004W01-01",   "%YW%m-%d" ), QDate( 2004, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2004w01-01",   "%YW%m-%d" ), QDate( 2004, 1, 1 ) );
    QCOMPARE( calendar->readDate( "2004W01-01",   "%Yw%m-%d" ), QDate( 2004, 1, 1 ) );

    //Test enums and non-strict version of readDate()
    locale->setDateFormat( "%d/%m/%Y" );
    locale->setDateFormatShort( "%Y/%m/%d" );
    QCOMPARE( calendar->readDate( "01/02/2004" ),  QDate( 2004, 2, 1 ) ); //KLocale::NormalFormat
    QCOMPARE( calendar->readDate( "2004/02/01" ),  QDate( 2004, 2, 1 ) ); //KLocale::ShortFormat
    QCOMPARE( calendar->readDate( "2004-02-01" ),  QDate( 2004, 2, 1 ) ); //KLocale::IsoFormat
    QCOMPARE( calendar->readDate( "2004-W02-01" ), QDate( 2004, 1, 5 ) ); //KLocale::IsoWeekFormat
    QCOMPARE( calendar->readDate( "2004-001" ),    QDate( 2004, 1, 1 ) ); //KLocale::IsoOrdinalFormat

    //Test enums and strict version of readDate()
    QCOMPARE( calendar->readDate( "01/02/2004",  KLocale::NormalFormat ),     QDate( 2004, 2, 1 ) );
    QCOMPARE( calendar->readDate( "2004/02/01",  KLocale::ShortFormat ),      QDate( 2004, 2, 1 ) );
    QCOMPARE( calendar->readDate( "2004-02-01",  KLocale::IsoFormat ),        QDate( 2004, 2, 1 ) );
    QCOMPARE( calendar->readDate( "2004-W02-01", KLocale::IsoWeekFormat ),    QDate( 2004, 1, 5 ) );
    QCOMPARE( calendar->readDate( "2004-001",    KLocale::IsoOrdinalFormat ), QDate( 2004, 1, 1 ) );
}

void KCalendarTest::testStringForms()
{
    testStrings( KLocale::ArabicDigits );
    testStrings( KLocale::ArabicIndicDigits );
}

void KCalendarTest::testStrings( KLocale::DigitSet testDigitSet )
{
    KLocale *locale = new KLocale( *KGlobal::locale() );
    locale->setDigitSet( testDigitSet );
    const KCalendarSystem *calendar = KCalendarSystem::create( QString( "gregorian" ), locale );

    //Test default settings
    QDate testDate( 2005, 1, 3 );

    //Test Year String
    QCOMPARE( calendar->yearString( testDate ), locale->convertDigits( "2005", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->yearString( testDate, KCalendarSystem::LongFormat ), locale->convertDigits( "2005", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->yearString( testDate, KCalendarSystem::ShortFormat ), locale->convertDigits( "05", locale->dateTimeDigitSet() ) );

    //Test Month String
    QCOMPARE( calendar->monthString( testDate ), locale->convertDigits( "01", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->monthString( testDate, KCalendarSystem::LongFormat ), locale->convertDigits( "01", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->monthString( testDate, KCalendarSystem::ShortFormat ), locale->convertDigits( "1", locale->dateTimeDigitSet() ) );

    //Test Day String
    QCOMPARE( calendar->dayString( testDate ), locale->convertDigits( "03", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->dayString( testDate, KCalendarSystem::LongFormat ), locale->convertDigits( "03", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->dayString( testDate, KCalendarSystem::ShortFormat ), locale->convertDigits( "3", locale->dateTimeDigitSet() ) );

    //Test Day of Year String
    QCOMPARE( calendar->dayOfYearString( testDate ), locale->convertDigits( "003", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->dayOfYearString( testDate, KCalendarSystem::LongFormat ), locale->convertDigits( "003", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->dayOfYearString( testDate, KCalendarSystem::ShortFormat ), locale->convertDigits( "3", locale->dateTimeDigitSet() ) );

    //Test Day of Week String
    QCOMPARE( calendar->dayOfWeekString( testDate ), locale->convertDigits( "1", locale->dateTimeDigitSet() ) );

    //Test Week Number String
    QCOMPARE( calendar->weekNumberString( testDate ), locale->convertDigits( "01", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->weekNumberString( testDate, KCalendarSystem::LongFormat ), locale->convertDigits( "01", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->weekNumberString( testDate, KCalendarSystem::ShortFormat ), locale->convertDigits( "1", locale->dateTimeDigitSet() ) );

    //Test Months In Year String
    QCOMPARE( calendar->monthsInYearString( testDate ), locale->convertDigits( "12", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->monthsInYearString( testDate, KCalendarSystem::LongFormat ), locale->convertDigits( "12", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->monthsInYearString( testDate, KCalendarSystem::ShortFormat ), locale->convertDigits( "12", locale->dateTimeDigitSet() ) );

    //Test Weeks In Year String
    QCOMPARE( calendar->weeksInYearString( testDate ), locale->convertDigits( "52", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->weeksInYearString( testDate, KCalendarSystem::LongFormat ), locale->convertDigits( "52", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->weeksInYearString( testDate, KCalendarSystem::ShortFormat ), locale->convertDigits( "52", locale->dateTimeDigitSet() ) );

    //Test Days In Year String
    QCOMPARE( calendar->daysInYearString( testDate ), locale->convertDigits( "365", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->daysInYearString( testDate, KCalendarSystem::LongFormat ), locale->convertDigits( "365", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->daysInYearString( testDate, KCalendarSystem::ShortFormat ), locale->convertDigits( "365", locale->dateTimeDigitSet() ) );

    //Test Days In Month String
    QCOMPARE( calendar->daysInMonthString( testDate ), locale->convertDigits( "31", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->daysInMonthString( testDate, KCalendarSystem::LongFormat ), locale->convertDigits( "31", locale->dateTimeDigitSet() ) );
    QCOMPARE( calendar->daysInMonthString( testDate, KCalendarSystem::ShortFormat ), locale->convertDigits( "31", locale->dateTimeDigitSet() ) );

    //Test Days In Week String
    QCOMPARE( calendar->daysInWeekString( testDate ), locale->convertDigits( "7", locale->dateTimeDigitSet() ) );
}

void KCalendarTest::testIsoWeekDate()
{
    KLocale *locale = new KLocale( *KGlobal::locale() );
    const KCalendarSystem *calendar = KCalendarSystem::create( QString( "gregorian" ), locale );

    int earliestValidYear = calendar->year( calendar->earliestValidDate() );
    int latestValidYear = calendar->year( calendar->latestValidDate() );

    QCOMPARE( calendar->isValidIsoWeekDate( earliestValidYear - 1, 1, 1 ), false );
    QCOMPARE( calendar->isValidIsoWeekDate( earliestValidYear,     1, 1 ), false ); //In prev year
    QCOMPARE( calendar->isValidIsoWeekDate( earliestValidYear,     1, 2 ), false ); //In prev year
    QCOMPARE( calendar->isValidIsoWeekDate( earliestValidYear,     1, 3 ), true );
    QCOMPARE( calendar->isValidIsoWeekDate( -1,                    1, 1 ), true );
    QCOMPARE( calendar->isValidIsoWeekDate( 0,                     1, 1 ), false );
    QCOMPARE( calendar->isValidIsoWeekDate( 1,                     1, 1 ), true );
    QCOMPARE( calendar->isValidIsoWeekDate( latestValidYear,      52, 5 ), true );
    QCOMPARE( calendar->isValidIsoWeekDate( latestValidYear,      52, 6 ), false ); //In following year
    QCOMPARE( calendar->isValidIsoWeekDate( latestValidYear,      52, 7 ), false ); //In following year
    QCOMPARE( calendar->isValidIsoWeekDate( latestValidYear + 1,   1, 1 ), false );

    QCOMPARE( calendar->isValidIsoWeekDate( 2004,  0, 1 ), false );
    QCOMPARE( calendar->isValidIsoWeekDate( 2004,  1, 1 ), true );
    QCOMPARE( calendar->isValidIsoWeekDate( 2004, 53, 1 ), true );
    QCOMPARE( calendar->isValidIsoWeekDate( 2000, 54, 1 ), false );

    QCOMPARE( calendar->isValidIsoWeekDate( 2004, 1, 0 ), false );
    QCOMPARE( calendar->isValidIsoWeekDate( 2004, 1, 1 ), true );
    QCOMPARE( calendar->isValidIsoWeekDate( 2004, 1, 7 ), true );
    QCOMPARE( calendar->isValidIsoWeekDate( 2004, 1, 8 ), false );

    QCOMPARE( setIsoWeekDate( calendar, earliestValidYear - 1, 1, 1 ), QDate() );
    QCOMPARE( setIsoWeekDate( calendar, earliestValidYear,     1, 1 ), QDate() ); //In prev year
    QCOMPARE( setIsoWeekDate( calendar, earliestValidYear,     1, 2 ), QDate() ); //In prev year
    QCOMPARE( setIsoWeekDate( calendar, earliestValidYear,     1, 3 ), QDate( -4712,  1,  1 ) );
    QCOMPARE( setIsoWeekDate( calendar, -1,                    1, 1 ), QDate(    -2, 12, 29 ) );
    QCOMPARE( setIsoWeekDate( calendar, 0,                     1, 1 ), QDate() );
    QCOMPARE( setIsoWeekDate( calendar, 1,                     1, 1 ), QDate(     1,  1,  3 ) );
    QCOMPARE( setIsoWeekDate( calendar, latestValidYear,      52, 5 ), QDate(  9999, 12, 31 ) );
    QCOMPARE( setIsoWeekDate( calendar, latestValidYear,      52, 6 ), QDate() ); //In next year
    QCOMPARE( setIsoWeekDate( calendar, latestValidYear,      52, 7 ), QDate() ); //In next year
    QCOMPARE( setIsoWeekDate( calendar, latestValidYear + 1,   1, 1 ), QDate() );

    QCOMPARE( setIsoWeekDate( calendar, 2004,  0, 1 ), QDate() );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 1 ), QDate( 2003, 12, 29 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 53, 1 ), QDate( 2004, 12, 27 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2000, 54, 1 ), QDate() );

    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 0 ), QDate() );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 1 ), QDate( 2003, 12, 29 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 7 ), QDate( 2004,  1,  4 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 8 ), QDate() );

    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 1 ), QDate( 2003, 12, 29 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 2 ), QDate( 2003, 12, 30 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 3 ), QDate( 2003, 12, 31 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 4 ), QDate( 2004,  1,  1 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 5 ), QDate( 2004,  1,  2 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 6 ), QDate( 2004,  1,  3 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004,  1, 7 ), QDate( 2004,  1,  4 ) );

    QCOMPARE( setIsoWeekDate( calendar, 2004, 24, 1 ), QDate( 2004, 6,  7 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 24, 2 ), QDate( 2004, 6,  8 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 24, 3 ), QDate( 2004, 6,  9 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 24, 4 ), QDate( 2004, 6, 10 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 24, 5 ), QDate( 2004, 6, 11 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 24, 6 ), QDate( 2004, 6, 12 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 24, 7 ), QDate( 2004, 6, 13 ) );

    QCOMPARE( setIsoWeekDate( calendar, 2004, 53, 1 ), QDate( 2004, 12, 27 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 53, 2 ), QDate( 2004, 12, 28 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 53, 3 ), QDate( 2004, 12, 29 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 53, 4 ), QDate( 2004, 12, 30 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 53, 5 ), QDate( 2004, 12, 31 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 53, 6 ), QDate( 2005,  1,  1 ) );
    QCOMPARE( setIsoWeekDate( calendar, 2004, 53, 7 ), QDate( 2005,  1,  2 ) );

}

QDate KCalendarTest::setIsoWeekDate( const KCalendarSystem * calendar, int year, int isoWeek, int dayOfWeek )
{
    QDate resultDate;
    calendar->setDateIsoWeek( resultDate, year, isoWeek, dayOfWeek );
    return resultDate;
}

void KCalendarTest::testDayOfYearDate()
{
    KLocale *locale = new KLocale( *KGlobal::locale() );
    const KCalendarSystem *calendar = KCalendarSystem::create( QString( "gregorian" ), locale );

    int earliestValidYear = calendar->year( calendar->earliestValidDate() );
    int latestValidYear = calendar->year( calendar->latestValidDate() );

    QCOMPARE( calendar->isValid( earliestValidYear - 1, 1 ), false );
    QCOMPARE( calendar->isValid( earliestValidYear,     1 ), true );
    QCOMPARE( calendar->isValid( -1,                    1 ), true );
    QCOMPARE( calendar->isValid( 0,                     1 ), false );
    QCOMPARE( calendar->isValid( 1,                     1 ), true );
    QCOMPARE( calendar->isValid( latestValidYear,       1 ), true );
    QCOMPARE( calendar->isValid( latestValidYear + 1,   1 ), false );

    QCOMPARE( calendar->isValid( 2005,   0 ), false );
    QCOMPARE( calendar->isValid( 2005,   1 ), true );
    QCOMPARE( calendar->isValid( 2005, 365 ), true );
    QCOMPARE( calendar->isValid( 2005, 366 ), false );
    QCOMPARE( calendar->isValid( 2000, 366 ), true );
    QCOMPARE( calendar->isValid( 2000, 367 ), false );

    QCOMPARE( setDayOfYearDate( calendar, earliestValidYear - 1, 1 ), QDate() );
    QCOMPARE( setDayOfYearDate( calendar, earliestValidYear,     1 ), QDate( earliestValidYear, 1, 1 ) );
    QCOMPARE( setDayOfYearDate( calendar, -1,                    1 ), QDate( -1,                1, 1 ) );
    QCOMPARE( setDayOfYearDate( calendar, 0,                     1 ), QDate() );
    QCOMPARE( setDayOfYearDate( calendar, 1,                     1 ), QDate( 1,                 1, 1 ) );
    QCOMPARE( setDayOfYearDate( calendar, latestValidYear,       1 ), QDate( latestValidYear,   1, 1 ) );
    QCOMPARE( setDayOfYearDate( calendar, latestValidYear + 1,   1 ), QDate() );

    QCOMPARE( setDayOfYearDate( calendar, 2005,   0 ), QDate() );
    QCOMPARE( setDayOfYearDate( calendar, 2005,   1 ), QDate( 2005,  1,  1 ) );
    QCOMPARE( setDayOfYearDate( calendar, 2005, 365 ), QDate( 2005, 12, 31 ) );
    QCOMPARE( setDayOfYearDate( calendar, 2005, 366 ), QDate() );
    QCOMPARE( setDayOfYearDate( calendar, 2000, 366 ), QDate( 2000, 12, 31 ) );
    QCOMPARE( setDayOfYearDate( calendar, 2000, 367 ), QDate() );
}

QDate KCalendarTest::setDayOfYearDate( const KCalendarSystem * calendar, int year, int dayOfYear )
{
    QDate resultDate;
    calendar->setDate( resultDate, year, dayOfYear );
    return resultDate;
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

void KCalendarTest::testHebrew()
{
/*
    const KCalendarSystem *calendar = KCalendarSystem::create(QString( "hebrew" ));
    QDate testDate( 2005, 9, 10 );
    QCOMPARE( calendar->dayOfYear( testDate ), 253 );

    QVERIFY( calendar->setYMD( testDate, 5760, 12, 24 ) );
    QCOMPARE( calendar->year( testDate ), 5760 );
    QCOMPARE( calendar->month( testDate ), 12 );
    QCOMPARE( calendar->day( testDate ), 24 );
    QCOMPARE( calendar->daysInYear( testDate ), 999 );

    QDate newDate = calendar->addYears( testDate, 4);
    QCOMPARE( newDate.year(), 5760 );
    QCOMPARE( calendar->daysInYear( newDate ), 999 );

    newDate = calendar->addMonths( testDate, -4 );
    QCOMPARE( newDate.year(), 5760 );
    QCOMPARE( newDate.month(), 8 );
    QCOMPARE( newDate.day(), 24 );

    newDate = calendar->addDays( newDate, 20 );
    QCOMPARE( newDate.year(), 5760 );
    QCOMPARE( newDate.month(), 9 );
    QCOMPARE( newDate.day(), 11 );
*/
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


void KCalendarTest::testIndianNational()
{
    const KCalendarSystem *calendar = KCalendarSystem::create("indian-national");
    QDate testDate( 2005, 9, 10 );

    calendar->setYMD( testDate, 2000, 1, 1 );
    calendar->setYMD( testDate, 2000, 1, 10 );
    calendar->setYMD( testDate, 2000, 2, 10 );
    calendar->setYMD( testDate, 2000, 5, 20 );
    calendar->setYMD( testDate, 2000, 7, 1 );
    calendar->setYMD( testDate, 2000, 7, 10 );
    calendar->setYMD( testDate, 2000, 9, 10 );
    calendar->setYMD( testDate, 2000, 12, 20 );

    QCOMPARE( calendar->isLeapYear( 2009 ), false );
    QCOMPARE( calendar->isLeapYear( testDate ), false );

    QVERIFY( calendar->setYMD( testDate, 2000, 3, 1 ) );
    QCOMPARE( calendar->year( testDate ), 2000 );
    QCOMPARE( calendar->month( testDate ), 3 );
    QCOMPARE( calendar->day( testDate ), 1 );

}

void KCalendarTest::testGregorianBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(QString("gregorian"));

    QCOMPARE( calendar->calendarType(), QString("gregorian") );
    QCOMPARE( KCalendarSystem::calendarLabel( QString("gregorian") ), QString("Gregorian") );

    QEXPECT_FAIL("", "QDate is Julian, get right date", Continue);
    QCOMPARE( calendar->epoch(), QDate( 1, 1, 1 ) );
    QCOMPARE( calendar->earliestValidDate(), QDate( -4712, 1, 1 ) );
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


// Test Hebrew Calendar System

void KCalendarTest::testHebrewBasic()
{
/*
    const KCalendarSystem *calendar = KCalendarSystem::create(QString("hebrew"));

    QCOMPARE( calendar->calendarType(), QString("hebrew") );
    QCOMPARE( KCalendarSystem::calendarLabel( QString("hebrew") ), QString("Hebrew") );
    QCOMPARE( calendar->epoch(), QDate( 1, 1, 1 ) );
    QCOMPARE( calendar->earliestValidDate(), QDate( -4712, 1, 2 ) );
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
    QCOMPARE( calendar->weekDayOfPray(), 6 );

    QCOMPARE( calendar->isProleptic(), false );
    QCOMPARE( calendar->isLunar(), false );
    QCOMPARE( calendar->isLunisolar(), true );
    QCOMPARE( calendar->isSolar(), false );
*/
}

void KCalendarTest::testHebrewYmd()
{
//    const KCalendarSystem *calendar = KCalendarSystem::create(QString("hebrew"));
//    testYmd( calendar, 2007, 1, 1, QDate( 2007, 1, 1 ).toJulianDay() );
}

void KCalendarTest::testHebrewCompare()
{
/*
    const KCalendarSystemHebrew *calendar = new KCalendarSystemHebrew();
    QDate setOldDate, setNewDate;

    for ( QDate testDate( 1900, 1, 1 ); testDate.year() == 2100; testDate.addDays(1) ) {
        QCOMPARE( calendar->year(  testDate ), calendar->oldyear(  testDate ) );
        QCOMPARE( calendar->month( testDate ), calendar->oldmonth( testDate ) );
        QCOMPARE( calendar->day(   testDate ), calendar->oldday(   testDate ) );

        calendar->setYMD(setOldDate, calendar->oldyear( testDate ), calendar->oldmonth( testDate ), calendar->oldday( testDate ) );
        calendar->setDate(setNewDate, calendar->oldyear( testDate ), calendar->oldmonth( testDate ), calendar->oldday( testDate ) );
        QCOMPARE( setNewDate.toJulianDay(), setOldDate.toJulianDay() );
    }
*/
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

    testQDateYMD( calendar, -4712,  1,  2 );
    testQDateYMD( calendar, -4712,  6, 15 );
    testQDateYMD( calendar, -4712, 12, 31 );
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

    calendar->setDate( testDate, -4712,  1,  2 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -4712,  2, 29 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -4712,  6,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -4712, 12,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate( testDate, -4712, 12, 31 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );

    calendar->setDate( testDate, 9999,  1,  1 );
    QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    //QDate::dayOfYear doesn't check if invalid!!!
    //calendar->setDate( testDate, 9999,  2, 29 );
    //QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
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

    calendar->setDate( testDate, -4712,  1,  2 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4712,  1,  3 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4712,  1,  4 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4712,  1,  5 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4712,  1,  6 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4712,  1,  7 );
    QCOMPARE( calendar->dayOfWeek( testDate ), testDate.dayOfWeek() );
    calendar->setDate( testDate, -4712,  1,  8 );
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
