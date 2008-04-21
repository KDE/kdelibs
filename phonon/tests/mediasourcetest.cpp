/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#include "../mediasource.h"
#include "../abstractmediastream.h"
#include "loadfakebackend.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtCore/QBuffer>
#include <QtCore/QObject>
#include <QtTest/QTest>

class MediaSourceTest : public QObject
{
    Q_OBJECT
    private Q_SLOTS:
        void initTestCase();
        void testLocalFile();
        void testUrl();
        void testDiscType();
        void testStream();
        void testIODevice();
        void testQtResource();
        void cleanupTestCase();
};

using namespace Phonon;

void MediaSourceTest::initTestCase()
{
    // no need for a backend at all
    // Phonon::loadFakeBackend();
}

void MediaSourceTest::testLocalFile()
{
    QString filename("/usr/share/sounds/KDE_Beep.ogg");
    AbstractMediaStream *stream = 0;

    MediaSource a(filename);
    QCOMPARE(a.type(), MediaSource::LocalFile);
    QCOMPARE(a.fileName(), filename);
    QCOMPARE(a.url(), QUrl::fromLocalFile(filename));
    QCOMPARE(a.discType(), Phonon::NoDisc);
    QCOMPARE(a.stream(), stream);
    QCOMPARE(a.deviceName(), QString());
    //QCOMPARE(a.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(a.videoCaptureDevice(), VideoCaptureDevice());
    MediaSource b(a);
    MediaSource c;
    c = a;
    QCOMPARE(a, b);
    QCOMPARE(a, c);
    QCOMPARE(b, c);

    QCOMPARE(b.type(), MediaSource::LocalFile);
    QCOMPARE(b.fileName(), filename);
    QCOMPARE(b.url(), QUrl::fromLocalFile(filename));
    QCOMPARE(b.discType(), Phonon::NoDisc);
    QCOMPARE(b.stream(), stream);
    QCOMPARE(b.deviceName(), QString());
    //QCOMPARE(b.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(b.videoCaptureDevice(), VideoCaptureDevice());

    QCOMPARE(c.type(), MediaSource::LocalFile);
    QCOMPARE(c.fileName(), filename);
    QCOMPARE(c.url(), QUrl::fromLocalFile(filename));
    QCOMPARE(c.discType(), Phonon::NoDisc);
    QCOMPARE(c.stream(), stream);
    QCOMPARE(c.deviceName(), QString());
    //QCOMPARE(c.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(c.videoCaptureDevice(), VideoCaptureDevice());

    // non-existing files should become invalid sources
    filename = "/some/invalid/file.xyz";
    MediaSource invalid(filename);
    QCOMPARE(invalid.type(), MediaSource::Invalid);
    QCOMPARE(invalid.fileName(), QString());

    //test that a relative file path is correctly set as an absolute URL
    QFile testFile("foo.ogg");
    bool deleteFile = false;
    if (!testFile.exists()) {
        deleteFile = true;
        testFile.open(QIODevice::WriteOnly);
        testFile.close();
    }
    filename = "foo.ogg";
    MediaSource relative(filename);
    //QCOMPARE(relative.fileName(), filename);
    QFileInfo urlInfo(relative.url().toLocalFile());
    QVERIFY(urlInfo.isAbsolute());
    QCOMPARE(urlInfo.fileName(), filename);
    QCOMPARE(urlInfo.absolutePath(), QDir::currentPath());
    if (deleteFile) {
        testFile.remove();
    }
}

void MediaSourceTest::testUrl()
{
    QUrl url("http://www.example.com/music.ogg");
    AbstractMediaStream *stream = 0;

    MediaSource a(url);
    QCOMPARE(a.type(), MediaSource::Url);
    QCOMPARE(a.fileName(), QString());
    QCOMPARE(a.url(), url);
    QCOMPARE(a.discType(), Phonon::NoDisc);
    QCOMPARE(a.stream(), stream);
    QCOMPARE(a.deviceName(), QString());
    //QCOMPARE(a.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(a.videoCaptureDevice(), VideoCaptureDevice());
    MediaSource b(a);
    MediaSource c;
    c = a;
    QCOMPARE(a, b);
    QCOMPARE(a, c);
    QCOMPARE(b, c);

    QCOMPARE(b.type(), MediaSource::Url);
    QCOMPARE(b.fileName(), QString());
    QCOMPARE(b.url(), url);
    QCOMPARE(b.discType(), Phonon::NoDisc);
    QCOMPARE(b.stream(), stream);
    QCOMPARE(b.deviceName(), QString());
    //QCOMPARE(b.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(b.videoCaptureDevice(), VideoCaptureDevice());

    QCOMPARE(c.type(), MediaSource::Url);
    QCOMPARE(c.fileName(), QString());
    QCOMPARE(c.url(), url);
    QCOMPARE(c.discType(), Phonon::NoDisc);
    QCOMPARE(c.stream(), stream);
    QCOMPARE(c.deviceName(), QString());
    //QCOMPARE(c.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(c.videoCaptureDevice(), VideoCaptureDevice());
}

void MediaSourceTest::testDiscType()
{
    for (int i = 0; i <= Phonon::Vcd; ++i) {
        Phonon::DiscType discType = static_cast<Phonon::DiscType>(i);
        AbstractMediaStream *stream = 0;

        MediaSource a(discType);

        QCOMPARE(a.type(), MediaSource::Disc);
        QCOMPARE(a.fileName(), QString());
        QCOMPARE(a.url(), QUrl());
        QCOMPARE(a.discType(), discType);
        QCOMPARE(a.stream(), stream);
        QCOMPARE(a.deviceName(), QString());
        //QCOMPARE(a.audioCaptureDevice(), AudioCaptureDevice());
        //QCOMPARE(a.videoCaptureDevice(), VideoCaptureDevice());
        MediaSource b(a);
        MediaSource c;
        c = a;
        QCOMPARE(a, b);
        QCOMPARE(a, c);
        QCOMPARE(b, c);

        QCOMPARE(b.type(), MediaSource::Disc);
        QCOMPARE(b.fileName(), QString());
        QCOMPARE(b.url(), QUrl());
        QCOMPARE(b.discType(), discType);
        QCOMPARE(b.stream(), stream);
        QCOMPARE(b.deviceName(), QString());
        //QCOMPARE(b.audioCaptureDevice(), AudioCaptureDevice());
        //QCOMPARE(b.videoCaptureDevice(), VideoCaptureDevice());

        QCOMPARE(c.type(), MediaSource::Disc);
        QCOMPARE(c.fileName(), QString());
        QCOMPARE(c.url(), QUrl());
        QCOMPARE(c.discType(), discType);
        QCOMPARE(c.stream(), stream);
        QCOMPARE(c.deviceName(), QString());
        //QCOMPARE(c.audioCaptureDevice(), AudioCaptureDevice());
        //QCOMPARE(c.videoCaptureDevice(), VideoCaptureDevice());
    }
}

class Stream : public AbstractMediaStream
{
    public:
        void reset() {}
        void needData() {}
        void enoughData() {}
};

void MediaSourceTest::testStream()
{
    AbstractMediaStream *stream = new Stream;

    MediaSource a(stream);
    QCOMPARE(a.type(), MediaSource::Stream);
    QCOMPARE(a.fileName(), QString());
    QCOMPARE(a.url(), QUrl());
    QCOMPARE(a.discType(), Phonon::NoDisc);
    QCOMPARE(a.stream(), stream);
    QCOMPARE(a.deviceName(), QString());
    //QCOMPARE(a.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(a.videoCaptureDevice(), VideoCaptureDevice());
    MediaSource b(a);
    MediaSource c;
    c = a;
    QCOMPARE(a, b);
    QCOMPARE(a, c);
    QCOMPARE(b, c);

    QCOMPARE(b.type(), MediaSource::Stream);
    QCOMPARE(b.fileName(), QString());
    QCOMPARE(b.url(), QUrl());
    QCOMPARE(b.discType(), Phonon::NoDisc);
    QCOMPARE(b.stream(), stream);
    QCOMPARE(b.deviceName(), QString());
    //QCOMPARE(b.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(b.videoCaptureDevice(), VideoCaptureDevice());

    QCOMPARE(c.type(), MediaSource::Stream);
    QCOMPARE(c.fileName(), QString());
    QCOMPARE(c.url(), QUrl());
    QCOMPARE(c.discType(), Phonon::NoDisc);
    QCOMPARE(c.stream(), stream);
    QCOMPARE(c.deviceName(), QString());
    //QCOMPARE(c.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(c.videoCaptureDevice(), VideoCaptureDevice());

    delete stream;
    QCOMPARE(a.type(), MediaSource::Invalid);
    QCOMPARE(b.type(), MediaSource::Invalid);
    QCOMPARE(c.type(), MediaSource::Invalid);
    const AbstractMediaStream *null = 0;
    QCOMPARE(a.stream(), null);
    QCOMPARE(b.stream(), null);
    QCOMPARE(c.stream(), null);
}

void MediaSourceTest::testIODevice()
{
    const QByteArray data("0192380");
    QBuffer *buffer = new QBuffer;
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);

    MediaSource a(buffer);
    QCOMPARE(a.type(), MediaSource::Stream);
    QCOMPARE(a.fileName(), QString());
    QCOMPARE(a.url(), QUrl());
    QCOMPARE(a.discType(), Phonon::NoDisc);
    QVERIFY(a.stream() != 0);
    QCOMPARE(a.deviceName(), QString());
    //QCOMPARE(a.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(a.videoCaptureDevice(), VideoCaptureDevice());
    MediaSource b(a);
    MediaSource c;
    c = a;
    QCOMPARE(a, b);
    QCOMPARE(a, c);
    QCOMPARE(b, c);

    QCOMPARE(b.type(), MediaSource::Stream);
    QCOMPARE(b.fileName(), QString());
    QCOMPARE(b.url(), QUrl());
    QCOMPARE(b.discType(), Phonon::NoDisc);
    QVERIFY(b.stream() != 0);
    QCOMPARE(b.deviceName(), QString());
    //QCOMPARE(b.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(b.videoCaptureDevice(), VideoCaptureDevice());

    QCOMPARE(c.type(), MediaSource::Stream);
    QCOMPARE(c.fileName(), QString());
    QCOMPARE(c.url(), QUrl());
    QCOMPARE(c.discType(), Phonon::NoDisc);
    QVERIFY(c.stream() != 0);
    QCOMPARE(c.deviceName(), QString());
    //QCOMPARE(c.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(c.videoCaptureDevice(), VideoCaptureDevice());

    delete buffer;
    QCOMPARE(a.type(), MediaSource::Invalid);
    QCOMPARE(b.type(), MediaSource::Invalid);
    QCOMPARE(c.type(), MediaSource::Invalid);
    const AbstractMediaStream *null = 0;
    QCOMPARE(a.stream(), null);
    QCOMPARE(b.stream(), null);
    QCOMPARE(c.stream(), null);
}

void MediaSourceTest::testQtResource()
{
    const QString filename(":/ogg/zero.ogg");
    MediaSource a(filename);
    QCOMPARE(a.type(), MediaSource::Stream);
    QCOMPARE(a.fileName(), QString());
    QCOMPARE(a.url(), QUrl());
    QCOMPARE(a.discType(), Phonon::NoDisc);
    QVERIFY(a.stream() != 0);
    QCOMPARE(a.deviceName(), QString());
    //QCOMPARE(a.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(a.videoCaptureDevice(), VideoCaptureDevice());
    MediaSource b(a);
    MediaSource c;
    c = a;
    QCOMPARE(a, b);
    QCOMPARE(a, c);
    QCOMPARE(b, c);

    QCOMPARE(b.type(), MediaSource::Stream);
    QCOMPARE(b.fileName(), QString());
    QCOMPARE(b.url(), QUrl());
    QCOMPARE(b.discType(), Phonon::NoDisc);
    QVERIFY(b.stream() != 0);
    QCOMPARE(b.deviceName(), QString());
    //QCOMPARE(b.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(b.videoCaptureDevice(), VideoCaptureDevice());

    QCOMPARE(c.type(), MediaSource::Stream);
    QCOMPARE(c.fileName(), QString());
    QCOMPARE(c.url(), QUrl());
    QCOMPARE(c.discType(), Phonon::NoDisc);
    QVERIFY(c.stream() != 0);
    QCOMPARE(c.deviceName(), QString());
    //QCOMPARE(c.audioCaptureDevice(), AudioCaptureDevice());
    //QCOMPARE(c.videoCaptureDevice(), VideoCaptureDevice());
}

void MediaSourceTest::cleanupTestCase()
{
}

QTEST_APPLESS_MAIN(MediaSourceTest)

#include "mediasourcetest.moc"
// vim: sw=4 sts=4 et tw=100
