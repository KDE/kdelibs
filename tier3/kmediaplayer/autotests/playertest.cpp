/*
 * Copyright 2013  Alex Merry <alex.merry@kdemail.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the X Consortium shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from the X Consortium.
 */

#include "testplayer.h"
#include <QSignalSpy>
#include <QTest>

class PlayerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init() {
        player = new TestPlayer(this);
    }
    void cleanup() {
        delete player;
        player = 0;
    }

    void testParent();
    void testDefaultState();
    void testDefaultLooping();
    void testSetState();
    void testSetLooping();

private:
    TestPlayer *player;
};

using namespace KMediaPlayer;

void PlayerTest::testParent() {
    QCOMPARE(player->parent(), this);
}

void PlayerTest::testDefaultState() {
    QCOMPARE(player->state(), (int)Player::Empty);
}

void PlayerTest::testDefaultLooping() {
    QCOMPARE(player->isLooping(), false);
}

void PlayerTest::testSetState() {
    QSignalSpy spy(player, SIGNAL(stateChanged(int)));
    player->setStateWrapper((int)Player::Play);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.at(0).toInt(), (int)Player::Play);
    QCOMPARE(player->state(), (int)Player::Play);

    player->setStateWrapper((int)Player::Empty);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.at(0).toInt(), (int)Player::Empty);
    QCOMPARE(player->state(), (int)Player::Empty);
}

void PlayerTest::testSetLooping() {
    QSignalSpy spy(player, SIGNAL(loopingChanged(bool)));
    player->setLooping(true);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.at(0).toBool(), true);
    QCOMPARE(player->isLooping(), true);

    player->setLooping(false);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.at(0).toBool(), false);
    QCOMPARE(player->isLooping(), false);
}

QTEST_GUILESS_MAIN(PlayerTest)

#include "playertest.moc"
#include "moc_testplayer.cpp"

