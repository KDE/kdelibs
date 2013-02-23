/*
    Copyright 2011 John Layt <john@layt.net>

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

#include "kdatetimeedittest.h"

#include <QtCore/QDate>

#include <QtTestWidgets>
#include <QtTest/QtTest>
#include "kdebug.h"
#include "kdatetimeedit.h"
#include "kcalendarsystem.h"
#include "ksystemtimezone.h"

QTEST_MAIN(KDateTimeEditTest)


void KDateTimeEditTest::testDefaults()
{
    m_edit = new KDateTimeEdit(0);
    QCOMPARE(m_edit->dateTime(), KDateTime(QDate::currentDate(), QTime(0, 0, 0)));
    QCOMPARE(m_edit->date(), QDate::currentDate());
    QCOMPARE(m_edit->time(), QTime(0, 0, 0));
    QCOMPARE(m_edit->minimumDateTime(), KDateTime(KLocale::global()->calendar()->earliestValidDate(), QTime(0, 0, 0)));
    QCOMPARE(m_edit->maximumDateTime(), KDateTime(KLocale::global()->calendar()->latestValidDate(), QTime(23, 59, 59, 999)));
    QCOMPARE(m_edit->isValid(), true);
    QCOMPARE(m_edit->isNull(), false);
    QCOMPARE(m_edit->options(), KDateTimeEdit::ShowDate | KDateTimeEdit::EditDate | KDateTimeEdit::SelectDate | KDateTimeEdit::DatePicker | KDateTimeEdit::DateKeywords | KDateTimeEdit::ShowTime | KDateTimeEdit::EditTime | KDateTimeEdit::SelectTime);
    QCOMPARE(m_edit->dateDisplayFormat(), KLocale::ShortDate);
    QCOMPARE(m_edit->timeListInterval(), 15);
    QCOMPARE(m_edit->timeDisplayFormat(), KLocale::TimeDefault);
    delete m_edit;
}

void KDateTimeEditTest::testValidNull()
{
    m_edit = new KDateTimeEdit(0);
    QCOMPARE(m_edit->isValid(), true);
    QCOMPARE(m_edit->isNull(), false);
    m_edit->setDateTime(KDateTime());
    QCOMPARE(m_edit->isValid(), false);
    QCOMPARE(m_edit->isNull(), true);
    delete m_edit;
}

void KDateTimeEditTest::testDateTimeRange()
{
    m_edit = new KDateTimeEdit(0);
    m_edit->setDateTime(KDateTime(QDate(2000, 1, 1), QTime(12, 0, 0)));
    QCOMPARE(m_edit->minimumDateTime(), KDateTime(KLocale::global()->calendar()->earliestValidDate(), QTime(0, 0, 0)));
    QCOMPARE(m_edit->maximumDateTime(), KDateTime(KLocale::global()->calendar()->latestValidDate(), QTime(23, 59, 59, 999)));
    QCOMPARE(m_edit->isValid(), true);

    m_edit->setDateTimeRange(KDateTime(QDate(2001, 1, 1), QTime(10, 0, 0)),
                             KDateTime(QDate(2002, 1, 1), QTime(20, 0, 0)));
    QCOMPARE(m_edit->minimumDateTime(), KDateTime(QDate(2001, 1, 1), QTime(10, 0, 0)));
    QCOMPARE(m_edit->maximumDateTime(), KDateTime(QDate(2002, 1, 1), QTime(20, 0, 0)));
    QCOMPARE(m_edit->isValid(), false);

    m_edit->setDateTime(KDateTime(QDate(2001, 1, 1), QTime(9, 59, 59, 999)));
    QCOMPARE(m_edit->isValid(), false);
    m_edit->setDateTime(KDateTime(QDate(2001, 1, 1), QTime(10, 0, 0)));
    QCOMPARE(m_edit->isValid(), true);
    m_edit->setDateTime(KDateTime(QDate(2002, 1, 1), QTime(20, 0, 0, 1)));
    QCOMPARE(m_edit->isValid(), false);
    m_edit->setDateTime(KDateTime(QDate(2002, 1, 1), QTime(20, 0, 0, 0)));
    QCOMPARE(m_edit->isValid(), true);

    m_edit->setDateTimeRange(KDateTime(QDate(1995, 1, 1), QTime(10, 0, 0)),
                             KDateTime(QDate(1990, 1, 1), QTime(20, 0, 0)));
    QCOMPARE(m_edit->minimumDateTime(), KDateTime(QDate(2001, 1, 1), QTime(10, 0, 0)));
    QCOMPARE(m_edit->maximumDateTime(), KDateTime(QDate(2002, 1, 1), QTime(20, 0, 0)));

    m_edit->setMinimumDateTime(KDateTime(QDate(2000, 1, 1), QTime(0, 0, 0)));
    QCOMPARE(m_edit->minimumDateTime(), KDateTime(QDate(2000, 1, 1), QTime(0, 0, 0)));
    QCOMPARE(m_edit->maximumDateTime(), KDateTime(QDate(2002, 1, 1), QTime(20, 0, 0)));

    m_edit->setMaximumDateTime(KDateTime(QDate(2003, 1, 1), QTime(0, 0, 0)));
    QCOMPARE(m_edit->minimumDateTime(), KDateTime(QDate(2000, 1, 1), QTime(0, 0, 0)));
    QCOMPARE(m_edit->maximumDateTime(), KDateTime(QDate(2003, 1, 1), QTime(0, 0, 0)));

    delete m_edit;
}

void KDateTimeEditTest::testDateList()
{
    m_edit = new KDateTimeEdit(0);
    QMap<QDate, QString> map;

    // KDateTimeEditTest default map
    QCOMPARE(m_edit->dateMap(), map);

    // KDateTimeEditTest basic map
    map.clear();
    map.insert(QDate(2000, 1, 1), QLatin1String("New Years Day"));
    map.insert(QDate(2000, 1, 2), QString());
    map.insert(QDate(2000, 1, 3), QLatin1String("separator"));
    map.insert(QDate(), QLatin1String("No Date"));
    m_edit->setDateMap(map);
    QCOMPARE(m_edit->dateMap(), map);

    delete m_edit;
}

void KDateTimeEditTest::testOptions()
{
    m_edit = new KDateTimeEdit(0);
    KDateTimeEdit::Options options = KDateTimeEdit::ShowDate | KDateTimeEdit::EditDate | KDateTimeEdit::SelectDate | KDateTimeEdit::DatePicker | KDateTimeEdit::DateKeywords | KDateTimeEdit::ShowTime | KDateTimeEdit::EditTime | KDateTimeEdit::SelectTime;
    QCOMPARE(m_edit->options(), options);
    options = KDateTimeEdit::EditDate | KDateTimeEdit::WarnOnInvalid;
    m_edit->setOptions(options);
    QCOMPARE(m_edit->options(), options);
    delete m_edit;
}

void KDateTimeEditTest::testDateDisplayFormat()
{
    m_edit = new KDateTimeEdit(0);
    KLocale::DateFormat format = KLocale::ShortDate;
    QCOMPARE(m_edit->dateDisplayFormat(), format);
    format = KLocale::IsoDate;
    m_edit->setDateDisplayFormat(format);
    QCOMPARE(m_edit->dateDisplayFormat(), format);
    delete m_edit;
}

void KDateTimeEditTest::testTimeListInterval()
{
    m_edit = new KDateTimeEdit();
    QCOMPARE(m_edit->timeListInterval(), 15);
    m_edit-> setTimeListInterval(60);
    QCOMPARE(m_edit->timeListInterval(), 60);
    delete m_edit;
}

void KDateTimeEditTest::testTimeList()
{
    m_edit = new KDateTimeEdit();
    QList<QTime> list;

    // KDateTimeEditTest default list
    QTime thisTime = QTime(0, 0, 0);
    for (int i = 0; i < 1440; i = i + 15) {
        list << thisTime.addSecs(i * 60);
    }
    list << QTime(23, 59, 59, 999);
    QCOMPARE(m_edit->timeList(), list);

    // KDateTimeEditTest basic list
    list.clear();
    list << QTime(3, 0, 0) << QTime(15, 16, 17);
    m_edit->setTimeList(list);
    QCOMPARE(m_edit->timeList(), list);

    delete m_edit;
}

void KDateTimeEditTest::testTimeDisplayFormat()
{
    m_edit = new KDateTimeEdit();
    KLocale::TimeFormatOptions format = KLocale::TimeDefault;
    QCOMPARE(m_edit->timeDisplayFormat(), format);
    format = KLocale::TimeWithoutSeconds;
    m_edit->setTimeDisplayFormat(format);
    QCOMPARE(m_edit->timeDisplayFormat(), format);
    delete m_edit;
}

void KDateTimeEditTest::testCalendarSystem()
{
    m_edit = new KDateTimeEdit();
    QList<KLocale::CalendarSystem> calendars = KCalendarSystem::calendarSystemsList();

    QCOMPARE(m_edit->calendarSystem(), KLocale::global()->calendarSystem());
    QCOMPARE(m_edit->calendarSystemsList(), calendars);

    m_edit->setCalendarSystem(KLocale::JulianCalendar);
    QCOMPARE(m_edit->calendarSystem(), KLocale::JulianCalendar);

    calendars.clear();
    calendars.append(KLocale::QDateCalendar);
    calendars.append(KLocale::GregorianCalendar);
    calendars.append(KLocale::JulianCalendar);
    m_edit->setCalendarSystemsList(calendars);
    QCOMPARE(m_edit->calendarSystemsList(), calendars);

    m_edit->setCalendarSystem(KLocale::IslamicCivilCalendar);
    QCOMPARE(m_edit->calendarSystem(), KLocale::JulianCalendar);

    delete m_edit;
}

void KDateTimeEditTest::testTimeSpec()
{
    m_edit = new KDateTimeEdit();

    QCOMPARE(m_edit->timeSpec(), KDateTime::currentLocalDateTime().timeSpec());
    QCOMPARE(m_edit->timeZones(), KSystemTimeZones::zones());

    KDateTime::Spec spec = KDateTime::Spec::OffsetFromUTC(3600);
    m_edit->setTimeSpec(spec);
    QCOMPARE(m_edit->timeSpec(), spec);

    KTimeZones::ZoneMap map;
    map.insert("Africa/Cairo", KSystemTimeZones::zone("Africa/Cairo"));
    m_edit->setTimeZones(map);
    QCOMPARE(m_edit->timeZones(), map);

    delete m_edit;
}
