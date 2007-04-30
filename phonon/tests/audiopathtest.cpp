/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include "audiopathtest.h"
#include "loadfakebackend.h"

#include <cstdlib>
#include <qtest_kde.h>
#include <QTime>
#include <QtDebug>

using namespace Phonon;

void AudioPathTest::initTestCase()
{
    Phonon::loadFakeBackend();

    //m_url.setUrl(getenv("PHONON_TESTURL"));
    //if (!m_url.isValid())
        //QFAIL("You need to set PHONON_TESTURL to a valid URL");

    //m_media = new MediaObject(this);
    m_path = new AudioPath(this);
    m_output = new AudioOutput(Phonon::NotificationCategory, this);

    //m_media->setUrl(m_url);
}

void AudioPathTest::checkForDefaults()
{
    QCOMPARE(m_path->effects().size(), 0);
    QCOMPARE(m_path->outputs().size(), 0);
}

void AudioPathTest::addOutputs()
{
    //this is the first output, this has to work:
    QCOMPARE(m_path->addOutput(m_output), true);
    QCOMPARE(m_path->outputs().size(), 1);
    QVERIFY(m_path->outputs().contains(m_output));
    AudioOutput *o2 = new AudioOutput(Phonon::NotificationCategory, this);
    if (m_path->addOutput(o2))
    {
        QCOMPARE(m_path->outputs().size(), 2);
        QVERIFY(m_path->outputs().contains(m_output));
        QVERIFY(m_path->outputs().contains(o2));

        QCOMPARE(m_path->removeOutput(o2), true);
        QCOMPARE(m_path->outputs().size(), 1);
        QVERIFY(m_path->outputs().contains(m_output));
        QVERIFY(!m_path->outputs().contains(o2));

        QCOMPARE(m_path->addOutput(o2), true);
        QCOMPARE(m_path->outputs().size(), 2);
        QVERIFY(m_path->outputs().contains(m_output));
        QVERIFY(m_path->outputs().contains(o2));

        delete o2;
        QCOMPARE(m_path->outputs().size(), 1);
        QVERIFY(m_path->outputs().contains(m_output));
        QVERIFY(!m_path->outputs().contains(o2));
    }
    else
    {
        QWARN("AudioPath not able to use more than one AudioOutput");
        delete o2;
    }
}

void AudioPathTest::addEffects()
{
    QWARN("AudioEffects not implemented");
}

void AudioPathTest::cleanupTestCase()
{
    delete m_output;
    delete m_path;
    //delete m_media;
}

QTEST_KDEMAIN_CORE(AudioPathTest)
#include "audiopathtest.moc"
// vim: sw=4 ts=4
