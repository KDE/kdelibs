#ifndef BOX_H_INCLUDED
#define BOX_H_INCLUDED

#include <qpainter.h>
#include <qcstring.h>

#define MIN_FONT_SIZE  10
//pixels for spacing:
#define SPACE          (4)

struct charinfo;

//each BoxType must be equal to the character that represents it in
//the string.  The parser depends on that.
enum BoxType {
  PLUS = '+',
  MINUS = '-',
  TIMES = '*',
  DIVIDE = '/',
  POWER = '^',
  SQRT = '@',
  TEXT = 't',
  CAT = '#',
  SUB = '_',
  PAREN = '(',
  EQUAL = '=',
  MORE = '>',
  LESS = '<',
  ABS = '|'
};

//a box is a unit of a formula.
//it has up to two children, and routines
//for arranging and displaying them.

class box {
friend class KFormula;

private:
  int type; //the box type
  QCString text; //if a TEXT box, the text
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

public:
  box(int setNum);
  box(QCString setText);
  box(BoxType setType, box * setB1 = NULL, box * setB2 = NULL);

  virtual ~box();

  void setText(QCString newText);
  void calculate(QPainter &p, int setFontsize = -1);
  QRect getRect() { return rect; }
  char getType() { return type; }
  QCString getText() { return text; }
  void draw(QPainter &p, int x, int y);

  QRect getCursorPos(charinfo i, int x, int y);
  QRect getLastRect();
};


#endif // BOX_H_INCLUDED


