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
	VERIFY(completion.isEmpty());
	VERIFY(completion.items().isEmpty());
}

void
Test_KCompletion::insertionOrder()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	QSignalSpy spy1(&completion, SIGNAL(match(const QString &)));
	QSignalSpy spy3(&completion, SIGNAL(multipleMatches()));

	completion.setOrder(KCompletion::Insertion);
	VERIFY(completion.order() == KCompletion::Insertion);

	completion.setItems(strings);
	VERIFY(completion.items().count() == strings.count());

	completion.setCompletionMode(KGlobalSettings::CompletionShell);
	COMPARE(completion.makeCompletion("ca"), QString("carp"));
	VERIFY(spy1.count() == 1);
	VERIFY(spy1.takeFirst().at(0).toString() == QString("carp"));
	VERIFY(spy3.count() == 1); spy3.takeFirst();

	QSignalSpy spy2(&completion, SIGNAL(matches(const QStringList &)));
	completion.makeCompletion("ca");
	COMPARE(spy2.count(), 1);
	VERIFY(spy3.count() == 0); // shouldn't be signaled on 2nd call
	QStringList matches = spy2.takeFirst().at(0).toStringList();
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], carp);

	completion.setCompletionMode(KGlobalSettings::CompletionAuto);
	COMPARE(completion.makeCompletion("ca"), carpet);
	VERIFY(spy1.count() == 1);
	VERIFY(spy1.takeFirst().at(0).toString() == carpet);
}

void
Test_KCompletion::sortedOrder()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	QSignalSpy spy1(&completion, SIGNAL(match(const QString &)));
	QSignalSpy spy3(&completion, SIGNAL(multipleMatches()));

	completion.setOrder(KCompletion::Sorted);
	VERIFY(completion.order() == KCompletion::Sorted);

	completion.setItems(strings);
	VERIFY(completion.items().count() == 4);

	completion.setCompletionMode(KGlobalSettings::CompletionShell);
	COMPARE(completion.makeCompletion("ca"), QString("carp"));
	VERIFY(spy1.count() == 1);
	COMPARE(spy1.takeFirst().at(0).toString(), QString("carp"));
	VERIFY(spy3.count() == 1); spy3.takeFirst();

	QSignalSpy spy2(&completion, SIGNAL(matches(const QStringList &)));
	completion.makeCompletion("ca");
	COMPARE(spy2.count(), 1);
	VERIFY(spy3.count() == 0); // shouldn't be signaled on 2nd call

	QStringList matches = spy2.takeFirst().at(0).toStringList();
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carp);
	COMPARE(matches[1], carpet);

	completion.setCompletionMode(KGlobalSettings::CompletionAuto);
	COMPARE(completion.makeCompletion("ca"), carp);
	VERIFY(spy1.count() == 1);
	COMPARE(spy1.takeFirst().at(0).toString(), carp);
}

void
Test_KCompletion::weightedOrder()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	QSignalSpy spy1(&completion, SIGNAL(match(const QString &)));
	QSignalSpy spy3(&completion, SIGNAL(multipleMatches()));

	completion.setOrder(KCompletion::Weighted);
	VERIFY(completion.order() == KCompletion::Weighted);

	completion.setItems(wstrings);
	VERIFY(completion.items().count() == 4);

	completion.setCompletionMode(KGlobalSettings::CompletionShell);
	COMPARE(completion.makeCompletion("ca"), QString("carp"));
	spy1.takeFirst(); // empty the list
	VERIFY(spy3.count() == 1); spy3.takeFirst();

	QSignalSpy spy2(&completion, SIGNAL(matches(const QStringList &)));
	completion.makeCompletion("ca");
	COMPARE(spy2.count(), 1);
	VERIFY(spy3.count() == 0); // shouldn't be signaled on 2nd call

	QStringList matches = spy2.takeFirst().at(0).toStringList();
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], carp);

	completion.setCompletionMode(KGlobalSettings::CompletionAuto);
	COMPARE(completion.makeCompletion("ca"), carpet);

	matches = completion.substringCompletion("ca");
	VERIFY(matches.count() == 3);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], coolcat);
	COMPARE(matches[2], carp);
}

void
Test_KCompletion::substringCompletion_Insertion()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Insertion);
	completion.setItems(strings);
	VERIFY(completion.items().count() == 4);

	QStringList matches = completion.substringCompletion("c");
	VERIFY(matches.count() == 4);
	COMPARE(matches[0], clampet);
	COMPARE(matches[1], coolcat);
	COMPARE(matches[2], carpet);
	COMPARE(matches[3], carp);

	matches = completion.substringCompletion("ca");
	VERIFY(matches.count() == 3);
	COMPARE(matches[0], coolcat);
	COMPARE(matches[1], carpet);
	COMPARE(matches[2], carp);

	matches = completion.substringCompletion("car");
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], carp);

	matches = completion.substringCompletion("pet");
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], clampet);
	COMPARE(matches[1], carpet);
}

void
Test_KCompletion::substringCompletion_Sorted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Sorted);
	completion.setItems(strings);
	VERIFY(completion.items().count() == 4);

	QStringList matches = completion.substringCompletion("c");
	VERIFY(matches.count() == 4);
	COMPARE(matches[0], carp);
	COMPARE(matches[1], carpet);
	COMPARE(matches[2], clampet);
	COMPARE(matches[3], coolcat);

	matches = completion.substringCompletion("ca");
	VERIFY(matches.count() == 3);
	COMPARE(matches[0], carp);
	COMPARE(matches[1], carpet);
	COMPARE(matches[2], coolcat);

	matches = completion.substringCompletion("car");
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carp);
	COMPARE(matches[1], carpet);

	matches = completion.substringCompletion("pet");
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], clampet);
}

void
Test_KCompletion::substringCompletion_Weighted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Weighted);
	completion.setItems(wstrings);
	VERIFY(completion.items().count() == 4);

	QStringList matches = completion.substringCompletion("c");
	VERIFY(matches.count() == 4);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], clampet);
	COMPARE(matches[2], coolcat);
	COMPARE(matches[3], carp);

	matches = completion.substringCompletion("ca");
	VERIFY(matches.count() == 3);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], coolcat);
	COMPARE(matches[2], carp);

	matches = completion.substringCompletion("car");
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], carp);

	matches = completion.substringCompletion("pet");
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], clampet);
}

void
Test_KCompletion::allMatches_Insertion()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Insertion);
	completion.setItems(strings);
	VERIFY(completion.items().count() == 4);

	QStringList matches = completion.allMatches("c");
	VERIFY(matches.count() == 4);
	COMPARE(matches[0], clampet);
	COMPARE(matches[1], coolcat);
	COMPARE(matches[2], carpet);
	COMPARE(matches[3], carp);

	matches = completion.allMatches("ca");
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], carp);

	matches = completion.allMatches("pet");
	VERIFY(matches.count() == 0);
}

void
Test_KCompletion::allMatches_Sorted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Sorted);
	completion.setItems(strings);
	VERIFY(completion.items().count() == 4);

	QStringList matches = completion.allMatches("c");
	VERIFY(matches.count() == 4);
	COMPARE(matches[0], carp);
	COMPARE(matches[1], carpet);
	COMPARE(matches[2], clampet);
	COMPARE(matches[3], coolcat);

	matches = completion.allMatches("ca");
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carp);
	COMPARE(matches[1], carpet);

	matches = completion.allMatches("pet");
	VERIFY(matches.count() == 0);
}

void
Test_KCompletion::allMatches_Weighted()
{
	KCompletion completion;
	completion.setEnableSounds(false);
	completion.setCompletionMode(KGlobalSettings::CompletionAuto);

	completion.setOrder(KCompletion::Weighted);
	completion.setItems(wstrings);
	VERIFY(completion.items().count() == 4);

	QStringList matches = completion.allMatches("c");
	VERIFY(matches.count() == 4);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], clampet);
	COMPARE(matches[2], coolcat);
	COMPARE(matches[3], carp);

	matches = completion.allMatches("ca");
	VERIFY(matches.count() == 2);
	COMPARE(matches[0], carpet);
	COMPARE(matches[1], carp);

	matches = completion.allMatches("pet");
	VERIFY(matches.count() == 0);
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
	COMPARE(completion.nextMatch(), carpet);
	COMPARE(completion.nextMatch(), carp);
	COMPARE(completion.previousMatch(), carpet);
	COMPARE(completion.previousMatch(), carp);
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
	COMPARE(completion.nextMatch(), carp);
	COMPARE(completion.nextMatch(), carpet);
	COMPARE(completion.previousMatch(), carp);
	COMPARE(completion.previousMatch(), carpet);
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
	COMPARE(completion.nextMatch(), carpet);
	COMPARE(completion.nextMatch(), carp);
	COMPARE(completion.previousMatch(), carpet);
	COMPARE(completion.previousMatch(), carp);
}

QTTEST_KDEMAIN(Test_KCompletion, 0)
