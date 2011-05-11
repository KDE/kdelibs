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
#include "kdatetimeedit_p.h"
#include "kdatetimeedit_p.moc"

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

#include "moc_kdatetimeedit.cpp"

KDateComboBoxPrivate::KDateComboBoxPrivate(QWidget *parent)
              :KComboBox(parent),
               m_dateMenu(new QMenu(parent))
{
}

KDateComboBoxPrivate::~KDateComboBoxPrivate()
{
}

//copied form KDateEdit, still to refactor.
void KDateComboBoxPrivate::showPopup()
{
    if (!isEditable() || !m_dateMenu) {
        return;
    }

    const QRect desk = KGlobalSettings::desktopGeometry(this);

    QPoint popupPoint = mapToGlobal(QPoint(0, 0));

    const int dateFrameHeight = m_dateMenu->sizeHint().height();
    if (popupPoint.y() + height() + dateFrameHeight > desk.bottom()) {
        popupPoint.setY(popupPoint.y() - dateFrameHeight);
    } else {
        popupPoint.setY(popupPoint.y() + height());
    }

    const int dateFrameWidth = m_dateMenu->sizeHint().width();
    if (popupPoint.x() + dateFrameWidth > desk.right()) {
        popupPoint.setX(desk.right() - dateFrameWidth);
    }

    if (popupPoint.x() < desk.left()) {
        popupPoint.setX(desk.left());
    }

    if (popupPoint.y() < desk.top()) {
        popupPoint.setY(desk.top());
    }

    m_dateMenu->popup(popupPoint);

    //Update when done

    // Now, simulate an Enter to unpress it
    QAbstractItemView *lb = view();
    if (lb) {
        lb->setCurrentIndex(lb->model()->index(0, 0));
        QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
        QApplication::postEvent(lb, keyEvent);
    }
}

KDateTimeEditPrivate::KDateTimeEditPrivate(KDateTimeEdit *q)
                     :QObject(q),
                      q(q),
                      m_calendar(0),
                      m_datePicker(0),
                      m_datePickerAction(0),
                      m_timeInterval(15)
{
    m_options = KDateTimeEdit::ShowDate | KDateTimeEdit::EditDate | KDateTimeEdit::SelectDate |
                KDateTimeEdit::ShowTime | KDateTimeEdit::EditTime | KDateTimeEdit::SelectTime |
                KDateTimeEdit::FancyDate;
    m_calendarSystem = calendar()->calendarSystem();
    m_minDateTime = defaultMinDateTime();
    m_maxDateTime = defaultMaxDateTime();
    m_dateTime = KDateTime::currentLocalDateTime();
    m_datePicker = new KDatePicker(q);
    m_datePicker->setCloseButton(false);
    m_datePickerAction = new QWidgetAction(q);
    m_datePickerAction->setDefaultWidget(m_datePicker);
    m_minErrorMsg = i18n("The entered date and time is before the minimum allowed date and time.");
    m_maxErrorMsg = i18n("The entered date and time is after the maximum allowed date and time.");
    m_zones = KSystemTimeZones::zones();
}

KDateTimeEditPrivate::~KDateTimeEditPrivate()
{
}

const KCalendarSystem *KDateTimeEditPrivate::calendar() const
{
    if (m_calendar) {
        return m_calendar;
    } else {
        return KGlobal::locale()->calendar();
    }
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
    ui.m_calendarEditCombo->clear();
    foreach (KLocale::CalendarSystem calendar, KCalendarSystem::calendarSystemsList()) {
        ui.m_calendarEditCombo->addItem(KCalendarSystem::calendarLabel(calendar), calendar);
    }
    ui.m_calendarEditCombo->setCurrentIndex(ui.m_calendarEditCombo->findData(m_calendarSystem));

    connect( ui.m_calendarEditCombo, SIGNAL(activated(int)),
             this,                   SLOT(selectCalendar(int)));
}

void KDateTimeEditPrivate::selectCalendar(int index)
{
    enterCalendar((KLocale::CalendarSystem) ui.m_calendarEditCombo->itemData(index).toInt());
}

void KDateTimeEditPrivate::enterCalendar(KLocale::CalendarSystem calendarSystem)
{
    q->setCalendarSystem(calendarSystem);
    emit q->calendarEntered(m_calendarSystem);
}

void KDateTimeEditPrivate::initDateWidget()
{
    ui.m_dateEditCombo->setEditable(true);
    ui.m_dateEditCombo->setMaxCount(1);
    ui.m_dateEditCombo->addItem(locale()->formatDate(m_dateTime.date(), KLocale::ShortDate));
    ui.m_dateEditCombo->setCurrentIndex(0);
    ui.m_dateEditCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    ui.m_dateEditCombo->m_dateMenu->clear();
    ui.m_dateEditCombo->m_dateMenu->addAction(m_datePickerAction);
    ui.m_dateEditCombo->m_dateMenu->addSeparator();
    ui.m_dateEditCombo->m_dateMenu->addAction(i18nc("@option today", "&Today"), q, SLOT( slotToday() ) );
    ui.m_dateEditCombo->m_dateMenu->addAction(i18nc("@option tomorrow", "To&morrow" ), q, SLOT( slotTomorrow() ) );
    ui.m_dateEditCombo->m_dateMenu->addAction(i18nc("@option next week", "Next &Week" ), q, SLOT( slotNextWeek() ) );
    ui.m_dateEditCombo->m_dateMenu->addAction(i18nc("@option next month", "Next M&onth" ), q, SLOT( slotNextMonth() ) );
    ui.m_dateEditCombo->m_dateMenu->addSeparator();
    ui.m_dateEditCombo->m_dateMenu->addAction(i18nc("@option do not specify a date", "No Date" ), q, SLOT( slotNoDate() ) );

    connect( m_datePicker, SIGNAL(dateChanged(const QDate&)),
             this,         SLOT(enterDate(const QDate&)));
    connect( m_datePicker, SIGNAL(dateSelected(const QDate&)),
             this,         SLOT(enterDate(const QDate&)));
}

void KDateTimeEditPrivate::updateDateWidget()
{
    ui.m_dateEditCombo->blockSignals(true);
    int pos = 0;
    if (ui.m_dateEditCombo->lineEdit()) {
        pos = ui.m_dateEditCombo->lineEdit()->cursorPosition();
        ui.m_dateEditCombo->lineEdit()->setText(locale()->formatDate(m_dateTime.date()));
        m_datePicker->setDate(m_dateTime.date());
        ui.m_dateEditCombo->lineEdit()->setCursorPosition(pos);
    }
    ui.m_dateEditCombo->blockSignals(false);
}

void KDateTimeEditPrivate::selectDate(int index)
{
    //enterTime();
}

//TODO sure we want to update durign edit, should we wait for either exit widget or call to time()?
void KDateTimeEditPrivate::editDate(const QString &text)
{
    //TODO read the text and set it
    //enterTime(locale()->readTime(text));
}

void KDateTimeEditPrivate::parseDate()
{
    enterDate(calendar()->readDate(ui.m_dateEditCombo->lineEdit()->text()));
}

void KDateTimeEditPrivate::enterDate(const QDate &date)
{
    q->setDate(date);
    emit q->dateTimeEntered(m_dateTime);
    emit q->dateTimeEntered(m_dateTime.dateTime());
    emit q->dateEntered(m_dateTime.date());
}

void KDateTimeEditPrivate::initTimeWidget()
{
    ui.m_timeEditCombo->clear();
    ui.m_timeEditCombo->setEditable(true);
    ui.m_timeEditCombo->setInsertPolicy(QComboBox::NoInsert);
    ui.m_timeEditCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    //TODO localise the mask based on KLocale
    ui.m_timeEditCombo->lineEdit()->setInputMask("09:00");

    QTime startTime = m_minDateTime.time();
    QTime thisTime(startTime.hour(), 0, 0, 0);
    while (thisTime.isValid() && thisTime <= startTime) {
        thisTime = thisTime.addSecs(m_timeInterval * 60);
    }
    QTime endTime = m_maxDateTime.time();
    ui.m_timeEditCombo->addItem(locale()->formatTime(startTime), startTime);
    while (thisTime.isValid() && thisTime < endTime) {
        ui.m_timeEditCombo->addItem(locale()->formatTime(thisTime), thisTime);
        QTime newTime = thisTime.addSecs(m_timeInterval * 60);
        if (newTime.isValid() && newTime > thisTime) {
            thisTime = newTime;
        } else {
            thisTime = QTime();
        }
    }
    ui.m_timeEditCombo->addItem(locale()->formatTime(endTime), endTime);

    connect( ui.m_timeEditCombo, SIGNAL(activated(int)),
             this,               SLOT(selectTime(int)));
    connect( ui.m_timeEditCombo, SIGNAL(editTextChanged(const QString&)),
             this,               SLOT(editTime(const QString&)));
}

void KDateTimeEditPrivate::updateTimeWidget()
{
    ui.m_timeEditCombo->blockSignals(true);
    int pos = 0;
    if (ui.m_timeEditCombo->lineEdit()) {
        pos = ui.m_timeEditCombo->lineEdit()->cursorPosition();
        ui.m_timeEditCombo->lineEdit()->setText(locale()->formatTime(m_dateTime.time()));
    }
    int i = 0;
    while (ui.m_timeEditCombo->itemData(i).toTime() < m_dateTime.time()) {
        ++i;
    }
    ui.m_timeEditCombo->setCurrentIndex(i);
    if (ui.m_timeEditCombo->lineEdit()) {
        ui.m_timeEditCombo->lineEdit()->setCursorPosition(pos);
    }
    ui.m_timeEditCombo->blockSignals(false);
}

void KDateTimeEditPrivate::selectTime(int index)
{
    enterTime(ui.m_timeEditCombo->itemData(index).toTime());
}

//TODO sure we want to update during edit, should we wait for either exit widget or call to time()?
void KDateTimeEditPrivate::editTime(const QString &text)
{
    //TODO read the text and set it
    //enterTime(locale()->readTime(text));
}

void KDateTimeEditPrivate::parseTime()
{
    enterTime(locale()->readTime(ui.m_timeEditCombo->lineEdit()->text()));
}

void KDateTimeEditPrivate::enterTime(const QTime &time)
{
    q->setTime(time);
    emit q->dateTimeEntered(m_dateTime);
    emit q->dateTimeEntered(m_dateTime.dateTime());
    emit q->timeEntered(m_dateTime.time());
}

void KDateTimeEditPrivate::initTimezoneWidget()
{
    ui.m_timezoneEditCombo->clear();
    ui.m_timezoneEditCombo->addItem(i18n("UTC"), "UTC");
    ui.m_timezoneEditCombo->addItem(i18n("Floating"), "Floating");
    QStringList keys = m_zones.keys();
    QMap<QString, QString> names;
    foreach (const QString &key, keys) {
        names.insert(i18n(key.toUtf8()).replace('_', ' '), key);
    }
    QMapIterator<QString, QString> i(names);
    while (i.hasNext()) {
        i.next();
        ui.m_timezoneEditCombo->addItem(i.key(), i.value());
    }

    connect( ui.m_timezoneEditCombo, SIGNAL(activated(int)),
             this,                   SLOT(selectTimezone(int)));
}

void KDateTimeEditPrivate::selectTimezone(int index)
{
    enterTimezone(ui.m_timeEditCombo->itemData(index).toString());
}

void KDateTimeEditPrivate::enterTimezone(const QString &zone)
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
    d->ui.m_calendarEditCombo->installEventFilter(this);
    d->ui.m_dateEditCombo->installEventFilter(this);
    d->ui.m_dateEditCombo->m_dateMenu->installEventFilter(this);
    d->m_datePicker->installEventFilter(this);
    d->ui.m_timeEditCombo->installEventFilter(this);
    d->ui.m_timezoneEditCombo->installEventFilter(this);
    d->initCalendarWidget();
    d->initDateWidget();
    d->updateDateWidget();
    d->initTimeWidget();
    d->updateTimeWidget();
    d->initTimezoneWidget();
}

KDateTimeEdit::~KDateTimeEdit()
{
    delete d;
}

KDateTime KDateTimeEdit::dateTimeSpec() const
{
    d->parseDate();
    d->parseTime();
    return d->m_dateTime;
}

QDateTime KDateTimeEdit::dateTime() const
{
    d->parseDate();
    d->parseTime();
    return d->m_dateTime.dateTime();
}

QDate KDateTimeEdit::date() const
{
    d->parseDate();
    return d->m_dateTime.date();
}

KLocale::CalendarSystem KDateTimeEdit::calendarSystem()
{
    //TODO Do this properly
    return d->m_calendarSystem;
}

QTime KDateTimeEdit::time() const
{
    d->parseTime();
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

    if ((d->m_options &AcceptInvalid) != AcceptInvalid &&
        (!dateTime.isValid() || d->calendar()->isValid(dateTime.date()))) {
        return;
    }

    if ((d->m_options &ApplyMinMax) == ApplyMinMax &&
        dateTime.isValid() &&
        (dateTime.dateTime() <= d->m_minDateTime || dateTime.dateTime() >= d->m_maxDateTime)) {
        return;
    }

    assignDateTime(dateTime);
    d->updateDateWidget();
    d->updateTimeWidget();
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

    if ((d->m_options &AcceptInvalid) != AcceptInvalid &&
        (!dateTime.isValid() || d->calendar()->isValid(dateTime.date()))) {
        return;
    }

    if ((d->m_options &ApplyMinMax) == ApplyMinMax &&
        dateTime.isValid() &&
        (dateTime <= d->m_minDateTime || dateTime >= d->m_maxDateTime)) {
        return;
    }

    assignDateTime(dateTime);
    d->updateDateWidget();
    d->updateTimeWidget();
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

    if ((d->m_options &AcceptInvalid) != AcceptInvalid &&
        (!date.isValid() || d->calendar()->isValid(date))) {
        return;
    }

    if ((d->m_options &ApplyMinMax) == ApplyMinMax &&
        date.isValid() &&
        (date <= d->m_minDateTime.date() || date >= d->m_maxDateTime.date())) {
        return;
    }

    assignDate(date);
    d->updateDateWidget();
    emit dateTimeChanged(d->m_dateTime);
    emit dateTimeChanged(d->m_dateTime.dateTime());
    emit dateChanged(d->m_dateTime.date());
}

void KDateTimeEdit::setCalendarSystem(KLocale::CalendarSystem calendarSystem)
{
    //TODO Do this properly
    d->m_calendarSystem = calendarSystem;
    emit calendarChanged(d->m_calendarSystem);
}

void KDateTimeEdit::setCalendar(KCalendarSystem *calendar)
{
    if (calendar == KGlobal::locale()->calendar()) {
        d->m_calendar = 0;
    } else {
        d->m_calendar = calendar;
    }
    d->updateDateWidget();
}

void KDateTimeEdit::setTime(const QTime &time)
{
    if (time == d->m_dateTime.time()) {
        return;
    }

    if ((d->m_options &AcceptInvalid) != AcceptInvalid &&
        !time.isValid()) {
        return;
    }

    if ((d->m_options &ApplyMinMax) == ApplyMinMax &&
        time.isValid() &&
        (time <= d->m_minDateTime.time() || time >= d->m_maxDateTime.time())) {
        return;
    }

    assignTime(time);
    d->updateTimeWidget();
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
}

void KDateTimeEdit::assignDateTime(const QDateTime &dateTime)
{
    d->m_dateTime.setDateTime(dateTime);
}

void KDateTimeEdit::assignDate(const QDate &date)
{
    d->m_dateTime.setDate(date);
}

void KDateTimeEdit::assignTime(const QTime &time)
{
    d->m_dateTime.setTime(time);
}

void KDateTimeEdit::setOptions(Options options)
{
    d->m_options = options;
}

QDateTime KDateTimeEdit::minimumDateTime() const
{
    return d->m_minDateTime;
}

void KDateTimeEdit::setMinimumDateTime(const QDateTime &minDateTime, const QString &minErrorMsg)
{
    if (minDateTime.isValid() &&
        minDateTime <= d->m_maxDateTime &&
        d->calendar()->isValid(minDateTime.date())) {
        d->m_minDateTime = minDateTime;
        d->m_minErrorMsg = minErrorMsg;
        d->initTimeWidget();
    }
}

void KDateTimeEdit::clearMinimumDateTime()
{
    setMinimumDateTime(d->defaultMinDateTime());
}

QDateTime KDateTimeEdit::maximumDateTime() const
{
    return d->m_maxDateTime;
}

void KDateTimeEdit::setMaximumDateTime(const QDateTime &maxDateTime, const QString &maxErrorMsg)
{
    if (maxDateTime.isValid() &&
        maxDateTime >= d->m_minDateTime &&
        d->calendar()->isValid(maxDateTime.date())) {
        d->m_maxDateTime = maxDateTime;
        d->m_maxErrorMsg = maxErrorMsg;
        d->initTimeWidget();
    }
}

void KDateTimeEdit::clearMaximumDateTime()
{
    setMaximumDateTime(d->defaultMaxDateTime());
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
        d->initTimeWidget();
    }
}

void KDateTimeEdit::clearDateTimeRange()
{
    setDateTimeRange(d->defaultMinDateTime(), d->defaultMaxDateTime());
}

QDate KDateTimeEdit::minimumDate() const
{
    return d->m_minDateTime.date();
}

void KDateTimeEdit::setMinimumDate(const QDate &minDate, const QString &minErrorMsg)
{
    setMinimumDateTime(QDateTime(minDate, d->m_minDateTime.time()), minErrorMsg);
}

void KDateTimeEdit::clearMinimumDate()
{
    setMinimumDate(d->defaultMinDate());
}

QDate KDateTimeEdit::maximumDate() const
{
    return d->m_maxDateTime.date();
}

void KDateTimeEdit::setMaximumDate(const QDate &maxDate, const QString &maxErrorMsg)
{
    setMaximumDateTime(QDateTime(maxDate, d->m_maxDateTime.time()), maxErrorMsg);
}

void KDateTimeEdit::clearMaximumDate()
{
    setMaximumDate(d->defaultMaxDate());
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

void KDateTimeEdit::clearDateRange()
{
    setDateRange(d->defaultMinDate(), d->defaultMaxDate());

}

QTime KDateTimeEdit::minimumTime() const
{
    return d->m_minDateTime.time();
}

void KDateTimeEdit::setMinimumTime(const QTime &minTime, const QString &minErrorMsg)
{
    setMinimumDateTime(QDateTime(d->m_minDateTime.date(), minTime), minErrorMsg);
}

void KDateTimeEdit::clearMinimumTime()
{
    setMinimumTime(d->defaultMinTime());
}

QTime KDateTimeEdit::maximumTime() const
{
    return d->m_maxDateTime.time();
}

void KDateTimeEdit::setMaximumTime(const QTime &maxTime, const QString &maxErrorMsg)
{
    setMaximumDateTime(QDateTime(d->m_maxDateTime.date(), maxTime), maxErrorMsg);
}

void KDateTimeEdit::clearMaximumTime()
{
    setMinimumTime(d->defaultMaxTime());
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

void KDateTimeEdit::clearTimeRange()
{
    setTimeRange(d->defaultMinTime(), d->defaultMaxTime());
}

void KDateTimeEdit::setSelectTimeInterval(int minutes)
{
    d->m_timeInterval = minutes;
}

int KDateTimeEdit::selectTimeInterval() const
{
    return d->m_timeInterval;
}

void KDateTimeEdit::setTimeZones(const KTimeZones::ZoneMap &zones)
{
    d->m_zones = zones;
}

bool KDateTimeEdit::eventFilter(QObject *object, QEvent *event)
{
    if (object == d->m_datePicker) {
        return false;
    }

    if (object == d->ui.m_dateEditCombo) {

        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            switch (keyEvent->key()) {
            case Qt::Key_Down:
                d->enterDate(d->calendar()->addDays(d->m_dateTime.date(), -1));
                return true;
            case Qt::Key_Up:
                d->enterDate(d->calendar()->addDays(d->m_dateTime.date(), 1));
                return true;
            case Qt::Key_PageDown:
                d->enterDate(d->calendar()->addMonths(d->m_dateTime.date(), -1));
                return true;
            case Qt::Key_PageUp:
                d->enterDate(d->calendar()->addMonths(d->m_dateTime.date(), 1));
                return true;
            }
            return false;
        }

        if (event->type() == QEvent::FocusOut) {
            d->parseDate();
        }

        return false;

    }

    if (object == d->ui.m_timeEditCombo) {

        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            switch (keyEvent->key()) {
            case Qt::Key_Down:
                d->enterTime(d->m_dateTime.time().addSecs(-60));
                return true;
            case Qt::Key_Up:
                d->enterTime(d->m_dateTime.time().addSecs(60));
                return true;
            case Qt::Key_PageDown:
                d->enterTime(d->m_dateTime.time().addSecs(-3600));
                return true;
            case Qt::Key_PageUp:
                d->enterTime(d->m_dateTime.time().addSecs(3600));
                return true;
            }
            return false;
        }

        if (event->type() == QEvent::FocusOut) {
            d->parseTime();
        }

        return false;
    }

    return false;
}
