/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of 
    the License, or (at your option) version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "loadfakebackend.h"
#include "qtesthelper.h"

#include <QtCore/QObject>
#include <QtGui/QApplication>
#include <QtTest/QTest>

#include "../factory_p.h"
#include <phonon/streaminterface.h>
#include <phonon/mediaobjectinterface.h>
#include <phonon/backendinterface.h>
#include <phonon/audiooutputinterface.h>
#include <phonon/addoninterface.h>
#include <phonon/effectinterface.h>
#include <phonon/videowidgetinterface.h>
#include <phonon/volumefaderinterface.h>

class MethodTest : public QObject
{
    Q_OBJECT
    private Q_SLOTS:
        void initTestCase() { Phonon::loadFakeBackend(); }
        void checkBackendInterface();

//X         void checkAudioDataOutputMethods_data();
//X         void checkAudioDataOutputMethods();
        void checkEffectInterface();
        void checkAudioOutputMethods_data();
        void checkAudioOutputMethods();
        void checkBackendMethods_data();
        void checkBackendMethods();
        void checkMediaObjectMethods_data();
        void checkMediaObjectMethods();
//X         void checkVideoDataOutputMethods_data();
//X         void checkVideoDataOutputMethods();
//X         void checkVisualizationMethods_data();
//X         void checkVisualizationMethods();
        void checkVolumeFaderEffectMethods_data();
        void checkVolumeFaderEffectMethods();
        void checkVideoWidgetMethods_data();
        void checkVideoWidgetMethods();
        void checkMediaObjectInterfaces();
        void checkAudioOutputInterface();

        void checkBackendInfos();

    private:
        void addColumns();
        void addMethod(const char *returnType, const char *signature, bool optional = false);
        void addSignal(const char *signature);
        void checkMethods(QObject *m_backendObject);
};

using namespace Phonon;

void MethodTest::checkBackendInterface()
{
    QVERIFY(qobject_cast<Phonon::BackendInterface *>(Factory::backend()) != 0);
}

//X void MethodTest::checkAudioDataOutputMethods_data() { addColumns();
//X #include "methods/abstractaudiooutput.cpp"
//X #include "../experimental/tests/methods/audiodataoutput.cpp"
//X }
void MethodTest::checkAudioOutputMethods_data() { addColumns();
#include "methods/abstractaudiooutput.cpp"
#include "methods/audiooutput.cpp"
}
void MethodTest::checkBackendMethods_data() { addColumns();
#include "methods/factory.cpp"
#include "methods/objectdescription.cpp"
#include "methods/backendcapabilities.cpp"
}
void MethodTest::checkMediaObjectMethods_data() { addColumns();
#include "methods/mediaobject.cpp"
#include "methods/mediacontroller.cpp"
}
//X void MethodTest::checkVideoDataOutputMethods_data() { addColumns();
//X #include "methods/abstractvideooutput.cpp"
//X #include "../experimental/tests/methods/videodataoutput.cpp"
//X }
//X void MethodTest::checkVisualizationMethods_data() { addColumns();
//X #include "../experimental/tests/methods/visualization.cpp"
//X }
void MethodTest::checkVolumeFaderEffectMethods_data() { addColumns();
#include "methods/volumefadereffect.cpp"
}
void MethodTest::checkVideoWidgetMethods_data() { addColumns();
#include "methods/videowidget.cpp"
}
//X void MethodTest::checkAudioDataOutputMethods()   { checkMethods(Factory::createAudioDataOutput()); }
void MethodTest::checkAudioOutputMethods()       { checkMethods(Factory::createAudioOutput()); }
void MethodTest::checkBackendMethods()           { checkMethods(Factory::backend()); }
void MethodTest::checkMediaObjectMethods()       { checkMethods(Factory::createMediaObject()); }
//X void MethodTest::checkVideoDataOutputMethods()   { checkMethods(Factory::createVideoDataOutput()); }
//X void MethodTest::checkVisualizationMethods()     { checkMethods(Factory::createVisualization()); }

void MethodTest::checkVolumeFaderEffectMethods() {
    QObject *m_backendObject = Factory::createVolumeFaderEffect();
    if (!m_backendObject) {
        QSKIP("The back-end's create method returned 0. No tests possible.", SkipAll);
    }
    //empty: checkMethods(m_backendObject);
    QVERIFY(qobject_cast<Phonon::VolumeFaderInterface *>(m_backendObject) != 0);
    delete m_backendObject;
}

void MethodTest::checkVideoWidgetMethods()       {
    QObject *m_backendObject = Factory::createVideoWidget();
    if (!m_backendObject) {
        QSKIP("The back-end's create method returned 0. No tests possible.", SkipAll);
    }
    //empty: checkMethods(m_backendObject);
    QVERIFY(qobject_cast<Phonon::VideoWidgetInterface *>(m_backendObject) != 0);
    delete m_backendObject;
}

void MethodTest::checkEffectInterface()
{
    QObject *m_backendObject = Factory::createEffect(1);
    if (!m_backendObject) {
        QSKIP("The back-end's create method returned 0. No tests possible.", SkipAll);
    }
    QVERIFY(qobject_cast<Phonon::EffectInterface *>(m_backendObject) != 0);
    delete m_backendObject;
}

void MethodTest::checkMediaObjectInterfaces()
{
    QObject *m_backendObject = Factory::createMediaObject();
    if (!m_backendObject)
        QSKIP("The back-end's create method returned 0. No tests possible.", SkipAll);
    QVERIFY(qobject_cast<Phonon::MediaObjectInterface *>(m_backendObject) != 0);
    QVERIFY(qobject_cast<Phonon::AddonInterface *>(m_backendObject) != 0);
    delete m_backendObject;
}

void MethodTest::checkAudioOutputInterface()
{
    QObject *m_backendObject = Factory::createAudioOutput();
    if (!m_backendObject)
        QSKIP("The back-end's create method returned 0. No tests possible.", SkipAll);
    QVERIFY(qobject_cast<Phonon::AudioOutputInterface *>(m_backendObject) != 0);
    delete m_backendObject;
}

void MethodTest::addColumns()
{
    QTest::addColumn<QByteArray>("returnType");
    QTest::addColumn<QByteArray>("signature");
    QTest::addColumn<bool>("optional");
    QTest::addColumn<bool>("isSignal");
}

void MethodTest::addMethod(const char *returnType, const char *signature, bool optional)
{
    QByteArray name(returnType);
    name += ' ';
    name += signature;
    QTest::newRow(name.constData()) << QByteArray(returnType) << QByteArray(signature) << optional << false;
}

void MethodTest::addSignal(const char *signature)
{
    QTest::newRow(signature) << QByteArray() << QByteArray(signature) << false << true;
}

void MethodTest::checkMethods(QObject *m_backendObject)
{
    if (!m_backendObject)
        QSKIP("The back-end's create method returned 0. No tests possible.", SkipAll);
    const QMetaObject *meta = m_backendObject->metaObject();

    QFETCH(QByteArray, returnType);
    QFETCH(QByteArray, signature);
    QFETCH(bool, optional);
    QFETCH(bool, isSignal);

    if (isSignal) {
        QVERIFY(meta->indexOfSignal(QMetaObject::normalizedSignature(signature.constData())) != -1);
    } else {
        int index = meta->indexOfMethod(QMetaObject::normalizedSignature(signature.constData()));
        if (index == -1 && optional) {
            QWARN("method is not available - default behaviour will be used instead");
        } else {
            if (index == -1) {
                QFAIL(qPrintable(QString("Method %1 not available!").arg(signature.constData())));
            }
            QVERIFY(index != -1);
            QMetaMethod method = meta->method(index);
            QCOMPARE(method.typeName(), returnType.constData());
        }
    }
}

void MethodTest::checkBackendInfos()
{
    QObject *backend = Factory::backend();
    QVERIFY(backend != 0);
    QCOMPARE(backend->property("identifier").type(), QVariant::String);
    QCOMPARE(backend->property("backendName").type(), QVariant::String);
    QCOMPARE(backend->property("backendComment").type(), QVariant::String);
    QCOMPARE(backend->property("backendVersion").type(), QVariant::String);
    QCOMPARE(backend->property("backendIcon").type(), QVariant::String);
    QCOMPARE(backend->property("backendWebsite").type(), QVariant::String);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("methodtest");
    MethodTest tc;
    return QTest::qExec(&tc, argc, argv);
}
#include "methodtest.moc"
// vim: sw=4 ts=4
