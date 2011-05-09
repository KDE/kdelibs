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

#include "kdatecombobox.h"

#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QLineEdit>
#include <QtGui/QWidgetAction>

#include "kdebug.h"
#include "klocale.h"
#include "klocalizeddate.h"
#include "kcombobox.h"
#include "kdatepicker.h"

class KDateComboBoxPrivate
{
public:

    KDateComboBoxPrivate(KDateComboBox *q);
    virtual ~KDateComboBoxPrivate();

    QDate defaultMinDate();
    QDate defaultMaxDate();

    void initDateWidget();
    void updateDateWidget();

// Q_PRIVATE_SLOTs
    void selectDate(int index);
    void editDate(const QString &text);
    void enterDate(const QDate &date);
    void parseDate();

    KDateComboBox *const q;
    QMenu *m_dateMenu;
    KDatePicker *m_datePicker;
    QWidgetAction *m_datePickerAction;

    KLocalizedDate m_date;
    KDateComboBox::Options m_options;
    QDate m_minDate;
    QDate m_maxDate;
    QString m_minErrorMsg;
    QString m_maxErrorMsg;
    KLocale::DateFormat m_displayFormat;
};

KDateComboBoxPrivate::KDateComboBoxPrivate(KDateComboBox *q)
                     :q(q),
                      m_dateMenu(new QMenu(q)),
                      m_datePicker(new KDatePicker(q)),
                      m_datePickerAction(new QWidgetAction(q)),
                      m_displayFormat(KLocale::ShortDate)
{
    m_minDate = defaultMinDate();
    m_maxDate = defaultMaxDate();
    m_datePicker->setCloseButton(false);
    m_datePickerAction->setDefaultWidget(m_datePicker);
    m_minErrorMsg = i18n("The entered date is before the minimum allowed date.");
    m_maxErrorMsg = i18n("The entered date is after the maximum allowed date.");
    QString stringFreeze;
    stringFreeze = i18nc( "@option tomorrow",   "Tomorrow" );
    stringFreeze = i18nc( "@option today",      "Today" );
    stringFreeze = i18nc( "@option yesterday",  "Yesterday" );
    stringFreeze = i18nc( "@option next week",  "Next week" );
    stringFreeze = i18nc( "@option last week",  "Last week" );
    stringFreeze = i18nc( "@option next month", "Next Month" );
    stringFreeze = i18nc( "@option last month", "Last Month" );
    stringFreeze = i18nc( "@option next year",  "Next Year" );
    stringFreeze = i18nc( "@option last year",  "Last Year" );
    stringFreeze = i18nc( "@info", "Date cannot be earlier than %1", QString() );
    stringFreeze = i18nc( "@info", "Date cannot be later than %1", QString() );
    stringFreeze = i18nc( "@info/plain", "today" );
    stringFreeze = i18n("The date you entered is invalid");
}

KDateComboBoxPrivate::~KDateComboBoxPrivate()
{
}

QDate KDateComboBoxPrivate::defaultMinDate()
{
    return m_date.calendar()->earliestValidDate();
}

QDate KDateComboBoxPrivate::defaultMaxDate()
{
    return m_date.calendar()->latestValidDate();
}

void KDateComboBoxPrivate::initDateWidget()
{
    q->setEditable(true);
    q->setMaxCount(1);
    q->addItem(m_date.formatDate(m_displayFormat));
    q->setCurrentIndex(0);
    q->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    m_dateMenu->clear();
    m_dateMenu->addAction(m_datePickerAction);
    m_dateMenu->addSeparator();
    m_dateMenu->addAction(i18nc("@option today", "Today"), q, SLOT( slotToday() ) );
    m_dateMenu->addAction(i18nc("@option tomorrow", "Tomorrow" ), q, SLOT( slotTomorrow() ) );
    m_dateMenu->addAction(i18nc("@option next week", "Next Week" ), q, SLOT( slotNextWeek() ) );
    m_dateMenu->addAction(i18nc("@option next month", "Next Month" ), q, SLOT( slotNextMonth() ) );
    m_dateMenu->addSeparator();
    m_dateMenu->addAction(i18nc("@option do not specify a date", "No Date" ), q, SLOT( slotNoDate() ) );
}

void KDateComboBoxPrivate::updateDateWidget()
{
    q->blockSignals(true);
    int pos = 0;
    if (q->lineEdit()) {
        pos = q->lineEdit()->cursorPosition();
        q->lineEdit()->setText(m_date.formatDate(m_displayFormat));
        m_datePicker->setDate(m_date.date());
        q->lineEdit()->setCursorPosition(pos);
    }
    q->blockSignals(false);
}

void KDateComboBoxPrivate::selectDate(int index)
{
    //enterTime();
}

//TODO sure we want to update durign edit, should we wait for either exit widget or call to time()?
void KDateComboBoxPrivate::editDate(const QString &text)
{
    //TODO read the text and set it
    //enterTime(locale()->readTime(text));
}

void KDateComboBoxPrivate::parseDate()
{
    enterDate(m_date.readDate(q->lineEdit()->text()).date());
}

void KDateComboBoxPrivate::enterDate(const QDate &date)
{
    q->setDate(date);
    emit q->dateEntered(m_date.date());
}


KDateComboBox::KDateComboBox(QWidget *parent)
              :KComboBox(parent),
               d(new KDateComboBoxPrivate(this))
{
    d->m_datePicker->installEventFilter(this);
    d->initDateWidget();
    d->updateDateWidget();
    connect( d->m_datePicker, SIGNAL(dateChanged(const QDate&)),
             this,            SLOT(enterDate(const QDate&)));
    connect( d->m_datePicker, SIGNAL(dateSelected(const QDate&)),
             this,            SLOT(enterDate(const QDate&)));
}

KDateComboBox::~KDateComboBox()
{
    delete d;
}

QDate KDateComboBox::date() const
{
    d->parseDate();
    return d->m_date.date();
}

void KDateComboBox::setDate(const QDate &date)
{
    if (date == d->m_date.date()) {
        return;
    }

    assignDate(date);
    d->updateDateWidget();
    emit dateChanged(d->m_date.date());
}

void KDateComboBox::assignDate(const QDate &date)
{
    d->m_date = date;
}

KLocale::CalendarSystem KDateComboBox::calendarSystem()
{
    return d->m_date.calendarSystem();
}

void KDateComboBox::setCalendarSystem(KLocale::CalendarSystem calendarSystem)
{
    assignCalendarSystem(calendarSystem);
}

void KDateComboBox::assignCalendarSystem(KLocale::CalendarSystem calendarSystem)
{
    d->m_date.setCalendarSystem(calendarSystem);
}

const KCalendarSystem *KDateComboBox::calendar() const
{
    return d->m_date.calendar();
}

void KDateComboBox::setCalendar(KCalendarSystem *calendar)
{
    d->m_date = KLocalizedDate(d->m_date.date(), calendar);
}

bool KDateComboBox::isValid() const
{
    if (d->m_minDate.isValid() && d->m_maxDate.isValid()) {
        return d->m_date.isValid() &&
               d->m_date >= d->m_minDate &&
               d->m_date <= d->m_maxDate;
    }
    return d->m_date.isValid();
}

KDateComboBox::Options KDateComboBox::options() const
{
    return d->m_options;
}

void KDateComboBox::setOptions(Options options)
{
    d->m_options = options;
}

QDate KDateComboBox::minimumDate() const
{
    return d->m_minDate;
}

QDate KDateComboBox::maximumDate() const
{
    return d->m_maxDate;
}

void KDateComboBox::setDateRange(const QDate &minDate,
                                 const QDate &maxDate,
                                 const QString &minErrorMsg,
                                 const QString &maxErrorMsg)
{
    if (minDate.isValid() != maxDate.isValid()) {
        return;
    }

    if (minDate <= maxDate) {
        d->m_minDate = minDate;
        d->m_maxDate = maxDate;
        d->m_minErrorMsg = minErrorMsg;
        d->m_maxErrorMsg = maxErrorMsg;
    }
}

void KDateComboBox::resetMinimumDate()
{
    d->m_minDate = d->defaultMinDate();
}

void KDateComboBox::resetMaximumDate()
{
    d->m_maxDate = d->defaultMaxDate();
}

void KDateComboBox::resetDateRange()
{
    setDateRange(d->defaultMinDate(), d->defaultMaxDate());
}

void KDateComboBox::clearDateRange()
{
    resetMinimumDate();
    resetMaximumDate();
}

KLocale::DateFormat KDateComboBox::displayFormat()
{
    return d->m_displayFormat;
}

void KDateComboBox::setDisplayFormat(KLocale::DateFormat format)
{
    d->m_displayFormat = format;
}

bool KDateComboBox::eventFilter(QObject *object, QEvent *event)
{
    return KComboBox::eventFilter(object, event);
}

void KDateComboBox::keyPressEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
    case Qt::Key_Down:
        d->enterDate(d->m_date.addDays(-1).date());
        return;
    case Qt::Key_Up:
        d->enterDate(d->m_date.addDays(1).date());
        return;
    case Qt::Key_PageDown:
        d->enterDate(d->m_date.addMonths(-1).date());
        return;
    case Qt::Key_PageUp:
        d->enterDate(d->m_date.addMonths(1).date());
        return;
    default:
        KComboBox::keyPressEvent(keyEvent);
    }
}

void KDateComboBox::focusOutEvent(QFocusEvent *event)
{
    if (event->type() == QEvent::FocusOut) {
        d->parseDate();
    }
    KComboBox::focusOutEvent(event);
}

//copied form KDateEdit, still to refactor.
void KDateComboBox::showPopup()
{
    if (!isEditable() || !d->m_dateMenu) {
        return;
    }

    const QRect desk = KGlobalSettings::desktopGeometry(this);

    QPoint popupPoint = mapToGlobal(QPoint(0, 0));

    const int dateFrameHeight = d->m_dateMenu->sizeHint().height();
    if (popupPoint.y() + height() + dateFrameHeight > desk.bottom()) {
        popupPoint.setY(popupPoint.y() - dateFrameHeight);
    } else {
        popupPoint.setY(popupPoint.y() + height());
    }

    const int dateFrameWidth = d->m_dateMenu->sizeHint().width();
    if (popupPoint.x() + dateFrameWidth > desk.right()) {
        popupPoint.setX(desk.right() - dateFrameWidth);
    }

    if (popupPoint.x() < desk.left()) {
        popupPoint.setX(desk.left());
    }

    if (popupPoint.y() < desk.top()) {
        popupPoint.setY(desk.top());
    }

    d->m_dateMenu->popup(popupPoint);

    //Update when done

    // Now, simulate an Enter to unpress it
    QAbstractItemView *lb = view();
    if (lb) {
        lb->setCurrentIndex(lb->model()->index(0, 0));
        QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
        QApplication::postEvent(lb, keyEvent);
    }
}

void KDateComboBox::hidePopup()
{
    KComboBox::hidePopup();
}

void KDateComboBox::mousePressEvent(QMouseEvent *event)
{
    KComboBox::mousePressEvent(event);
}

void KDateComboBox::wheelEvent(QWheelEvent *event)
{
    KComboBox::wheelEvent(event);
}

void KDateComboBox::focusInEvent(QFocusEvent *event)
{
    KComboBox::focusInEvent(event);
}

void KDateComboBox::resizeEvent(QResizeEvent *event)
{
    KComboBox::resizeEvent(event);
}


#include "kdatecombobox.moc"
