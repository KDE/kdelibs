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

#include "kdatepicker.h"
#include "kdatecombobox.h"
#include "kmessagebox.h"

#include "ui_kdatetimeedit.h"

class KDateTimeEditPrivate
{
public:
    KDateTimeEditPrivate(KDateTimeEdit *q);
    virtual ~KDateTimeEditPrivate();

    QDateTime defaultMinDateTime();
    QDateTime defaultMaxDateTime();

    void initWidgets();
    void initDateWidget();
    void initTimeWidget();
    void initCalendarWidget();
    void updateCalendarWidget();
    void initTimeZoneWidget();
    void updateTimeZoneWidget();

    void warnDateTime();

//private Q_SLOTS:
    void selectCalendar(int index);
    void enterCalendar(const QLocale &calendarLocale);
    void selectTimeZone(int index);
    void enterTimeZone(const QByteArray &zoneId);

    KDateTimeEdit *const q;

    KDateTimeEdit::Options m_options;
    QDateTime m_dateTime;
    QDateTime m_minDateTime;
    QDateTime m_maxDateTime;
    QString m_minWarnMsg;
    QString m_maxWarnMsg;

    QList<QLocale> m_calendarLocales;
    QList<QTimeZone> m_zones;

    Ui::KDateTimeEdit ui;
};

KDateTimeEditPrivate::KDateTimeEditPrivate(KDateTimeEdit *q)
                     :q(q)
{
    m_options = KDateTimeEdit::ShowDate | KDateTimeEdit::EditDate | KDateTimeEdit::SelectDate |
                KDateTimeEdit::ShowTime | KDateTimeEdit::EditTime | KDateTimeEdit::SelectTime |
                KDateTimeEdit::DatePicker | KDateTimeEdit::DateKeywords;
    m_dateTime = QDateTime::currentDateTime();
    m_dateTime.setTime(QTime(0, 0, 0));
    m_calendarLocales << q->locale();
    foreach (const QByteArray &zoneId, QTimeZone::availableTimeZoneIds()) {
        m_zones << QTimeZone(zoneId);
    }
}

KDateTimeEditPrivate::~KDateTimeEditPrivate()
{
}

QDateTime KDateTimeEditPrivate::defaultMinDateTime()
{
    // TODO: Find a way to get it from QLocale
    //return KDateTime(calendar()->earliestValidDate(), QTime(0, 0, 0, 0));
    return QDateTime();
}

QDateTime KDateTimeEditPrivate::defaultMaxDateTime()
{
    // TODO: Find a way to get it from QLocale
    //return KDateTime(calendar()->latestValidDate(), QTime(23, 59, 59, 999));
    return QDateTime();
}

void KDateTimeEditPrivate::initWidgets()
{
    initDateWidget();
    initTimeWidget();
    initCalendarWidget();
    initTimeZoneWidget();
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
    ui.m_timeCombo->setVisible((m_options &KDateTimeEdit::ShowTime) == KDateTimeEdit::ShowTime);
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
    foreach (const QLocale calendarLocale, m_calendarLocales) {
        ui.m_calendarCombo->addItem(calendarLocale.name(), calendarLocale);
    }
    ui.m_calendarCombo->setCurrentIndex(ui.m_calendarCombo->findData(q->locale()));
    ui.m_calendarCombo->setVisible((m_options &KDateTimeEdit::ShowCalendar) == KDateTimeEdit::ShowCalendar);
    ui.m_calendarCombo->setEnabled((m_options &KDateTimeEdit::SelectCalendar) == KDateTimeEdit::SelectCalendar);
    ui.m_calendarCombo->setEditable(false);
    ui.m_calendarCombo->blockSignals(false);
}

void KDateTimeEditPrivate::updateCalendarWidget()
{
    ui.m_calendarCombo->blockSignals(true);
    ui.m_calendarCombo->setCurrentIndex(ui.m_calendarCombo->findData(q->locale()));
    ui.m_calendarCombo->blockSignals(false);
}

void KDateTimeEditPrivate::selectCalendar(int index)
{
    enterCalendar(ui.m_calendarCombo->itemData(index).toLocale());
}

void KDateTimeEditPrivate::enterCalendar(const QLocale &calendarLocale)
{
    q->setLocale(calendarLocale);
    emit q->calendarEntered(q->locale());
}

void KDateTimeEditPrivate::initTimeZoneWidget()
{
    ui.m_timeZoneCombo->blockSignals(true);
    ui.m_timeZoneCombo->clear();
    ui.m_timeZoneCombo->addItem(q->tr("UTC", "UTC time zone"), QByteArray("UTC"));
    ui.m_timeZoneCombo->addItem(q->tr("Floating", "No specific time zone"), QByteArray());
    foreach (const QTimeZone &zone, m_zones) {
        ui.m_timeZoneCombo->addItem(zone.id(), zone.id());
    }
    ui.m_timeZoneCombo->setVisible((m_options &KDateTimeEdit::ShowTimeZone) == KDateTimeEdit::ShowTimeZone);
    ui.m_timeZoneCombo->setEnabled((m_options &KDateTimeEdit::SelectTimeZone) == KDateTimeEdit::SelectTimeZone);
    ui.m_timeZoneCombo->setEditable(false);
    ui.m_timeZoneCombo->blockSignals(false);
}

void KDateTimeEditPrivate::updateTimeZoneWidget()
{
    ui.m_timeZoneCombo->blockSignals(true);
    ui.m_timeZoneCombo->blockSignals(false);
}

void KDateTimeEditPrivate::selectTimeZone(int index)
{
    enterTimeZone(ui.m_timeCombo->itemData(index).toByteArray());
}

void KDateTimeEditPrivate::enterTimeZone(const QByteArray &zoneId)
{
    q->setTimeZone(QTimeZone(zoneId));
    emit q->dateTimeEntered(m_dateTime);
    emit q->timeZoneEntered(m_dateTime.timeZone());
}

void KDateTimeEditPrivate::warnDateTime()
{
    if (!q->isValid() &&
        (m_options &KDateTimeEdit::WarnOnInvalid) == KDateTimeEdit::WarnOnInvalid) {
        QString warnMsg;
        if (!m_dateTime.isValid()) {
            //TODO Add missing string
            //warnMsg = i18n("The date or time you entered is invalid");
        } else if (m_minDateTime.isValid() && m_dateTime < m_minDateTime) {
            if (m_minWarnMsg.isEmpty()) {
                //TODO Add datetime to string
                //warnMsg = q->tr("Date and time cannot be earlier than %1", "@info").arg(formatDate(m_minDate));
                warnMsg = q->tr("The entered date and time is before the minimum allowed date and time.", "@info");
            } else {
                warnMsg = m_minWarnMsg;
                //TODO localize properly
                warnMsg.replace("%1", q->locale().toString(m_minDateTime));
            }
        } else if (m_maxDateTime.isValid() && m_dateTime > m_maxDateTime) {
            if (m_maxWarnMsg.isEmpty()) {
                //TODO Add datetime to string
                //warnMsg = q->tr("Date cannot be later than %1", "@info").arg(formatDate(m_maxDate));
                warnMsg = q->tr("The entered date and time is after the maximum allowed date and time.", "@info");
            } else {
                warnMsg = m_maxWarnMsg;
                warnMsg.replace("%1", q->locale().toString(m_maxDateTime));
            }
        }
        KMessageBox::sorry(q, warnMsg);
    }
}


KDateTimeEdit::KDateTimeEdit(QWidget *parent)
              :QWidget(parent),
               d(new KDateTimeEditPrivate(this))
{
    d->ui.setupUi(this);
    //Need to do the min/max defaults here and not in private init as need to wait for ui to init
    //the KDateComboBox which holds the calendar object.  Revisit this???
    d->m_minDateTime = d->defaultMinDateTime();
    d->m_maxDateTime = d->defaultMaxDateTime();
    d->ui.m_calendarCombo->installEventFilter(this);
    d->ui.m_dateCombo->installEventFilter(this);
    d->ui.m_timeCombo->installEventFilter(this);
    d->ui.m_timeZoneCombo->installEventFilter(this);
    d->initWidgets();

    connect( d->ui.m_calendarCombo, SIGNAL(activated(int)),
             this,                      SLOT(selectCalendar(int)));
    connect( d->ui.m_timeZoneCombo, SIGNAL(activated(int)),
             this,                      SLOT(selectTimeZone(int)));
}

KDateTimeEdit::~KDateTimeEdit()
{
    delete d;
}

QDateTime KDateTimeEdit::dateTime() const
{
    return d->m_dateTime;
}

QDate KDateTimeEdit::date() const
{
    return d->m_dateTime.date();
}

QTime KDateTimeEdit::time() const
{
    return d->m_dateTime.time();
}

QTimeZone KDateTimeEdit::timeZone() const
{
    return d->m_dateTime.timeZone();
}

bool KDateTimeEdit::isValid() const
{
    return d->m_dateTime.isValid() &&
           (!d->m_minDateTime.isValid() || d->m_dateTime >= d->m_minDateTime) &&
           (!d->m_maxDateTime.isValid() || d->m_dateTime <= d->m_maxDateTime);
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

void KDateTimeEdit::setDateTime(const QDateTime &dateTime)
{
    if (dateTime != d->m_dateTime) {
        assignDateTime(dateTime);
        emit dateTimeChanged(d->m_dateTime);
        emit dateChanged(d->m_dateTime.date());
        emit timeChanged(d->m_dateTime.time());
    }
}

void KDateTimeEdit::assignDateTime(const QDateTime &dateTime)
{
    d->m_dateTime = dateTime;
    d->ui.m_dateCombo->setDate(dateTime.date());
    d->ui.m_timeCombo->setTime(dateTime.time());
}

void KDateTimeEdit::setDate(const QDate &date)
{
    if (date != d->m_dateTime.date()) {
        assignDate(date);
        emit dateTimeChanged(d->m_dateTime);
        emit dateChanged(d->m_dateTime.date());
    }
}

void KDateTimeEdit::assignDate(const QDate &date)
{
    d->m_dateTime.setDate(date);
    d->ui.m_dateCombo->setDate(date);
}

void KDateTimeEdit::setTime(const QTime &time)
{
    if (time != d->m_dateTime.time()) {
        assignTime(time);
        emit dateTimeChanged(d->m_dateTime);
        emit timeChanged(d->m_dateTime.time());
    }
}

void KDateTimeEdit::assignTime(const QTime &time)
{
    d->m_dateTime.setTime(time);
    d->ui.m_timeCombo->setTime(time);
}

void KDateTimeEdit::setTimeZone(const QTimeZone &zone)
{
    if (zone == d->m_dateTime.timeZone() || !zone.isValid()) {
        return;
    }

    assignTimeZone(zone);
    emit dateTimeChanged(d->m_dateTime);
    emit timeZoneChanged(d->m_dateTime.timeZone());
}

void KDateTimeEdit::assignTimeZone(const QTimeZone &zone)
{
    d->m_dateTime.setTimeZone(zone);
    d->updateTimeZoneWidget();
}

void KDateTimeEdit::setMinimumDateTime(const QDateTime &minDateTime, const QString &minWarnMsg)
{
    setDateTimeRange(minDateTime, maximumDateTime(), minWarnMsg, d->m_maxWarnMsg);
}

QDateTime KDateTimeEdit::minimumDateTime() const
{
    return d->m_minDateTime;
}

void KDateTimeEdit::resetMinimumDateTime()
{
    d->m_minDateTime = d->defaultMinDateTime();
}

void KDateTimeEdit::setMaximumDateTime(const QDateTime &maxDateTime, const QString &maxWarnMsg)
{
    setDateTimeRange(minimumDateTime(), maxDateTime, d->m_minWarnMsg, maxWarnMsg);
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
        minDateTime <= maxDateTime) {

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

void KDateTimeEdit::setCalendarLocalesList(const QList<QLocale> &calendarLocales)
{
    if (calendarLocales != d->m_calendarLocales) {
        d->m_calendarLocales = calendarLocales;
        d->updateCalendarWidget();
    }
}

QList<QLocale> KDateTimeEdit::calendarLocalesList() const
{
    return d->m_calendarLocales;
}

void KDateTimeEdit::setDateDisplayFormat(QLocale::FormatType format)
{
    d->ui.m_dateCombo->setDisplayFormat(format);
}

QLocale::FormatType KDateTimeEdit::dateDisplayFormat() const
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

void KDateTimeEdit::setTimeDisplayFormat(QLocale::FormatType formatOptions)
{
    d->ui.m_timeCombo->setDisplayFormat(formatOptions);
}

QLocale::FormatType KDateTimeEdit::timeDisplayFormat() const
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

void KDateTimeEdit::setTimeZones(const QList<QTimeZone> &zones)
{
    if (zones != d->m_zones) {
        d->m_zones = zones;
        d->updateTimeZoneWidget();
    }
}

QList<QTimeZone> KDateTimeEdit::timeZones() const
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
