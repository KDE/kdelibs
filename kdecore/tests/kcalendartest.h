#ifndef KCALENDARTEST_H
#define KCALENDARTEST_H

#include <QtCore/QObject>

#include "klocale.h"

class QString;
class QDate;

class KCalendarSystem;

class KCalendarTest : public QObject
{
    Q_OBJECT

protected:
    void testCalendarSystemType( const KCalendarSystem *calendar,
                                 KLocale::CalendarSystem system,
                                 const QString &type, const QString &label );
    void testValid( const KCalendarSystem *calendar, int lowInvalidYear, int highInvalidYear,
                    int highInvalidMonth, int highInvalidDay, const QDate &invalidDate );
    void testEpoch( const KCalendarSystem *calendar, int y, int m, int d, int jd );
    void testEarliestValidDate( const KCalendarSystem *calendar, int y, int m, int d, int jd );
    void testLatestValidDate( const KCalendarSystem *calendar, int y, int m, int d, int jd );
    void testYear( const KCalendarSystem *calendar, const QDate &date,
                   int year, const QString &shortString, const QString &longString );
    void testMonth( const KCalendarSystem *calendar, const QDate &date,
                    int month, const QString &shortString, const QString &longString );
    void testDay( const KCalendarSystem *calendar, const QDate &date,
                  int day, const QString &shortString, const QString &longString );
    void testEraDate( const KCalendarSystem *calendar, const QDate &date,
                      int yearInEra, const QString &shortYearInEraString, const QString &longYearInEraString,
                      const QString &shortEraName, const QString &longEraName );
    void testYmd( const KCalendarSystem *calendar, int y, int m, int d, int jd );
    void testWeekDayName( const KCalendarSystem *calendar, int weekDay, const QDate &date,
                          const QString &narrowName, const QString &shortName, const QString &longName );
    void testMonthName( const KCalendarSystem *calendar, int month, int year, const QDate &date,
                        const QString &narrowName, const QString &shortName, const QString &longName,
                        const QString &shortNamePossessive, const QString &longNamePossessive );
    void testQDateCompareQDate( const KCalendarSystem *calendar, int year, int month, int day );
    void testQDateYMD( const KCalendarSystem *calendar, int y, int m, int d );

    void testStrings( KLocale::DigitSet testDigitSet );
    QDate setDayOfYearDate( const KCalendarSystem * calendar, int year, int dayOfYear );
    void compareDateDifference( const KCalendarSystem *calendar,
                                const QDate &lowDate, const QDate &highDate,
                                int yearsDiff, int monthsDiff, int daysDiff );
    void compareYearDifference( const KCalendarSystem *calendar,
                                const QDate &lowDate, const QDate &highDate,
                                int yearsDiff );
    void compareMonthDifference( const KCalendarSystem *calendar,
                                 const QDate &lowDate, const QDate &highDate,
                                 int monthsDiff );
    QDate setIsoWeekDate( const KCalendarSystem *calendar, int year, int isoWeek, int dayOfWeek );
    QDate setEraDate( const KCalendarSystem *calendar, const QString &era, int yearInEra, int month, int day );
    void testRoundTrip( const KCalendarSystem *calendar, bool testFullRange = false );
    void compareFormatUnicode( const KCalendarSystem *calendar, const QDate &testDate, const QString &testFormat );

private Q_SLOTS:
    void testTypes();
    void testFormatDate();
    void testFormatUnicode();
    void testReadDate();
    void testStringForms();
    void testHebrewStrings();
    void testIsoWeekDate();
    void testDayOfYearDate();
    void testDateDifference();
    void testFirstLast();
    void testEra();
    void testQDateCalendar();
    void testHebrew();
    void testIndianNational();
    void testIslamicCivil();
    void testQDateCalendarBasic();
    void testQDateCalendarYmd();
    void testQDateCalendarSpecialCases();
    void testGregorianBasic();
    void testHebrewBasic();
    void testHebrewYmd();
    void testHebrewCompare();
    void testIslamicCivilBasic();
    void testIslamicCivilYmd();
    void testJalaliBasic();
    void testJalaliYmd();
    void testJapanese();
    void testJapaneseBasic();
    void testJapaneseYmd();
    void testJapaneseSpecialCases();
    void testMinguoBasic();
    void testThaiBasic();
    void testQDateYearMonthDay();
    void testQDateAddYears();
    void testQDateAddMonths();
    void testQDateAddDays();
    void testQDateDaysInYear();
    void testQDateDaysInMonth();
    void testQDateDayOfYear();
    void testQDateDayOfWeek();
    void testQDateIsLeapYear();
    void testKLocalizedDate();
    void testWeekNumberSystem();
};

#endif
