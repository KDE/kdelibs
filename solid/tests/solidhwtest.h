/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLIDHWTEST_H
#define SOLIDHWTEST_H

#include <QObject>
#include <QMap>

class FakeManager;

class SolidHwTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void testAllDevices();
    void testDeviceExists();
    void testDeviceBasicFeatures();
    void testDeviceLocking();
    void testManagerSignals();
    void testDeviceSignals();
    void testCapabilityIntrospection_data();
    void testCapabilityIntrospection();
    void testCapabilityIntrospectionCornerCases();
    void testDeviceCapabilities();
    void testPredicate();

    void slotPropertyChanged( const QMap<QString,int> &changes );
private:
    FakeManager *fakeManager;
    QList< QMap<QString,int> > m_changesList;
};

#endif
