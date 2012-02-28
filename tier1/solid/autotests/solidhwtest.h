/*
    Copyright 2005 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLIDHWTEST_H
#define SOLIDHWTEST_H

#include <QtCore/QObject>
#include <QtCore/QMap>

namespace Solid {
    namespace Backends {
        namespace Fake {
            class FakeManager;
        }
    }
}

class SolidHwTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void testAllDevices();
    void testDeviceBasicFeatures();
    void testManagerSignals();
    void testDeviceSignals();
    void testDeviceExistence();
    void testDeviceInterfaceIntrospection_data();
    void testDeviceInterfaceIntrospection();
    void testDeviceInterfaceIntrospectionCornerCases();
    void testDeviceInterfaces();
    void testPredicate();
    void testSetupTeardown();

    void slotPropertyChanged(const QMap<QString,int> &changes);
private:
    Solid::Backends::Fake::FakeManager *fakeManager;
    QList< QMap<QString,int> > m_changesList;
};

#endif
