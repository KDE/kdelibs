/*
 * Copyright 2012 Ni Hui <shuizhuyuanluo@126.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kencodingprobertest.h"

#include <QTest>
#include <kencodingprober.h>

static KEncodingProber* ep = 0;

void KEncodingProberTest::initTestCase()
{
    ep = new KEncodingProber;
}

void KEncodingProberTest::cleanupTestCase()
{
    delete ep;
    ep = 0;
}

void KEncodingProberTest::cleanup()
{
    ep->reset();
}

void KEncodingProberTest::testReset()
{
    ep->feed(QByteArray("some random data @*@#&jd"));
    ep->reset();
    QCOMPARE(ep->state(), KEncodingProber::Probing);
    QCOMPARE(ep->encoding().toLower(), QByteArray("utf-8"));
}

void KEncodingProberTest::testProbe()
{
    // utf-8
    ep->setProberType(KEncodingProber::Universal);
    ep->feed(QByteArray::fromHex("e998bfe5b094e58d91e696afe5b1b1e88489"));
    QCOMPARE(ep->encoding().toLower(), QByteArray("utf-8"));
    ep->reset();

    // gb18030
    ep->setProberType(KEncodingProber::ChineseSimplified);
    ep->feed(QByteArray::fromHex("d7d4d3c9b5c4b0d9bfc6c8abcae9"));
    QCOMPARE(ep->encoding().toLower(), QByteArray("gb18030"));
    ep->reset();

    // shift_jis
    ep->setProberType(KEncodingProber::Japanese);
    ep->feed(QByteArray::fromHex("8374838a815b955389c88e969354"));
    QCOMPARE(ep->encoding().toLower(), QByteArray("shift_jis"));
    ep->reset();

    // big5
    ep->setProberType(KEncodingProber::ChineseTraditional);
    ep->feed(QByteArray::fromHex("aefcafc7a6caa474a141a6b3ae65a444a46a"));
    QCOMPARE(ep->encoding().toLower(), QByteArray("big5"));
    ep->reset();
}

QTEST_MAIN(KEncodingProberTest)
