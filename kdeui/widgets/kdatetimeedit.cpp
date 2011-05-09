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
#include "klocale.h"
#include "kcalendarsystem.h"
#include "kcombobox.h"
#include "kdatepicker.h"

#include "ui_kdatetimeedit.h"

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

    void initDateWidget();
    void updateDateWidget();
    void initTimeWidget();
    void updateTimeWidget();
    void initTimezoneWidget();
    void showDatePopup();

    KDateTimeEdit *const q;
    KDatePicker *m_datePicker;
    QWidgetAction *m_datePickerAction;

    KDateTime m_dateTime;
    QDateTime m_minDateTime;
    QDateTime m_maxDateTime;
    QString m_minErrorMsg;
    QString m_maxErrorMsg;

    int m_timeInterval;
    bool m_readOnly;

    Ui::KDateTimeEdit ui;
};

KDateTimeEditPrivate::KDateTimeEditPrivate(KDateTimeEdit *q)
                     :q(q),
                      m_timeInterval(15),
                      m_readOnly(false)
{
    m_minDateTime = defaultMinDateTime();
    m_maxDateTime = defaultMaxDateTime();
    m_dateTime = KDateTime::currentLocalDateTime();
    m_datePicker = new KDatePicker(q);
    m_datePicker->setCloseButton(false);
    m_datePickerAction = new QWidgetAction(q);
    m_datePickerAction->setDefaultWidget(m_datePicker);
}

KDateTimeEditPrivate::~KDateTimeEditPrivate()
{
}

const KCalendarSystem *KDateTimeEditPrivate::calendar() const
{
    return KGlobal::locale()->calendar();
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

void KDateTimeEditPrivate::initDateWidget()
{
    ui.m_dateEditCombo->setEditable(!m_readOnly);
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

void KDateTimeEditPrivate::initTimeWidget()
{
    ui.m_timeEditCombo->clear();
    ui.m_timeEditCombo->setEditable(!m_readOnly);
    ui.m_timeEditCombo->setInsertPolicy(QComboBox::NoInsert);
    ui.m_dateEditCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

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
    ui.m_timeEditCombo->setCurrentIndex(i - 1);
    if (ui.m_timeEditCombo->lineEdit()) {
        ui.m_timeEditCombo->lineEdit()->setCursorPosition(pos);
    }
    ui.m_timeEditCombo->blockSignals(false);
}

void KDateTimeEditPrivate::initTimezoneWidget()
{
    ui.m_timezoneEditCombo->clear();
    ui.m_timezoneEditCombo->addItem(i18n("UTC"), "UTC");
    ui.m_timezoneEditCombo->addItem(i18n("Floating"), "Floating");
    QStringList zones = KSystemTimeZones::zones().keys();
    zones.sort();
    foreach (const QString &zone, zones) {
        ui.m_timezoneEditCombo->addItem(i18n(zone.toUtf8()).replace('_', ' '), zone);
    }
}

KDateTimeEdit::KDateTimeEdit(QWidget *parent)
              :QWidget(parent),
               d(new KDateTimeEditPrivate(this))
{
    KGlobal::locale()->insertCatalog("timezones4");
    d->ui.setupUi(this);
    d->ui.m_dateEditCombo->installEventFilter(this);
    d->ui.m_dateEditCombo->m_dateMenu->installEventFilter(this);
    d->m_datePicker->installEventFilter(this);
    d->ui.m_timeEditCombo->installEventFilter(this);
    d->ui.m_timezoneEditCombo->installEventFilter(this);
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

QDateTime KDateTimeEdit::dateTime() const
{
    return d->m_dateTime.dateTime();
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

void KDateTimeEdit::setDateTime( const KDateTime &dateTime )
{
    if (dateTime.isValid() &&
        dateTime != d->m_dateTime &&
        d->calendar()->isValid(dateTime.date()) &&
        dateTime.dateTime() >= d->m_minDateTime &&
        dateTime.dateTime() <= d->m_maxDateTime) {
        d->m_dateTime = dateTime;
        d->updateDateWidget();
        d->updateTimeWidget();
        emit dateTimeChanged(d->m_dateTime);
        emit dateTimeChanged(d->m_dateTime.dateTime());
        emit dateChanged(d->m_dateTime.date());
        emit timeChanged(d->m_dateTime.time());
    }
}

void KDateTimeEdit::setDateTime( const QDateTime &dateTime )
{
    //TODO probably need to do tz aware compare?
    if (dateTime.isValid() &&
        dateTime != d->m_dateTime.dateTime() &&
        d->calendar()->isValid(dateTime.date()) &&
        dateTime >= d->m_minDateTime &&
        dateTime <= d->m_maxDateTime) {
        d->m_dateTime.setDateTime(dateTime);
        d->updateDateWidget();
        d->updateTimeWidget();
        emit dateTimeChanged(d->m_dateTime);
        emit dateTimeChanged(d->m_dateTime.dateTime());
        emit dateChanged(d->m_dateTime.date());
        emit timeChanged(d->m_dateTime.time());
    }
}

void KDateTimeEdit::setDate( const QDate &date )
{
    if (date.isValid() &&
        d->calendar()->isValid(date) &&
        date != d->m_dateTime.date() &&
        date >= d->m_minDateTime.date() &&
        date <= d->m_maxDateTime.date()) {
        d->m_dateTime.setDate(date);
        d->updateDateWidget();
        emit dateTimeChanged(d->m_dateTime);
        emit dateTimeChanged(d->m_dateTime.dateTime());
        emit dateChanged(d->m_dateTime.date());
    }
}

void KDateTimeEdit::setTime( const QTime &time )
{
    if (time.isValid() &&
        time != d->m_dateTime.time() &&
        time >= d->m_minDateTime.time() &&
        time <= d->m_maxDateTime.time()) {
        d->m_dateTime.setTime(time);
        d->updateTimeWidget();
        emit dateTimeChanged(d->m_dateTime);
        emit dateTimeChanged(d->m_dateTime.dateTime());
        emit timeChanged(d->m_dateTime.time());
    }
}

void KDateTimeEdit::setTimeSpec(const KDateTime::Spec &spec)
{
    if (spec.isValid()) {
        d->m_dateTime.setTimeSpec(spec);
    }
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

void KDateTimeEdit::setTimeInterval(int minutes)
{
    d->m_timeInterval = minutes;
}

int KDateTimeEdit::timeInterval() const
{
    return d->m_timeInterval;
}

void KDateTimeEdit::setReadOnly(bool readOnly)
{
    d->m_readOnly = readOnly;
}

bool KDateTimeEdit::isReadOnly() const
{
    return d->m_readOnly;
}

bool KDateTimeEdit::eventFilter(QObject *object, QEvent *event)
{
    if (object == d->ui.m_timeEditCombo) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            switch (keyEvent->key()) {
                case Qt::Key_Down:
                    setTime(d->m_dateTime.addSecs(-60).time());
                    return true;
                case Qt::Key_Up:
                    setTime(d->m_dateTime.addSecs(60).time());
                    return true;
                case Qt::Key_PageUp:
                    setTime(d->m_dateTime.addSecs(-3600).time());
                    return true;
                case Qt::Key_PageDown:
                    setTime(d->m_dateTime.addSecs(3600).time());
                    return true;
            }
        }
    }
    return false;
}
