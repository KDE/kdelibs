/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "datetimetest.h"
#include <kmetadata/datetime.h>

#include <kdatetime.h>
#include <qtest_kde.h>

using namespace Nepomuk::KMetaData;

void DateTimeTest::testDateTime()
{
    QDate date1( QDate::currentDate() );
    QDate date2( -10, 3, 23 );

    QTime time1( QTime::currentTime() );
    QTime time2( 13, 2, 10, 99 );
    QTime time3( 13, 2, 10 );

    QDateTime dateTime1( QDateTime::currentDateTime() );
    QDateTime dateTime2( date2, time2, Qt::UTC );
    QDateTime dateTime3( date2, time2, Qt::LocalTime );

    // check if the strings are actually correct
    QCOMPARE( DateTime::toString( date2 ), QString("-0010-03-23") );
    QCOMPARE( DateTime::toString( time3 ), QString("13:02:10Z") );
    QCOMPARE( DateTime::toString( time2 ), QString("13:02:10.099Z") );
    QCOMPARE( DateTime::toString( dateTime2 ), QString("-0010-03-23T13:02:10.099Z") );

//     QCOMPARE( KDateTime( date2 ).toString( KDateTime::ISODate ), QString("-0010-03-23") );
//     QCOMPARE( KDateTime( time3, KDateTime::UTC ).toString( KDateTime::ISODate ), QString("13:02:10Z") );
//     QCOMPARE( KDateTime( time2, KDateTime::UTC ).toUtc().toString( KDateTime::ISODate ), QString("13:02:10.099Z") );
//     QCOMPARE( KDateTime( dateTime2 ).toUtc().toString( KDateTime::ISODate ), QString("-0010-03-23T13:02:10.099Z") );

    // check timezone conversion
//     QCOMPARE( DateTime::fromTimeString( "13:02:10.099Z" ),
//               KDateTime::fromString( "13:02:10.099Z", KDateTime::ISODate ).toUtc().time() );
//     QCOMPARE( DateTime::fromTimeString( "15:02:10.099+02:00" ),
//               KDateTime::fromString( "15:02:10.099+02:00", KDateTime::ISODate ).toUtc().time() );
//     QCOMPARE( DateTime::fromTimeString( "11:02:10.099-02:00" ),
//               KDateTime::fromString( "11:02:10.099-02:00", KDateTime::ISODate ).toUtc().time() );

    // check conversion in general
    QVERIFY( !DateTime::toString( date1 ).isEmpty() );
    QVERIFY( !DateTime::toString( date2 ).isEmpty() );
    QVERIFY( !DateTime::toString( time1 ).isEmpty() );
    QVERIFY( !DateTime::toString( time2 ).isEmpty() );
    QVERIFY( !DateTime::toString( time3 ).isEmpty() );
    QVERIFY( !DateTime::toString( dateTime2 ).isEmpty() );
    QCOMPARE( DateTime::fromDateString( DateTime::toString( date1 ) ), date1 );
    QCOMPARE( DateTime::fromDateString( DateTime::toString( date2 ) ), date2 );
    QCOMPARE( DateTime::fromTimeString( DateTime::toString( time1 ) ), time1 );
    QCOMPARE( DateTime::fromTimeString( DateTime::toString( time2 ) ), time2 );
    QCOMPARE( DateTime::fromTimeString( DateTime::toString( time3 ) ), time3 );

    // for some weird reason QDateTime does not properly convert to UTC when comparing objects
    QCOMPARE( DateTime::fromDateTimeString( DateTime::toString( dateTime1 ) ), dateTime1.toUTC() );
    QCOMPARE( DateTime::fromDateTimeString( DateTime::toString( dateTime2 ) ), dateTime2.toUTC() );
    QCOMPARE( DateTime::fromDateTimeString( DateTime::toString( dateTime3 ) ), dateTime3.toUTC() );

    // check if KDateTime provides the same
//     QCOMPARE( DateTime::toString( dateTime1 ), KDateTime( dateTime1 ).toUtc().toString( KDateTime::ISODate ) );
//     QCOMPARE( DateTime::toString( dateTime2 ), KDateTime( dateTime2 ).toUtc().toString( KDateTime::ISODate ) );
//     QCOMPARE( DateTime::toString( dateTime3 ), KDateTime( dateTime3 ).toUtc().toString( KDateTime::ISODate ) );

//     QCOMPARE( DateTime::fromDateTimeString( KDateTime( dateTime1 ).toString( KDateTime::ISODate ) ), dateTime1.toUTC() );
//     QCOMPARE( DateTime::fromDateTimeString( KDateTime( dateTime2 ).toString( KDateTime::ISODate ) ), dateTime2.toUTC() );
//     QCOMPARE( DateTime::fromDateTimeString( KDateTime( dateTime3 ).toString( KDateTime::ISODate ) ), dateTime3.toUTC() );
}

QTEST_KDEMAIN(DateTimeTest,  NoGUI)

#include "datetimetest.moc"
