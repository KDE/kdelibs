/*
    Copyright (C) 2004, Arend van Beelen jr. <arend@auton.nl>
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

#include "../kfind.h"
#include "../kfinddialog.h"
#include "kfindtest.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <stdlib.h>
#include <assert.h>

static bool check(QString txt, QString a, QString b) // from kurltest
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

void KFindTest::changeText(int line, const QString &text)
{
	Q_ASSERT(line < m_text.count());
	Q_ASSERT(m_find != 0);

	m_line = line;
	m_text[line] = text;
	m_find->setData(line, text);
}

void KFindTest::find(const QString &pattern, long options)
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

void KFindTest::findNext(const QString &pattern)
{
	Q_ASSERT(m_find != 0);

	if(!pattern.isNull())
	{
		m_find->setPattern(pattern);
	}

	KFind::Result result = KFind::NoMatch;
	do
	{
		//kdDebug() << "m_line: " << m_line << endl;

		result = m_find->find();

		if(result == KFind::NoMatch && m_line < m_text.count())
		{
			//kdDebug() << "incrementing m_line..." << endl;
			if(m_find->options() & KFind::FindIncremental)
				m_find->setData(m_line, m_text[m_line]);
			else
				m_find->setData(m_text[m_line]);

			m_line++;
		}
	} while(result == KFind::NoMatch && m_line < m_text.count());
	//kdDebug() << "find next completed" << m_line << endl;
}

void KFindTest::slotHighlight(const QString &text, int index, int matchedLength)
{
	m_hits.append("line: \"" + text + "\", index: " + QString::number(index) +
	              ", length: " + QString::number(matchedLength) + "\n");
}

void KFindTest::slotHighlight(int id, int index, int matchedLength)
{
	m_hits.append("line: \"" + m_text[id] + "\", index: " + QString::number(index) +
	              ", length: " + QString::number(matchedLength) + "\n");
}

int main(int argc, char **argv)
{
	KCmdLineArgs::init(argc, argv, "kfindtest", "KFindTest", 0, 0, false);
	KApplication app;

	QString text = "This file is part of the KDE project.\n"
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

	QString output1 = "line: \"This file is part of the KDE project.\", index: 0, length: 4\n"
	                  "line: \"This library is free software; you can redistribute it and/or\", index: 0, length: 4\n"
	                  "line: \"    This library is distributed in the hope that it will be useful,\", index: 4, length: 4\n"
	                  "line: \"    along with this library; see the file COPYING.LIB.  If not, write to\", index: 15, length: 4\n";

	QString output2 = "line: \"This file is part of the KDE project.\", index: 0, length: 0\n"
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

	QString output3 = "line: \"This file is part of the KDE project.\", index: 0, length: 0\n"
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

	KFindTest *test = new KFindTest(QStringList::split('\n', text, true));

	kdDebug() << "Plain static search..." << endl;

	// first we do a simple text searching the text and doing a few find nexts
	test->find("This", 0);
	test->findNext();
	test->findNext();
	test->findNext();
	test->findNext();
	test->findNext();

	check("result", test->hits().join(""), output1);
	test->clearHits();
	kdDebug() << "PASSED" << endl;

	kdDebug() << "FindIncremental with static contents..." << endl;

	// now we'll do some searches using FindIncremental
	test->find("", KFind::FindIncremental);
	test->findNext("i");
	test->findNext("is");
	test->findNext("ist");
	test->findNext();
	test->findNext("istri");
	test->findNext("istr");
	test->findNext("ist");
	test->findNext("is");
	test->findNext("W");
	test->findNext("WA");
	test->findNext("WARRANTY");
	test->findNext("Free");
	test->findNext("Software Foundation");

	check("result", test->hits().join(""), output2);
	test->clearHits();
	kdDebug() << "PASSED" << endl;

	kdDebug() << "FindIncremental with dynamic contents..." << endl;

	// now do that again but with pages that change between searches
	test->find("", KFind::FindIncremental);
	test->findNext("i");
	test->findNext("is");
	test->findNext("ist");
	test->findNext("istr");
	test->findNext();
	test->changeText(1, "The second line now looks a whole lot different.");
	test->findNext("istri");
	test->findNext("istr");
	test->findNext("ist");
	test->findNext("is");
	test->findNext("i");
	test->findNext("W");
	test->findNext("WA");
	test->findNext("WARRANTY");
	test->changeText(6, "    but WITHOUT ANY xxxx; without even the implied warranty of");
	test->findNext("WARRAN");
	test->findNext("Free");
	test->findNext("Software Foundation");

	check("result", test->hits().join(""), output3);
	test->clearHits();
	kdDebug() << "PASSED" << endl;

	//return app.exec();
	delete test;
	return 0;
}

#include "kfindtest.moc"
