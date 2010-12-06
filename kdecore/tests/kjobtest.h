/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef KJOBTEST_H
#define KJOBTEST_H

#include <QtCore/QObject>
#include "kjob.h"
#include "kjobuidelegate.h"

class TestJob : public KJob
{
    Q_OBJECT
public:
    TestJob();
    virtual ~TestJob();

    virtual void start();

protected:
    virtual bool doKill();

public:
    void setError( int errorCode );
    void setErrorText( const QString &errorText );
    void setProcessedSize( qulonglong size );
    void setTotalSize( qulonglong size );
    void setPercent( unsigned long percentage );

private slots:
    void doEmit();
};

class TestJobUiDelegate : public KJobUiDelegate
{
    Q_OBJECT
protected:
    virtual void connectJob( KJob *job );
};

class WaitJob;

class KJobTest : public QObject
{
    Q_OBJECT
public:
    KJobTest();

public Q_SLOTS:

    // These slots need to be public, otherwise qtestlib calls them as part of the test
    void slotStartInnerJob();
    void slotFinishOuterJob();
    void slotFinishInnerJob();

private Q_SLOTS:
    void testEmitResult_data();
    void testEmitResult();
    void testProgressTracking();
    void testExec_data();
    void testExec();
    void testKill_data();
    void testKill();
    void testDelegateUsage();
    void testNestedExec();

    void slotResult( KJob *job );
    void slotFinished(KJob *job);

private:
    QEventLoop loop;
    int m_lastError;
    QString m_lastErrorText;
    int m_resultCount;
    int m_finishedCount;

    WaitJob *m_outerJob;
    WaitJob *m_innerJob;
};

class WaitJob : public KJob
{
    Q_OBJECT
public:

    virtual void start();
    void makeItFinish();
};

#endif

