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

#ifndef _KATE_LINERANGE_H_
#define _KATE_LINERANGE_H_


#include "katecursor.h"
#include "katetextline.h"

class QTextLayout;
class KateDocument;

class KateLineRange
{
  public:
    KateLineRange(KateDocument* doc = 0L);
    KateLineRange(const KateLineRange& copy);
    ~KateLineRange();

    void operator=(const KateLineRange& r);

    KateDocument* doc() const;
    KateTextCursor rangeStart() const;

    void clear();
    bool isValid() const;

    bool includesCursor(const KateTextCursor& realCursor) const;

    friend bool operator> (const KateLineRange& r, const KateTextCursor& c);
    friend bool operator>= (const KateLineRange& r, const KateTextCursor& c);
    friend bool operator< (const KateLineRange& r, const KateTextCursor& c);
    friend bool operator<= (const KateLineRange& r, const KateTextCursor& c);

    // Override current textLine. Only use when you know what you're doing.
    void setSpecial(const KateTextLine::Ptr& textLine);

    const KateTextLine::Ptr& textLine() const;

    int line() const;
    /**
     * Only pass virtualLine if you know it (and thus we shouldn't try to look it up)
     */
    void setLine(int line, int virtualLine = -1);

    int virtualLine() const;
    void setVirtualLine(int virtualLine);

    int viewLine() const;
    void setViewLine(int viewLine);

    int startCol() const;
    void setStartCol(int startCol);

    int endCol() const;
    void setEndCol(int endCol);

    bool wrap() const;
    void setWrap(bool wrap);

    bool isDirty() const;
    bool setDirty(bool dirty = true);

    int startX() const;
    void setStartX(int startX);

    int endX() const;
    void setEndX(int endX);

    // This variable is used as follows:
    // non-dynamic-wrapping mode: unused
    // dynamic wrapping mode:
    //   first viewLine of a line: the X position of the first non-whitespace char
    //   subsequent viewLines: the X offset from the left of the display.
    //
    // this is used to provide a dynamic-wrapping-retains-indent feature.
    int shiftX() const;
    void setShiftX(int shiftX);

    int xOffset() const;

    bool startsInvisibleBlock() const;
    void setStartsInvisibleBlock(bool sib);

    void debugOutput() const;

private:
    QTextLayout* takeLayout() const;

    KateDocument* m_doc;
    mutable KateTextLine::Ptr m_textLine;
    int m_line;
    int m_virtualLine;
    int m_viewLine;
    int m_startCol;
    int m_endCol;
    int m_startX;
    int m_endX;
    int m_shiftX;

    bool m_dirty : 1;
    bool m_wrap : 1;
    bool m_startsInvisibleBlock : 1;
    bool m_special : 1;
};

#endif
