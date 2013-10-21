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

#include <QAbstractItemView>
#include <QApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QLineEdit>
#include <QWidgetAction>
#include <QDesktopWidget>
#include <QVector>

#include "kdatepicker.h"
#include "kmessagebox.h"

class KDateComboBoxPrivate
{
public:

    KDateComboBoxPrivate(KDateComboBox *q);
    virtual ~KDateComboBoxPrivate();

    // TODO: Find a way to get that from QLocale
#if 0
    QDate defaultMinDate();
    QDate defaultMaxDate();
#endif

    QString dateFormat(QLocale::FormatType format);
    QString formatDate(const QDate &date);

    void initDateWidget();
    void addMenuAction(const QString &text, const QDate &date);
    void enableMenuDates();
    void updateDateWidget();

// Q_PRIVATE_SLOTs
    void clickDate();
    void selectDate(QAction *action);
    void editDate(const QString &text);
    void enterDate(const QDate &date);
    void parseDate();
    void warnDate();

    KDateComboBox *const q;
    QMenu *m_dateMenu;
    QVector<QAction*> m_actions;
    KDatePicker *m_datePicker;
    QWidgetAction *m_datePickerAction;

    QDate m_date;
    KDateComboBox::Options m_options;
    QDate m_minDate;
    QDate m_maxDate;
    QString m_minWarnMsg;
    QString m_maxWarnMsg;
    bool m_warningShown;
    QLocale::FormatType m_displayFormat;
    QMap<QDate, QString> m_dateMap;
};

KDateComboBoxPrivate::KDateComboBoxPrivate(KDateComboBox *q)
                     :q(q),
                      m_dateMenu(new QMenu(q)),
                      m_datePicker(new KDatePicker(q)),
                      m_datePickerAction(new QWidgetAction(q)),
                      m_warningShown(false),
                      m_displayFormat(QLocale::ShortFormat)
{
    m_options = KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::DateKeywords;
    m_date = QDate::currentDate();
    //m_minDate = defaultMinDate();
    //m_maxDate = defaultMaxDate();
    m_datePicker->setCloseButton(false);
    m_datePickerAction->setObjectName(QLatin1String("DatePicker"));
    m_datePickerAction->setDefaultWidget(m_datePicker);
}

KDateComboBoxPrivate::~KDateComboBoxPrivate()
{
}

#if 0
QDate KDateComboBoxPrivate::defaultMinDate()
{
    return m_date.calendar()->earliestValidDate();
}

QDate KDateComboBoxPrivate::defaultMaxDate()
{
    return m_date.calendar()->latestValidDate();
}
#endif

QString KDateComboBoxPrivate::dateFormat(QLocale::FormatType format)
{
    // Clearly a workaround for QLocale using "yy" way too often for years
    // and so you get no way to distinguish between 1913 and 2013 anymore...
    // bummer.
    QString res = q->locale().dateFormat(format);
    res.replace("yy", "yyyy");
    res.replace("yyyyyyyy", "yyyy");
    return res;
}

QString KDateComboBoxPrivate::formatDate(const QDate &date)
{
    return q->locale().toString(date, dateFormat(m_displayFormat));
}

void KDateComboBoxPrivate::initDateWidget()
{
    q->blockSignals(true);
    q->clear();

    // If EditTime then set the line edit
    q->lineEdit()->setReadOnly((m_options &KDateComboBox::EditDate) != KDateComboBox::EditDate);

    // If SelectTime then make list items visible
    if ((m_options &KDateComboBox::SelectDate) == KDateComboBox::SelectDate ||
        (m_options &KDateComboBox::DatePicker) == KDateComboBox::DatePicker ||
        (m_options &KDateComboBox::DatePicker) == KDateComboBox::DateKeywords) {
        q->setMaxVisibleItems(1);
    } else {
        q->setMaxVisibleItems(0);
    }

    q->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    q->addItem(formatDate(m_date));
    q->setCurrentIndex(0);
    q->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    q->blockSignals(false);

    m_dateMenu->clear();
    m_actions.clear();

    if ((m_options & KDateComboBox::SelectDate) == KDateComboBox::SelectDate) {

        if ((m_options & KDateComboBox::DatePicker) == KDateComboBox::DatePicker) {
            m_dateMenu->addAction(m_datePickerAction);
            m_dateMenu->addSeparator();
        }

        if ((m_options & KDateComboBox::DateKeywords) == KDateComboBox::DateKeywords) {
            if (m_dateMap.isEmpty()) {
                addMenuAction(q->tr("Next Year",  "@option next year" ), m_date.addYears(1));
                addMenuAction(q->tr("Next Month", "@option next month"), m_date.addMonths(1));
                addMenuAction(q->tr("Next Week",  "@option next week" ), m_date.addDays(7));
                addMenuAction(q->tr("Tomorrow",   "@option tomorrow"  ), m_date.addDays(1));
                addMenuAction(q->tr("Today",      "@option today"     ), m_date);
                addMenuAction(q->tr("Yesterday",  "@option yesterday" ), m_date.addDays(-1));
                addMenuAction(q->tr("Last Week",  "@option last week" ), m_date.addDays(-7));
                addMenuAction(q->tr("Last Month", "@option last month"), m_date.addMonths(-1));
                addMenuAction(q->tr("Last Year",  "@option last year" ), m_date.addYears(-1));
                m_dateMenu->addSeparator();
                addMenuAction(q->tr("No Date", "@option do not specify a date"), QDate());
            } else {
                QMapIterator<QDate, QString> i(m_dateMap);
                while (i.hasNext()) {
                    i.next();
                    if (i.value().isEmpty()) {
                        addMenuAction(formatDate(i.key()), i.key());
                    } else if (i.value().toLower() == QLatin1String("separator")) {
                        m_dateMenu->addSeparator();
                    } else {
                        addMenuAction(i.value(), i.key());
                    }
                }
            }
            enableMenuDates();
        }
    }
}

void KDateComboBoxPrivate::addMenuAction(const QString &text, const QDate &date)
{
    QAction *action = new QAction(m_dateMenu);
    action->setText(text);
    action->setData(date);
    m_dateMenu->addAction(action);
    m_actions << action;
}

void KDateComboBoxPrivate::enableMenuDates()
{
    // Hide menu dates if they are outside the date range
    for (int i = 0; i < m_actions.count(); ++i) {
        QDate date = m_actions[i]->data().toDate();
        m_actions[i]->setVisible(!date.isValid() || (date >= m_minDate && date <= m_maxDate));
    }
}

void KDateComboBoxPrivate::updateDateWidget()
{
    q->blockSignals(true);
    m_datePicker->blockSignals(true);
    m_datePicker->setDate(m_date);
    int pos = q->lineEdit()->cursorPosition();
    q->setItemText(0, formatDate(m_date));
    q->lineEdit()->setText(formatDate(m_date));
    q->lineEdit()->setCursorPosition(pos);
    m_datePicker->blockSignals(false);
    q->blockSignals(false);
}

void KDateComboBoxPrivate::selectDate(QAction *action)
{
    if (action->objectName() != QLatin1String("DatePicker")) {
        enterDate(action->data().toDate());
    }
}

void KDateComboBoxPrivate::clickDate()
{
    enterDate(m_datePicker->date());
}

void KDateComboBoxPrivate::editDate(const QString &text)
{
    m_warningShown = false;
    m_date = q->locale().toDate(text, dateFormat(m_displayFormat));
    emit q->dateEdited(m_date);
}

void KDateComboBoxPrivate::parseDate()
{
    m_date = q->locale().toDate(q->lineEdit()->text(), dateFormat(m_displayFormat));
}

void KDateComboBoxPrivate::enterDate(const QDate &date)
{
    q->setDate(date);
    // Re-add the combo box item in order to retain the correct widget width
    q->blockSignals(true);
    q->clear();
    q->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    q->addItem(formatDate(m_date));
    q->setCurrentIndex(0);
    q->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    q->blockSignals(false);

    m_dateMenu->hide();
    warnDate();
    emit q->dateEntered(m_date);
}

void KDateComboBoxPrivate::warnDate()
{
    if (!m_warningShown && !q->isValid() &&
        (m_options &KDateComboBox::WarnOnInvalid) == KDateComboBox::WarnOnInvalid) {
        QString warnMsg;
        if (!m_date.isValid()) {
            warnMsg = q->tr("The date you entered is invalid", "@info");
        } else if (m_minDate.isValid() && m_date < m_minDate) {
            if (m_minWarnMsg.isEmpty()) {
                warnMsg = q->tr("Date cannot be earlier than %1", "@info").arg(formatDate(m_minDate));
            } else {
                warnMsg = m_minWarnMsg;
                warnMsg.replace("%1", formatDate(m_minDate));
            }
        } else if (m_maxDate.isValid() && m_date > m_maxDate) {
            if (m_maxWarnMsg.isEmpty()) {
                warnMsg = q->tr("Date cannot be later than %1", "@info").arg(formatDate(m_maxDate));
            } else {
                warnMsg = m_maxWarnMsg;
                warnMsg.replace("%1", formatDate(m_maxDate));
            }
        }
        m_warningShown = true;
        KMessageBox::sorry(q, warnMsg);
    }
}


KDateComboBox::KDateComboBox(QWidget *parent)
              :QComboBox(parent),
               d(new KDateComboBoxPrivate(this))
{
    setEditable(true);
    setMaxVisibleItems(1);
    setInsertPolicy(QComboBox::NoInsert);
    d->m_datePicker->installEventFilter(this);
    d->initDateWidget();
    d->updateDateWidget();

    connect(d->m_dateMenu,         SIGNAL(triggered(QAction*)),
            this,                  SLOT(selectDate(QAction*)));
    connect(this,                  SIGNAL(editTextChanged(QString)),
            this,                  SLOT(editDate(QString)));
    connect(d->m_datePicker,       SIGNAL(dateEntered(QDate)),
            this,                  SLOT(enterDate(QDate)));
    connect(d->m_datePicker,       SIGNAL(tableClicked()),
            this,                  SLOT(clickDate()));
}

KDateComboBox::~KDateComboBox()
{
    delete d;
}

QDate KDateComboBox::date() const
{
    d->parseDate();
    return d->m_date;
}

void KDateComboBox::setDate(const QDate &date)
{
    if (date == d->m_date) {
        return;
    }

    assignDate(date);
    d->updateDateWidget();
    emit dateChanged(d->m_date);
}

void KDateComboBox::assignDate(const QDate &date)
{
    d->m_date = date;
}

bool KDateComboBox::isValid() const
{
    d->parseDate();
    return d->m_date.isValid() &&
           (!d->m_minDate.isValid() || d->m_date >= d->m_minDate) &&
           (!d->m_maxDate.isValid() || d->m_date <= d->m_maxDate);
}

bool KDateComboBox::isNull() const
{
    return lineEdit()->text().isEmpty();
}

KDateComboBox::Options KDateComboBox::options() const
{
    return d->m_options;
}

void KDateComboBox::setOptions(Options options)
{
    if (options != d->m_options) {
        d->m_options = options;
        d->initDateWidget();
        d->updateDateWidget();
    }
}

QDate KDateComboBox::minimumDate() const
{
    return d->m_minDate;
}

void KDateComboBox::setMinimumDate(const QDate &minDate, const QString &minWarnMsg)
{
    setDateRange(minDate, d->m_maxDate, minWarnMsg, d->m_maxWarnMsg);
}

void KDateComboBox::resetMinimumDate()
{
    //setDateRange(d->m_minDate, d->defaultMaxDate(), d->m_minWarnMsg, QString());
    setDateRange(QDate(), d->m_maxDate, QString(), d->m_maxWarnMsg);
}

QDate KDateComboBox::maximumDate() const
{
    return d->m_maxDate;
}

void KDateComboBox::setMaximumDate(const QDate &maxDate, const QString &maxWarnMsg)
{
    setDateRange(d->m_minDate, maxDate, d->m_minWarnMsg, maxWarnMsg);
}

void KDateComboBox::resetMaximumDate()
{
    //setDateRange(d->m_minDate, d->defaultMaxDate(), d->m_minWarnMsg, QString());
    setDateRange(d->m_minDate, QDate(), d->m_minWarnMsg, QString());
}

void KDateComboBox::setDateRange(const QDate &minDate,
                                 const QDate &maxDate,
                                 const QString &minWarnMsg,
                                 const QString &maxWarnMsg)
{
    if (!minDate.isValid() || !maxDate.isValid() || minDate > maxDate) {
        return;
    }

    if (minDate != d->m_minDate || maxDate != d->m_maxDate ||
        minWarnMsg != d->m_minWarnMsg || maxWarnMsg != d->m_maxWarnMsg) {
        d->m_minDate = minDate;
        d->m_maxDate = maxDate;
        d->m_minWarnMsg = minWarnMsg;
        d->m_maxWarnMsg = maxWarnMsg;
    }
    d->enableMenuDates();
}

void KDateComboBox::resetDateRange()
{
    //setDateRange(d->defaultMinDate(), d->defaultMaxDate(), QString(), QString());
    setDateRange(QDate(), QDate(), QString(), QString());
}

QLocale::FormatType KDateComboBox::displayFormat() const
{
    return d->m_displayFormat;
}

void KDateComboBox::setDisplayFormat(QLocale::FormatType format)
{
    if (format != d->m_displayFormat) {
        d->m_displayFormat = format;
        d->initDateWidget();
        d->updateDateWidget();
    }
}

QMap<QDate, QString> KDateComboBox::dateMap() const
{
    return d->m_dateMap;
}

void KDateComboBox::setDateMap(QMap<QDate, QString> dateMap)
{
    if (dateMap != d->m_dateMap) {
        d->m_dateMap.clear();
        d->m_dateMap = dateMap;
        d->initDateWidget();
    }
}

bool KDateComboBox::eventFilter(QObject *object, QEvent *event)
{
    return QComboBox::eventFilter(object, event);
}

void KDateComboBox::keyPressEvent(QKeyEvent *keyEvent)
{
    QDate temp;
    switch (keyEvent->key()) {
    case Qt::Key_Down:
        temp = d->m_date.addDays(-1);
        break;
    case Qt::Key_Up:
        temp = d->m_date.addDays(1);
        break;
    case Qt::Key_PageDown:
        temp = d->m_date.addMonths(-1);
        break;
    case Qt::Key_PageUp:
        temp = d->m_date.addMonths(1);
        break;
    default:
        QComboBox::keyPressEvent(keyEvent);
        return;
    }
    if (temp.isValid() && temp >= d->m_minDate && temp <= d->m_maxDate) {
        d->enterDate(temp);
    }
}

void KDateComboBox::focusOutEvent(QFocusEvent *event)
{
    d->parseDate();
    d->warnDate();
    QComboBox::focusOutEvent(event);
}

void KDateComboBox::showPopup()
{
    if (!isEditable() ||
        !d->m_dateMenu ||
        (d->m_options &KDateComboBox::SelectDate) != KDateComboBox::SelectDate) {
        return;
    }

    d->m_datePicker->blockSignals(true);
    d->m_datePicker->setDate(d->m_date);
    d->m_datePicker->blockSignals(false);

    const QRect desk = QApplication::desktop()->screenGeometry(this);

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
}

void KDateComboBox::hidePopup()
{
    QComboBox::hidePopup();
}

void KDateComboBox::mousePressEvent(QMouseEvent *event)
{
    QComboBox::mousePressEvent(event);
}

void KDateComboBox::wheelEvent(QWheelEvent *event)
{
    QComboBox::wheelEvent(event);
}

void KDateComboBox::focusInEvent(QFocusEvent *event)
{
    QComboBox::focusInEvent(event);
}

void KDateComboBox::resizeEvent(QResizeEvent *event)
{
    QComboBox::resizeEvent(event);
}

#include "moc_kdatecombobox.cpp"
