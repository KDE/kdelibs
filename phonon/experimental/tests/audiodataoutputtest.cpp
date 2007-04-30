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

#include "audiodataoutputtest.h"
#include "../audiodataoutput.h"
#include "../../mediaobject.h"
#include "../../audiopath.h"
#include "../../audiooutput.h"
#include <qtest_kde.h>
#include <cstdlib>
#include <kurl.h>
#include "../../tests/loadfakebackend.h"

using namespace Phonon;
using namespace Phonon::Experimental;

void AudioDataOutputTest::initTestCase()
{
    Phonon::loadFakeBackend();
    AudioDataOutput test(this);
}

void AudioDataOutputTest::testSampleRate()
{
    AudioDataOutput test(this);
    QVERIFY(test.sampleRate() > 0);
}

void AudioDataOutputTest::testFormat()
{
    qRegisterMetaType<QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<float> > >
        ("QMap<Phonon::Experimental::AudioDataOutput::Channel,QVector<float> >");
    qRegisterMetaType<QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<qint16> > >
        ("QMap<Phonon::Experimental::AudioDataOutput::Channel,QVector<qint16> >");
    AudioDataOutput test(this);
    QSignalSpy floatReadySpy(&test, SIGNAL(dataReady(const QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<float> > &)));
    QSignalSpy intReadySpy(&test, SIGNAL(dataReady(const QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<qint16> > &)));
    QCOMPARE(floatReadySpy.size(), 0);
    QCOMPARE(intReadySpy.size(), 0);
    QCOMPARE(test.format(), Phonon::Experimental::AudioDataOutput::IntegerFormat);
    test.setFormat(Phonon::Experimental::AudioDataOutput::IntegerFormat);
    QCOMPARE(test.format(), Phonon::Experimental::AudioDataOutput::IntegerFormat);
    test.setFormat(Phonon::Experimental::AudioDataOutput::FloatFormat);
    QCOMPARE(test.format(), Phonon::Experimental::AudioDataOutput::FloatFormat);
    test.setFormat(Phonon::Experimental::AudioDataOutput::FloatFormat);
    QCOMPARE(test.format(), Phonon::Experimental::AudioDataOutput::FloatFormat);
    test.setFormat(Phonon::Experimental::AudioDataOutput::IntegerFormat);
    QCOMPARE(test.format(), Phonon::Experimental::AudioDataOutput::IntegerFormat);

    MediaObject media(this);
    KUrl url(getenv("PHONON_TESTURL"));
    media.setCurrentSource(url);
    AudioPath path(this);
    media.addAudioPath(&path);
    path.addOutput(&test);
    QVERIFY(media.audioPaths().contains(&path));
    QVERIFY(path.outputs().contains(&test));

    QCOMPARE(floatReadySpy.size(), 0);
    QCOMPARE(intReadySpy.size(), 0);

    media.play();
    QTime startTime;
    startTime.start();
    while (startTime.elapsed() < 1000)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QVERIFY(intReadySpy.size() > 0);
    QCOMPARE(floatReadySpy.size(), 0);

    media.pause();
    test.setFormat(Phonon::Experimental::AudioDataOutput::FloatFormat);
    QCOMPARE(test.format(), Phonon::Experimental::AudioDataOutput::FloatFormat);
    intReadySpy.clear();
    media.play();
    startTime.start();
    while (startTime.elapsed() < 1000)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QVERIFY(floatReadySpy.size() > 0);
    QCOMPARE(intReadySpy.size(), 0);

    media.pause();
    test.setFormat(Phonon::Experimental::AudioDataOutput::IntegerFormat);
    QCOMPARE(test.format(), Phonon::Experimental::AudioDataOutput::IntegerFormat);
    floatReadySpy.clear();
    media.play();
    startTime.start();
    while (startTime.elapsed() < 1000)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QVERIFY(intReadySpy.size() > 0);
    QCOMPARE(floatReadySpy.size(), 0);
}

void AudioDataOutputTest::testDataSize()
{
    AudioDataOutput test(this);
    QVERIFY(test.dataSize() > 0);
}

void AudioDataOutputTest::cleanupTestCase()
{
}

QTEST_KDEMAIN(AudioDataOutputTest, NoGUI)
#include "audiodataoutputtest.moc"
// vim: sw=4 ts=4
