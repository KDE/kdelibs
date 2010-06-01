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
                          const QString &shortName, const QString &longName );
    void testMonthName( const KCalendarSystem *calendar, int month, int year, const QDate &date,
                        const QString &shortName, const QString &longName,
                        const QString &shortNamePossessive, const QString &longNamePossessive );
    void testGregorianCompareQDate( const KCalendarSystem *calendar, int year, int month, int day );
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
    void testRoundTrip( const KCalendarSystem *calendar );
    void compareFormatUnicode( const KCalendarSystem *calendar, const QDate &testDate, const QString &testFormat );

private Q_SLOTS:
    void testTypes();
    void testLocale();
    void testFormatDate();
    void testFormatUnicode();
    void testReadDate();
    void testStringForms();
    void testHebrewStrings();
    void testIsoWeekDate();
    void testDayOfYearDate();
    void testDateDifference();
    void testEra();
    void testGregorian();
    void testHebrew();
    void testHijri();
    void testIndianNational();
    void testGregorianBasic();
    void testGregorianYmd();
    void testGregorianSpecialCases();
    void testGregorianProlepticBasic();
    void testHebrewBasic();
    void testHebrewYmd();
    void testHebrewCompare();
    void testHijriBasic();
    void testHijriYmd();
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
};

#endif
