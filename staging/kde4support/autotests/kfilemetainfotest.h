/* This file is part of the KDE project
   Copyright (C) 2010 Peter Penz <peter.penz19@gmail.com>

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

#ifndef KFILEMETAINFOTEST_H
#define KFILEMETAINFOTEST_H

#include <qtemporarydir.h>
#include <QtCore/QEventLoop>
#include <QtCore/QObject>

class KFileMetaInfoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testMetaInfo();
    void testReentrancy();

protected Q_SLOTS:
    void exitLoop();

private:
    int m_exitCount;
    QEventLoop m_eventLoop;
    QTemporaryDir m_tempDir;
};

#endif
