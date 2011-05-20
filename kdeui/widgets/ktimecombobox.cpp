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
#include <QtGui/QKeyEvent>
#include <QtGui/QLineEdit>

#include "kglobal.h"
#include "kdebug.h"
#include "klocale.h"
#include "kcombobox.h"

class KTimeComboBoxPrivate
{
public:

    KTimeComboBoxPrivate(KTimeComboBox *q);
    virtual ~KTimeComboBoxPrivate();

    QTime defaultMinTime();
    QTime defaultMaxTime();

    void initTimeWidget();
    void updateTimeWidget();

// Q_PRIVATE_SLOTs
    void selectTime(int index);
    void editTime(const QString &text);
    void enterTime(const QTime &time);
    void parseTime();

    KTimeComboBox *const q;

    QTime m_time;
    KTimeComboBox::Options m_options;
    QTime m_minTime;
    QTime m_maxTime;
    QString m_minErrorMsg;
    QString m_maxErrorMsg;
    KLocale::TimeFormatOptions m_displayFormat;
    int m_timeInterval;
};

KTimeComboBoxPrivate::KTimeComboBoxPrivate(KTimeComboBox *q)
                     :q(q),
                      m_displayFormat(KLocale::TimeDefault),
                      m_timeInterval(15)
{
    m_minTime = defaultMinTime();
    m_maxTime = defaultMaxTime();
    m_minErrorMsg = i18n("The entered time is before the minimum allowed time.");
    m_maxErrorMsg = i18n("The entered time is after the maximum allowed time.");
    QString stringFreeze;
    stringFreeze = i18nc( "@info", "Time cannot be earlier than %1", QString() );
    stringFreeze = i18nc( "@info", "Time cannot be later than %1", QString() );
    stringFreeze = i18n("The time you entered is invalid");
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

void KTimeComboBoxPrivate::initTimeWidget()
{
    q->clear();
    q->setEditable(true);
    q->setInsertPolicy(QComboBox::NoInsert);
    q->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    //TODO localise the mask based on KLocale
    q->lineEdit()->setInputMask("09:99");

    QTime startTime = m_time;
    QTime thisTime(startTime.hour(), 0, 0, 0);
    while (thisTime.isValid() && thisTime <= startTime) {
        thisTime = thisTime.addSecs(m_timeInterval * 60);
    }
    QTime endTime = m_maxTime;
    q->addItem(KGlobal::locale()->formatTime(startTime), startTime);
    while (thisTime.isValid() && thisTime < endTime) {
        q->addItem(KGlobal::locale()->formatTime(thisTime), thisTime);
        QTime newTime = thisTime.addSecs(m_timeInterval * 60);
        if (newTime.isValid() && newTime > thisTime) {
            thisTime = newTime;
        } else {
            thisTime = QTime();
        }
    }
    q->addItem(KGlobal::locale()->formatTime(endTime), endTime);
}

void KTimeComboBoxPrivate::updateTimeWidget()
{
    q->blockSignals(true);
    int pos = 0;
    if (q->lineEdit()) {
        pos = q->lineEdit()->cursorPosition();
        q->lineEdit()->setText(KGlobal::locale()->formatTime(m_time));
    }
    int i = 0;
    while (q->itemData(i).toTime() < m_time) {
        ++i;
    }
    q->setCurrentIndex(i);
    if (q->lineEdit()) {
        q->lineEdit()->setCursorPosition(pos);
    }
    q->blockSignals(false);
}

void KTimeComboBoxPrivate::selectTime(int index)
{
    enterTime(q->itemData(index).toTime());
}

//TODO sure we want to uptime during edit, should we wait for either exit widget or call to time()?
void KTimeComboBoxPrivate::editTime(const QString &text)
{
    //TODO read the text and set it
    //enterTime(KGlobal::locale()->readTime(q->lineEdit()->text()));
}

void KTimeComboBoxPrivate::parseTime()
{
    enterTime(KGlobal::locale()->readTime(q->lineEdit()->text()));
}

void KTimeComboBoxPrivate::enterTime(const QTime &time)
{
    q->setTime(time);
    emit q->timeEntered(m_time);
}

KTimeComboBox::KTimeComboBox(QWidget *parent)
              :KComboBox(parent),
               d(new KTimeComboBoxPrivate(this))
{
    d->initTimeWidget();
    d->updateTimeWidget();

    connect( this, SIGNAL(activated(int)),
             this, SLOT(selectTime(int)));
    connect( this, SIGNAL(editTextChanged(const QString&)),
             this, SLOT(editTime(const QString&)));
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

    assignTime(time);
    d->updateTimeWidget();
    emit timeChanged(d->m_time);
}

void KTimeComboBox::assignTime(const QTime &time)
{
    d->m_time = time;
}

bool KTimeComboBox::isValid() const
{
    if (d->m_minTime.isValid() && d->m_maxTime.isValid()) {
        return d->m_time.isValid() &&
               d->m_time >= d->m_minTime &&
               d->m_time <= d->m_maxTime;
    }
    return d->m_time.isValid();
}

KTimeComboBox::Options KTimeComboBox::options() const
{
    return d->m_options;
}

void KTimeComboBox::setOptions(Options options)
{
    d->m_options = options;
}

QTime KTimeComboBox::minimumTime() const
{
    return d->m_minTime;
}

QTime KTimeComboBox::maximumTime() const
{
    return d->m_maxTime;
}

void KTimeComboBox::setTimeRange(const QTime &minTime,
                                 const QTime &maxTime,
                                 const QString &minErrorMsg,
                                 const QString &maxErrorMsg)
{
    if (minTime.isValid() != maxTime.isValid()) {
        return;
    }

    if (minTime <= maxTime) {
        d->m_minTime = minTime;
        d->m_maxTime = maxTime;
        d->m_minErrorMsg = minErrorMsg;
        d->m_maxErrorMsg = maxErrorMsg;
    }
}

void KTimeComboBox::resetMinimumTime()
{
    d->m_minTime = d->defaultMinTime();
}

void KTimeComboBox::resetMaximumTime()
{
    d->m_maxTime = d->defaultMaxTime();
}

void KTimeComboBox::resetTimeRange()
{
    setTimeRange(d->defaultMinTime(), d->defaultMaxTime());
}

void KTimeComboBox::clearTimeRange()
{
    resetMinimumTime();
    resetMaximumTime();
}

KLocale::TimeFormatOptions KTimeComboBox::displayFormat()
{
    return d->m_displayFormat;
}

void KTimeComboBox::setDisplayFormat(KLocale::TimeFormatOptions format)
{
    d->m_displayFormat = format;
}

int KTimeComboBox::timeInterval() const
{
    return d->m_timeInterval;
}

void KTimeComboBox::setTimeInterval(int minutes)
{
    //Must be able to exactly divide the day
    if (1440 % minutes == 0) {
        d->m_timeInterval = minutes;
    }
}

bool KTimeComboBox::eventFilter(QObject *object, QEvent *event)
{
    return KComboBox::eventFilter(object, event);
}

void KTimeComboBox::keyPressEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
    case Qt::Key_Down:
        d->enterTime(d->m_time.addSecs(-60));
        break;
    case Qt::Key_Up:
        d->enterTime(d->m_time.addSecs(60));
        break;
    case Qt::Key_PageDown:
        d->enterTime(d->m_time.addSecs(-3600));
        break;
    case Qt::Key_PageUp:
        d->enterTime(d->m_time.addSecs(3600));
        break;
    default:
        KComboBox::keyPressEvent(keyEvent);
    }
}

void KTimeComboBox::focusOutEvent(QFocusEvent *event)
{
    if (event->type() == QEvent::FocusOut) {
        d->parseTime();
    }
    KComboBox::focusOutEvent(event);
}

void KTimeComboBox::showPopup()
{
    KComboBox::showPopup();
}

void KTimeComboBox::hidePopup()
{
    KComboBox::hidePopup();
}

void KTimeComboBox::mousePressEvent(QMouseEvent *event)
{
    KComboBox::mousePressEvent(event);
}

void KTimeComboBox::wheelEvent(QWheelEvent *event)
{
    KComboBox::wheelEvent(event);
}

void KTimeComboBox::focusInEvent(QFocusEvent *event)
{
    KComboBox::focusInEvent(event);
}

void KTimeComboBox::resizeEvent(QResizeEvent *event)
{
    KComboBox::resizeEvent(event);
}

#include "ktimecombobox.moc"
