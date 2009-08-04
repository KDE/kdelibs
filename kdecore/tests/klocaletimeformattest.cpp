/*
 * This file is part of the KDE libraries.
 * Copyright 2009  Michael Leupold <lemma@confuego.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "klocaletimeformattest.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <QtCore/QStringList>
#include <QtTest/QTest>

static QString loadTimeFormat(const QString &country)
{
    KConfig entryFile(KStandardDirs::locate("locale",
                                            QString::fromLatin1("l10n/%1/entry.desktop")
                                            .arg(country)));
    KConfigGroup entry(&entryFile, "KCM Locale");
    return entry.readEntry("TimeFormat", "");
}

void KLocaleTimeFormatTest::initTestCase()
{
    KAboutData about("KLocaleTimeFormatTest", 0, ki18n("klocaletimeformattest"), "");
    KComponentData comp(&about);

    // get countries with according time formats
    // for formatAndReadAllCountriesTest().
    QStringList countries(KGlobal::locale()->allCountriesList());
    QString timeFormat;
    Q_FOREACH(const QString &c, countries) {
        timeFormat = loadTimeFormat(c);
        if (!timeFormat.isEmpty()) {
            m_allFormats.insert(c, timeFormat);
        }
    }

    // additional test-data (for (format|read)TimeTest)
    m_timeFormats["ampm"].timeFormat = "%I:%M:%S %p";
    m_timeFormats["ampm"].exp[KLocale::TimeDefault] = "03:59:59 pm";
    m_timeFormats["ampm"].exp[KLocale::TimeWithoutAmPm] = "03:59:59";
    m_timeFormats["ampm"].exp[KLocale::TimeDuration] = "15:59:59";
    m_timeFormats["ampm"].exp[KLocale::TimeWithoutSeconds] = "03:59 pm";
    m_timeFormats["ampm"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm] = "03:59";
    m_timeFormats["ampm"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeDuration] = "15:59";

    m_timeFormats["24h"].timeFormat = "%H:%M:%S";
    m_timeFormats["24h"].exp[KLocale::TimeDefault] = "15:59:59";
    m_timeFormats["24h"].exp[KLocale::TimeWithoutAmPm] = "15:59:59";
    m_timeFormats["24h"].exp[KLocale::TimeDuration] = "15:59:59";
    m_timeFormats["24h"].exp[KLocale::TimeWithoutSeconds] = "15:59";
    m_timeFormats["24h"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm] = "15:59";
    m_timeFormats["24h"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeDuration] = "15:59";

    m_timeFormats["ampm2"].timeFormat = "%l:%M:%S %p";
    m_timeFormats["ampm2"].exp[KLocale::TimeDefault] = "3:59:59 pm";
    m_timeFormats["ampm2"].exp[KLocale::TimeWithoutAmPm] = "3:59:59";
    m_timeFormats["ampm2"].exp[KLocale::TimeDuration] = "15:59:59";
    m_timeFormats["ampm2"].exp[KLocale::TimeWithoutSeconds] = "3:59 pm";
    m_timeFormats["ampm2"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm] = "3:59";
    m_timeFormats["ampm2"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeDuration] = "15:59";

    m_timeFormats["ag"].timeFormat = "%I.%M.%S.%p";
    m_timeFormats["ag"].exp[KLocale::TimeDefault] = "03.59.59.pm";
    m_timeFormats["ag"].exp[KLocale::TimeWithoutAmPm] = "03.59.59";
    m_timeFormats["ag"].exp[KLocale::TimeDuration] = "15.59.59";
    m_timeFormats["ag"].exp[KLocale::TimeWithoutSeconds] = "03.59.pm";
    m_timeFormats["ag"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm] = "03.59";
    m_timeFormats["ag"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeDuration] = "15.59";

    m_timeFormats["ampre"].timeFormat = "%p %l-%M-%S";
    m_timeFormats["ampre"].exp[KLocale::TimeDefault] = "pm 3-59-59";
    m_timeFormats["ampre"].exp[KLocale::TimeWithoutAmPm] = "3-59-59";
    m_timeFormats["ampre"].exp[KLocale::TimeDuration] = "15-59-59";
    m_timeFormats["ampre"].exp[KLocale::TimeWithoutSeconds] = "pm 3-59";
    m_timeFormats["ampre"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm] = "3-59";
    m_timeFormats["ampre"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeDuration] = "15-59";

    m_timeFormats["ampresep"].timeFormat = "%p-%l-%M-%S";
    m_timeFormats["ampresep"].exp[KLocale::TimeDefault] = "pm-3-59-59";
    m_timeFormats["ampresep"].exp[KLocale::TimeWithoutAmPm] = "3-59-59";
    m_timeFormats["ampresep"].exp[KLocale::TimeDuration] = "15-59-59";
    m_timeFormats["ampresep"].exp[KLocale::TimeWithoutSeconds] = "pm-3-59";
    m_timeFormats["ampresep"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm] = "3-59";
    m_timeFormats["ampresep"].exp[KLocale::TimeWithoutSeconds | KLocale::TimeDuration] = "15-59";
}

void KLocaleTimeFormatTest::formatTimeTest()
{
    KLocale locale(*KGlobal::locale());
    QTime time(15, 59, 59);

    QMap<QString, TimeFormatTestValue>::const_iterator it;
    QMap<QString, TimeFormatTestValue>::const_iterator end(m_timeFormats.constEnd());
    for (it = m_timeFormats.constBegin(); it != end; ++it) {
        locale.setTimeFormat(it.value().timeFormat);

        QMap<KLocale::TimeFormatOptions, QString>::const_iterator it2;
        QMap<KLocale::TimeFormatOptions, QString>::const_iterator end2(it.value().exp.constEnd());
        for (it2 = it.value().exp.constBegin(); it2 != end2; ++it2) {
            QCOMPARE(locale.formatLocaleTime(time, it2.key()), it2.value());
        }
    }
}

void KLocaleTimeFormatTest::readTimeTest()
{
    KLocale locale(*KGlobal::locale());
    QMap<KLocale::TimeFormatOptions, QTime> timeMap;
    timeMap[KLocale::TimeDefault] = QTime(15, 59, 59);
    timeMap[KLocale::TimeWithoutSeconds] = QTime(15, 59, 0);
    timeMap[KLocale::TimeDuration] = QTime(15, 59, 59);
    timeMap[KLocale::TimeDuration | KLocale::TimeWithoutSeconds] = QTime(15, 59, 0);
    timeMap[KLocale::TimeWithoutAmPm] = QTime(3, 59, 59);
    timeMap[KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm] = QTime(3, 59, 0);

    QMap<QString, TimeFormatTestValue>::const_iterator it;
    QMap<QString, TimeFormatTestValue>::const_iterator end(m_timeFormats.constEnd());
    for (it = m_timeFormats.constBegin(); it != end; ++it) {
        locale.setTimeFormat(it.value().timeFormat);

        bool ampm = locale.timeFormat().contains(QLatin1String("%p"));

        QMap<KLocale::TimeFormatOptions, QString>::const_iterator it2;
        QMap<KLocale::TimeFormatOptions, QString>::const_iterator end2(it.value().exp.constEnd());
        for (it2 = it.value().exp.constBegin(); it2 != end2; ++it2) {
            KLocale::TimeFormatOptions options = it2.key();
            if ((options & KLocale::TimeDuration) != KLocale::TimeDuration && !ampm) {
                options &= ~KLocale::TimeWithoutAmPm;
            }
            if (!timeMap.contains(options)) {
                QWARN("Missing entry in timeMap, skipping");
                continue;
            }
            bool ok;
            QCOMPARE(locale.readLocaleTime(it2.value(), &ok, it2.key()), timeMap[options]);
            QVERIFY(ok);
        }
    }
}

void KLocaleTimeFormatTest::formatAndReadAllCountriesTest()
{
    KLocale locale(*KGlobal::locale());

    // for each of the countries, format with any options, read again
    // and check if times match.
    QTime timeS(15, 59, 59);
    QTime timeNs(15, 59, 0);
    QTime timeAm(3, 59, 59);
    QTime timeNsAm(3, 59, 0);

    QMap<QString, QString>::const_iterator it(m_allFormats.constBegin());
    QMap<QString, QString>::const_iterator end(m_allFormats.constEnd());
    for ( ; it != end; ++it) {
        locale.setTimeFormat(it.value());

        bool ampm = locale.timeFormat().contains(QLatin1String("%p"));

        bool ok;
        QCOMPARE(locale.readLocaleTime(locale.formatLocaleTime(timeS, KLocale::TimeDefault), &ok, KLocale::TimeDefault), timeS);
        QVERIFY(ok);
        QCOMPARE(locale.readLocaleTime(locale.formatLocaleTime(timeS, KLocale::TimeWithoutAmPm), &ok, KLocale::TimeWithoutAmPm), ampm ? timeAm : timeS);
        QVERIFY(ok);
        QCOMPARE(locale.readLocaleTime(locale.formatLocaleTime(timeS, KLocale::TimeDuration), &ok, KLocale::TimeDuration), timeS);
        QVERIFY(ok);

        QCOMPARE(locale.readLocaleTime(locale.formatLocaleTime(timeS, KLocale::TimeWithoutSeconds), &ok, KLocale::TimeWithoutSeconds), timeNs);
        QVERIFY(ok);
        QCOMPARE(locale.readLocaleTime(locale.formatLocaleTime(timeS, KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm), &ok, KLocale::TimeWithoutSeconds | KLocale::TimeWithoutAmPm), ampm ? timeNsAm : timeNs);
        QVERIFY(ok);
        QCOMPARE(locale.readLocaleTime(locale.formatLocaleTime(timeS, KLocale::TimeWithoutSeconds | KLocale::TimeDuration), &ok, KLocale::TimeWithoutSeconds | KLocale::TimeDuration), timeNs);
        QVERIFY(ok);
    }
}

QTEST_MAIN(KLocaleTimeFormatTest)
#include "klocaletimeformattest.moc"
