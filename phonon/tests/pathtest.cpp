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

#include <QtCore/QDate>
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtTest/QSignalSpy>

#include <phonon/effect.h>
#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <phonon/audiooutput.h>
#include <phonon/backendcapabilities.h>

#include <cstdlib>

class PathTest : public QObject
{
    Q_OBJECT

    private Q_SLOTS:
        void initTestCase();

        void checkPathCreation();
        void checkForDefaults();
        void connectSecondMedia();
        void testEffects();

        void cleanupTestCase();

    private:
        QUrl m_url;
        Phonon::MediaObject *m_media;
        Phonon::Path m_path;
        Phonon::AudioOutput *m_output;
};

using namespace Phonon;

void PathTest::initTestCase()
{
    QCoreApplication::setApplicationName("pathtest");
    Phonon::loadFakeBackend();

    m_media = new MediaObject(this);
    m_media->setCurrentSource(testUrl());
    m_output = new AudioOutput(Phonon::NotificationCategory, this);
    QCOMPARE(m_path.isValid(), false);
    //just checking for crashes
    QCOMPARE(m_path.insertEffect(0,0), false);
    QCOMPARE(m_path.removeEffect(0), false);
}

void PathTest::checkPathCreation()
{
    m_path = createPath(m_media, m_output);
    QCOMPARE(m_path.disconnect(), true);
    QCOMPARE(m_path.isValid(), false);
    m_path.reconnect(m_media, m_output);
    QCOMPARE(m_path.isValid(), true);
    //just checking for crashes
    QCOMPARE(m_path.insertEffect(0,0), false);
    QCOMPARE(m_path.removeEffect(0), false);
}

void PathTest::checkForDefaults()
{
    QCOMPARE(m_path.effects().size(), 0);
}

void PathTest::connectSecondMedia()
{
    MediaObject *media2 = new MediaObject(this);
    media2->setCurrentSource(testUrl());
    Path p = createPath(media2, m_output);
    QCOMPARE(p.isValid(), false);
}

void PathTest::testEffects()
{
    QList<EffectDescription> list = BackendCapabilities::availableAudioEffects();
    if (list.isEmpty()) {
        QWARN("No effects available -> no tests");
        return;
    }
    Effect *e1 = m_path.insertEffect(list.first());
    QVERIFY(e1 != 0);
    QVERIFY(e1->isValid());
    QCOMPARE(m_path.effects().size(), 1);
    QCOMPARE(m_path.effects().first(), e1);
    delete e1;
    QCOMPARE(m_path.effects().size(), 0);

    {
        MediaObject *m = new MediaObject;
        AudioOutput *o = new AudioOutput(Phonon::MusicCategory);
        e1 = new Effect(list.first());
        QVERIFY(e1 != 0);
        {
            Path p = Phonon::createPath(m, o);
            QCOMPARE(p.isValid(), true);
            p.insertEffect(e1);
            QCOMPARE(p.effects().size(), 1);
            delete o;
            QCOMPARE(p.isValid(), false);
            QCOMPARE(p.effects().size(), 1);
        }
        delete m; // deletes the last ref to PathPrivate so that it get's deleted
        delete e1; // if this doesn't crash then all refs to Path(Private) were correctly cleaned up
    }
    {
        MediaObject *m = new MediaObject;
        AudioOutput *o = new AudioOutput(Phonon::MusicCategory);
        e1 = new Effect(list.first());
        QVERIFY(e1 != 0);
        {
            Path p = Phonon::createPath(m, o);
            QCOMPARE(p.isValid(), true);
            p.insertEffect(e1);
            QCOMPARE(p.effects().size(), 1);
            delete o;
            QCOMPARE(p.isValid(), false);
            o = new AudioOutput(Phonon::MusicCategory);
            QVERIFY(p.reconnect(m, o));
            QCOMPARE(p.isValid(), true);
            QCOMPARE(p.effects().size(), 1);
            QCOMPARE(p.effects().first(), e1);
        }
        delete o;
        delete m; // deletes the last ref to PathPrivate so that it get's deleted
        delete e1; // if this doesn't crash then all refs to Path(Private) were correctly cleaned up
    }
    { //let's test the deletion of the source node
        MediaObject *m = new MediaObject;
        AudioOutput *o = new AudioOutput(Phonon::MusicCategory);
        e1 = new Effect(list.first());
        QVERIFY(e1 != 0);
        {
            Path p = Phonon::createPath(m, o);
            QCOMPARE(p.isValid(), true);
            p.insertEffect(e1);
            QCOMPARE(p.effects().size(), 1);
            delete m;
            QCOMPARE(p.isValid(), false);
            m = new MediaObject;
            QVERIFY(p.reconnect(m, o));
            QCOMPARE(p.isValid(), true);
            QCOMPARE(p.effects().size(), 1);
            QCOMPARE(p.effects().first(), e1);
        }
        delete o;
        delete m; // deletes the last ref to PathPrivate so that it get's deleted
        delete e1; // if this doesn't crash then all refs to Path(Private) were correctly cleaned up
    }
}

void PathTest::cleanupTestCase()
{
    delete m_media;
    QCOMPARE(m_path.isValid(), false);
}

QTEST_MAIN(PathTest)
#include "pathtest.moc"
