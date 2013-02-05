/*  This file is part of the KDE project
    Copyright (C) 2013 Kevin Funk <kevin@kfunk.org>

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

#ifndef KCOMPOSITEJOBTEST_H
#define KCOMPOSITEJOBTEST_H

#include <QEventLoop>
#include <QObject>

#include "kcompositejob.h"

class TestJob : public KJob
{
    Q_OBJECT

public:
    explicit TestJob(QObject* parent = 0);

    /// Takes 1 second to finish
    virtual void start();

private Q_SLOTS:
    void doEmit();
};

class CompositeJob : public KCompositeJob
{
    Q_OBJECT

public:
    explicit CompositeJob(QObject* parent = 0) : KCompositeJob(parent) {}

    virtual void start();
    virtual bool addSubjob(KJob* job);

protected Q_SLOTS:
    virtual void slotResult(KJob* job);
};

class KCompositeJobTest : public QObject
{
    Q_OBJECT

public:
    KCompositeJobTest();

private Q_SLOTS:
    void testDeletionDuringExecution();

private:
    QEventLoop loop;
};

#endif // KCOMPOSITEJOBTEST_H
