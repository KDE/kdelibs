/* This file is part of the KDE libraries
    Copyright (C) 1999 Ilya Baran (ibaran@acs.bu.edu)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef BOX_H_INCLUDED
#define BOX_H_INCLUDED

#include <qpainter.h>
#include <qstring.h>
#include <qfont.h>
#include <kdebug.h> //will be included in practically all source files

#define DEFAULT_FONT_SIZE 25
#define MIN_FONT_SIZE  10

//pixels for spacing:
#define SPACE          (4)

//symbols have values above this
#define SYMBOL_ABOVE   20000

//can be set to 0 if you want to print the string out somewhere.
//UNUSED_OFFSET is only used in this file.
#define UNUSED_OFFSET  1000

//temporary workaround--otherwise it warns since
//there are two possible QChar constructors:
#define QChar(__x) QChar((int)(__x))

struct charinfo;

//each BoxType must be equal to the character that represents it in
//the string.  The parser depends on that.
enum BoxType {
  PLUS = '+',
  MINUS = '-',
  TIMES = '*',
  DIVIDE = '\\' + UNUSED_OFFSET,
  POWER = '^' + UNUSED_OFFSET, //just a test to see if it works
  SQRT = '@' + UNUSED_OFFSET,
  TEXT = 't',
  CAT = '#' + UNUSED_OFFSET,
  SUB = '_' + UNUSED_OFFSET,
  LSUP = '6' + UNUSED_OFFSET,
  LSUB = '%' + UNUSED_OFFSET,
  PAREN = '(',
  EQUAL = '=',
  MORE = '>',
  LESS = '<',
  ABS = '|',
  BRACKET = '[',
  SLASH = '/',
  MATRIX = 'm' + UNUSED_OFFSET,
  SEPARATOR = '&' + UNUSED_OFFSET, // separator for matrices
  ABOVE = ')' + UNUSED_OFFSET, //something useless
  BELOW = ']' + UNUSED_OFFSET,
  SYMBOL = 's' + UNUSED_OFFSET, // whatever
  // char for keeping track of cursor position in undo/redo:
  CURSOR = 'c' + UNUSED_OFFSET
};

enum SymbolType {
  INTEGRAL = SYMBOL_ABOVE, // symbols have values above that
  SUM,
  PRODUCT,
  ARROW
  // elements of the symbol font are their own codes + SYMBOL_ABOVE
};

//a box is a unit of a formula.
//it has up to two children, and routines
//for arranging and displaying them.

class box {
friend class KFormula;
friend class matrixbox;

protected:
  int type; //the box type
  QString text; //if a TEXT box, the text
  QFont lastFont; //the last font used
  QRect rect; //its bounding rectangle
  int relx, rely; //used internally for storing calculated locations
  int offsx, offsy; //its offset from its parent
  int fontsize; //its base fontsize
  box *b1, *b2; //the children
  box *parent;  //duh
  int b1x, b1y, b2x, b2y; //the offsets of the children
  int dirty; //number of days since last shower.  No--whether it's been
             //changed since calculate was last called on it.

  void makeDirty();
  void offset(int xoffs, int yoffs);

  static QRect symbolRect(QPainter &p, SymbolType s, int size);
  static void drawSymbol(QPainter &p, SymbolType s, int size, int x, int y);

public:
  box();
  box(int setNum);
  box(QString setText);
  box(BoxType setType, box * setB1 = NULL, box * setB2 = NULL);

  virtual ~box();

  void setText(QString newText);
  virtual void calculate(QPainter &p, int setFontsize = -1);
  QRect getRect() { return rect; }
  int getType() { return type; }
  box *getParent() { return parent; }
  QString getText() { return text; }
  virtual void draw(QPainter &p, int x, int y);

  virtual QRect getCursorPos(charinfo i, int x, int y);
  QRect getLastRect();
};


#endif // BOX_H_INCLUDED










