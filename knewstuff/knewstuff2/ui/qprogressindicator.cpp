/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2007 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "qprogressindicator.h"

#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include <kiconloader.h>

QProgressIndicator::QProgressIndicator(QWidget *parent)
        : QFrame(parent)
{
    setFrameStyle(QFrame::NoFrame);
    m_pb = new QProgressBar();
    m_pb->setMinimum(0);
    m_pb->setMaximum(100);

    m_pbdetails = new QPushButton();
    m_pbdetails->setFixedWidth(32); // FIXME: I want a squared button
    m_pbdetails->setIcon(SmallIcon("go-up"));
    m_pbdetails->setEnabled(false);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setMargin(0);
    hbox->addWidget(m_pbdetails);
    hbox->addWidget(m_pb);

    m_details = new QWidget(this);
    m_details->setWindowFlags(Qt::Popup);
    m_details->hide();

    m_detailsvbox = new QVBoxLayout(m_details);

    connect(m_pbdetails, SIGNAL(clicked()), SLOT(slotClicked()));
}

void QProgressIndicator::slotClicked()
{
    QPoint indicatorpos = mapToGlobal(pos());
    m_details->move(indicatorpos.x(), indicatorpos.y());

    if (m_details->isVisible())
        m_details->hide();
    else
        m_details->show();
}

void QProgressIndicator::addProgress(const QString & message, int percentage)
{
    QProgressBar *pb;

    m_progress[message] = percentage;

    if (!m_progresswidgets.contains(message)) {
        QWidget *pbcontainer = new QWidget();

        // make a label to show the url
        QLabel * urlLabel = new QLabel(pbcontainer);
        urlLabel->setText(message);

        // make a progress bar
        pb = new QProgressBar(pbcontainer);
        pb->setMinimum(0);
        pb->setMaximum(100);
        m_progresswidgets.insert(message, pb);

        // make a cancel button
        QPushButton *pbcancel = new QPushButton();
        pbcancel->setFixedWidth(32); // FIXME: I want a squared button
        pbcancel->setIcon(SmallIcon("dialog-cancel"));

        QGridLayout *layout = new QGridLayout(pbcontainer);
        layout->addWidget(urlLabel, 0, 0, 1, 2);
        layout->addWidget(pbcancel, 1, 0);
        layout->addWidget(pb, 1, 1);

        m_detailsvbox->addWidget(pbcontainer);

        pbcontainer->show();
    } else {
        pb = m_progresswidgets[message];
    }

    pb->setValue(percentage);

    if (m_progress.count() > 0)
        m_pbdetails->setEnabled(true);

    if (percentage == 100)
        removeProgress(message);

    calculateAverage();
}

void QProgressIndicator::removeProgress(const QString & message)
{
    m_progress.remove(message);

    if (m_progresswidgets[message]) {
        delete m_progresswidgets[message]->parentWidget();
        m_progresswidgets.remove(message);
    }

    if (m_progress.count() == 0) {
        m_pbdetails->setEnabled(false);
        m_details->hide();
    }

    calculateAverage();
}

void QProgressIndicator::calculateAverage()
{
    if (m_progress.count() == 0) {
        m_pb->reset();
        return;
    }

    int average = 0;
    QHashIterator<QString, int> it(m_progress);
    while (it.hasNext()) {
        it.next();
        average += it.value();
    }
    average = (average / m_progress.count());

    m_pb->setValue(average);
}

#include "qprogressindicator.moc"
