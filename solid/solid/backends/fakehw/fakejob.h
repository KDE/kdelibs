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

#ifndef FAKEJOB_H
#define FAKEJOB_H

#include <kjob.h>
#include <QTimer>

class FakeJob : public KJob
{
    Q_OBJECT
public:
    FakeJob(QObject *parent = 0);
    virtual ~FakeJob();

public Q_SLOTS:
    void start();

public:
    void setBroken(bool broken);
    bool isBroken();

private Q_SLOTS:
    void slotTimeout();

private:
    QTimer m_timer;
    qulonglong m_stepsCount;
    qulonglong m_stepsTotal;
    qulonglong m_stepsIncrement;
    bool m_broken;
};

#endif
