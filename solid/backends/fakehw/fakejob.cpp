/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>
                       Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "fakejob.h"

#include <QTimer>

FakeJob::FakeJob(QObject *parent)
    : KJob(parent)
{
    m_stepsCount = 0;
    m_stepsTotal = 10;
    m_stepsIncrement = 1;

    connect(&m_timer, SIGNAL(timeout()),
             this, SLOT(slotTimeout()));
}

FakeJob::~FakeJob()
{

}

void FakeJob::start()
{
    setTotalAmount(KJob::Bytes, m_stepsTotal);

    m_timer.start(250);
}

void FakeJob::setBroken(bool broken)
{
    m_broken = broken;
}

bool FakeJob::isBroken()
{
    return m_broken;
}

void FakeJob::slotTimeout()
{
    if (isBroken())
    {
        setError(UserDefinedError+1);
        setErrorText("Failed: Fake backend, job broken.");
        m_timer.stop();
        emitResult();
        return;
    }

    m_stepsCount+=m_stepsIncrement;
    setProcessedAmount(KJob::Bytes, m_stepsCount);

    if (m_stepsCount >= m_stepsTotal)
    {
        m_timer.stop();
        emitResult();
    }
}

#include "fakejob.moc"
