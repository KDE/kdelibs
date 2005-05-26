/* This file is part of the KDE libraries
   Copyright (C) 2002,2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2003      Anakim Border <aborder@sources.sourceforge.net>

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

#include "katelinerange.h"

#include <kdebug.h>

#include "katedocument.h"

KateLineRange::KateLineRange(KateDocument* doc)
  : m_doc(doc)
  , m_textLine(0L)
  , m_line(-1)
  , m_virtualLine(-1)
  , m_viewLine(-1)
  , m_startCol(-1)
  , m_endCol(-1)
  , m_startX(-1)
  , m_endX(-1)
  , m_shiftX(0)
  , m_dirty(true)
  , m_wrap(false)
  , m_startsInvisibleBlock(false)
  , m_special(false)
{
}

KateLineRange::KateLineRange(const KateLineRange& copy)
{
  *this = copy;
}

KateLineRange& KateLineRange::operator= (const KateLineRange& r)
{
  if (this == &r)
    return *this;

  m_doc = r.doc();
  m_textLine = 0L;
  m_line = r.line();
  m_virtualLine = r.virtualLine();
  m_viewLine = r.viewLine();
  m_startCol = r.startCol();
  m_endCol = r.endCol();
  m_startX = r.startX();
  m_endX = r.endX();
  m_shiftX = r.shiftX();
  m_wrap = r.wrap();
  m_startsInvisibleBlock = r.startsInvisibleBlock();
  m_dirty = r.isDirty();
  m_special = false;
  Q_ASSERT(!r.m_special);
  
  return *this;
}

KateTextCursor KateLineRange::rangeStart() const
{
  return KateTextCursor(line(), startCol());
}

KateLineRange::~KateLineRange()
{
}

void KateLineRange::clear()
{
  m_textLine = 0L;
  m_line = -1;
  m_virtualLine = -1;
  m_startCol = -1;
  m_endCol = -1;
  m_startX = -1;
  m_shiftX = 0;
  m_endX = -1;
  m_viewLine = -1;
  m_wrap = false;
  m_startsInvisibleBlock = false;
  // not touching dirty
}

bool KateLineRange::includesCursor(const KateTextCursor& realCursor) const
{
  return realCursor.line() == line() && realCursor.col() >= startCol() && (!wrap() || realCursor.col() < endCol());
}

int KateLineRange::xOffset() const
{
  return startX() ? shiftX() : 0;
}

void KateLineRange::debugOutput() const
{
  kdDebug() << "KateLineRange: line " << line() << " cols [" << startCol() << " -> " << endCol() << "] x [" << startX() << " -> " << endX() << " off " << shiftX() << "] wrap " << wrap() << endl;
}

bool operator> (const KateLineRange& r, const KateTextCursor& c)
{
  return r.line() > c.line() || r.endCol() > c.col();
}

bool operator>= (const KateLineRange& r, const KateTextCursor& c)
{
  return r.line() > c.line() || r.endCol() >= c.col();
}

bool operator< (const KateLineRange& r, const KateTextCursor& c)
{
  return r.line() < c.line() || r.startCol() < c.col();
}

bool operator<= (const KateLineRange& r, const KateTextCursor& c)
{
  return r.line() < c.line() || r.startCol() <= c.col();
}

const KateTextLine::Ptr& KateLineRange::textLine() const
{
  if (m_special)
    return m_textLine;
  
  if (!m_textLine)
    m_textLine = m_doc->kateTextLine(line());
  
  if (!m_textLine)
    Q_ASSERT(m_textLine);
    
  return m_textLine;
}

int KateLineRange::line( ) const
{
  return m_line;
}

void KateLineRange::setLine( int line, int virtualLine )
{
  m_line = line;
  m_virtualLine = (virtualLine == -1) ? m_doc->getVirtualLine(line) : virtualLine;
  m_textLine = 0L;
  Q_ASSERT(!m_special);
}

int KateLineRange::virtualLine( ) const
{
  return m_virtualLine;
}

void KateLineRange::setVirtualLine( int virtualLine )
{
  m_virtualLine = virtualLine;
}

int KateLineRange::viewLine( ) const
{
  return m_viewLine;
}

void KateLineRange::setViewLine( int viewLine )
{
  m_viewLine = viewLine;
}

int KateLineRange::startCol( ) const
{
  return m_startCol;
}

void KateLineRange::setStartCol( int startCol )
{
  m_startCol = startCol;
}

int KateLineRange::endCol( ) const
{
  return m_endCol;
}

void KateLineRange::setEndCol( int endCol )
{
  m_endCol = endCol;
}

bool KateLineRange::wrap( ) const
{
  return m_wrap;
}

void KateLineRange::setWrap( bool wrap )
{
  m_wrap = wrap;
}

bool KateLineRange::isDirty( ) const
{
  return m_dirty;
}

bool KateLineRange::setDirty( bool dirty )
{
  m_dirty = dirty;
  return m_dirty;
}

int KateLineRange::startX( ) const
{
  return m_startX;
}

void KateLineRange::setStartX( int startX )
{
  m_startX = startX;
}

int KateLineRange::endX( ) const
{
  return m_endX;
}

void KateLineRange::setEndX(int endX)
{
  m_endX = endX;
}

bool KateLineRange::startsInvisibleBlock() const
{
  return m_startsInvisibleBlock;
}

void KateLineRange::setStartsInvisibleBlock(bool sib)
{
  m_startsInvisibleBlock = sib;
}

int KateLineRange::shiftX() const
{
  return m_shiftX;
}

void KateLineRange::setShiftX(int shiftX)
{
  m_shiftX = shiftX;
}

KateDocument* KateLineRange::doc() const
{
  return m_doc;
}

void KateLineRange::setSpecial( const KateTextLine::Ptr & textLine )
{
  m_textLine = textLine;
  m_special = true;
}

bool KateLineRange::isValid( ) const
{
  return line() != -1 && textLine();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
