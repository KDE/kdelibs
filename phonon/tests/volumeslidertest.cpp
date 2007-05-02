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

#include <qtest_kde.h>

#include "volumeslidertest.h"
#include "../volumeslider.h"
#include "../mediaobject.h"
#include "../audiooutput.h"
#include "loadfakebackend.h"
#include <QtGui/QSlider>
#include <kurl.h>
#include <cstdlib>
#include <QtGui/QToolButton>

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

QTEST_KDEMAIN(VolumeSliderTest, GUI)
#include "volumeslidertest.moc"
// vim: ts=4
