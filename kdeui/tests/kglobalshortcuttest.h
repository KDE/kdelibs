/*
    This file is part of the KDE libraries

    Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#ifndef KGLOBALSHORTCUTTEST_H
#define KGLOBALSHORTCUTTEST_H

#include <QtCore/QObject>

class KAction;

class KGlobalShortcutTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSetShortcut();
    void testFindActionByKey();
    void testChangeShortcut();
    void testStealShortcut();
    void testSaveRestore();
    void testListActions();
    void testComponentAssignment();
    void testConfigurationActions();
    void testNotification();
    // This has to be the last before forgetGlobalShortcut
    void testOverrideMainComponentData();
    void testForgetGlobalShortcut();

public:

    KGlobalShortcutTest() : m_actionA(0) ,m_actionB(0)
        {}

private:
    void setupTest(QString id);

    KAction *m_actionA;
    KAction *m_actionB;
};

#endif
