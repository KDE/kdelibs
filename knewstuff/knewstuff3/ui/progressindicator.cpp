/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2007 Jeremy Whiting <jeremy@scitools.com>
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include "progressindicator.h"

#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include "kjob.h"
#include "kdebug.h"

#include <kiconloader.h>

using namespace KNS3;

ProgressIndicator::ProgressIndicator(QWidget *parent)
        : QFrame(parent)
{
    setFrameStyle(QFrame::NoFrame);
    
    m_finished = 0;
    m_total = 0;
    
    m_pb = new QProgressBar();
    m_pb->setVisible(false);
        
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setMargin(0);
    hbox->addWidget(m_pb);
}

void ProgressIndicator::addJob(KJob* job, const QString& label)
{
    m_jobs.insert(job, label);
    connect(job, SIGNAL(finished(KJob *)), this, SLOT(jobFinished(KJob*)));
    ++m_total;
}

void ProgressIndicator::jobFinished(KJob* job)
{
    m_jobs.remove(job);
    calculateAverage();
    ++m_finished;
}

void ProgressIndicator::calculateAverage()
{
    if (m_jobs.isEmpty()) {
        m_pb->setVisible(false);
        m_total = 0;
        m_finished = 0;
        return;
    }
    
    m_pb->setVisible(true);
    m_pb->setMaximum(m_total);
    m_pb->setValue(m_finished);
}

#include "progressindicator.moc"
