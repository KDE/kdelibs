#ifndef KCALENDARTEST_H
#define KCALENDARTEST_H

#include <QtCore/QObject>

class KCalendarTest : public QObject
{
    Q_OBJECT

protected:
    void testValid( const KCalendarSystem *calendar, int highInvalidYear,
                    int highInvalidMonth, int highInvalidDay, QDate invalidDate );
    void testYear( const KCalendarSystem *calendar, QDate date,
                   int year, QString shortString, QString longString );
    void testMonth( const KCalendarSystem *calendar, QDate date, 
                    int month, QString shortString, QString longString );
    void testDay( const KCalendarSystem *calendar, QDate date, 
                               int day, QString shortString, QString longString );
    void testYmd( const KCalendarSystem *calendar, int y, int m, int d, int jd );
    void testWeekDayName( const KCalendarSystem *calendar, int weekDay, QDate date, 
                          QString shortName, QString longName );
    void testMonthName( const KCalendarSystem *calendar, int month, int year, QDate date,
                        QString shortName, QString longName,
                        QString shortNamePossessive, QString longNamePossessive );
    void testGregorianCompareQDate( const KCalendarSystem *calendar, int year, int month, int day );
    void testJalaliCompare();
    void testCompare( const KCalendarSystem *calendar, int year, int month, int day );
    void testQDateYMD( const KCalendarSystem *calendar, int y, int m, int d );

private Q_SLOTS:
    void testTypes();
    void testLocale();
    void testGregorian();
    void testHijri();
    void testGregorianBasic();
    void testGregorianYmd();
    void testHijriBasic();
    void testHijriYmd();
    void testJalaliBasic();
    void testJalaliYmd();
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
