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

#include "loadfakebackend.h"
#include "../volumeslider.h"
#include "../mediaobject.h"
#include "../audiooutput.h"

#include <QtCore/QObject>
#include <QtGui/QSlider>
#include <QtGui/QToolButton>

#include <qtest_kde.h>

#include <cstdlib>

class VolumeSliderTest : public QObject
{
    Q_OBJECT
    private Q_SLOTS:
        void initTestCase();
        void testEnabled();
        void seekWithSlider();
        void cleanupTestCase();
    private:
        Phonon::VolumeSlider *vs;
        QSlider *qslider;
        QToolButton *qbutton;
        Phonon::MediaObject *media;
        Phonon::AudioOutput *output;
};

using namespace Phonon;

void VolumeSliderTest::initTestCase()
{
    Phonon::loadFakeBackend();
    vs = new VolumeSlider;
    QVERIFY(vs != 0);
    qslider = vs->findChild<QSlider *>();
    qbutton = vs->findChild<QToolButton *>();
    QVERIFY(qslider != 0);
    QVERIFY(qbutton != 0);
    media = new MediaObject(this);
    output = new AudioOutput(MusicCategory, this);
}

void VolumeSliderTest::testEnabled()
{
    QVERIFY(!qslider->isEnabled());
    vs->setAudioOutput(0);
    QVERIFY(!qslider->isEnabled());
    vs->setAudioOutput(output);
    QVERIFY(qslider->isEnabled());
    vs->setAudioOutput(0);
    QVERIFY(!qslider->isEnabled());
    vs->setAudioOutput(output);
    QVERIFY(qslider->isEnabled());
}

void VolumeSliderTest::seekWithSlider()
{
    // click on the slider to seek
}

void VolumeSliderTest::cleanupTestCase()
{
    delete media;
    delete output;
    qslider = 0;
    qbutton = 0;
    delete vs;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("volumeslidertest");
    VolumeSliderTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "volumeslidertest.moc"
// vim: ts=4
