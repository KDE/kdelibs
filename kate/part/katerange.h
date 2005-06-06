/* This file is part of the KDE libraries
   Copyright (C) 2002 Christian Couder <christian@kdevelop.org>
   Copyright (C) 2001, 2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

#ifndef kate_range_h
#define kate_range_h

#include "katecursor.h"

class KateRange
{
  public:
    KateRange ();
    virtual ~KateRange ();

    bool includes(uint lineNum) const;
    bool includesCol( int col ) const;
    int includes( const KTextEditor::Cursor & cursor ) const;
    bool contains( const KateRange & range ) const;
    bool overlaps( const KateRange & range ) const;
    bool boundaryAt(const KTextEditor::Cursor& cursor) const;
    bool boundaryOn(uint lineNum) const;
    operator QString( ) const;

    virtual bool isValid() const = 0;
    virtual KTextEditor::Cursor& start() = 0;
    virtual KTextEditor::Cursor& end() = 0;
    virtual const KTextEditor::Cursor& start() const = 0;
    virtual const KTextEditor::Cursor& end() const = 0;
};

class KateTextRange : public KateRange
{
  public:
    KateTextRange();
    KateTextRange(int startline, int startcol, int endline, int endcol);
    KateTextRange(const KTextEditor::Cursor& start, const KTextEditor::Cursor& end);
    virtual ~KateTextRange ();

    virtual bool isValid() const { return m_valid; };
    void setValid(bool valid) {
      m_valid = valid;
      if( valid )
        normalize();
    };

    virtual KTextEditor::Cursor& start();
    virtual KTextEditor::Cursor& end();
    virtual const KTextEditor::Cursor& start() const;
    virtual const KTextEditor::Cursor& end() const;

    /* if range is not valid, the result is undefined
      if cursor is before start -1 is returned, if cursor is within range 0 is returned if cursor is after end 1 is returned*/
    inline int cursorInRange(const KTextEditor::Cursor &cursor) const {
      return ((cursor<m_start)?(-1):((cursor>m_end)?1:0));
    }

    inline void normalize() {
      if( m_start > m_end )
      {
        KTextEditor::Cursor tmp  (m_start);
        m_start = m_end;
        m_end = tmp;
      }
    }

  protected:
    KTextEditor::Cursor m_start, m_end;
    bool m_valid;
};


class KateBracketRange : public KateTextRange
{
  public:
    KateBracketRange()
      : KateTextRange()
      , m_minIndent(0)
    {
    };

    KateBracketRange(int startline, int startcol, int endline, int endcol, int minIndent)
      : KateTextRange(startline, startcol, endline, endcol)
      , m_minIndent(minIndent)
    {
    };

    KateBracketRange(const KTextEditor::Cursor& start, const KTextEditor::Cursor& end, int minIndent)
      : KateTextRange(start, end)
      , m_minIndent(minIndent)
    {
    };

    int getMinIndent() const
    {
      return m_minIndent;
    }

    void setIndentMin(int m)
    {
      m_minIndent = m;
    }

  protected:
    int m_minIndent;
};


#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
