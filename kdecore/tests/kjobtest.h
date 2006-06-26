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

#include <qobject.h>
#include "kjob.h"

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

class KJobTest : public QObject
{
    Q_OBJECT
public:
    KJobTest();

private Q_SLOTS:
    void testEmitResult_data();
    void testEmitResult();
    void testProgressTracking();
    void testExec_data();
    void testExec();
    void testKill_data();
    void testKill();

    void slotResult( KJob *job );

private:
    QEventLoop loop;
    int m_lastError;
    QString m_lastErrorText;
};

#endif

