/* This file is part of the KDE libraries
   Copyright (C) 2003,2004,2005 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "katerange.h"

#include "kateattribute.h"

KateRange::KateRange()
{
}

KateRange::~KateRange()
{
}

bool KateRange::includes(uint lineNum) const
{
  return isValid() && (int)lineNum >= start().line() && (int)lineNum <= end().line();
}

bool KateRange::includesCol( int col ) const
{
  return start().col() <= col && end().col() > col;
}

int KateRange::includes( const KateTextCursor & cursor ) const
{
  return ((cursor < start()) ? -1 : ((cursor > end()) ? 1:0));
}

bool KateRange::contains( const KateRange & range ) const
{
  return range.start() >= start() && range.end() <= end();
}

bool KateRange::overlaps( const KateRange & range ) const
{ 
  if (range.start() <= start())
    return range.end() > start();
  
  else if (range.end() >= end())
    return range.start() < end();
  
  else
    return contains(range);
}

bool KateRange::boundaryAt(const KateTextCursor& cursor) const
{
  return isValid() && (cursor == start() || cursor == end());
}

bool KateRange::boundaryOn(uint lineNum) const
{
  return isValid() && (start().line() == (int)lineNum || end().line() == (int)lineNum);
}

KateRange::operator QString( ) const
{
  if (!this)
    return QString("[No Range]");
  
  return QString("[(%1, %2) to (%3, %4)]").arg(start().line()).arg(start().col()).arg(end().line()).arg(end().col());
}

KateTextRange::KateTextRange()
  : m_valid(false)
{
}

KateTextRange::KateTextRange( const KateTextCursor & start, const KateTextCursor & end )
  : m_start(start)
  , m_end(end)
  , m_valid(true)
{
}

KateTextRange::KateTextRange( int startline, int startcol, int endline, int endcol )
  : m_start(startline, startcol)
  , m_end(endline, endcol)
  , m_valid(true)
{
}

KateTextRange::~KateTextRange()
{
}

KateTextCursor& KateTextRange::start()
{
  return m_start;
}

KateTextCursor& KateTextRange::end()
{
  return m_end;
}

const KateTextCursor& KateTextRange::start() const
{
  return m_start;
}

const KateTextCursor& KateTextRange::end() const
{
  return m_end;
}
