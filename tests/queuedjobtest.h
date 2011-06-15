/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QUEUEDJOBTEST_H
#define QUEUEDJOBTEST_H

#include <QtTest/QtTest>
#include <queuedjob.h>

class JobQueue;

class TestJob : public QueuedJob
{
    Q_OBJECT

public:
    TestJob(JobQueue *queue);
    virtual ~TestJob();
    virtual bool isImmediate() const;
    virtual void exec();
    virtual void start();

protected Q_SLOTS:
    void doWork();
};

class QueuedJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testSync();
    void testAsync();
    void testAsyncOrder();
    void testAsyncOrderInFront();

    void cleanupTestCase();

private:
    JobQueue *m_queue;
};

#endif // QUEUEDJOBTEST_H
