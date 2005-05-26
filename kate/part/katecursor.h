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

#ifndef kate_cursor_h
#define kate_cursor_h

#include "../interfaces/document.h"

class KateDocument;
class KateAttribute;

/**
  Simple cursor class with no document pointer.
*/
class KateTextCursor
{
  public:
    KateTextCursor() : m_line(0), m_col(0) {};
    KateTextCursor(int line, int col) : m_line(line), m_col(col) {};
    virtual ~KateTextCursor () {};

    friend bool operator==(const KateTextCursor& c1, const KateTextCursor& c2)
      { return c1.m_line == c2.m_line && c1.m_col == c2.m_col; }

    friend bool operator!=(const KateTextCursor& c1, const KateTextCursor& c2)
      { return !(c1 == c2); }

    friend bool operator>(const KateTextCursor& c1, const KateTextCursor& c2)
      { return c1.m_line > c2.m_line || (c1.m_line == c2.m_line && c1.m_col > c2.m_col); }

    friend bool operator>=(const KateTextCursor& c1, const KateTextCursor& c2)
      { return c1.m_line > c2.m_line || (c1.m_line == c2.m_line && c1.m_col >= c2.m_col); }

    friend bool operator<(const KateTextCursor& c1, const KateTextCursor& c2)
      { return !(c1 >= c2); }

    friend bool operator<=(const KateTextCursor& c1, const KateTextCursor& c2)
      { return !(c1 > c2); }
      
#ifndef Q_WS_WIN //not needed
    friend void qSwap(KateTextCursor & c1, KateTextCursor & c2) {
      KateTextCursor tmp = c1;
      c1 = c2;
      c2 = tmp;
    }
#endif

    inline void pos(int *pline, int *pcol) const {
      if(pline) *pline = m_line;
      if(pcol) *pcol = m_col;
    }

    inline int line() const { return m_line; };
    inline int col() const { return m_col; };

    virtual void setLine(int line) { m_line = line; };
    virtual void setCol(int col) { m_col = col; };
    virtual void setPos(const KateTextCursor& pos) { m_line = pos.line(); m_col = pos.col(); };
    virtual void setPos(int line, int col) { m_line = line; m_col = col; };

  protected:
    int m_line;
    int m_col;
};

class KateTextCursorList : public Q3PtrList<KateTextCursor>
{
  protected:
    virtual int compareItems(Q3PtrCollection::Item item1, Q3PtrCollection::Item item2);
};

/**
  Cursor class with a pointer to its document.
*/
class KateDocCursor : public KateTextCursor
{
  public:
    KateDocCursor(KateDocument *doc);
    KateDocCursor(int line, int col, KateDocument *doc);
    virtual ~KateDocCursor() {};

    bool validPosition(uint line, uint col);
    bool validPosition();

    bool gotoNextLine();
    bool gotoPreviousLine();
    bool gotoEndOfNextLine();
    bool gotoEndOfPreviousLine();

    int nbCharsOnLineAfter();
    bool moveForward(uint nbChar);
    bool moveBackward(uint nbChar);

    // KTextEditor::Cursor interface
    void position(uint *line, uint *col) const;
    bool setPosition(uint line, uint col);
    bool insertText(const QString& text);
    bool removeText(uint numberOfCharacters);
    QChar currentChar() const;

    uchar currentAttrib() const;

    /**
      Find the position (line and col) of the next char
      that is not a space. If found KateDocCursor points to the
      found character. Otherwise to a invalid Position such that
      validPosition() returns false.
      @return True if the specified or a following character is not a space
               Otherwise false.
    */
    bool nextNonSpaceChar();

    /**
      Find the position (line and col) of the previous char
      that is not a space. If found KateDocCursor points to the
      found character. Otherwise to a invalid Position such that
      validPosition() returns false.
      @return True if the specified or a preceding character is not a space
               Otherwise false.
    */
    bool previousNonSpaceChar();

  protected:
    KateDocument *m_doc;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
