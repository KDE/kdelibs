/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "loadfakebackend.h"

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QStringList>

#include <QtTest/QTest>

#include "../backendcapabilities.h"
#include "../factory_p.h"
#include "../objectdescription.h"
#include "../backendinterface.h"

class BackendCapabilitiesTest : public QObject
{
    Q_OBJECT
    private Q_SLOTS:
        void initTestCase();
        void checkMimeTypes();
        void sensibleValues();
        void checkSignals();
        void cleanupTestCase();
};

using namespace Phonon;

void BackendCapabilitiesTest::initTestCase()
{
    QVERIFY(BackendCapabilities::notifier());
}

void BackendCapabilitiesTest::checkMimeTypes()
{
    QVERIFY(Factory::backend(false) == 0);
    QString mimeType("foobar/x-nonexistent");
    QVERIFY(Factory::backend(false) == 0); // the backend should not have been created at this point
    QVERIFY(!BackendCapabilities::isMimeTypeAvailable(mimeType));
    QVERIFY(Factory::backend(false) == 0); // the backend should not have been created at this point
    QStringList mimeTypes = BackendCapabilities::availableMimeTypes();
    QVERIFY(Factory::backend(false) != 0); // the backend should have been created at this point
    QVERIFY(mimeTypes.size() > 0); // a backend that doesn't know any mimetypes is useless
    foreach (const QString &mimeType, mimeTypes) {
        qDebug("%s", qPrintable(mimeType));
        QVERIFY(BackendCapabilities::isMimeTypeAvailable(mimeType));
    }
}

#define VERIFY_TUPLE(T) \
QVERIFY(BackendCapabilities::available##T##s().size() >= 0); \
do { \
    for (int i = 0; i < BackendCapabilities::available##T##s().size(); ++i) { \
        ObjectDescription<T ## Type> device = BackendCapabilities::available##T##s().at(i); \
        QVERIFY(device.index() >= 0); \
        QList<int> indexes = iface->objectDescriptionIndexes(Phonon::T##Type);\
        QVERIFY(indexes.contains(device.index())); \
        QVERIFY(!device.name().isEmpty()); \
    } \
} while(false)

void BackendCapabilitiesTest::sensibleValues()
{
    BackendInterface *iface = qobject_cast<BackendInterface *>(Factory::backend());
    //if (BackendCapabilities::supportsVideo()) create VideoWidget and such - needs UI libs
    VERIFY_TUPLE(AudioOutputDevice);
    /*
    VERIFY_TUPLE(AudioCaptureDevice);
    VERIFY_TUPLE(VideoOutputDevice);
    VERIFY_TUPLE(VideoCaptureDevice);
    VERIFY_TUPLE(Visualization);
    VERIFY_TUPLE(AudioCodec);
    VERIFY_TUPLE(VideoCodec);
    VERIFY_TUPLE(ContainerFormat);
    */

    for (int i = 0; i < BackendCapabilities::availableAudioEffects().size(); ++i) {
        ObjectDescription<EffectType> device = BackendCapabilities::availableAudioEffects().at(i);
        QVERIFY(device.index() >= 0);
        QList<int> indexes = iface->objectDescriptionIndexes(Phonon::EffectType);
        QVERIFY(indexes.contains(device.index()));
        QVERIFY(!device.name().isEmpty());
    }
}

void BackendCapabilitiesTest::checkSignals()
{
    QSignalSpy spy(BackendCapabilities::notifier(), SIGNAL(capabilitiesChanged()));
    QCOMPARE(spy.size(), 0);
    QMetaObject::invokeMethod(Factory::sender(), "phononBackendChanged", Qt::DirectConnection);
    QCOMPARE(spy.size(), 1);
}

void BackendCapabilitiesTest::cleanupTestCase()
{
}

QTEST_MAIN(BackendCapabilitiesTest)
#include "backendcapabilitiestest.moc"
// vim: sw=4 ts=4
