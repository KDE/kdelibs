/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#include "bytestreamtest.h"
#include "qtesthelper.h"
#include <cstdlib>
#include <qtest_kde.h>
#include <QtCore/QDate>
#include <QtCore/QDebug>
#include <phonon/videopath.h>
#include <kio/job.h>
#include <kio/filejob.h>
#include <kprotocolmanager.h>
#include <unistd.h>

#include "loadfakebackend.h"

const qint64 ALLOWED_TIME_FOR_SEEKING = 1000; // 1s
const qint64 ALLOWED_SEEK_INACCURACY = 300; // 0.3s
const qint64 ALLOWED_TICK_INACCURACY = 350; // allow +/- 350 ms inaccuracy

using namespace Phonon;

static qint64 castQVariantToInt64(const QVariant &variant)
{
    return *reinterpret_cast<const qint64 *>(variant.constData());
}

static qint32 castQVariantToInt32(const QVariant &variant)
{
    return *reinterpret_cast<const qint32 *>(variant.constData());
}

void ByteStreamTest::init()
{
    initByteStream();
    initOutput();
    setMedia();
}

void ByteStreamTest::cleanup()
{
    m_stateChangedSignalSpy->clear();
}

void ByteStreamTest::startPlayback(Phonon::State currentState)
{
    m_stateChangedSignalSpy->clear();
    Phonon::State s = m_media->state();
    QCOMPARE(s, currentState);
    m_media->play();
    while (s != Phonon::PlayingState) {
        if (m_stateChangedSignalSpy->isEmpty()) {
            waitForSignal(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
        }
        while (!m_stateChangedSignalSpy->isEmpty()) {
            QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
            Phonon::State laststate = qvariant_cast<Phonon::State>(args.at(1));
            QCOMPARE(laststate, s);
            s = qvariant_cast<Phonon::State>(args.at(0));
            QVERIFY(s == Phonon::BufferingState || s == Phonon::PlayingState);
        }
    }
    QCOMPARE(s, Phonon::PlayingState);
    QCOMPARE(m_media->state(), Phonon::PlayingState);
}

void ByteStreamTest::stopPlayback(Phonon::State currentState)
{
    m_stateChangedSignalSpy->clear();
    Phonon::State s = m_media->state();
    QCOMPARE(s, currentState);
    m_media->stop();
    while (s != Phonon::StoppedState) {
        if (m_stateChangedSignalSpy->isEmpty()) {
            waitForSignal(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
        }
        while (!m_stateChangedSignalSpy->isEmpty()) {
            QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
            Phonon::State laststate = qvariant_cast<Phonon::State>(args.at(1));
            QCOMPARE(laststate, s);
            s = qvariant_cast<Phonon::State>(args.at(0));
            if (s == Phonon::StoppedState) {
                QVERIFY(m_stateChangedSignalSpy->isEmpty());
                break;
            }
            QVERIFY(s == Phonon::BufferingState || s == Phonon::PlayingState);
        }
    }
    QCOMPARE(s, Phonon::StoppedState);
    QCOMPARE(m_media->state(), Phonon::StoppedState);
}

void ByteStreamTest::pausePlayback()
{
    m_stateChangedSignalSpy->clear();
    Phonon::State s = m_media->state();
    m_media->pause();
    while (s != Phonon::PausedState) {
        if (m_stateChangedSignalSpy->isEmpty()) {
            waitForSignal(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
        }
        while (!m_stateChangedSignalSpy->isEmpty()) {
            QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
            Phonon::State laststate = qvariant_cast<Phonon::State>(args.at(1));
            QCOMPARE(laststate, s);
            s = qvariant_cast<Phonon::State>(args.at(0));
            if (s == Phonon::PausedState) {
                QVERIFY(m_stateChangedSignalSpy->isEmpty());
                break;
            }
            QVERIFY(s == Phonon::BufferingState || s == Phonon::PlayingState);
        }
    }
    QCOMPARE(s, Phonon::PausedState);
    QCOMPARE(m_media->state(), Phonon::PausedState);
}

void ByteStreamTest::waitForSignal(QObject *obj, const char *signalName, int timeout)
{
    QEventLoop loop;
    connect(obj, signalName, &loop, SLOT(quit()));
    if (timeout > 0) {
        QTimer::singleShot(timeout, &loop, SLOT(quit()));
    }
    loop.exec();
}

void ByteStreamTest::initTestCase()
{
    Phonon::loadFakeBackend();

    m_endOfDataSent = false;
    m_reading = false;
    m_seeking = false;

    m_url = testUrl();

    initByteStream();
}

void ByteStreamTest::initByteStream()
{
    delete m_media;
    delete m_stateChangedSignalSpy;
    m_media = new ByteStream(this);
    m_stateChangedSignalSpy = new QSignalSpy(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
    QVERIFY(m_stateChangedSignalSpy->isValid());
    m_stateChangedSignalSpy->clear();
}

void ByteStreamTest::kioSeekDone(KIO::Job *, KIO::filesize_t)
{
    m_seeking = false;
    m_endOfDataSent = false;
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_job);
    QVERIFY(filejob);
    if (m_reading) {
        filejob->read(32768);
    }
}

void ByteStreamTest::needData()
{
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_job);
    if (filejob) {
        if (m_seeking) {
            m_reading = true;
            return;
        }
        if (!m_reading) {
            m_reading = true;
            filejob->read(32768);
        }
    } else if (m_job && m_job->isSuspended()) {
        m_job->resume();
    }
}

void ByteStreamTest::enoughData()
{
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_job);
    if (filejob) {
        m_reading = false;
    } else {
        m_job->suspend();
    }
}

void ByteStreamTest::seekStream(qint64 offset)
{
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_job);
    QVERIFY(filejob);
    m_seeking = true;
    filejob->seek(offset);
}

void ByteStreamTest::kioJobOpen(KIO::Job *)
{
    m_endOfDataSent = false;
    KIO::FileJob *filejob = static_cast<KIO::FileJob *>(m_job);
    QVERIFY(filejob);
    m_media->setStreamSize(filejob->size());
    if (m_reading) {
        filejob->read(32768);
    }
}

void ByteStreamTest::kioTotalSize(KJob *,qulonglong size)
{
    m_media->setStreamSize(size);
}

void ByteStreamTest::kioData(KIO::Job *,const QByteArray &data)
{
    if (m_seeking) {
        return;
    }

    if (data.isEmpty()) {
        m_reading = false;
        if (!m_endOfDataSent) {
            m_endOfDataSent = true;
            m_media->endOfData();
        }
        return;
    }
    m_media->writeData(data);
    if (m_reading) {
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_job);
        QVERIFY(filejob);
        filejob->read(32768);
    }
}

void ByteStreamTest::kioResult(KJob *)
{
    m_job = 0; // the job kills itself
    m_endOfDataSent = true;
    m_media->endOfData();
    m_reading = false;
}

void ByteStreamTest::setMedia()
{
    QSignalSpy lengthSignalSpy(m_media, SIGNAL(length(qint64)));
    QCOMPARE(m_media->state(), Phonon::LoadingState);
    QCOMPARE(m_stateChangedSignalSpy->count(), 0);

    if (m_job)
    {
        disconnect(m_job, SIGNAL(data(KIO::Job *,const QByteArray &)),
                this, SLOT(kioData(KIO::Job *,const QByteArray &)));
        disconnect(m_job, SIGNAL(result(KJob *)),
                this, SLOT(kioResult(KJob *)));
        disconnect(m_job, SIGNAL(totalSize(KJob *, qulonglong)),
                this, SLOT(kioTotalSize(KJob *,qulonglong)));
        m_job->kill();
    }

    if (KProtocolManager::supportsOpening(m_url)) {
        m_job = KIO::open(m_url, QIODevice::ReadOnly);
        QVERIFY(m_job);
        m_media->setStreamSeekable(true);
        QObject::connect(m_job, SIGNAL(open(KIO::Job *)),
                this, SLOT(kioJobOpen(KIO::Job *)));
        QObject::connect(m_job, SIGNAL(position(KIO::Job *, KIO::filesize_t)),
                this, SLOT(kioSeekDone(KIO::Job *, KIO::filesize_t)));
    } else {
        m_job = KIO::get(m_url, false, false);
        m_media->setStreamSeekable(false);
        connect(m_job, SIGNAL(totalSize(KJob *, qulonglong)),
                this, SLOT(kioTotalSize(KJob *,qulonglong)));
    }
    m_endOfDataSent = false;

    m_job->addMetaData("UserAgent", QLatin1String("KDE Phonon"));
    connect(m_job, SIGNAL(data(KIO::Job *,const QByteArray &)),
            this, SLOT(kioData(KIO::Job *,const QByteArray &)));
    connect(m_job, SIGNAL(result(KJob *)),
            this, SLOT(kioResult(KJob *)));

    connect(m_media, SIGNAL(needData()), SLOT(needData()));
    connect(m_media, SIGNAL(enoughData()), SLOT(enoughData()));
    connect(m_media, SIGNAL(seekStream(qint64)), SLOT(seekStream(qint64)));

    int emits = m_stateChangedSignalSpy->count();
    Phonon::State s = m_media->state();
    if (s == Phonon::LoadingState)
    {
        // still in LoadingState, there should be no state change
        QCOMPARE(emits, 0);
        waitForSignal(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), 30000);
        emits = m_stateChangedSignalSpy->count();
        if (emits == 0) {
            QFAIL("no state change from LoadingState after 30 seconds");
        }
        QVERIFY(emits > 0);
        s = m_media->state();
    }
    if (s != Phonon::LoadingState)
    {
        // there should exactly be one state change
        QCOMPARE(emits, 1);
        QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
        Phonon::State newstate = qvariant_cast<Phonon::State>(args.at(0));
        Phonon::State oldstate = qvariant_cast<Phonon::State>(args.at(1));

        QCOMPARE(Phonon::LoadingState, oldstate);
        QCOMPARE(s, newstate);
        if (Phonon::ErrorState == s)
            QFAIL("Loading the data put the ByteStream into the ErrorState. Check that PHONON_TESTURL is set to a valid URL.");
        QCOMPARE(Phonon::StoppedState, s);
        QCOMPARE(m_stateChangedSignalSpy->count(), 0);

        // check for length signal
        QVERIFY(lengthSignalSpy.count() > 0);
        args = lengthSignalSpy.takeLast();
        QCOMPARE(m_media->totalTime(), castQVariantToInt64(args.at(0)));
    }
    else
    {
        QFAIL("Still in LoadingState after a stateChange signal was emitted. Cannot go on.");
    }
}

void ByteStreamTest::checkForDefaults()
{
    QCOMPARE(m_media->tickInterval(), qint32(0));
    QCOMPARE(m_media->aboutToFinishTime(), qint32(0));
}

void ByteStreamTest::stopToStop()
{
    QCOMPARE(m_stateChangedSignalSpy->count(), 0);
    QCOMPARE(m_media->state(), Phonon::StoppedState);
    m_media->stop();
    QCOMPARE(m_stateChangedSignalSpy->count(), 0);
    QCOMPARE(m_media->state(), Phonon::StoppedState);
}

void ByteStreamTest::stopToPause()
{
    QCOMPARE(m_stateChangedSignalSpy->count(), 0);
    QCOMPARE(m_media->state(), Phonon::StoppedState);
    m_media->pause();
    QCOMPARE(m_stateChangedSignalSpy->count(), 0);
    QCOMPARE(m_media->state(), Phonon::StoppedState);
}

void ByteStreamTest::stopToPlay()
{
    startPlayback();
    ::sleep(1);
    stopPlayback(Phonon::PlayingState);
}

void ByteStreamTest::playToPlay()
{
    startPlayback();

    m_media->play();
    QCOMPARE(m_stateChangedSignalSpy->count(), 0);
    QCOMPARE(m_media->state(), Phonon::PlayingState);

    stopPlayback(Phonon::PlayingState);
}

void ByteStreamTest::playToPause()
{
    startPlayback();
    pausePlayback();
    stopPlayback(Phonon::PausedState);
}

void ByteStreamTest::playToStop()
{
    startPlayback();
    stopPlayback(Phonon::PlayingState);
}

void ByteStreamTest::pauseToPause()
{
    startPlayback();
    pausePlayback();

    m_media->pause();
    QCOMPARE(m_stateChangedSignalSpy->count(), 0);
    QCOMPARE(m_media->state(), Phonon::PausedState);

    stopPlayback(Phonon::PausedState);
}

void ByteStreamTest::pauseToPlay()
{
    startPlayback();
    pausePlayback();
    startPlayback(Phonon::PausedState);
    stopPlayback(Phonon::PlayingState);
}

void ByteStreamTest::pauseToStop()
{
    startPlayback();
    pausePlayback();
    stopPlayback(Phonon::PausedState);
}

void ByteStreamTest::testOneSeek(qint64 seekTo)
{
    qint64 t = m_media->totalTime();
    qint64 oldTime = m_media->currentTime();
    if (oldTime == seekTo) {
        return;
    }

    QTime seekDuration;
    seekDuration.start();
    m_media->seek(seekTo);

    int bufferingTime = 0;
    Phonon::State s = m_media->state();
    QTime timer;
    if (s == Phonon::BufferingState) {
        timer.start();
    }
    QEventLoop loop;
    connect(m_media, SIGNAL(tick(qint64)), &loop, SLOT(quit()));
    connect(m_media, SIGNAL(stateChanged(Phonon::State,Phonon::State)), &loop, SLOT(quit()));

    qint64 c = m_media->currentTime();
    qint64 r = m_media->remainingTime();
    int elapsed = 0;
    while (
            (oldTime < seekTo && c < seekTo) || // seek forwards
            (oldTime > seekTo && c >= oldTime) // seek backwards
         ) {
        QTimer::singleShot(ALLOWED_TIME_FOR_SEEKING, &loop, SLOT(quit()));
        loop.exec();
        c = m_media->currentTime();
        r = m_media->remainingTime();
        if (s == Phonon::BufferingState) {
            bufferingTime += timer.restart();
        } else {
            timer.start();
        }
        s = m_media->state();
        elapsed = seekDuration.elapsed();
        QVERIFY(elapsed - bufferingTime < ALLOWED_TIME_FOR_SEEKING);
    }
    if (c < seekTo) {
        qDebug() << "currentTime:" << c
            << "seek to:" << seekTo;
    }
    QVERIFY(c >= seekTo);
    if (s == Phonon::PausedState) {
        QVERIFY(bufferingTime == 0);
        elapsed = 0;
    }
    if (c > seekTo + ALLOWED_SEEK_INACCURACY + elapsed - bufferingTime) {
        qDebug()
            << "oldTime:" << oldTime
            << "currentTime:" << c
            << "seek to:" << seekTo
            << "elapsed:" << elapsed - bufferingTime;
        QFAIL("currentTime is greater than the requested time + the time that elapsed since the seek started.");
    }
    if (c + r > t + 200 || c + r < t - 200) {
        qDebug()
            << "oldTime:" << oldTime
            << "currentTime:" << c
            << "remainingTime:" << r
            << "totalTime:" << t;
        QFAIL("currentTime + remainingTime doesn't come close enough to totalTime");
    }
}

void ByteStreamTest::testSeek()
{
    startPlayback();
    QTime timer;
    timer.start();
    qint64 t = m_media->totalTime();
    qint64 c = m_media->currentTime();
    qint64 r = m_media->remainingTime();
    int elapsed = timer.elapsed();
    if (c + r > t + elapsed || c + r < t - elapsed) {
        qDebug() << "currentTime:" << c
            << "remainingTime:" << r
            << "totalTime:" << t;
        QFAIL("currentTime + remainingTime doesn't come close enough to totalTime");
    }
    QVERIFY(c + r <= t + elapsed);
    QVERIFY(c + r >= t - elapsed);
    if (m_media->isSeekable())
        if (r > 0)
        {
            m_media->setTickInterval(20);
            qint64 s = c + r / 2;
            testOneSeek(s);
//X             timer.start();
//X             m_media->seek(s);
//X             // ugh, after the seek call it may need a few cycles until the seek has actually
//X             // happened, the best way to check that the seek is done must be the tick signal
//X             c = m_media->currentTime();
//X             r = m_media->remainingTime();
//X             while (c < s) {
//X                 waitForSignal(m_media, SIGNAL(tick(qint64)), ALLOWED_TIME_FOR_SEEKING);
//X                 c = m_media->currentTime();
//X                 r = m_media->remainingTime();
//X                 elapsed = timer.elapsed();
//X                 QVERIFY(elapsed < ALLOWED_TIME_FOR_SEEKING);
//X             }
//X             if (c < s) {
//X                 qDebug() << "currentTime:" << c
//X                     << "seek to:" << s;
//X             }
//X             QVERIFY(c >= s);
//X             if (c > s + ALLOWED_SEEK_INACCURACY + elapsed) {
//X                 qDebug() << "currentTime:" << c
//X                     << "seek to:" << s
//X                     << "elapsed:" << elapsed;
//X                 QFAIL("currentTime is greater than the requested time + the time that elapsed since the seek started.");
//X             }
//X             if (c + r > t + 200 || c + r < t - 200) {
//X                 qDebug() << "currentTime:" << c
//X                     << "remainingTime:" << r
//X                     << "totalTime:" << t;
//X                 QFAIL("currentTime + remainingTime doesn't come close enough to totalTime");
//X             }

            s /= 2;
            testOneSeek(s);
            s = s * 3 / 2;
            testOneSeek(s);

            pausePlayback();

            s = s * 3 / 2;
            testOneSeek(s);
            s /= 2;
            testOneSeek(s);

            m_media->setTickInterval(0);
            stopPlayback(Phonon::PausedState);
            return;
        }
        else
            QWARN("didn't test seeking as the ByteStream reported a remaining size <= 0");
    else
        QWARN("didn't test seeking as the ByteStream is not seekable");
    stopPlayback(Phonon::PlayingState);
}

void ByteStreamTest::testAboutToFinish()
{
    const qint32 requestedAboutToFinishTime = 2000;
    m_media->setAboutToFinishTime(requestedAboutToFinishTime);
    QCOMPARE(m_media->aboutToFinishTime(), requestedAboutToFinishTime);
    QSignalSpy aboutToFinishSpy(m_media, SIGNAL(aboutToFinish(qint32)));
    QSignalSpy finishSpy(m_media, SIGNAL(finished()));
    startPlayback();
    State s = m_media->state();
    if (m_media->isSeekable()) {
        m_media->seek(m_media->totalTime() - 4000 - requestedAboutToFinishTime); // give it 4 seconds
    }
    int wait = 10000;
    while (aboutToFinishSpy.count() == 0 && (m_media->state() == Phonon::PlayingState ||
                m_media->state() == Phonon::BufferingState)) {
        wait = qMax(1000, wait / 2);
        waitForSignal(m_media, SIGNAL(aboutToFinish(qint32)), wait);
    }
    // at this point the media should be about to finish playing
    qint64 r = m_media->remainingTime();
    Phonon::State state = m_media->state();
    QCOMPARE(aboutToFinishSpy.count(), 1);
    const qint32 aboutToFinishTime = castQVariantToInt32(aboutToFinishSpy.first().at(0));
    QVERIFY(aboutToFinishTime <= requestedAboutToFinishTime + 150); // allow it to be up to 150ms too early
    qDebug() << "received aboutToFinishTime" << aboutToFinishTime << ", requested" << requestedAboutToFinishTime;
    if (state == Phonon::PlayingState || state == Phonon::BufferingState) {
        if (r > aboutToFinishTime) {
            qDebug() << "remainingTime =" << r;
            QFAIL("remainingTime needs to be less than or equal to aboutToFinishTime");
        }
        QVERIFY(r <= aboutToFinishTime);
        waitForSignal(m_media, SIGNAL(finished()));
    } else {
        QVERIFY(aboutToFinishTime >= 0);
    }
    QCOMPARE(finishSpy.count(), 1);

    m_media->setAboutToFinishTime(0);
    QCOMPARE(m_media->aboutToFinishTime(), qint32(0));

    while (m_stateChangedSignalSpy->count() > 1) {
        QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
        State oldstate = qvariant_cast<Phonon::State>(args.at(1));
        QCOMPARE(oldstate, s);
        s = qvariant_cast<Phonon::State>(args.at(0));
        QVERIFY(s == Phonon::PlayingState || s == Phonon::BufferingState);
        QVERIFY(s != oldstate);
    }
    QCOMPARE(s, Phonon::PlayingState);

    QCOMPARE(m_stateChangedSignalSpy->count(), 1);
    QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
    Phonon::State newstate = qvariant_cast<Phonon::State>(args.at(0));
    Phonon::State oldstate = qvariant_cast<Phonon::State>(args.at(1));
    QCOMPARE(oldstate, Phonon::PlayingState);
    QCOMPARE(newstate, Phonon::StoppedState);
    QCOMPARE(m_media->state(), Phonon::StoppedState);
    QCoreApplication::processEvents();
    QCOMPARE(aboutToFinishSpy.count(), 1);
}

void ByteStreamTest::testTickSignal()
{
    QTime start1;
    QTime start2;
    for (qint32 tickInterval = 80; tickInterval <= 500; tickInterval *= 2)
    {
        QSignalSpy tickSpy(m_media, SIGNAL(tick(qint64)));
        qDebug() << "Test 20 ticks with an interval of" <<  tickInterval << "ms";
        m_media->setTickInterval(tickInterval);
        QVERIFY(m_media->tickInterval() <= tickInterval);
        QVERIFY(m_media->tickInterval() >= tickInterval/2);
        QVERIFY(tickSpy.isEmpty());
        start1.start();
        startPlayback();
        start2.start();
        int lastCount = 0;
        qint64 s1, s2 = start2.elapsed();
        while (tickSpy.count() < 20 && (m_media->state() == Phonon::PlayingState || m_media->state() == Phonon::BufferingState))
        {
            // FIXME if the ByteStream goes into BufferingState this will break. The timers have to
            // stop while it's buffering.
            if (tickSpy.count() > lastCount)
            {
                s1 = start1.elapsed();
                qint64 tickTime = castQVariantToInt64(tickSpy.last().at(0));
                lastCount = tickSpy.count();
                // s1 is the time from before the beginning of the playback to
                // after the tick signal
                // s2 is the time from after the beginning of the playback to
                // before the tick signal
                // so: s2 <= s1

                QVERIFY(tickTime <= m_media->currentTime());
                if (s1 + ALLOWED_TICK_INACCURACY < tickTime || s2 - ALLOWED_TICK_INACCURACY > tickTime) {
                    qDebug()
                        << "\n" << lastCount << "ticks have been received"
                        << "\ntime from before playback was started to after the tick signal was received:" << s1 << "ms"
                        << "\ntime from after playback was started to before the tick signal was received:" << s2 << "ms"
                        << "\nreported tick time:" << tickTime << "ms"
                        << "\nallowed inaccuracy: +/-" << ALLOWED_TICK_INACCURACY << "ms";
                    for (int i = 0; i < tickSpy.count(); ++i) {
                        qDebug() << castQVariantToInt64(tickSpy[i].at(0));
                    }
                }
                QVERIFY(s1 + ALLOWED_TICK_INACCURACY >= tickTime);
                QVERIFY(s2 - ALLOWED_TICK_INACCURACY <= tickTime);
                QVERIFY(s1 >= lastCount * m_media->tickInterval());
                if (s2 > (lastCount + 1) * m_media->tickInterval())
                    QWARN(qPrintable(QString("%1. tick came too late: %2ms elapsed while this tick should have come before %3ms")
                            .arg(lastCount).arg(s2).arg((lastCount + 1) * m_media->tickInterval())));
            } else if (lastCount == 0 && s2 > 20 * m_media->tickInterval()) {
                QFAIL("no tick signals are being received");
            }
            s2 = start2.elapsed();
            waitForSignal(m_media, SIGNAL(tick(qint64)), 2000);
        }
        stopPlayback(Phonon::PlayingState);
    }
}

void ByteStreamTest::addPaths()
{
    AudioPath *a1 = new AudioPath(this);
    AudioPath *a2 = new AudioPath(this);
    VideoPath *v1 = new VideoPath(this);
    VideoPath *v2 = new VideoPath(this);
    QCOMPARE(m_media->audioPaths().size(), 0);
    QCOMPARE(m_media->videoPaths().size(), 0);
    QVERIFY(m_media->addAudioPath(a1));
    QCOMPARE(m_media->audioPaths().size(), 1); // one should always work
    QVERIFY(m_media->audioPaths().contains(a1));
    QCOMPARE(m_media->addAudioPath(a1), false); // adding the same one should not work
    QCOMPARE(m_media->audioPaths().size(), 1);
    QVERIFY(m_media->audioPaths().contains(a1));
    if (m_media->addAudioPath(a2))
    {
        QCOMPARE(m_media->audioPaths().size(), 2);
        QVERIFY(m_media->audioPaths().contains(a1));
        QVERIFY(m_media->audioPaths().contains(a2));
        QCOMPARE(m_media->addAudioPath(a1), false); // adding the same one should not work
        QCOMPARE(m_media->audioPaths().size(), 2);
        QCOMPARE(m_media->addAudioPath(a2), false); // adding the same one should not work
        QCOMPARE(m_media->audioPaths().size(), 2);
        delete a2;
        QCOMPARE(m_media->audioPaths().size(), 1);
        QVERIFY(m_media->audioPaths().contains(a1));
        QVERIFY(!m_media->audioPaths().contains(a2));
        a2 = new AudioPath(this);
        QCOMPARE(m_media->addAudioPath(a2), true);
        QCOMPARE(m_media->audioPaths().size(), 2);
        QVERIFY(m_media->audioPaths().contains(a1));
        QVERIFY(m_media->audioPaths().contains(a2));
        delete a2;
        QCOMPARE(m_media->audioPaths().size(), 1);
        QVERIFY(m_media->audioPaths().contains(a1));
        QVERIFY(!m_media->audioPaths().contains(a2));
        a2 = 0;
    }
    else
        QWARN("backend does not allow usage of more than one AudioPath");
    delete a1;
    QCOMPARE(m_media->audioPaths().size(), 0);
    a1 = 0;

    QVERIFY(m_media->addVideoPath(v1));
    QCOMPARE(m_media->videoPaths().size(), 1); // one should always work
    QVERIFY(m_media->videoPaths().contains(v1));
    QCOMPARE(m_media->addVideoPath(v1), false); // adding the same one should not work
    QCOMPARE(m_media->videoPaths().size(), 1);
    QVERIFY(m_media->videoPaths().contains(v1));
    if (m_media->addVideoPath(v2))
    {
        QCOMPARE(m_media->videoPaths().size(), 2);
        QVERIFY(m_media->videoPaths().contains(v1));
        QVERIFY(m_media->videoPaths().contains(v2));
        QCOMPARE(m_media->addVideoPath(v1), false); // adding the same one should not work
        QCOMPARE(m_media->videoPaths().size(), 2);
        QCOMPARE(m_media->addVideoPath(v2), false); // adding the same one should not work
        QCOMPARE(m_media->videoPaths().size(), 2);
        delete v2;
        QCOMPARE(m_media->videoPaths().size(), 1);
        QVERIFY(m_media->videoPaths().contains(v1));
        QVERIFY(!m_media->videoPaths().contains(v2));
        v2 = new VideoPath(this);
        QCOMPARE(m_media->addVideoPath(v2), true);
        QCOMPARE(m_media->videoPaths().size(), 2);
        QVERIFY(m_media->videoPaths().contains(v1));
        QVERIFY(m_media->videoPaths().contains(v2));
        delete v2;
        QCOMPARE(m_media->videoPaths().size(), 1);
        QVERIFY(m_media->videoPaths().contains(v1));
        QVERIFY(!m_media->videoPaths().contains(v2));
        v2 = 0;
    }
    else
        QWARN("backend does not allow usage of more than one VideoPath");
    delete v1;
    QCOMPARE(m_media->videoPaths().size(), 0);
    v1 = 0;
}

void ByteStreamTest::initOutput()
{
    // AudioPath and AudioOutput are needed else the backend might finish in no time
    if (!m_audioPath && !m_audioOutput)
    {
        m_audioPath = new AudioPath(this);
        m_audioOutput = new AudioOutput(Phonon::MusicCategory, this);
        //m_audioOutput->setVolume(0.0f);
        m_audioPath->addOutput(m_audioOutput);
    }
    QVERIFY(m_media->addAudioPath(m_audioPath));
}

void ByteStreamTest::cleanupTestCase()
{
    delete m_stateChangedSignalSpy;
    delete m_media;
}

QTEST_KDEMAIN(ByteStreamTest, GUI)
#include "bytestreamtest.moc"
// vim: sw=4 ts=4
