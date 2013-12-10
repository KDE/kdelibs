/*  This file is part of the KDE Frameworks

    Copyright (C) 2013 John Layt <jlayt@kde.org>,
    Copyright (C) 2010 Michael Leupold <lemma@confuego.org>
    Copyright (C) 2009 Michael Pyne <mpyne@kde.org>
    Copyright (C) 2008 Albert Astals Cid <aacid@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kformattest.h"

#include <QtTest/QtTest>

#include "kformat.h"

void KFormatTest::formatByteSize()
{
    KFormat format(QLocale::c());

    QCOMPARE(format.formatByteSize(0), QString("0 B"));
    QCOMPARE(format.formatByteSize(50), QString("50 B"));
    QCOMPARE(format.formatByteSize(500), QString("500 B"));
    QCOMPARE(format.formatByteSize(5000), QString("4.9 KiB"));
    QCOMPARE(format.formatByteSize(50000), QString("48.8 KiB"));
    QCOMPARE(format.formatByteSize(500000), QString("488.3 KiB"));
    QCOMPARE(format.formatByteSize(5000000), QString("4.8 MiB"));
    QCOMPARE(format.formatByteSize(50000000), QString("47.7 MiB"));
    QCOMPARE(format.formatByteSize(500000000), QString("476.8 MiB"));
#if (__WORDSIZE == 64) || defined (_LP64) || defined(__LP64__) || defined(__ILP64__)
    QCOMPARE(format.formatByteSize(5000000000), QString("4.7 GiB"));
    QCOMPARE(format.formatByteSize(50000000000), QString("46.6 GiB"));
    QCOMPARE(format.formatByteSize(500000000000), QString("465.7 GiB"));
    QCOMPARE(format.formatByteSize(5000000000000), QString("4.5 TiB"));
    QCOMPARE(format.formatByteSize(50000000000000), QString("45.5 TiB"));
    QCOMPARE(format.formatByteSize(500000000000000), QString("454.7 TiB"));
#endif

    QCOMPARE(format.formatByteSize(1024.0, 1, KFormat::IECBinaryDialect), QString("1.0 KiB"));
    QCOMPARE(format.formatByteSize(1023.0, 1, KFormat::IECBinaryDialect), QString("1,023 B"));
    QCOMPARE(format.formatByteSize(1163000.0, 1, KFormat::IECBinaryDialect), QString("1.1 MiB")); // 1.2 metric

    QCOMPARE(format.formatByteSize(-1024.0, 1, KFormat::IECBinaryDialect), QString("-1.0 KiB"));
    QCOMPARE(format.formatByteSize(-1023.0, 1, KFormat::IECBinaryDialect), QString("-1,023 B"));
    QCOMPARE(format.formatByteSize(-1163000.0, 1, KFormat::IECBinaryDialect), QString("-1.1 MiB")); // 1.2 metric

    QCOMPARE(format.formatByteSize(1024.0, 1, KFormat::JEDECBinaryDialect), QString("1.0 KB"));
    QCOMPARE(format.formatByteSize(1023.0, 1, KFormat::JEDECBinaryDialect), QString("1,023 B"));
    QCOMPARE(format.formatByteSize(1163000.0, 1, KFormat::JEDECBinaryDialect), QString("1.1 MB"));

    QCOMPARE(format.formatByteSize(-1024.0, 1, KFormat::JEDECBinaryDialect), QString("-1.0 KB"));
    QCOMPARE(format.formatByteSize(-1023.0, 1, KFormat::JEDECBinaryDialect), QString("-1,023 B"));
    QCOMPARE(format.formatByteSize(-1163000.0, 1, KFormat::JEDECBinaryDialect), QString("-1.1 MB"));

    QCOMPARE(format.formatByteSize(1024.0, 1, KFormat::MetricBinaryDialect), QString("1.0 kB"));
    QCOMPARE(format.formatByteSize(1023.0, 1, KFormat::MetricBinaryDialect), QString("1.0 kB"));
    QCOMPARE(format.formatByteSize(1163000.0, 1, KFormat::MetricBinaryDialect), QString("1.2 MB"));

    QCOMPARE(format.formatByteSize(-1024.0, 1, KFormat::MetricBinaryDialect), QString("-1.0 kB"));
    QCOMPARE(format.formatByteSize(-1023.0, 1, KFormat::MetricBinaryDialect), QString("-1.0 kB"));
    QCOMPARE(format.formatByteSize(-1163000.0, 1, KFormat::MetricBinaryDialect), QString("-1.2 MB"));

    // Ensure all units are represented
    QCOMPARE(format.formatByteSize(2.0e9, 1, KFormat::MetricBinaryDialect), QString("2.0 GB"));
    QCOMPARE(format.formatByteSize(3.2e12, 1, KFormat::MetricBinaryDialect), QString("3.2 TB"));
    QCOMPARE(format.formatByteSize(4.1e15, 1, KFormat::MetricBinaryDialect), QString("4.1 PB"));
    QCOMPARE(format.formatByteSize(6.7e18, 2, KFormat::MetricBinaryDialect), QString("6.70 EB"));
    QCOMPARE(format.formatByteSize(5.6e20, 2, KFormat::MetricBinaryDialect), QString("560.00 EB"));
    QCOMPARE(format.formatByteSize(2.3e22, 2, KFormat::MetricBinaryDialect), QString("23.00 ZB"));
    QCOMPARE(format.formatByteSize(1.0e27, 1, KFormat::MetricBinaryDialect), QString("1,000.0 YB"));

    // Spattering of specific units
    QCOMPARE(format.formatByteSize(823000, 3, KFormat::IECBinaryDialect, KFormat::UnitMegaByte), QString("0.785 MiB"));
    QCOMPARE(format.formatByteSize(1234034.0, 4, KFormat::JEDECBinaryDialect, KFormat::UnitByte), QString("1,234,034 B"));
}

enum TimeConstants {
    MSecsInDay = 86400000,
    MSecsInHour = 3600000,
    MSecsInMinute = 60000,
    MSecsInSecond = 1000
};

void KFormatTest::formatDuration()
{
    KFormat format(QLocale::c());

    quint64 singleSecond = 3 * MSecsInSecond + 700;
    quint64 doubleSecond = 33 * MSecsInSecond + 700;
    quint64 singleMinute = 8 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 doubleMinute = 38 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 singleHour = 5 * MSecsInHour + 8 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 doubleHour = 15 * MSecsInHour + 8 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 singleDay = 1 * MSecsInDay + 5 * MSecsInHour + 8 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 doubleDay = 10 * MSecsInDay + 5 * MSecsInHour + 8 * MSecsInMinute + 3 * MSecsInSecond + 700;

    // Default format
    QCOMPARE(format.formatDuration(singleSecond), QString("0:00:04"));
    QCOMPARE(format.formatDuration(doubleSecond), QString("0:00:34"));
    QCOMPARE(format.formatDuration(singleMinute), QString("0:08:04"));
    QCOMPARE(format.formatDuration(doubleMinute), QString("0:38:04"));
    QCOMPARE(format.formatDuration(singleHour), QString("5:08:04"));
    QCOMPARE(format.formatDuration(doubleHour), QString("15:08:04"));
    QCOMPARE(format.formatDuration(singleDay), QString("29:08:04"));
    QCOMPARE(format.formatDuration(doubleDay), QString("245:08:04"));

    // ShowMilliseconds format
    KFormat::DurationFormatOptions options = KFormat::ShowMilliseconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QString("0:00:03.700"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QString("0:00:33.700"));
    QCOMPARE(format.formatDuration(singleMinute, options), QString("0:08:03.700"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QString("0:38:03.700"));
    QCOMPARE(format.formatDuration(singleHour, options), QString("5:08:03.700"));
    QCOMPARE(format.formatDuration(doubleHour, options), QString("15:08:03.700"));
    QCOMPARE(format.formatDuration(singleDay, options), QString("29:08:03.700"));
    QCOMPARE(format.formatDuration(doubleDay, options), QString("245:08:03.700"));

    // HideSeconds format
    options = KFormat::HideSeconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QString("0:00"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QString("0:01"));
    QCOMPARE(format.formatDuration(singleMinute, options), QString("0:08"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QString("0:38"));
    QCOMPARE(format.formatDuration(singleHour, options), QString("5:08"));
    QCOMPARE(format.formatDuration(doubleHour, options), QString("15:08"));
    QCOMPARE(format.formatDuration(singleDay, options), QString("29:08"));
    QCOMPARE(format.formatDuration(doubleDay, options), QString("245:08"));

    // FoldHours format
    options = KFormat::FoldHours;
    QCOMPARE(format.formatDuration(singleSecond, options), QString("0:04"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QString("0:34"));
    QCOMPARE(format.formatDuration(singleMinute, options), QString("8:04"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QString("38:04"));
    QCOMPARE(format.formatDuration(singleHour, options), QString("308:04"));
    QCOMPARE(format.formatDuration(doubleHour, options), QString("908:04"));
    QCOMPARE(format.formatDuration(singleDay, options), QString("1748:04"));
    QCOMPARE(format.formatDuration(doubleDay, options), QString("14708:04"));

    // FoldHours ShowMilliseconds format
    options = KFormat::FoldHours;
    options = options | KFormat::ShowMilliseconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QString("0:03.700"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QString("0:33.700"));
    QCOMPARE(format.formatDuration(singleMinute, options), QString("8:03.700"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QString("38:03.700"));
    QCOMPARE(format.formatDuration(singleHour, options), QString("308:03.700"));
    QCOMPARE(format.formatDuration(doubleHour, options), QString("908:03.700"));
    QCOMPARE(format.formatDuration(singleDay, options), QString("1748:03.700"));
    QCOMPARE(format.formatDuration(doubleDay, options), QString("14708:03.700"));

    // InitialDuration format
    options = KFormat::InitialDuration;
    QCOMPARE(format.formatDuration(singleSecond, options), QString("0h00m04s"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QString("0h00m34s"));
    QCOMPARE(format.formatDuration(singleMinute, options), QString("0h08m04s"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QString("0h38m04s"));
    QCOMPARE(format.formatDuration(singleHour, options), QString("5h08m04s"));
    QCOMPARE(format.formatDuration(doubleHour, options), QString("15h08m04s"));
    QCOMPARE(format.formatDuration(singleDay, options), QString("29h08m04s"));
    QCOMPARE(format.formatDuration(doubleDay, options), QString("245h08m04s"));

    // InitialDuration and ShowMilliseconds format
    options = KFormat::InitialDuration;
    options = options | KFormat::ShowMilliseconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QString("0h00m03.700s"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QString("0h00m33.700s"));
    QCOMPARE(format.formatDuration(singleMinute, options), QString("0h08m03.700s"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QString("0h38m03.700s"));
    QCOMPARE(format.formatDuration(singleHour, options), QString("5h08m03.700s"));
    QCOMPARE(format.formatDuration(doubleHour, options), QString("15h08m03.700s"));
    QCOMPARE(format.formatDuration(singleDay, options), QString("29h08m03.700s"));
    QCOMPARE(format.formatDuration(doubleDay, options), QString("245h08m03.700s"));

    // InitialDuration and HideSeconds format
    options = KFormat::InitialDuration;
    options = options | KFormat::HideSeconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QString("0h00m"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QString("0h01m"));
    QCOMPARE(format.formatDuration(singleMinute, options), QString("0h08m"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QString("0h38m"));
    QCOMPARE(format.formatDuration(singleHour, options), QString("5h08m"));
    QCOMPARE(format.formatDuration(doubleHour, options), QString("15h08m"));
    QCOMPARE(format.formatDuration(singleDay, options), QString("29h08m"));
    QCOMPARE(format.formatDuration(doubleDay, options), QString("245h08m"));

    // InitialDuration and FoldHours format
    options = KFormat::InitialDuration;
    options = options | KFormat::FoldHours;
    QCOMPARE(format.formatDuration(singleSecond, options), QString("0m04s"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QString("0m34s"));
    QCOMPARE(format.formatDuration(singleMinute, options), QString("8m04s"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QString("38m04s"));
    QCOMPARE(format.formatDuration(singleHour, options), QString("308m04s"));
    QCOMPARE(format.formatDuration(doubleHour, options), QString("908m04s"));
    QCOMPARE(format.formatDuration(singleDay, options), QString("1748m04s"));
    QCOMPARE(format.formatDuration(doubleDay, options), QString("14708m04s"));

    // InitialDuration and FoldHours and ShowMilliseconds format
    options = KFormat::InitialDuration;
    options = options | KFormat::FoldHours | KFormat::ShowMilliseconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QString("0m03.700s"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QString("0m33.700s"));
    QCOMPARE(format.formatDuration(singleMinute, options), QString("8m03.700s"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QString("38m03.700s"));
    QCOMPARE(format.formatDuration(singleHour, options), QString("308m03.700s"));
    QCOMPARE(format.formatDuration(doubleHour, options), QString("908m03.700s"));
    QCOMPARE(format.formatDuration(singleDay, options), QString("1748m03.700s"));
    QCOMPARE(format.formatDuration(doubleDay, options), QString("14708m03.700s"));
}

void KFormatTest::formatDecimalDuration()
{
    KFormat format(QLocale::c());

    QCOMPARE(format.formatDecimalDuration(10), QString("10 millisecond(s)"));
    QCOMPARE(format.formatDecimalDuration(10, 3), QString("10 millisecond(s)"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInSecond + 10), QString("1.01 seconds"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInSecond + 1, 3), QString("1.001 seconds"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInMinute + 10 * MSecsInSecond), QString("1.17 minutes"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInMinute + 10 * MSecsInSecond, 3), QString("1.167 minutes"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInHour + 10 * MSecsInMinute), QString("1.17 hours"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInHour + 10 * MSecsInMinute, 3), QString("1.167 hours"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInDay + 10 * MSecsInHour), QString("1.42 days"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInDay + 10 * MSecsInHour, 3), QString("1.417 days"));
}

void KFormatTest::formatSpelloutDuration()
{
    KFormat format(QLocale::c());

    QCOMPARE(format.formatSpelloutDuration(1000), QString("1 second(s)"));
    QCOMPARE(format.formatSpelloutDuration(5000), QString("5 second(s)"));
    QCOMPARE(format.formatSpelloutDuration(60000), QString("1 minute(s)"));
    QCOMPARE(format.formatSpelloutDuration(300000), QString("5 minute(s)"));
    QCOMPARE(format.formatSpelloutDuration(3600000), QString("1 hour(s)"));
    QCOMPARE(format.formatSpelloutDuration(18000000), QString("5 hour(s)"));
    QCOMPARE(format.formatSpelloutDuration(75000), QString("1 minute(s) and 15 second(s)"));
    // Problematic case #1 (there is a reference to this case on kformat.cpp)
    QCOMPARE(format.formatSpelloutDuration(119999), QString("2 minute(s)"));
    // This case is strictly 2 hours, 15 minutes and 59 seconds. However, since the range is
    // pretty high between hours and seconds, formatSpelloutDuration always omits seconds when there
    // are hours in scene.
    QCOMPARE(format.formatSpelloutDuration(8159000), QString("2 hour(s) and 15 minute(s)"));
    // This case is strictly 1 hour and 10 seconds. For the same reason, formatSpelloutDuration
    // detects that 10 seconds is just garbage compared to 1 hour, and omits it on the result.
    QCOMPARE(format.formatSpelloutDuration(3610000), QString("1 hour(s)"));
}

void KFormatTest::formatRelativeDate()
{
    KFormat format(QLocale::c());

    QDate testDate = QDate::currentDate();
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat), QString("Today"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat), QString("Today"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat), QString("Today"));

    testDate = QDate::currentDate().addDays(1);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat), QString("Tomorrow"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat), QString("Tomorrow"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat), QString("Tomorrow"));

    testDate = QDate::currentDate().addDays(-1);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat), QString("Yesterday"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat), QString("Yesterday"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat), QString("Yesterday"));

    testDate = QDate::currentDate().addDays(-7);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat),
             QString("Last %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::LongFormat)));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat),
             QString("Last %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::ShortFormat)));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat),
             QString("Last %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::NarrowFormat)));

    testDate = QDate::currentDate().addDays(7);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat),
             QString("Next %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::LongFormat)));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat),
             QString("Next %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::ShortFormat)));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat),
             QString("Next %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::NarrowFormat)));

    testDate = QDate::currentDate().addDays(-8);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat),
             QLocale::c().toString(testDate, QLocale::LongFormat));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat),
             QLocale::c().toString(testDate, QLocale::ShortFormat));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat),
             QLocale::c().toString(testDate, QLocale::NarrowFormat));

    testDate = QDate::currentDate().addDays(8);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat),
             QLocale::c().toString(testDate, QLocale::LongFormat));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat),
             QLocale::c().toString(testDate, QLocale::ShortFormat));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat),
             QLocale::c().toString(testDate, QLocale::NarrowFormat));

    QDateTime testDateTime = QDateTime(QDate::currentDate(), QTime(3, 0, 0));
    QCOMPARE(format.formatRelativeDateTime(testDateTime, QLocale::ShortFormat), QString("Today, 03:00:00"));

    testDateTime = QDateTime(QDate::currentDate().addDays(8), QTime(3, 0, 0));
    QCOMPARE(format.formatRelativeDateTime(testDateTime, QLocale::LongFormat),
             QLocale::c().toString(testDateTime, QLocale::LongFormat));
}

QTEST_MAIN(KFormatTest)
