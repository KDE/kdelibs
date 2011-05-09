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

#include "kdatetimeedit.h"

#include <QtCore/QStringList>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QLineEdit>
#include <QtGui/QWidgetAction>

#include "kdebug.h"
#include "kdatetime.h"
#include "ksystemtimezone.h"
#include "kcalendarsystem.h"
#include "kcombobox.h"
#include "kdatepicker.h"
#include "kdatecombobox.h"

#include "ui_kdatetimeedit.h"

class KDateTimeEditPrivate
{
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
    void initTimeZoneWidget();

//private slots:
    void selectCalendar(int index);
    void enterCalendar(KLocale::CalendarSystem calendarSystem);
    void selectTimeZone(int index);
    void enterTimeZone(const QString &zone);

    KDateTimeEdit *const q;

    KDateTimeEdit::Options m_options;
    KDateTime m_dateTime;
    QDateTime m_minDateTime;
    QDateTime m_maxDateTime;
    QString m_minErrorMsg;
    QString m_maxErrorMsg;

    KTimeZones::ZoneMap m_zones;

    Ui::KDateTimeEdit ui;
};

KDateTimeEditPrivate::KDateTimeEditPrivate(KDateTimeEdit *q)
                     :q(q)
{
    m_options = KDateTimeEdit::ShowDate | KDateTimeEdit::EditDate | KDateTimeEdit::SelectDate |
                KDateTimeEdit::ShowTime | KDateTimeEdit::EditTime | KDateTimeEdit::SelectTime;
    m_dateTime = KDateTime::currentLocalDateTime();
    m_minErrorMsg = i18n("The entered date and time is before the minimum allowed date and time.");
    m_maxErrorMsg = i18n("The entered date and time is after the maximum allowed date and time.");
    m_zones = KSystemTimeZones::zones();
}

KDateTimeEditPrivate::~KDateTimeEditPrivate()
{
}

const KCalendarSystem *KDateTimeEditPrivate::calendar() const
{
    return ui.m_dateCombo->calendar();
}

KLocale *KDateTimeEditPrivate::locale()
{
    return KGlobal::locale();
}

QDateTime KDateTimeEditPrivate::defaultMinDateTime()
{
    return QDateTime(calendar()->earliestValidDate(), QTime(0, 0, 0, 0));
}

QDate KDateTimeEditPrivate::defaultMinDate()
{
    return calendar()->earliestValidDate();
}

QTime KDateTimeEditPrivate::defaultMinTime()
{
    return QTime(0, 0, 0, 0);
}

QDateTime KDateTimeEditPrivate::defaultMaxDateTime()
{
    return QDateTime(calendar()->latestValidDate(), QTime(23, 59, 59, 999));
}

QDate KDateTimeEditPrivate::defaultMaxDate()
{
    return calendar()->latestValidDate();
}

QTime KDateTimeEditPrivate::defaultMaxTime()
{
    return QTime(23, 59, 59, 999);
}

void KDateTimeEditPrivate::initCalendarWidget()
{
    ui.m_calendarCombo->clear();
    foreach (KLocale::CalendarSystem calendar, KCalendarSystem::calendarSystemsList()) {
        ui.m_calendarCombo->addItem(KCalendarSystem::calendarLabel(calendar), calendar);
    }
    ui.m_calendarCombo->setCurrentIndex(ui.m_calendarCombo->findData(ui.m_dateCombo->calendarSystem()));
}

void KDateTimeEditPrivate::selectCalendar(int index)
{
    enterCalendar((KLocale::CalendarSystem) ui.m_calendarCombo->itemData(index).toInt());
}

void KDateTimeEditPrivate::enterCalendar(KLocale::CalendarSystem calendarSystem)
{
    q->setCalendarSystem(calendarSystem);
    emit q->calendarEntered(ui.m_dateCombo->calendarSystem());
}

void KDateTimeEditPrivate::initTimeZoneWidget()
{
    ui.m_timeZoneCombo->clear();
    ui.m_timeZoneCombo->addItem(i18n("UTC"), "UTC");
    ui.m_timeZoneCombo->addItem(i18n("Floating"), "Floating");
    QStringList keys = m_zones.keys();
    QMap<QString, QString> names;
    foreach (const QString &key, keys) {
        names.insert(i18n(key.toUtf8()).replace('_', ' '), key);
    }
    QMapIterator<QString, QString> i(names);
    while (i.hasNext()) {
        i.next();
        ui.m_timeZoneCombo->addItem(i.key(), i.value());
    }
}

void KDateTimeEditPrivate::selectTimeZone(int index)
{
    enterTimeZone(ui.m_timeCombo->itemData(index).toString());
}

void KDateTimeEditPrivate::enterTimeZone(const QString &zone)
{
    q->setTimeSpec(m_zones.value(zone));
    emit q->dateTimeEntered(m_dateTime);
    emit q->timeSpecEntered(m_dateTime.timeSpec());
}

KDateTimeEdit::KDateTimeEdit(QWidget *parent)
              :QWidget(parent),
               d(new KDateTimeEditPrivate(this))
{
    KGlobal::locale()->insertCatalog("timezones4");
    d->ui.setupUi(this);
    //Need to do the min/max defaults here and not in private init as need to wait for ui to init
    //the KDateComboBox which holds the calendar object.  Revisit this???
    d->m_minDateTime = d->defaultMinDateTime();
    d->m_maxDateTime = d->defaultMaxDateTime();
    d->ui.m_calendarCombo->installEventFilter(this);
    d->ui.m_dateCombo->installEventFilter(this);
    d->ui.m_timeCombo->installEventFilter(this);
    d->ui.m_timeZoneCombo->installEventFilter(this);
    d->initCalendarWidget();
    d->initTimeZoneWidget();
    connect( d->ui.m_calendarCombo, SIGNAL(activated(int)),
             this,                      SLOT(selectCalendar(int)));
    connect( d->ui.m_timeZoneCombo, SIGNAL(activated(int)),
             this,                      SLOT(selectTimeZone(int)));
}

KDateTimeEdit::~KDateTimeEdit()
{
    delete d;
}

KDateTime KDateTimeEdit::fullDateTime() const
{
    return d->m_dateTime;
}

QDateTime KDateTimeEdit::dateTime() const
{
    return d->m_dateTime.dateTime();
}

QDate KDateTimeEdit::date() const
{
    return d->m_dateTime.date();
}

KLocale::CalendarSystem KDateTimeEdit::calendarSystem()
{
   return d-> ui.m_dateCombo->calendarSystem();
}

QTime KDateTimeEdit::time() const
{
    return d->m_dateTime.time();
}

KDateTime::Spec KDateTimeEdit::timeSpec() const
{
    return d->m_dateTime.timeSpec();
}

bool KDateTimeEdit::isValid() const
{
    return d->m_dateTime.isValid() &&
           d->m_dateTime.dateTime() >= d->m_minDateTime &&
           d->m_dateTime.dateTime() <= d->m_maxDateTime;
}

KDateTimeEdit::Options KDateTimeEdit::options() const
{
    return d->m_options;
}

void KDateTimeEdit::setDateTime(const KDateTime &dateTime)
{
    if (dateTime == d->m_dateTime) {
        return;
    }

    assignDateTime(dateTime);
    emit dateTimeChanged(d->m_dateTime);
    emit dateTimeChanged(d->m_dateTime.dateTime());
    emit dateChanged(d->m_dateTime.date());
    emit timeChanged(d->m_dateTime.time());
}

void KDateTimeEdit::setDateTime(const QDateTime &dateTime)
{
    if (dateTime == d->m_dateTime.dateTime()) {
        return;
    }

    assignDateTime(dateTime);
    emit dateTimeChanged(d->m_dateTime);
    emit dateTimeChanged(d->m_dateTime.dateTime());
    emit dateChanged(d->m_dateTime.date());
    emit timeChanged(d->m_dateTime.time());
}

void KDateTimeEdit::setDate(const QDate &date)
{
    if (date == d->m_dateTime.date()) {
        return;
    }

    assignDate(date);
    emit dateTimeChanged(d->m_dateTime);
    emit dateTimeChanged(d->m_dateTime.dateTime());
    emit dateChanged(d->m_dateTime.date());
}

void KDateTimeEdit::setCalendarSystem(KLocale::CalendarSystem calendarSystem)
{
    //TODO Do this properly
    d->ui.m_dateCombo->setCalendarSystem(calendarSystem);
    emit calendarChanged(d->ui.m_dateCombo->calendarSystem());
}

void KDateTimeEdit::setCalendar(KCalendarSystem *calendar)
{
    d->ui.m_dateCombo->setCalendar(calendar);
}

void KDateTimeEdit::setTime(const QTime &time)
{
    if (time == d->m_dateTime.time()) {
        return;
    }

    assignTime(time);
    emit dateTimeChanged(d->m_dateTime);
    emit dateTimeChanged(d->m_dateTime.dateTime());
    emit timeChanged(d->m_dateTime.time());
}

void KDateTimeEdit::setTimeSpec(const KDateTime::Spec &spec)
{
    if (spec.isValid()) {
        d->m_dateTime.setTimeSpec(spec);
    }
    emit dateTimeChanged(d->m_dateTime);
    emit timeSpecChanged(d->m_dateTime.timeSpec());
}

void KDateTimeEdit::assignDateTime(const KDateTime &dateTime)
{
    d->m_dateTime = dateTime;
    d->ui.m_dateCombo->setDate(dateTime.date());
    d->ui.m_timeCombo->setTime(dateTime.time());
}

void KDateTimeEdit::assignDateTime(const QDateTime &dateTime)
{
    d->m_dateTime.setDateTime(dateTime);
    d->ui.m_dateCombo->setDate(dateTime.date());
    d->ui.m_timeCombo->setTime(dateTime.time());
}

void KDateTimeEdit::assignDate(const QDate &date)
{
    d->m_dateTime.setDate(date);
    d->ui.m_dateCombo->setDate(date);
}

void KDateTimeEdit::assignTime(const QTime &time)
{
    d->m_dateTime.setTime(time);
    d->ui.m_timeCombo->setTime(time);
}

void KDateTimeEdit::setOptions(Options options)
{
    d->m_options = options;
}

QDateTime KDateTimeEdit::minimumDateTime() const
{
    return d->m_minDateTime;
}

void KDateTimeEdit::resetMinimumDateTime()
{
    d->m_minDateTime = d->defaultMinDateTime();
}

QDateTime KDateTimeEdit::maximumDateTime() const
{
    return d->m_maxDateTime;
}

void KDateTimeEdit::resetMaximumDateTime()
{
    d->m_maxDateTime = d->defaultMaxDateTime();
}

void KDateTimeEdit::setDateTimeRange(const QDateTime &minDateTime,
                                     const QDateTime &maxDateTime,
                                     const QString &minErrorMsg,
                                     const QString &maxErrorMsg)
{
    if (minDateTime.isValid() &&
        maxDateTime.isValid() &&
        minDateTime <= maxDateTime &&
        d->calendar()->isValid(minDateTime.date()) &&
        d->calendar()->isValid(maxDateTime.date())) {
        d->m_minDateTime = minDateTime;
        d->m_minErrorMsg = minErrorMsg;
        d->m_maxDateTime = maxDateTime;
        d->m_maxErrorMsg = maxErrorMsg;
    }
}

void KDateTimeEdit::resetDateTimeRange()
{
    setDateTimeRange(d->defaultMinDateTime(), d->defaultMaxDateTime());
}

void KDateTimeEdit::clearDateTimeRange()
{
    setDateTimeRange(QDateTime(), QDateTime());
}

QDate KDateTimeEdit::minimumDate() const
{
    return d->m_minDateTime.date();
}

void KDateTimeEdit::resetMinimumDate()
{
    d->m_minDateTime.setDate(d->defaultMinDate());
}

QDate KDateTimeEdit::maximumDate() const
{
    return d->m_maxDateTime.date();
}

void KDateTimeEdit::resetMaximumDate()
{
    d->m_maxDateTime.setDate(d->defaultMaxDate());
}

void KDateTimeEdit::setDateRange(const QDate &minDate,
                                 const QDate &maxDate,
                                 const QString &minErrorMsg,
                                 const QString &maxErrorMsg)
{
    setDateTimeRange(QDateTime(minDate, d->m_minDateTime.time()),
                     QDateTime(maxDate, d->m_maxDateTime.time()),
                     minErrorMsg, maxErrorMsg);
}

void KDateTimeEdit::resetDateRange()
{
    setDateRange(d->defaultMinDate(), d->defaultMaxDate());
}

void KDateTimeEdit::clearDateRange()
{
    setDateRange(QDate(), QDate());
}

QTime KDateTimeEdit::minimumTime() const
{
    return d->m_minDateTime.time();
}

void KDateTimeEdit::resetMinimumTime()
{
    d->m_minDateTime.setTime(d->defaultMinTime());
}

QTime KDateTimeEdit::maximumTime() const
{
    return d->m_maxDateTime.time();
}

void KDateTimeEdit::resetMaximumTime()
{
    d->m_maxDateTime.setTime(d->defaultMaxTime());
}

void KDateTimeEdit::setTimeRange(const QTime &minTime,
                                 const QTime &maxTime,
                                 const QString &minErrorMsg,
                                 const QString &maxErrorMsg)
{
    setDateTimeRange(QDateTime(d->m_minDateTime.date(), minTime),
                     QDateTime(d->m_maxDateTime.date(), maxTime),
                     minErrorMsg, maxErrorMsg);
}

void KDateTimeEdit::resetTimeRange()
{
    setTimeRange(d->defaultMinTime(), d->defaultMaxTime());
}

void KDateTimeEdit::clearTimeRange()
{
    setTimeRange(QTime(), QTime());
}

void KDateTimeEdit::setTimeInterval(int minutes)
{
    d->ui.m_timeCombo->setTimeInterval(minutes);
}

int KDateTimeEdit::timeInterval() const
{
    return d->ui.m_timeCombo->timeInterval();
}

void KDateTimeEdit::setTimeZones(const KTimeZones::ZoneMap &zones)
{
    d->m_zones = zones;
}

bool KDateTimeEdit::eventFilter(QObject *object, QEvent *event)
{
    return false;
}

#include "kdatetimeedit.moc"
