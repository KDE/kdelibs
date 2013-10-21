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

#include "ktimecombobox.h"

#include <QtCore/QTime>
#include <QKeyEvent>
#include <QLineEdit>

#include "kmessagebox.h"

class KTimeComboBoxPrivate
{
public:

    KTimeComboBoxPrivate(KTimeComboBox *q);
    virtual ~KTimeComboBoxPrivate();

    QTime defaultMinTime();
    QTime defaultMaxTime();

    QString timeFormatToInputMask(const QString &format, bool nullMask = false);
    QTime nearestIntervalTime(const QTime &time);
    QString formatTime(const QTime &time);

    void initTimeWidget();
    void updateTimeWidget();

// Q_PRIVATE_SLOTs
    void selectTime(int index);
    void editTime(const QString &text);
    void enterTime(const QTime &time);
    void parseTime();
    void warnTime();

    KTimeComboBox *const q;

    QTime m_time;
    KTimeComboBox::Options m_options;
    QTime m_minTime;
    QTime m_maxTime;
    QString m_minWarnMsg;
    QString m_maxWarnMsg;
    QString m_nullString;
    bool m_warningShown;
    QLocale::FormatType m_displayFormat;
    int m_timeListInterval;
    QList<QTime> m_timeList;
};

KTimeComboBoxPrivate::KTimeComboBoxPrivate(KTimeComboBox *q)
                     :q(q),
                      m_time(QTime(0, 0, 0)),
                      m_warningShown(false),
                      m_displayFormat(QLocale::ShortFormat),
                      m_timeListInterval(15)
{
    m_options = KTimeComboBox::EditTime | KTimeComboBox::SelectTime;
    m_minTime = defaultMinTime();
    m_maxTime = defaultMaxTime();
}

KTimeComboBoxPrivate::~KTimeComboBoxPrivate()
{
}

QTime KTimeComboBoxPrivate::defaultMinTime()
{
    return QTime(0, 0, 0, 0);
}

QTime KTimeComboBoxPrivate::defaultMaxTime()
{
    return QTime(23, 59, 59, 999);
}

QString KTimeComboBoxPrivate::timeFormatToInputMask(const QString &format, bool nullMask)
{
    const QLocale locale = q->locale();

    //TODO not sure this will always work, does it support DigitSets, am/pm is dodgy?
    QString mask = formatTime(QTime(12,34,56,789));
    QString null = mask;
    mask.replace(locale.toString(12), QLatin1String("09"));
    null.replace(locale.toString(12), QLatin1String(""));
    mask.replace(locale.toString(34), QLatin1String("99"));
    null.replace(locale.toString(34), QLatin1String(""));
    mask.replace(locale.toString(56), QLatin1String("99"));
    null.replace(locale.toString(56), QLatin1String(""));
    mask.replace(locale.toString(789), QLatin1String("900"));
    null.replace(locale.toString(789), QLatin1String(""));
    if (format.contains(QLatin1String("ap")) ||
        format.contains(QLatin1String("AP"))) {
        QString am = locale.amText();
        QString pm = locale.pmText();
        int ampmLen = qMax(am.length(), pm.length());
        QString ampmMask;
        for (int i = 0; i < ampmLen; ++i) {
            ampmMask.append(QLatin1Char('a'));
        }
        mask.replace(pm, ampmMask);
        null.replace(pm, QLatin1String(""));
    }

    if (nullMask) {
        return null;
    } else {
        return mask;
    }
}

QTime KTimeComboBoxPrivate::nearestIntervalTime(const QTime &time)
{
    int i = 0;
    while (q->itemData(i).toTime() < time) {
        ++i;
    }
    QTime before = q->itemData(i).toTime();
    QTime after = q->itemData(i + 1).toTime();
    if (before.secsTo(time) <= time.secsTo(after)) {
        return before;
    } else {
        return after;
    }
}

QString KTimeComboBoxPrivate::formatTime(const QTime &time)
{
    return q->locale().toString(time, m_displayFormat);
}

void KTimeComboBoxPrivate::initTimeWidget()
{
    q->blockSignals(true);
    q->clear();

    // Set the input mask from the current format
    const QLocale locale = q->locale();
    q->lineEdit()->setInputMask(timeFormatToInputMask(locale.timeFormat(m_displayFormat)));
    m_nullString = timeFormatToInputMask(locale.timeFormat(m_displayFormat), true);

    // If EditTime then set the line edit
    q->lineEdit()->setReadOnly((m_options &KTimeComboBox::EditTime) != KTimeComboBox::EditTime);

    // If SelectTime then make list items visible
    if ((m_options &KTimeComboBox::SelectTime) == KTimeComboBox::SelectTime) {
        q->setMaxVisibleItems(10);
    } else {
        q->setMaxVisibleItems(0);
    }

    // Populate the drop-down time list
    // If no time list set the use the time interval
    if (m_timeList.isEmpty()) {
        QTime startTime = m_minTime;
        QTime thisTime(startTime.hour(), 0, 0, 0);
        while (thisTime.isValid() && thisTime <= startTime) {
            thisTime = thisTime.addSecs(m_timeListInterval * 60);
        }
        QTime endTime = m_maxTime;
        q->addItem(formatTime(startTime), startTime);
        while (thisTime.isValid() && thisTime < endTime) {
            q->addItem(formatTime(thisTime), thisTime);
            QTime newTime = thisTime.addSecs(m_timeListInterval * 60);
            if (newTime.isValid() && newTime > thisTime) {
                thisTime = newTime;
            } else {
                thisTime = QTime();
            }
        }
        q->addItem(formatTime(endTime), endTime);
    } else {
        foreach (const QTime &thisTime, m_timeList) {
            if (thisTime.isValid() && thisTime >= m_minTime && thisTime <= m_maxTime) {
                q->addItem(formatTime(thisTime), thisTime);
            }
        }
    }
    q->blockSignals(false);
}

void KTimeComboBoxPrivate::updateTimeWidget()
{
    q->blockSignals(true);
    int pos = q->lineEdit()->cursorPosition();
    //Set index before setting text otherwise it overwrites
    int i = 0;
    if (!m_time.isValid() || m_time < m_minTime) {
        i = 0;
    } else if (m_time > m_maxTime) {
        i = q->count() - 1;
    } else {
        while (q->itemData(i).toTime() < m_time && i < q->count() - 1) {
            ++i;
        }
    }
    q->setCurrentIndex(i);
    if (m_time.isValid()) {
        q->lineEdit()->setText(formatTime(m_time));
    } else {
        q->lineEdit()->setText(QString());
    }
    q->lineEdit()->setCursorPosition(pos);
    q->blockSignals(false);
}

void KTimeComboBoxPrivate::selectTime(int index)
{
    enterTime(q->itemData(index).toTime());
}

void KTimeComboBoxPrivate::editTime(const QString &text)
{
    m_warningShown = false;
    emit q->timeEdited(q->locale().toTime(text, m_displayFormat));
}

void KTimeComboBoxPrivate::parseTime()
{
    m_time = q->locale().toTime(q->lineEdit()->text(), m_displayFormat);
}

void KTimeComboBoxPrivate::enterTime(const QTime &time)
{
    q->setTime(time);
    warnTime();
    emit q->timeEntered(m_time);
}

void KTimeComboBoxPrivate::warnTime()
{
    if (!m_warningShown && !q->isValid() &&
        (m_options &KTimeComboBox::WarnOnInvalid) == KTimeComboBox::WarnOnInvalid) {
        QString warnMsg;
        if (!m_time.isValid()) {
            warnMsg = q->tr("The time you entered is invalid", "@info");
        } else if (m_time < m_minTime) {
            if (m_minWarnMsg.isEmpty()) {
                warnMsg = q->tr("Time cannot be earlier than %1", "@info").arg(formatTime(m_minTime));
            } else {
                warnMsg = m_minWarnMsg;
                warnMsg.replace("%1", formatTime(m_minTime));
            }
        } else if (m_time > m_maxTime) {
            if (m_maxWarnMsg.isEmpty()) {
                warnMsg = q->tr("Time cannot be later than %1", "@info").arg(formatTime(m_maxTime));
            } else {
                warnMsg = m_maxWarnMsg;
                warnMsg.replace("%1", formatTime(m_maxTime));
            }
        }
        m_warningShown = true;
        KMessageBox::sorry(q, warnMsg);
    }
}

KTimeComboBox::KTimeComboBox(QWidget *parent)
              :QComboBox(parent),
               d(new KTimeComboBoxPrivate(this))
{
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    d->initTimeWidget();
    d->updateTimeWidget();

    connect( this, SIGNAL(activated(int)),
             this, SLOT(selectTime(int)));
    connect( this, SIGNAL(editTextChanged(QString)),
             this, SLOT(editTime(QString)));
}

KTimeComboBox::~KTimeComboBox()
{
    delete d;
}

QTime KTimeComboBox::time() const
{
    d->parseTime();
    return d->m_time;
}

void KTimeComboBox::setTime(const QTime &time)
{
    if (time == d->m_time) {
        return;
    }

    if ((d->m_options &KTimeComboBox::ForceTime) == KTimeComboBox::ForceTime) {
        assignTime(d->nearestIntervalTime(time));
    } else {
        assignTime(time);
    }

    d->updateTimeWidget();
    emit timeChanged(d->m_time);
}

void KTimeComboBox::assignTime(const QTime &time)
{
    d->m_time = time;
}

bool KTimeComboBox::isValid() const
{
    d->parseTime();
    return d->m_time.isValid() &&
           d->m_time >= d->m_minTime &&
           d->m_time <= d->m_maxTime;
}

bool KTimeComboBox::isNull() const
{
    return lineEdit()->text() == d->m_nullString;
}

KTimeComboBox::Options KTimeComboBox::options() const
{
    return d->m_options;
}

void KTimeComboBox::setOptions(Options options)
{
    if (options != d->m_options) {
        d->m_options = options;
        d->initTimeWidget();
        d->updateTimeWidget();
    }
}

QTime KTimeComboBox::minimumTime() const
{
    return d->m_minTime;
}

void KTimeComboBox::setMinimumTime(const QTime &minTime, const QString &minWarnMsg)
{
    setTimeRange(minTime, d->m_maxTime, minWarnMsg, d->m_maxWarnMsg);
}

void KTimeComboBox::resetMinimumTime()
{
    setTimeRange(d->defaultMinTime(), d->m_maxTime, QString(), d->m_maxWarnMsg);
}

QTime KTimeComboBox::maximumTime() const
{
    return d->m_maxTime;
}

void KTimeComboBox::setMaximumTime(const QTime &maxTime, const QString &maxWarnMsg)
{
    setTimeRange(d->m_minTime, maxTime, d->m_minWarnMsg, maxWarnMsg);
}

void KTimeComboBox::resetMaximumTime()
{
    setTimeRange(d->m_minTime, d->defaultMaxTime(), d->m_minWarnMsg, QString());
}

void KTimeComboBox::setTimeRange(const QTime &minTime, const QTime &maxTime,
                                 const QString &minWarnMsg, const QString &maxWarnMsg)
{
    if (!minTime.isValid() || !maxTime.isValid() || minTime > maxTime) {
        return;
    }

    if (minTime != d->m_minTime || maxTime != d->m_maxTime ||
        minWarnMsg != d->m_minWarnMsg || maxWarnMsg != d->m_maxWarnMsg) {
        d->m_minTime = minTime;
        d->m_maxTime = maxTime;
        d->m_minWarnMsg = minWarnMsg;
        d->m_maxWarnMsg = maxWarnMsg;
        d->initTimeWidget();
        d->updateTimeWidget();
    }
}

void KTimeComboBox::resetTimeRange()
{
    setTimeRange(d->defaultMinTime(), d->defaultMaxTime(), QString(), QString());
}

QLocale::FormatType KTimeComboBox::displayFormat() const
{
    return d->m_displayFormat;
}

void KTimeComboBox::setDisplayFormat(QLocale::FormatType format)
{
    if (format != d->m_displayFormat) {
        d->m_displayFormat = format;
        d->initTimeWidget();
        d->updateTimeWidget();
    }
}

int KTimeComboBox::timeListInterval() const
{
    return d->m_timeListInterval;
}

void KTimeComboBox::setTimeListInterval(int minutes)
{
    if (minutes != d->m_timeListInterval) {
        //Must be able to exactly divide the valid time period
        int lowMins = (d->m_minTime.hour() * 60) + d->m_minTime.minute();
        int hiMins = (d->m_maxTime.hour() * 60) + d->m_maxTime.minute();
        if (d->m_minTime.minute() == 0 && d->m_maxTime.minute() == 59) {
            ++hiMins;
        }
        if ((hiMins - lowMins) % minutes == 0) {
            d->m_timeListInterval = minutes;
            d->m_timeList.clear();
        } else {
            return;
        }
        d->initTimeWidget();
    }
}

QList<QTime> KTimeComboBox::timeList() const
{
    //Return the drop down list as it is what can be selected currently
    QList<QTime> list;
    int c = count();
    for (int i = 0; i < c; ++i) {
        list.append(itemData(i).toTime());
    }
    return list;
}

void KTimeComboBox::setTimeList(QList<QTime> timeList,
                                const QString &minWarnMsg, const QString &maxWarnMsg)
{
    if (timeList != d->m_timeList) {
        d->m_timeList.clear();
        foreach (const QTime &time, timeList) {
            if (time.isValid() && !d->m_timeList.contains(time)) {
                d->m_timeList.append(time);
            }
        }
        qSort(d->m_timeList);
        // Does the updateTimeWidget call for us
        setTimeRange(d->m_timeList.first(), d->m_timeList.last(),
                    minWarnMsg, maxWarnMsg);
    }
}

bool KTimeComboBox::eventFilter(QObject *object, QEvent *event)
{
    return QComboBox::eventFilter(object, event);
}

void KTimeComboBox::keyPressEvent(QKeyEvent *keyEvent)
{
    QTime temp;
    switch (keyEvent->key()) {
    case Qt::Key_Down:
        temp = d->m_time.addSecs(-60);
        break;
    case Qt::Key_Up:
        temp = d->m_time.addSecs(60);
        break;
    case Qt::Key_PageDown:
        temp = d->m_time.addSecs(-3600);
        break;
    case Qt::Key_PageUp:
        temp = d->m_time.addSecs(3600);
        break;
    default:
        QComboBox::keyPressEvent(keyEvent);
        return;
    }
    if (temp.isValid() && temp >= d->m_minTime && temp <= d->m_maxTime) {
        d->enterTime(temp);
    }
}

void KTimeComboBox::focusOutEvent(QFocusEvent *event)
{
    d->parseTime();
    d->warnTime();
    QComboBox::focusOutEvent(event);
}

void KTimeComboBox::showPopup()
{
    QComboBox::showPopup();
}

void KTimeComboBox::hidePopup()
{
    QComboBox::hidePopup();
}

void KTimeComboBox::mousePressEvent(QMouseEvent *event)
{
    QComboBox::mousePressEvent(event);
}

void KTimeComboBox::wheelEvent(QWheelEvent *event)
{
    QComboBox::wheelEvent(event);
}

void KTimeComboBox::focusInEvent(QFocusEvent *event)
{
    QComboBox::focusInEvent(event);
}

void KTimeComboBox::resizeEvent(QResizeEvent *event)
{
    QComboBox::resizeEvent(event);
}

#include "moc_ktimecombobox.cpp"
