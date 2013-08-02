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
#include <QKeyEvent>
#include <QMenu>
#include <QLineEdit>
#include <QWidgetAction>

#include "kdatetime.h"
#include "ksystemtimezone.h"
#include "kcalendarsystem.h"
#include "kcombobox.h"
#include "kdatepicker.h"
#include "kdatecombobox.h"
#include "kmessagebox.h"

#include "ui_kdatetimeedit.h"

class KDateTimeEditPrivate
{
public:
    KDateTimeEditPrivate(KDateTimeEdit *q);
    virtual ~KDateTimeEditPrivate();

    const KCalendarSystem *calendar() const;

    KDateTime defaultMinDateTime();
    KDateTime defaultMaxDateTime();

    void initWidgets();
    void initDateWidget();
    void initTimeWidget();
    void initCalendarWidget();
    void updateCalendarWidget();
    void initTimeSpecWidget();
    void updateTimeSpecWidget();

    void warnDateTime();

//private Q_SLOTS:
    void selectCalendar(int index);
    void enterCalendar(KLocale::CalendarSystem calendarSystem);
    void selectTimeZone(int index);
    void enterTimeZone(const QString &zone);

    KDateTimeEdit *const q;

    KDateTimeEdit::Options m_options;
    KDateTime m_dateTime;
    KDateTime m_minDateTime;
    KDateTime m_maxDateTime;
    QString m_minWarnMsg;
    QString m_maxWarnMsg;

    QList<KLocale::CalendarSystem> m_calendarSystems;
    KTimeZones::ZoneMap m_zones;

    Ui::KDateTimeEdit ui;
};

KDateTimeEditPrivate::KDateTimeEditPrivate(KDateTimeEdit *q)
                     :q(q)
{
    m_options = KDateTimeEdit::ShowDate | KDateTimeEdit::EditDate | KDateTimeEdit::SelectDate |
                KDateTimeEdit::ShowTime | KDateTimeEdit::EditTime | KDateTimeEdit::SelectTime |
                KDateTimeEdit::DatePicker | KDateTimeEdit::DateKeywords;
    m_dateTime = KDateTime::currentLocalDateTime();
    m_dateTime.setTime(QTime(0, 0, 0));
    m_calendarSystems = KCalendarSystem::calendarSystemsList();
    m_zones = KSystemTimeZones::zones();
}

KDateTimeEditPrivate::~KDateTimeEditPrivate()
{
}

const KCalendarSystem *KDateTimeEditPrivate::calendar() const
{
    return ui.m_dateCombo->calendar();
}

KDateTime KDateTimeEditPrivate::defaultMinDateTime()
{
    return KDateTime(calendar()->earliestValidDate(), QTime(0, 0, 0, 0));
}

KDateTime KDateTimeEditPrivate::defaultMaxDateTime()
{
    return KDateTime(calendar()->latestValidDate(), QTime(23, 59, 59, 999));
}

void KDateTimeEditPrivate::initWidgets()
{
    initDateWidget();
    initTimeWidget();
    initCalendarWidget();
    initTimeSpecWidget();
}

void KDateTimeEditPrivate::initDateWidget()
{
    ui.m_dateCombo->blockSignals(true);
    ui.m_dateCombo->setVisible((m_options &KDateTimeEdit::ShowDate) == KDateTimeEdit::ShowDate);
    KDateComboBox::Options options;
    if ((m_options & KDateTimeEdit::EditDate) == KDateTimeEdit::EditDate) {
        options = options | KDateComboBox::EditDate;
    }
    if ((m_options & KDateTimeEdit::SelectDate) == KDateTimeEdit::SelectDate) {
        options = options | KDateComboBox::SelectDate;
    }
    if ((m_options & KDateTimeEdit::DatePicker) == KDateTimeEdit::DatePicker) {
        options = options | KDateComboBox::DatePicker;
    }
    if ((m_options & KDateTimeEdit::DateKeywords) == KDateTimeEdit::DateKeywords) {
        options = options | KDateComboBox::DateKeywords;
    }
    ui.m_dateCombo->setOptions(options);
    ui.m_dateCombo->blockSignals(false);
}

void KDateTimeEditPrivate::initTimeWidget()
{
    ui.m_timeCombo->blockSignals(true);
    ui.m_timeCombo->setVisible((m_options &KDateTimeEdit::ShowDate) == KDateTimeEdit::ShowDate);
    KTimeComboBox::Options options;
    if ((m_options &KDateTimeEdit::EditTime) == KDateTimeEdit::EditTime) {
        options = options | KTimeComboBox::EditTime;
    }
    if ((m_options &KDateTimeEdit::SelectTime) == KDateTimeEdit::SelectTime) {
        options = options | KTimeComboBox::SelectTime;
    }
    if ((m_options &KDateTimeEdit::ForceTime) == KDateTimeEdit::ForceTime) {
        options = options | KTimeComboBox::ForceTime;
    }
    ui.m_timeCombo->setOptions(options);
    ui.m_timeCombo->blockSignals(false);
}

void KDateTimeEditPrivate::initCalendarWidget()
{
    ui.m_calendarCombo->blockSignals(true);
    ui.m_calendarCombo->clear();
    Q_FOREACH (KLocale::CalendarSystem calendar, m_calendarSystems) {
        ui.m_calendarCombo->addItem(KCalendarSystem::calendarLabel(calendar), calendar);
    }
    ui.m_calendarCombo->setCurrentIndex(ui.m_calendarCombo->findData(ui.m_dateCombo->calendarSystem()));
    ui.m_calendarCombo->setVisible((m_options &KDateTimeEdit::ShowCalendar) == KDateTimeEdit::ShowCalendar);
    ui.m_calendarCombo->setEnabled((m_options &KDateTimeEdit::SelectCalendar) == KDateTimeEdit::SelectCalendar);
    ui.m_calendarCombo->setEditable(false);
    ui.m_calendarCombo->blockSignals(false);
}

void KDateTimeEditPrivate::updateCalendarWidget()
{
    ui.m_calendarCombo->blockSignals(true);
    ui.m_calendarCombo->setCurrentIndex(ui.m_calendarCombo->findData(ui.m_dateCombo->calendarSystem()));
    ui.m_calendarCombo->blockSignals(false);
}

void KDateTimeEditPrivate::selectCalendar(int index)
{
    enterCalendar((KLocale::CalendarSystem) ui.m_calendarCombo->itemData(index).toInt());
}

void KDateTimeEditPrivate::enterCalendar(KLocale::CalendarSystem calendarSystem)
{
    q->setCalendarSystem(calendarSystem);
    Q_EMIT q->calendarEntered(ui.m_dateCombo->calendarSystem());
}

void KDateTimeEditPrivate::initTimeSpecWidget()
{
    ui.m_timeSpecCombo->blockSignals(true);
    ui.m_timeSpecCombo->clear();
    ui.m_timeSpecCombo->addItem(i18nc("UTC time zone", "UTC"), "UTC");
    ui.m_timeSpecCombo->addItem(i18nc("No specific time zone", "Floating"), "Floating");
    QStringList keys = m_zones.keys();
    QMap<QString, QString> names;
    Q_FOREACH (const QString &key, keys) {
        names.insert(i18n(key.toUtf8()).replace('_', ' '), key);
    }
    QMapIterator<QString, QString> i(names);
    while (i.hasNext()) {
        i.next();
        ui.m_timeSpecCombo->addItem(i.key(), i.value());
    }
    ui.m_timeSpecCombo->setVisible((m_options &KDateTimeEdit::ShowTimeSpec) == KDateTimeEdit::ShowTimeSpec);
    ui.m_timeSpecCombo->setEnabled((m_options &KDateTimeEdit::SelectTimeSpec) == KDateTimeEdit::SelectTimeSpec);
    ui.m_timeSpecCombo->setEditable(false);
    ui.m_timeSpecCombo->blockSignals(false);
}

void KDateTimeEditPrivate::updateTimeSpecWidget()
{
    ui.m_timeSpecCombo->blockSignals(true);
    ui.m_timeSpecCombo->blockSignals(false);
}

void KDateTimeEditPrivate::selectTimeZone(int index)
{
    enterTimeZone(ui.m_timeCombo->itemData(index).toString());
}

void KDateTimeEditPrivate::enterTimeZone(const QString &zone)
{
    q->setTimeSpec(m_zones.value(zone));
    Q_EMIT q->dateTimeEntered(m_dateTime);
    Q_EMIT q->timeSpecEntered(m_dateTime.timeSpec());
}

void KDateTimeEditPrivate::warnDateTime()
{
    if (!q->isValid() &&
        (m_options &KDateTimeEdit::WarnOnInvalid) == KDateTimeEdit::WarnOnInvalid) {
        QString warnMsg;
        if (!m_dateTime.isValid()) {
            //TODO Add missing string
            //warnMsg = i18n("The date or time you entered is invalid");
        } else if (m_dateTime < m_minDateTime) {
            if (m_minWarnMsg.isEmpty()) {
                //TODO Add datetime to string
                //warnMsg = i18nc("@info", "Date and time cannot be earlier than %1", formatDate(m_minDate));
                warnMsg = i18nc("@info", "The entered date and time is before the minimum allowed date and time.");
            } else {
                warnMsg = m_minWarnMsg;
                //TODO localize properly
                warnMsg.replace("%1", KLocale::global()->formatDateTime(m_minDateTime));
            }
        } else if (m_dateTime > m_maxDateTime) {
            if (m_maxWarnMsg.isEmpty()) {
                //TODO Add datetime to string
                //warnMsg = i18nc("@info", "Date cannot be later than %1", formatDate(m_maxDate));
                warnMsg = i18nc("@info", "The entered date and time is after the maximum allowed date and time.");
            } else {
                warnMsg = m_maxWarnMsg;
                warnMsg.replace("%1", KLocale::global()->formatDateTime(m_maxDateTime));
            }
        }
        KMessageBox::sorry(q, warnMsg);
    }
}


KDateTimeEdit::KDateTimeEdit(QWidget *parent)
              :QWidget(parent),
               d(new KDateTimeEditPrivate(this))
{
    KLocalizedString::insertCatalog("timezones4");
    d->ui.setupUi(this);
    //Need to do the min/max defaults here and not in private init as need to wait for ui to init
    //the KDateComboBox which holds the calendar object.  Revisit this???
    d->m_minDateTime = d->defaultMinDateTime();
    d->m_maxDateTime = d->defaultMaxDateTime();
    d->ui.m_calendarCombo->installEventFilter(this);
    d->ui.m_dateCombo->installEventFilter(this);
    d->ui.m_timeCombo->installEventFilter(this);
    d->ui.m_timeSpecCombo->installEventFilter(this);
    d->initWidgets();

    connect( d->ui.m_calendarCombo, SIGNAL(activated(int)),
             this,                      SLOT(selectCalendar(int)));
    connect( d->ui.m_timeSpecCombo, SIGNAL(activated(int)),
             this,                      SLOT(selectTimeZone(int)));
}

KDateTimeEdit::~KDateTimeEdit()
{
    delete d;
}

KDateTime KDateTimeEdit::dateTime() const
{
    return d->m_dateTime;
}

KLocale::CalendarSystem KDateTimeEdit::calendarSystem() const
{
   return d-> ui.m_dateCombo->calendarSystem();
}

QDate KDateTimeEdit::date() const
{
    return d->m_dateTime.date();
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
           d->m_dateTime >= d->m_minDateTime &&
           d->m_dateTime <= d->m_maxDateTime;
}

bool KDateTimeEdit::isNull() const
{
    return isNullDate() && isNullTime();
}

bool KDateTimeEdit::isValidDate() const
{
    return d->ui.m_dateCombo->isValid();
}

bool KDateTimeEdit::isNullDate() const
{
    return d->ui.m_dateCombo->isNull();
}

bool KDateTimeEdit::isValidTime() const
{
    return d->ui.m_timeCombo->isValid();
}

bool KDateTimeEdit::isNullTime() const
{
    return d->ui.m_timeCombo->isNull();
}

void KDateTimeEdit::setOptions(Options options)
{
    if (options != d->m_options) {
        d->m_options = options;
        d->initWidgets();
    }
}

KDateTimeEdit::Options KDateTimeEdit::options() const
{
    return d->m_options;
}

void KDateTimeEdit::setDateTime(const KDateTime &dateTime)
{
    if (dateTime != d->m_dateTime) {
        assignDateTime(dateTime);
        Q_EMIT dateTimeChanged(d->m_dateTime);
        Q_EMIT dateChanged(d->m_dateTime.date());
        Q_EMIT timeChanged(d->m_dateTime.time());
    }
}

void KDateTimeEdit::assignDateTime(const KDateTime &dateTime)
{
    d->m_dateTime = dateTime;
    d->ui.m_dateCombo->setDate(dateTime.date());
    d->ui.m_timeCombo->setTime(dateTime.time());
}

void KDateTimeEdit::setDate(const QDate &date)
{
    if (date != d->m_dateTime.date()) {
        assignDate(date);
        Q_EMIT dateTimeChanged(d->m_dateTime);
        Q_EMIT dateChanged(d->m_dateTime.date());
    }
}

void KDateTimeEdit::assignDate(const QDate &date)
{
    d->m_dateTime.setDate(date);
    d->ui.m_dateCombo->setDate(date);
}

void KDateTimeEdit::setCalendarSystem(KLocale::CalendarSystem calendarSystem)
{
    if (calendarSystem == d->ui.m_dateCombo->calendarSystem() ||
        !d->m_calendarSystems.contains(calendarSystem)) {
        return;
    }

    assignCalendarSystem(calendarSystem);
    Q_EMIT calendarChanged(d->ui.m_dateCombo->calendarSystem());
}

void KDateTimeEdit::assignCalendarSystem(KLocale::CalendarSystem calendarSystem)
{
    d->ui.m_dateCombo->setCalendarSystem(calendarSystem);
    d->updateCalendarWidget();
}

void KDateTimeEdit::setCalendar(KCalendarSystem *calendar)
{
    d->ui.m_dateCombo->setCalendar(calendar);
    d->updateCalendarWidget();
}

void KDateTimeEdit::setTime(const QTime &time)
{
    if (time != d->m_dateTime.time()) {
        assignTime(time);
        Q_EMIT dateTimeChanged(d->m_dateTime);
        Q_EMIT timeChanged(d->m_dateTime.time());
    }
}

void KDateTimeEdit::assignTime(const QTime &time)
{
    d->m_dateTime.setTime(time);
    d->ui.m_timeCombo->setTime(time);
}

void KDateTimeEdit::setTimeSpec(const KDateTime::Spec &spec)
{
    if (spec == d->m_dateTime.timeSpec() || !spec.isValid()) {
        return;
    }

    assignTimeSpec(spec);
    Q_EMIT dateTimeChanged(d->m_dateTime);
    Q_EMIT timeSpecChanged(d->m_dateTime.timeSpec());
}

void KDateTimeEdit::assignTimeSpec(const KDateTime::Spec &spec)
{
    d->m_dateTime.setTimeSpec(spec);
    d->updateTimeSpecWidget();
}

void KDateTimeEdit::setMinimumDateTime(const KDateTime &minDateTime, const QString &minWarnMsg)
{
    setDateTimeRange(minDateTime, maximumDateTime(), minWarnMsg, d->m_maxWarnMsg);
}

KDateTime KDateTimeEdit::minimumDateTime() const
{
    return d->m_minDateTime;
}

void KDateTimeEdit::resetMinimumDateTime()
{
    d->m_minDateTime = d->defaultMinDateTime();
}

void KDateTimeEdit::setMaximumDateTime(const KDateTime &maxDateTime, const QString &maxWarnMsg)
{
    setDateTimeRange(minimumDateTime(), maxDateTime, d->m_minWarnMsg, maxWarnMsg);
}

KDateTime KDateTimeEdit::maximumDateTime() const
{
    return d->m_maxDateTime;
}

void KDateTimeEdit::resetMaximumDateTime()
{
    d->m_maxDateTime = d->defaultMaxDateTime();
}

void KDateTimeEdit::setDateTimeRange(const KDateTime &minDateTime,
                                     const KDateTime &maxDateTime,
                                     const QString &minErrorMsg,
                                     const QString &maxErrorMsg)
{
    if (minDateTime.isValid() &&
        maxDateTime.isValid() &&
        minDateTime <= maxDateTime &&
        d->calendar()->isValid(minDateTime.date()) &&
        d->calendar()->isValid(maxDateTime.date())) {

        d->m_minDateTime = minDateTime;
        d->m_minWarnMsg = minErrorMsg;
        d->m_maxDateTime = maxDateTime;
        d->m_maxWarnMsg = maxErrorMsg;

    }
}

void KDateTimeEdit::resetDateTimeRange()
{
    setDateTimeRange(d->defaultMinDateTime(), d->defaultMaxDateTime());
}

void KDateTimeEdit::setCalendarSystemsList(QList<KLocale::CalendarSystem> calendars)
{
    if (calendars != d->m_calendarSystems) {
        d->m_calendarSystems = calendars;
        d->updateCalendarWidget();
    }
}

QList<KLocale::CalendarSystem> KDateTimeEdit::calendarSystemsList() const
{
    return d->m_calendarSystems;
}

void KDateTimeEdit::setDateDisplayFormat(KLocale::DateFormat format)
{
    d->ui.m_dateCombo->setDisplayFormat(format);
}

KLocale::DateFormat KDateTimeEdit::dateDisplayFormat() const
{
    return d->ui.m_dateCombo->displayFormat();
}

void KDateTimeEdit::setDateMap(QMap<QDate, QString> dateMap)
{
    d->ui.m_dateCombo->setDateMap(dateMap);
}

QMap<QDate, QString> KDateTimeEdit::dateMap() const
{
    return d->ui.m_dateCombo->dateMap();
}

void KDateTimeEdit::setTimeDisplayFormat(KLocale::TimeFormatOptions formatOptions)
{
    d->ui.m_timeCombo->setDisplayFormat(formatOptions);
}

KLocale::TimeFormatOptions KDateTimeEdit::timeDisplayFormat() const
{
    return d->ui.m_timeCombo->displayFormat();
}

void KDateTimeEdit::setTimeListInterval(int minutes)
{
    d->ui.m_timeCombo->setTimeListInterval(minutes);
}

int KDateTimeEdit::timeListInterval() const
{
    return d->ui.m_timeCombo->timeListInterval();
}

void KDateTimeEdit::setTimeList(QList<QTime> timeList,
                 const QString &minWarnMsg,
                 const QString &maxWarnMsg)
{
    d->ui.m_timeCombo->setTimeList(timeList, minWarnMsg, maxWarnMsg);
}

QList<QTime> KDateTimeEdit::timeList() const
{
    return d->ui.m_timeCombo->timeList();
}

void KDateTimeEdit::setTimeZones(const KTimeZones::ZoneMap &zones)
{
    if (zones != d->m_zones) {
        d->m_zones = zones;
        d->updateTimeSpecWidget();
    }
}

KTimeZones::ZoneMap KDateTimeEdit::timeZones() const
{
    return d->m_zones;
}

bool KDateTimeEdit::eventFilter(QObject *object, QEvent *event)
{
    return QWidget::eventFilter(object, event);
}

void KDateTimeEdit::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
}

void KDateTimeEdit::focusOutEvent(QFocusEvent *event)
{
    d->warnDateTime();
    QWidget::focusOutEvent(event);
}

void KDateTimeEdit::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

#include "moc_kdatetimeedit.cpp"
