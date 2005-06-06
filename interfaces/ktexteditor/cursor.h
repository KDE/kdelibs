/* This file is part of the KDE project
   Copyright (C) 2001-2005 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Christian Couder <christian@kdevelop.org>
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

#ifndef __ktexteditor_cursor_h__
#define __ktexteditor_cursor_h__

namespace KTextEditor
{

/**
 * Class to represent a cursor in the text
 */
class Cursor
{
  public:
    Cursor() : m_line(0), m_column(0) {}

    Cursor(int line, int col) : m_line(line), m_column(col) {}

    virtual ~Cursor () {}

    friend bool operator==(const Cursor& c1, const Cursor& c2)
      { return c1.line() == c2.line() && c1.column() == c2.column(); }

    friend bool operator!=(const Cursor& c1, const Cursor& c2)
      { return !(c1 == c2); }

    friend bool operator>(const Cursor& c1, const Cursor& c2)
      { return c1.line() > c2.line() || (c1.line() == c2.line() && c1.m_column > c2.m_column); }

    friend bool operator>=(const Cursor& c1, const Cursor& c2)
      { return c1.line() > c2.line() || (c1.line() == c2.line() && c1.m_column >= c2.m_column); }

    friend bool operator<(const Cursor& c1, const Cursor& c2)
      { return !(c1 >= c2); }

    friend bool operator<=(const Cursor& c1, const Cursor& c2)
      { return !(c1 > c2); }

    inline void position (int &pline, int &pcol) const { pline = line(); pcol = m_column; }

    inline int line() const { return m_line; }
    inline int column() const { return m_column; }

    virtual void setLine (int line) { m_line = line; }

    virtual void setColumn (int col) { m_column = col; }

    virtual void setPosition (const Cursor& pos) { m_line = pos.line(); m_column = pos.column(); }

    virtual void setPosition (int line, int col) { m_line = line; m_column = col; }

  protected:
    int m_line;
    int m_column;
};

}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
