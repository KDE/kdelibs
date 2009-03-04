/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#ifndef KSERVICETEST_H
#define KSERVICETEST_H

#include <QAtomicInt>
#include <QtCore/QObject>

class KServiceTest : public QObject
{
    Q_OBJECT
public:
    KServiceTest() : m_sycocaUpdateDone(0) {}
private Q_SLOTS:
    void initTestCase();
    void testByName();
    void testProperty();
    void testAllServiceTypes();
    void testAllServices();
    void testServiceTypeTraderForReadOnlyPart();
    void testTraderConstraints();
    void testHasServiceType1();
    void testHasServiceType2();
    void testWriteServiceTypeProfile();
    void testDefaultOffers();
    void testDeleteServiceTypeProfile();
    void testDBUSStartupType();
    void testByStorageId();
    void testActionsAndDataStream();
    void testServiceGroups();
    void testKSycocaUpdate();
    void testReaderThreads();
    void testThreads();

private:
    void createFakeService();
    QString m_firstOffer;
    bool m_hasKde4Konsole;
    QAtomicInt m_sycocaUpdateDone;
};

#endif
