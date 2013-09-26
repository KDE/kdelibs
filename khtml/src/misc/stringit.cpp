/*
    This file is part of the KDE libraries

    Copyright (C) 2004 Apple Computer

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "stringit.h"

namespace khtml {

uint TokenizerString::length() const
{
    uint length = m_currentString.m_length;
    if (!m_pushedChar1.isNull()) {
        ++length;
        if (!m_pushedChar2.isNull())
            ++length;
    }
    if (m_composite) {
        QList<TokenizerSubstring>::ConstIterator i = m_substrings.begin();
        QList<TokenizerSubstring>::ConstIterator e = m_substrings.end();
        for (; i != e; ++i)
            length += (*i).m_length;
    }
    return length;
}

void TokenizerString::clear()
{
    m_pushedChar1 = 0;
    m_pushedChar2 = 0;
    m_currentChar = 0;
    m_currentString.clear();
    m_substrings.clear();
    m_lines = 0;
    m_composite = false;
}

void TokenizerString::append(const TokenizerSubstring &s)
{
    if (s.m_length) {
        if (!m_currentString.m_length) {
            m_currentString = s;
	} else {
            m_substrings.append(s);
            m_composite = true;
        }
    }
}

void TokenizerString::prepend(const TokenizerSubstring &s)
{
    assert(!escaped());
    if (s.m_length) {
        if (!m_currentString.m_length)
            m_currentString = s;
        else {
            // Shift our m_currentString into our list.
            m_substrings.prepend(m_currentString);
            m_currentString = s;
            m_composite = true;
        }
    }
}

void TokenizerString::append(const TokenizerString &s)
{
    assert(!s.escaped());
    append(s.m_currentString);
    if (s.m_composite) {
        QList<TokenizerSubstring>::ConstIterator i = s.m_substrings.begin();
        QList<TokenizerSubstring>::ConstIterator e = s.m_substrings.end();
        for (; i != e; ++i)
            append(*i);
    }
    m_currentChar = m_pushedChar1.isNull() ? m_currentString.m_current : &m_pushedChar1;
}

void TokenizerString::prepend(const TokenizerString &s)
{
    assert(!escaped());
    assert(!s.escaped());
    if (s.m_composite) {
        QList<TokenizerSubstring>::ConstIterator e = s.m_substrings.end();
		while (e != s.m_substrings.begin())
		{
			--e;
            prepend(*e);
		}
    }
    prepend(s.m_currentString);
    m_currentChar = m_pushedChar1.isNull() ? m_currentString.m_current : &m_pushedChar1;
}

void TokenizerString::advanceSubstring()
{
    if (m_composite) {
        m_currentString = m_substrings.first();
        m_substrings.removeFirst();
        if (m_substrings.isEmpty())
            m_composite = false;
    } else {
        m_currentString.clear();
    }
}

QString TokenizerString::toString() const
{
    QString result;
    if (!m_pushedChar1.isNull()) {
        result.append(m_pushedChar1);
        if (!m_pushedChar2.isNull())
            result.append(m_pushedChar2);
    }
    m_currentString.appendTo(result);
    if (m_composite) {
        QList<TokenizerSubstring>::ConstIterator i = m_substrings.begin();
        QList<TokenizerSubstring>::ConstIterator e = m_substrings.end();
        for (; i != e; ++i)
            (*i).appendTo(result);
    }
    return result;
}

}
