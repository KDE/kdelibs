/* This file is part of the KDE libraries
   Copyright 2010 Canonical Ltd
   Author: Aurélien Gâteau <aurelien.gateau@canonical.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef TEST_KCONF_UPDATE_H
#define TEST_KCONF_UPDATE_H

#include <QObject>

class TestKConfUpdate : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test_data();
    void test();
    void testScript_data();
    void testScript();
};

#endif /* TEST_KCONF_UPDATE_H */
