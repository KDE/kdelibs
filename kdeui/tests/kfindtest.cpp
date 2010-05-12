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
