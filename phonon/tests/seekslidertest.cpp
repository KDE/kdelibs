/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#include "seekslidertest.h"
#include "loadfakebackend.h"
#include "../seekslider.h"
#include "../mediaobject.h"
#include "../audiopath.h"
#include "../audiooutput.h"

#include <QtGui/QLabel>
#include <QtGui/QSlider>

#include <qtest_kde.h>

#include <cstdlib>

using namespace Phonon;

void SeekSliderTest::initTestCase()
{
    Phonon::loadFakeBackend();
    ss = new SeekSlider;
    QVERIFY(ss != 0);
    qslider = ss->findChild<QSlider *>();
    qlabel = ss->findChild<QLabel *>();
    QVERIFY(qslider != 0);
    QVERIFY(qlabel != 0);
    media = new MediaObject(this);
    AudioPath *ap = new AudioPath(media);
    AudioOutput *ao = new AudioOutput(Phonon::MusicCategory, media);
    media->addAudioPath(ap);
    ap->addOutput(ao);
}

void SeekSliderTest::testEnabled()
{
    QVERIFY(!qslider->isEnabled());
    ss->setMediaObject(0);
    QVERIFY(!qslider->isEnabled());
}

void SeekSliderTest::setMedia()
{
    QVERIFY(media->state() == Phonon::LoadingState);
    QUrl url(testUrl());
    media->setCurrentSource(url);
    ss->setMediaObject(media);
    QVERIFY(!qslider->isEnabled());
    ss->setMediaObject(0);
    QVERIFY(!qslider->isEnabled());
    ss->setMediaObject(media);
    QVERIFY(!qslider->isEnabled());
}

void SeekSliderTest::playMedia()
{
    media->play();
    QSignalSpy stateSpy(media, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
    while (media->state() != Phonon::PlayingState) {
        QVERIFY(QTest::kWaitForSignal(media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), 4000));
        QVERIFY(!stateSpy.isEmpty());
        switch (qvariant_cast<Phonon::State>(stateSpy.last().first())) {
        case Phonon::PlayingState:
        case Phonon::PausedState:
        case Phonon::BufferingState:
            QVERIFY(qslider->isEnabled());
            ss->setMediaObject(0);
            QVERIFY(!qslider->isEnabled());
            ss->setMediaObject(media);
            QVERIFY(qslider->isEnabled());
            break;
        case Phonon::ErrorState:
        case Phonon::StoppedState:
        case Phonon::LoadingState:
            QVERIFY(!qslider->isEnabled());
            break;
        }
    }
}

void SeekSliderTest::seekWithSlider()
{
    // click on the slider to seek
}

void SeekSliderTest::cleanupTestCase()
{
    delete media;
    qslider = 0;
    delete ss;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("seekslidertest");
    SeekSliderTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "seekslidertest.moc"
// vim: ts=4
