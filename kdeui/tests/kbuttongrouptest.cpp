/*
    This file is part of the KDE Libraries

    Copyright (C) 2006 Pino Toscano <toscano.pino@tiscali.it>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qtest_kde.h"

#include <QtTest/QTestEvent>

#include <QtCore/QList>
#include <QtGui/QRadioButton>
#include <QtTest/QSignalSpy>
#include <QtGui/QBoxLayout>

#include "kbuttongrouptest.h"
#include "kbuttongroup.h"

KButtonGroup* kbuttongroup;
QList<QRadioButton*> buttons;

void KButtonGroupTest::initTestCase()
{
  kbuttongroup = new KButtonGroup();

  QVBoxLayout* lay2 = new QVBoxLayout( kbuttongroup );
  for ( int i = 0; i < 8; ++i )
  {
    QRadioButton* r = new QRadioButton( kbuttongroup );
    r->setText( QString( "radio%1" ).arg( i ) );
    lay2->addWidget( r );
    buttons << r;
  }

  QCOMPARE( kbuttongroup->selected(), -1 );
}

void KButtonGroupTest::directSelectionTestCase()
{
  // test where setSelected is called before the 
  // ensurePolished() is called.
  KButtonGroup* kbuttongroup2 = new KButtonGroup();
  kbuttongroup2->setSelected( 3 );

  QVBoxLayout* lay2 = new QVBoxLayout( kbuttongroup2 );
  for ( int i = 0; i < 8; ++i )
  {
    QRadioButton* r = new QRadioButton( kbuttongroup2 );
    r->setText( QString( "radio%1" ).arg( i ) );
    lay2->addWidget( r );
    buttons << r;
  }
  QTest::qWait(250); // events should occur.
  QCOMPARE( kbuttongroup2->selected(), 3 );
}

void KButtonGroupTest::cleanupTestCase()
{
  kbuttongroup->deleteLater();
}

void KButtonGroupTest::testClicks()
{
  QTest::mouseClick( buttons[3], Qt::LeftButton, 0, QPoint(), 10 );
  QCOMPARE( kbuttongroup->selected(), 3 );

  QTest::mouseClick( buttons[5], Qt::LeftButton, 0, QPoint(), 10 );
  QCOMPARE( kbuttongroup->selected(), 5 );

  QTest::mouseClick( buttons[7], Qt::LeftButton, 0, QPoint(), 10 );
  QCOMPARE( kbuttongroup->selected(), 7 );

  QTest::mouseClick( buttons[1], Qt::LeftButton, 0, QPoint(), 10 );
  QCOMPARE( kbuttongroup->selected(), 1 );

  // QRadioButton's react only to LMB click events
  QTest::mouseClick( buttons[5], Qt::RightButton, 0, QPoint(), 10 );
  QCOMPARE( kbuttongroup->selected(), 1 );
  QTest::mouseClick( buttons[5], Qt::MidButton, 0, QPoint(), 10 );
  QCOMPARE( kbuttongroup->selected(), 1 );
}

void KButtonGroupTest::testManualSelection()
{
  kbuttongroup->setSelected( 3 );
  QCOMPARE( kbuttongroup->selected(), 3 );

  kbuttongroup->setSelected( 0 );
  QCOMPARE( kbuttongroup->selected(), 0 );

  kbuttongroup->setSelected( 7 );
  QCOMPARE( kbuttongroup->selected(), 7 );

  kbuttongroup->setSelected( 2 );
  QCOMPARE( kbuttongroup->selected(), 2 );

  // "bad" cases: ask for an invalid id -- the selection should not change
  kbuttongroup->setSelected( 10 );
  QCOMPARE( kbuttongroup->selected(), 2 );
  kbuttongroup->setSelected( -1 );
  QCOMPARE( kbuttongroup->selected(), 2 );
}

void KButtonGroupTest::testSignals()
{
  QSignalSpy spyClicked( kbuttongroup, SIGNAL( clicked( int ) ) );
  QSignalSpy spyPressed( kbuttongroup, SIGNAL( pressed( int ) ) );
  QSignalSpy spyReleased( kbuttongroup, SIGNAL( released( int ) ) );
  QSignalSpy spyChanged( kbuttongroup, SIGNAL( changed( int ) ) );

  QTest::mouseClick( buttons[2], Qt::LeftButton, 0, QPoint(), 10 );
  QCOMPARE( spyClicked.count(), 1 );
  QCOMPARE( spyPressed.count(), 1 );
  QCOMPARE( spyReleased.count(), 1 );
  QCOMPARE( spyChanged.count(), 1 );
  QList<QVariant> args = spyClicked.last();
  QList<QVariant> args2 = spyPressed.last();
  QList<QVariant> args3 = spyReleased.last();
  QCOMPARE( args.first().toInt(), 2 );
  QCOMPARE( args2.first().toInt(), 2 );
  QCOMPARE( args3.first().toInt(), 2 );
  QCOMPARE( kbuttongroup->selected(), 2 );

  QTest::mouseClick( buttons[6], Qt::LeftButton, 0, QPoint(), 10 );
  QCOMPARE( spyClicked.count(), 2 );
  QCOMPARE( spyPressed.count(), 2 );
  QCOMPARE( spyReleased.count(), 2 );
  QCOMPARE( spyChanged.count(), 2 );
  args = spyClicked.last();
  args2 = spyPressed.last();
  args3 = spyReleased.last();
  QCOMPARE( args.first().toInt(), 6 );
  QCOMPARE( args2.first().toInt(), 6 );
  QCOMPARE( args3.first().toInt(), 6 );
  QCOMPARE( kbuttongroup->selected(), 6 );

  // click with RMB on a radio -> no signal
  QTest::mouseClick( buttons[0], Qt::RightButton, 0, QPoint(), 10 );
  QCOMPARE( spyClicked.count(), 2 );
  QCOMPARE( spyPressed.count(), 2 );
  QCOMPARE( spyReleased.count(), 2 );
  QCOMPARE( spyChanged.count(), 2 );
  QCOMPARE( kbuttongroup->selected(), 6 );

  // manual selections
  kbuttongroup->setSelected( 7 );
  QCOMPARE( spyChanged.count(), 3 );
  QList<QVariant> args4 = spyChanged.last();
  QCOMPARE( args4.first().toInt(), 7 );
  QCOMPARE( kbuttongroup->selected(), 7 );

  kbuttongroup->setSelected( 2 );
  QCOMPARE( spyChanged.count(), 4 );
  args4 = spyChanged.last();
  QCOMPARE( args4.first().toInt(), 2 );
  QCOMPARE( kbuttongroup->selected(), 2 );

  // "bad" cases: ask for an invalid id -- the selection should not change
  kbuttongroup->setSelected( 10 );
  QCOMPARE( spyChanged.count(), 4 );
  QCOMPARE( kbuttongroup->selected(), 2 );

  kbuttongroup->setSelected( -1 );
  QCOMPARE( spyChanged.count(), 4 );
  QCOMPARE( kbuttongroup->selected(), 2 );
}


QTEST_KDEMAIN(KButtonGroupTest, GUI)

#include "kbuttongrouptest.moc"

