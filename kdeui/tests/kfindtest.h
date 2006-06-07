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

#ifndef KFINDTEST_H
#define KFINDTEST_H

#include <qobject.h>
#include <qstringlist.h>

class KFind;

class KFindTest : public QObject
{
	Q_OBJECT

	public:
		KFindTest(const QStringList &text) :
		  QObject(0),
		  m_find(0),
		  m_text(text),
		  m_line(0)
		{}

		void find(const QString &pattern, long options = 0);
		void findNext(const QString &pattern = QString());

		void changeText(int line, const QString &text);

		const QStringList &hits() const { return m_hits; }
		void clearHits() { m_hits.clear(); }

	public Q_SLOTS:
		void slotHighlight(const QString &text, int index, int matchedLength);
		void slotHighlight(int id, int index, int matchedLengthlength);

	private:
		KFind                 *m_find;
		QStringList            m_text;
		int                   m_line;
		QStringList            m_hits;
};

#endif
