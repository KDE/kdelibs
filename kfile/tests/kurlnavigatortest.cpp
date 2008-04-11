/***************************************************************************
 *   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#include "kurlnavigatortest.h"
#include <qtest_kde.h>
#include <kurlnavigator.h>

QTEST_KDEMAIN(KUrlNavigatorTest, GUI)

void KUrlNavigatorTest::initTestCase()
{
    m_navigator = new KUrlNavigator(0, KUrl("A"), 0);
}

void KUrlNavigatorTest::cleanupTestCase()
{
    delete m_navigator;
    m_navigator = 0;
}

void KUrlNavigatorTest::testHistorySizeAndIndex()
{
    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 1);

    m_navigator->setUrl(KUrl("A"));

    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 1);

    m_navigator->setUrl(KUrl("B"));

    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 2);

    m_navigator->setUrl(KUrl("C"));

    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 3);
}

void KUrlNavigatorTest::testGoBack()
{
    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 3);

    bool ok = m_navigator->goBack();

    QVERIFY(ok);
    QCOMPARE(m_navigator->historyIndex(), 1);
    QCOMPARE(m_navigator->historySize(), 3);

    ok = m_navigator->goBack();

    QVERIFY(ok);
    QCOMPARE(m_navigator->historyIndex(), 2);
    QCOMPARE(m_navigator->historySize(), 3);

    ok = m_navigator->goBack();

    QVERIFY(!ok);
    QCOMPARE(m_navigator->historyIndex(), 2);
    QCOMPARE(m_navigator->historySize(), 3);
}

void KUrlNavigatorTest::testGoForward()
{
    QCOMPARE(m_navigator->historyIndex(), 2);
    QCOMPARE(m_navigator->historySize(), 3);

    bool ok = m_navigator->goForward();

    QVERIFY(ok);
    QCOMPARE(m_navigator->historyIndex(), 1);
    QCOMPARE(m_navigator->historySize(), 3);

    ok = m_navigator->goForward();

    QVERIFY(ok);
    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 3);

    ok = m_navigator->goForward();

    QVERIFY(!ok);
    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 3);
}

void KUrlNavigatorTest::testHistoryInsert()
{
    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 3);

    m_navigator->setUrl(KUrl("D"));

    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 4);

    bool ok = m_navigator->goBack();
    QVERIFY(ok);
    QCOMPARE(m_navigator->historyIndex(), 1);
    QCOMPARE(m_navigator->historySize(), 4);

    m_navigator->setUrl(KUrl("E"));
    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 4);

    m_navigator->setUrl(KUrl("F"));
    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 5);

    ok = m_navigator->goBack();
    QVERIFY(ok);
    ok = m_navigator->goBack();
    QVERIFY(ok);
    QCOMPARE(m_navigator->historyIndex(), 2);
    QCOMPARE(m_navigator->historySize(), 5);

    m_navigator->setUrl(KUrl("G"));

    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 4);

    // insert same URL as the current history index
    m_navigator->setUrl(KUrl("G"));
    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 4);

    // insert same URL with a trailing slash as the current history index
    m_navigator->setUrl(KUrl("G/"));
    QCOMPARE(m_navigator->historyIndex(), 0);
    QCOMPARE(m_navigator->historySize(), 4);

    // jump to "C" and insert same URL as the current history index
    ok = m_navigator->goBack();
    QVERIFY(ok);
    QCOMPARE(m_navigator->historyIndex(), 1);
    QCOMPARE(m_navigator->historySize(), 4);

    m_navigator->setUrl(KUrl("C"));
    QCOMPARE(m_navigator->historyIndex(), 1);
    QCOMPARE(m_navigator->historySize(), 4);
}

#include "kurlnavigatortest.moc"
