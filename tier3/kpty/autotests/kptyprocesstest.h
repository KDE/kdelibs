/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>

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


#ifndef kptyprocesstest_h
#define kptyprocesstest_h

#include <kptyprocess.h>

#include <QtCore/QTimer>

class KPtyProcessTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void test_pty_basic();
    void test_pty_signals();
    void test_ctty();
    void test_shared_pty();
    void test_suspend_pty();

// for pty_signals
public Q_SLOTS:
    void slotReadyRead();
    void slotDoRead();
    void slotReadEof();
    void slotBytesWritten();
    void slotStep();

private:
    KPtyProcess sp;
    QTimer delay;
    QByteArray log;
    int phase;
};

#endif
