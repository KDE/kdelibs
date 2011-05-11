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

#ifndef KDATETIMEEDIT_P_H
#define KDATETIMEEDIT_P_H

#include <QtCore/QString>

#include "kdatetime.h"
#include "ksystemtimezone.h"
#include "kcalendarsystem.h"

#include "ui_kdatetimeedit.h"

class QWidgetAction;
class KLocale;
class KCalendarSystem;
class KDatePicker;
class KLocale;

class KDateTimeEditPrivate : public QObject
{
    Q_OBJECT

public:
    KDateTimeEditPrivate(KDateTimeEdit *q);
    virtual ~KDateTimeEditPrivate();

    const KCalendarSystem *calendar() const;
    KLocale *locale();

    QDateTime defaultMinDateTime();
    QDate defaultMinDate();
    QTime defaultMinTime();
    QDateTime defaultMaxDateTime();
    QDate defaultMaxDate();
    QTime defaultMaxTime();

    void initCalendarWidget();

    void initDateWidget();
    void updateDateWidget();

    void initTimeWidget();
    void updateTimeWidget();

    void initTimezoneWidget();
    void showDatePopup();

public Q_SLOTS:

    void selectCalendar(int index);
    void enterCalendar(KLocale::CalendarSystem calendarSystem);

    void selectDate(int index);
    void editDate(const QString &text);
    void enterDate(const QDate &date);
    void parseDate();

    void selectTime(int index);
    void editTime(const QString &text);
    void enterTime(const QTime &time);
    void parseTime();

    void selectTimezone(int index);
    void enterTimezone(const QString &zone);

public:

    KDateTimeEdit *const q;
    KCalendarSystem *m_calendar;
    KDatePicker *m_datePicker;
    QWidgetAction *m_datePickerAction;

    KDateTimeEdit::Options m_options;
    KDateTime m_dateTime;
    KLocale::CalendarSystem m_calendarSystem;
    QDateTime m_minDateTime;
    QDateTime m_maxDateTime;
    QString m_minErrorMsg;
    QString m_maxErrorMsg;

    KTimeZones::ZoneMap m_zones;
    int m_timeInterval;

    Ui::KDateTimeEdit ui;
};

#endif // KDATETIMEEDIT_P_H
