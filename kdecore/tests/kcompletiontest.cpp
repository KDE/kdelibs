/* This file is part of the KDE libraries
    Copyright (c) 2005 Thomas Braxton <brax108@cox.net>

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

#include "kcompletiontest.h"
#include "kcompletiontest.moc"
#include "QtTest/qsignalspy.h"

#define clampet strings[0]
#define coolcat strings[1]
#define carpet strings[2]
#define carp strings[3]
#define wclampet wstrings[0]
#define wcoolcat wstrings[1]
#define wcarpet wstrings[2]
#define wcarp wstrings[3]

void
Test_KCompletion::initTestCase()
{
	strings << QString("clampet@test.org")
		<< QString("coolcat@test.org")
		<< QString("carpet@test.org")
		<< QString("carp@test.org");
	wstrings << QString("clampet@test.org:30")
		 << QString("coolcat@test.org:20")
		 << QString("carpet@test.org:40")
		 << QString("carp@test.org:7");
	qRegisterMetaType<QStringList>("QStringList");
}

void
Test_KCompletion::isEmpty()
{
	KCompletion completion;
	QVERIFY(completion.isEmpty());
	QVERIFY(completion.items().isEmpty());
}

void
Test_KCompletion::insertionOrder()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	QSignalSpy spy1(&completion, SIGNAL(match(const QString &)));
	QSignalSpy spy3(&completion, SIGNAL(multipleMatches()));

	completion.setOrder(KCompletion::Insertion);
	QVERIFY(completion.order() == KCompletion::Insertion);

	completion.setItems(strings);
	QVERIFY(completion.items().count() == strings.count());

	completion.setCompletionMode(KGlobalSettings::CompletionShell);
	QCOMPARE(completion.makeCompletion("ca"), QString("carp"));
	QVERIFY(spy1.count() == 1);
	QVERIFY(spy1.takeFirst().at(0).toString() == QString("carp"));
	QVERIFY(spy3.count() == 1); spy3.takeFirst();

	QSignalSpy spy2(&completion, SIGNAL(matches(const QStringList &)));
	completion.makeCompletion("ca");
	QCOMPARE(spy2.count(), 1);
	QVERIFY(spy3.count() == 0); // shouldn't be signaled on 2nd call
	QStringList matches = spy2.takeFirst().at(0).toStringList();
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], carp);

	completion.setCompletionMode(KGlobalSettings::CompletionAuto);
	QCOMPARE(completion.makeCompletion("ca"), carpet);
	QVERIFY(spy1.count() == 1);
	QVERIFY(spy1.takeFirst().at(0).toString() == carpet);
}

void
Test_KCompletion::sortedOrder()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	QSignalSpy spy1(&completion, SIGNAL(match(const QString &)));
	QSignalSpy spy3(&completion, SIGNAL(multipleMatches()));

	completion.setOrder(KCompletion::Sorted);
	QVERIFY(completion.order() == KCompletion::Sorted);

	completion.setItems(strings);
	QVERIFY(completion.items().count() == 4);

	completion.setCompletionMode(KGlobalSettings::CompletionShell);
	QCOMPARE(completion.makeCompletion("ca"), QString("carp"));
	QVERIFY(spy1.count() == 1);
	QCOMPARE(spy1.takeFirst().at(0).toString(), QString("carp"));
	QVERIFY(spy3.count() == 1); spy3.takeFirst();

	QSignalSpy spy2(&completion, SIGNAL(matches(const QStringList &)));
	completion.makeCompletion("ca");
	QCOMPARE(spy2.count(), 1);
	QVERIFY(spy3.count() == 0); // shouldn't be signaled on 2nd call

	QStringList matches = spy2.takeFirst().at(0).toStringList();
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carp);
	QCOMPARE(matches[1], carpet);

	completion.setCompletionMode(KGlobalSettings::CompletionAuto);
	QCOMPARE(completion.makeCompletion("ca"), carp);
	QVERIFY(spy1.count() == 1);
	QCOMPARE(spy1.takeFirst().at(0).toString(), carp);
}

void
Test_KCompletion::weightedOrder()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	QSignalSpy spy1(&completion, SIGNAL(match(const QString &)));
	QSignalSpy spy3(&completion, SIGNAL(multipleMatches()));

	completion.setOrder(KCompletion::Weighted);
	QVERIFY(completion.order() == KCompletion::Weighted);

	completion.setItems(wstrings);
	QVERIFY(completion.items().count() == 4);

	completion.setCompletionMode(KGlobalSettings::CompletionShell);
	QCOMPARE(completion.makeCompletion("ca"), QString("carp"));
	spy1.takeFirst(); // empty the list
	QVERIFY(spy3.count() == 1); spy3.takeFirst();

	QSignalSpy spy2(&completion, SIGNAL(matches(const QStringList &)));
	completion.makeCompletion("ca");
	QCOMPARE(spy2.count(), 1);
	QVERIFY(spy3.count() == 0); // shouldn't be signaled on 2nd call

	QStringList matches = spy2.takeFirst().at(0).toStringList();
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], carp);

	completion.setCompletionMode(KGlobalSettings::CompletionAuto);
	QCOMPARE(completion.makeCompletion("ca"), carpet);

	matches = completion.substringCompletion("ca");
	QVERIFY(matches.count() == 3);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], coolcat);
	QCOMPARE(matches[2], carp);
}

void
Test_KCompletion::substringCompletion_Insertion()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Insertion);
	completion.setItems(strings);
	QVERIFY(completion.items().count() == 4);

	QStringList matches = completion.substringCompletion("c");
	QVERIFY(matches.count() == 4);
	QCOMPARE(matches[0], clampet);
	QCOMPARE(matches[1], coolcat);
	QCOMPARE(matches[2], carpet);
	QCOMPARE(matches[3], carp);

	matches = completion.substringCompletion("ca");
	QVERIFY(matches.count() == 3);
	QCOMPARE(matches[0], coolcat);
	QCOMPARE(matches[1], carpet);
	QCOMPARE(matches[2], carp);

	matches = completion.substringCompletion("car");
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], carp);

	matches = completion.substringCompletion("pet");
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], clampet);
	QCOMPARE(matches[1], carpet);
}

void
Test_KCompletion::substringCompletion_Sorted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Sorted);
	completion.setItems(strings);
	QVERIFY(completion.items().count() == 4);

	QStringList matches = completion.substringCompletion("c");
	QVERIFY(matches.count() == 4);
	QCOMPARE(matches[0], carp);
	QCOMPARE(matches[1], carpet);
	QCOMPARE(matches[2], clampet);
	QCOMPARE(matches[3], coolcat);

	matches = completion.substringCompletion("ca");
	QVERIFY(matches.count() == 3);
	QCOMPARE(matches[0], carp);
	QCOMPARE(matches[1], carpet);
	QCOMPARE(matches[2], coolcat);

	matches = completion.substringCompletion("car");
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carp);
	QCOMPARE(matches[1], carpet);

	matches = completion.substringCompletion("pet");
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], clampet);
}

void
Test_KCompletion::substringCompletion_Weighted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Weighted);
	completion.setItems(wstrings);
	QVERIFY(completion.items().count() == 4);

	QStringList matches = completion.substringCompletion("c");
	QVERIFY(matches.count() == 4);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], clampet);
	QCOMPARE(matches[2], coolcat);
	QCOMPARE(matches[3], carp);

	matches = completion.substringCompletion("ca");
	QVERIFY(matches.count() == 3);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], coolcat);
	QCOMPARE(matches[2], carp);

	matches = completion.substringCompletion("car");
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], carp);

	matches = completion.substringCompletion("pet");
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], clampet);
}

void
Test_KCompletion::allMatches_Insertion()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Insertion);
	completion.setItems(strings);
	QVERIFY(completion.items().count() == 4);

	QStringList matches = completion.allMatches("c");
	QVERIFY(matches.count() == 4);
	QCOMPARE(matches[0], clampet);
	QCOMPARE(matches[1], coolcat);
	QCOMPARE(matches[2], carpet);
	QCOMPARE(matches[3], carp);

	matches = completion.allMatches("ca");
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], carp);

	matches = completion.allMatches("pet");
	QVERIFY(matches.count() == 0);
}

void
Test_KCompletion::allMatches_Sorted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Sorted);
	completion.setItems(strings);
	QVERIFY(completion.items().count() == 4);

	QStringList matches = completion.allMatches("c");
	QVERIFY(matches.count() == 4);
	QCOMPARE(matches[0], carp);
	QCOMPARE(matches[1], carpet);
	QCOMPARE(matches[2], clampet);
	QCOMPARE(matches[3], coolcat);

	matches = completion.allMatches("ca");
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carp);
	QCOMPARE(matches[1], carpet);

	matches = completion.allMatches("pet");
	QVERIFY(matches.count() == 0);
}

void
Test_KCompletion::allMatches_Weighted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Weighted);
	completion.setItems(wstrings);
	QVERIFY(completion.items().count() == 4);

	QStringList matches = completion.allMatches("c");
	QVERIFY(matches.count() == 4);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], clampet);
	QCOMPARE(matches[2], coolcat);
	QCOMPARE(matches[3], carp);

	matches = completion.allMatches("ca");
	QVERIFY(matches.count() == 2);
	QCOMPARE(matches[0], carpet);
	QCOMPARE(matches[1], carp);

	matches = completion.allMatches("pet");
	QVERIFY(matches.count() == 0);
}

void
Test_KCompletion::cycleMatches_Insertion()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setOrder(KCompletion::Insertion);
	completion.setItems(strings);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.makeCompletion("ca");
	QCOMPARE(completion.nextMatch(), carpet);
	QCOMPARE(completion.nextMatch(), carp);
	QCOMPARE(completion.previousMatch(), carpet);
	QCOMPARE(completion.previousMatch(), carp);
}

void
Test_KCompletion::cycleMatches_Sorted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setOrder(KCompletion::Sorted);
	completion.setItems(strings);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.makeCompletion("ca");
	QCOMPARE(completion.nextMatch(), carp);
	QCOMPARE(completion.nextMatch(), carpet);
	QCOMPARE(completion.previousMatch(), carp);
	QCOMPARE(completion.previousMatch(), carpet);
}

void
Test_KCompletion::cycleMatches_Weighted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setOrder(KCompletion::Weighted);
	completion.setItems(wstrings);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.makeCompletion("ca");
	QCOMPARE(completion.nextMatch(), carpet);
	QCOMPARE(completion.nextMatch(), carp);
	QCOMPARE(completion.previousMatch(), carpet);
	QCOMPARE(completion.previousMatch(), carp);
}

QTEST_KDEMAIN(Test_KCompletion, 0)
