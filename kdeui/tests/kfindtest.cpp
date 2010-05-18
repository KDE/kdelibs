/*
    Copyright (C) 2004, Arend van Beelen jr. <arend@auton.nl>
    Copyright (C) 2010, David Faure <faure@kde.org>

    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kfindtest.h"
#include "qtest_kde.h"

#include "kfind.h"
#include <kdebug.h>

#include <assert.h>

void KFindRecorder::changeText(int line, const QString &text)
{
	Q_ASSERT(line < m_text.count());
	Q_ASSERT(m_find != 0);

	m_line = line;
	m_text[line] = text;
	m_find->setData(line, text);
}

void KFindRecorder::find(const QString &pattern, long options)
{
	delete m_find;
	m_find = new KFind(pattern, options, 0);
        // Prevent dialogs from popping up
        m_find->closeFindNextDialog();

	connect(m_find, SIGNAL(highlight(const QString &, int, int)),
	                SLOT(slotHighlight(const QString &, int, int)));
	connect(m_find, SIGNAL(highlight(int, int, int)),
	                SLOT(slotHighlight(int, int, int)));

	m_line = 0;
	KFind::Result result = KFind::NoMatch;
	do
	{
		if(options & KFind::FindIncremental)
			m_find->setData(m_line, m_text[m_line]);
		else
			m_find->setData(m_text[m_line]);

		m_line++;

		result = m_find->find();
	} while(result == KFind::NoMatch && m_line < m_text.count());
}

bool KFindRecorder::findNext(const QString &pattern)
{
	Q_ASSERT(m_find != 0);

	if(!pattern.isNull())
	{
		m_find->setPattern(pattern);
	}

	KFind::Result result = KFind::NoMatch;
	do
	{
		//kDebug() << "m_line: " << m_line;

		result = m_find->find();

		if(result == KFind::NoMatch && m_line < m_text.count())
		{
			//kDebug() << "incrementing m_line...";
			if(m_find->options() & KFind::FindIncremental)
				m_find->setData(m_line, m_text[m_line]);
			else
				m_find->setData(m_text[m_line]);

			m_line++;
		}
	} while(result == KFind::NoMatch && m_line < m_text.count());
	//kDebug() << "find next completed" << m_line;

        return result != KFind::NoMatch;
}

void KFindRecorder::slotHighlight(const QString &text, int index, int matchedLength)
{
	m_hits.append("line: \"" + text + "\", index: " + QString::number(index) +
	              ", length: " + QString::number(matchedLength) + "\n");
}

void KFindRecorder::slotHighlight(int id, int index, int matchedLength)
{
	m_hits.append("line: \"" + m_text[id] + "\", index: " + QString::number(index) +
	              ", length: " + QString::number(matchedLength) + "\n");
}

////

TestKFind::TestKFind()
    : QObject()
{
    m_text = "This file is part of the KDE project.\n"
             "This library is free software; you can redistribute it and/or\n"
             "modify it under the terms of the GNU Library General Public\n"
             "License version 2, as published by the Free Software Foundation.\n"
             "\n"
             "    This library is distributed in the hope that it will be useful,\n"
             "    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
             "    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
             "    Library General Public License for more details.\n"
             "\n"
             "    You should have received a copy of the GNU Library General Public License\n"
             "    along with this library; see the file COPYING.LIB.  If not, write to\n"
             "    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
             "    Boston, MA 02110-1301, USA.\n";
}

void TestKFind::testStaticFindString_data()
{
    // Tests for the core method "static KFind::find"
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<int>("startIndex");
    QTest::addColumn<int>("options");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<int>("expectedMatchedLength");

    QTest::newRow("simple (0)") << "abc" << "a" << 0 << 0 << 0 << 1;
    QTest::newRow("simple (1)") << "abc" << "b" << 0 << 0 << 1 << 1;
    QTest::newRow("not found") << "abca" << "ba" << 0 << 0 << -1 << 0;
    QTest::newRow("from index") << "abc bc" << "b" << 3 << 0 << 4 << 1;
    QTest::newRow("from exact index") << "abc bc" << "b" << 4 << 0 << 4 << 1;
    QTest::newRow("past index (not found)") << "abc bc" << "b" << 5 << 0 << -1 << 0;
    QTest::newRow("dot") << "ab." << "b." << 0 << 0 << 1 << 2;
    QTest::newRow("^should fail") << "text" << "^tex" << 0 << 0 << -1 << 0;
    QTest::newRow("multiline with \\n") << "foo\nbar" << "o\nb" << 0 << 0 << 2 << 3;
    QTest::newRow("whole words ok") << "abc bcbc bc bmore be" << "bc" << 0 << int(KFind::WholeWordsOnly) << 9 << 2;
    QTest::newRow("whole words not found") << "abab abx" << "ab" << 0 << int(KFind::WholeWordsOnly) << -1 << 0;
    QTest::newRow("whole words not found (_)") << "abab ab_" << "ab" << 0 << int(KFind::WholeWordsOnly) << -1 << 0;
    QTest::newRow("whole words ok (.)") << "ab." << "ab" << 0 << int(KFind::WholeWordsOnly) << 0 << 2;
    QTest::newRow("backwards") << "abc bcbc8bc" << "bc" << 10 << int(KFind::FindBackwards) << 9 << 2;
    QTest::newRow("backwards again") << "abc bcbc8bc" << "bc" << 8 << int(KFind::FindBackwards) << 6 << 2;
    QTest::newRow("backwards 2") << "abc bcbc8bc" << "bc" << 5 << int(KFind::FindBackwards) << 4 << 2;
    QTest::newRow("backwards 3") << "abc bcbc8bc" << "bc" << 3 << int(KFind::FindBackwards) << 1 << 2;
    QTest::newRow("empty (0)") << "a" << "" << 0 << int(0) << 0 << 0;
    QTest::newRow("empty (1)") << "a" << "" << 1 << int(0) << 1 << 0; // kreplacetest testReplaceBlankSearch relies on this
    QTest::newRow("at end, not found") << "a" << "b" << 1 << int(0) << -1 << 0; // just for catching the while(index<text.length()) bug
    QTest::newRow("back, not found") << "a" << "b" << 0 << int(KFind::FindBackwards) << -1 << 0;
    QTest::newRow("back, at begin, found") << "a" << "a" << 0 << int(KFind::FindBackwards) << 0 << 1;
    QTest::newRow("back, at end, found") << "a" << "a" << 1 << int(KFind::FindBackwards) << 0 << 1;
    QTest::newRow("back, text shorter than pattern") << "a" << "abcd" << 0 << int(KFind::FindBackwards) << -1 << 0;
}

void TestKFind::testStaticFindString()
{
    // Tests for the core method "static KFind::find(text, regexp)"
    QFETCH(QString, text);
    QFETCH(QString, pattern);
    QFETCH(int, startIndex);
    QFETCH(int, options);
    QFETCH(int, expectedResult);
    QFETCH(int, expectedMatchedLength);

    int matchedLength;
    const int result = KFind::find(text, pattern, startIndex, options, &matchedLength);
    QCOMPARE(result, expectedResult);
    QCOMPARE(matchedLength, expectedMatchedLength);
}

void TestKFind::testStaticFindRegexp_data()
{
    // Tests for the core method "static KFind::find"
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<int>("startIndex");
    QTest::addColumn<int>("options");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<int>("expectedMatchedLength");

    QTest::newRow("simple (0)") << "abc" << "a" << 0 << 0 << 0 << 1;
    QTest::newRow("simple (1)") << "abc" << "b" << 0 << 0 << 1 << 1;
    QTest::newRow("not found") << "abca" << "ba" << 0 << 0 << -1 << 0;
    QTest::newRow("from index") << "abc bc" << "b" << 3 << 0 << 4 << 1;
    QTest::newRow("from exact index") << "abc bc" << "b" << 4 << 0 << 4 << 1;
    QTest::newRow("past index (not found)") << "abc bc" << "b" << 5 << 0 << -1 << 0;
    QTest::newRow("dot") << "abc" << "b." << 0 << 0 << 1 << 2;
    QTest::newRow("^simple") << "text" << "^tex" << 0 << 0 << 0 << 3;
    QTest::newRow("^multiline first") << "foo\nbar" << "^f" << 0 << 0 << 0 << 1;
    QTest::newRow("^multiline last") << "foo\nbar" << "^bar" << 0 << 0 << 4 << 3;
    QTest::newRow("^multiline with index") << "boo\nbar" << "^b" << 1 << 0 << 4 << 1;
    QTest::newRow("simple$") << "text" << "xt$" << 0 << 0 << 2 << 2;
    QTest::newRow("$ backwards") << "text" << "xt$" << 4 << int(KFind::FindBackwards) << 2 << 2;
    QTest::newRow("multiline$") << "foo\nbar" << "oo$" << 0 << 0 << 1 << 2;
    QTest::newRow("multiline$ intermediary line") << "foo\nbar\nagain bar" << "r$" << 0 << 0 << 6 << 1;
    QTest::newRow("multiline$ with index, last line") << "foo\nbar\nagain bar" << "r$" << 7 << 0 << 16 << 1;
    QTest::newRow("multiline$ backwards") << "foo\nbar" << "oo$" << 7 << int(KFind::FindBackwards) << 1 << 2;
    QTest::newRow("multiline with \\n") << "foo\nbar" << "o\nb" << 0 << 0 << 2 << 3;
    QTest::newRow("whole words ok") << "abc bcbc bc bmore be" << "b." << 0 << int(KFind::WholeWordsOnly) << 9 << 2;
    QTest::newRow("whole words not found") << "abab abx" << "ab" << 0 << int(KFind::WholeWordsOnly) << -1 << 0;
    QTest::newRow("whole words not found (_)") << "abab ab_" << "ab" << 0 << int(KFind::WholeWordsOnly) << -1 << 0;
    QTest::newRow("whole words ok (.)") << "ab." << "ab" << 0 << int(KFind::WholeWordsOnly) << 0 << 2;
    QTest::newRow("backwards") << "abc bcbc bc" << "b." << 10 << int(KFind::FindBackwards) << 9 << 2;
    QTest::newRow("empty (0)") << "a" << "" << 0 << int(0) << 0 << 0;
    QTest::newRow("empty (1)") << "a" << "" << 1 << int(0) << 1 << 0; // kreplacetest testReplaceBlankSearch relies on this
    QTest::newRow("at end, not found") << "a" << "b" << 1 << int(0) << -1 << 0; // just for catching the while(index<text.length()) bug
    QTest::newRow("back, not found") << "a" << "b" << 0 << int(KFind::FindBackwards) << -1 << 0;
    QTest::newRow("back, at begin, found") << "a" << "a" << 0 << int(KFind::FindBackwards) << 0 << 1;
    QTest::newRow("back, at end, found") << "a" << "a" << 1 << int(KFind::FindBackwards) << 0 << 1;
    QTest::newRow("back, text shorter than pattern") << "a" << "abcd" << 0 << int(KFind::FindBackwards) << -1 << 0;
}

void TestKFind::testStaticFindRegexp()
{
    // Tests for the core method "static KFind::find(text, regexp)"
    QFETCH(QString, text);
    QFETCH(QString, pattern);
    QFETCH(int, startIndex);
    QFETCH(int, options);
    QFETCH(int, expectedResult);
    QFETCH(int, expectedMatchedLength);

    int matchedLength;
    const int result = KFind::find(text, QRegExp(pattern), startIndex, options, &matchedLength);
    QCOMPARE(result, expectedResult);
    QCOMPARE(matchedLength, expectedMatchedLength);
}

void TestKFind::testSimpleSearch()
{
    // first we do a simple text searching the text and doing a few find nexts
    KFindRecorder test(m_text.split('\n'));
    test.find("This", 0);
    while (test.findNext()) {}

    const QString output1 =
        "line: \"This file is part of the KDE project.\", index: 0, length: 4\n"
        "line: \"This library is free software; you can redistribute it and/or\", index: 0, length: 4\n"
        "line: \"    This library is distributed in the hope that it will be useful,\", index: 4, length: 4\n"
        "line: \"    along with this library; see the file COPYING.LIB.  If not, write to\", index: 15, length: 4\n";

    QCOMPARE(test.hits().join(""), output1);
}

void TestKFind::testSimpleRegexp()
{
    KFindRecorder test(m_text.split('\n'));
    test.find("W.R+ANT[YZ]", KFind::RegularExpression | KFind::CaseSensitive);
    while (test.findNext()) {}
    const QString output =
        "line: \"    but WITHOUT ANY WARRANTY; without even the implied warranty of\", index: 20, length: 8\n";
    QCOMPARE(test.hits().join(""), output);
}

void TestKFind::testLineBeginRegexp()
{
    // Let's see what QRegExp can do on a big text (like in KTextEdit)
    {
        const QString foobar = "foo\nbar";
        const int idx = foobar.indexOf(QRegExp("^bar"));
        QCOMPARE(idx, -1); // it doesn't find it. No /m support, as they say. Too bad.
    }

    // If we split, it works, but then looking for "foo\nbar" won't work...
    KFindRecorder test(m_text.split('\n'));
    test.find("^License", KFind::RegularExpression);
    while (test.findNext()) {}
    const QString output =
        "line: \"License version 2, as published by the Free Software Foundation.\", index: 0, length: 7\n";
    QCOMPARE(test.hits().join(""), output);
}

void TestKFind::testFindIncremental()
{
    // FindIncremental with static contents...

    KFindRecorder test(m_text.split('\n'));
    test.find("", KFind::FindIncremental);
    test.findNext("i");
    test.findNext("is");
    test.findNext("ist");
    test.findNext();
    test.findNext("istri");
    test.findNext("istr");
    test.findNext("ist");
    test.findNext("is");
    test.findNext("W");
    test.findNext("WA");
    test.findNext("WARRANTY");
    test.findNext("Free");
    test.findNext("Software Foundation");

    const QString output2 =
        "line: \"This file is part of the KDE project.\", index: 0, length: 0\n"
        "line: \"This file is part of the KDE project.\", index: 2, length: 1\n"
        "line: \"This file is part of the KDE project.\", index: 2, length: 2\n"
        "line: \"This library is free software; you can redistribute it and/or\", index: 42, length: 3\n"
        "line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 3\n"
        "line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 5\n"
        "line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 4\n"
        "line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 3\n"
        "line: \"This file is part of the KDE project.\", index: 2, length: 2\n"
        "line: \"This library is free software; you can redistribute it and/or\", index: 25, length: 1\n"
        "line: \"This library is free software; you can redistribute it and/or\", index: 25, length: 2\n"
        "line: \"    but WITHOUT ANY WARRANTY; without even the implied warranty of\", index: 20, length: 8\n"
        "line: \"This library is free software; you can redistribute it and/or\", index: 16, length: 4\n"
        "line: \"License version 2, as published by the Free Software Foundation.\", index: 44, length: 19\n";

    QCOMPARE(test.hits().join(""), output2);
}

void TestKFind::testFindIncrementalDynamic()
{
    // Now do that again but with pages that change between searches
    KFindRecorder test(m_text.split('\n'));

    test.find("", KFind::FindIncremental);
    test.findNext("i");
    test.findNext("is");
    test.findNext("ist");
    test.findNext("istr");
    test.findNext();
    test.changeText(1, "The second line now looks a whole lot different.");
    test.findNext("istri");
    test.findNext("istr");
    test.findNext("ist");
    test.findNext("is");
    test.findNext("i");
    test.findNext("W");
    test.findNext("WA");
    test.findNext("WARRANTY");
    test.changeText(6, "    but WITHOUT ANY xxxx; without even the implied warranty of");
    test.findNext("WARRAN");
    test.findNext("Free");
    test.findNext("Software Foundation");

    const QString output3 =
        "line: \"This file is part of the KDE project.\", index: 0, length: 0\n"
        "line: \"This file is part of the KDE project.\", index: 2, length: 1\n"
        "line: \"This file is part of the KDE project.\", index: 2, length: 2\n"
        "line: \"This library is free software; you can redistribute it and/or\", index: 42, length: 3\n"
        "line: \"This library is free software; you can redistribute it and/or\", index: 42, length: 4\n"
        "line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 4\n"
        "line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 5\n"
        "line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 4\n"
        "line: \"    This library is distributed in the hope that it will be useful,\", index: 21, length: 3\n"
        "line: \"This file is part of the KDE project.\", index: 2, length: 2\n"
        "line: \"This file is part of the KDE project.\", index: 2, length: 1\n"
        "line: \"The second line now looks a whole lot different.\", index: 18, length: 1\n"
        "line: \"License version 2, as published by the Free Software Foundation.\", index: 48, length: 2\n"
        "line: \"    but WITHOUT ANY WARRANTY; without even the implied warranty of\", index: 20, length: 8\n"
        "line: \"    but WITHOUT ANY xxxx; without even the implied warranty of\", index: 51, length: 6\n"
        "line: \"License version 2, as published by the Free Software Foundation.\", index: 39, length: 4\n"
        "line: \"License version 2, as published by the Free Software Foundation.\", index: 44, length: 19\n";

    QCOMPARE(test.hits().join(""), output3);
}

QTEST_KDEMAIN(TestKFind, GUI)

#include "kfindtest.moc"
