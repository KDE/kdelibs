#ifndef BOX_H_INCLUDED
#define BOX_H_INCLUDED

#include <qpainter.h>
#include <qstring.h>
#include <qfont.h>

#define MIN_FONT_SIZE  10
//pixels for spacing:
#define SPACE          (4)

//symbols have values above this
#define SYMBOL_ABOVE   20000


struct charinfo;

//each BoxType must be equal to the character that represents it in
//the string.  The parser depends on that.
enum BoxType {
  PLUS = '+',
  MINUS = '-',
  TIMES = '*',
  DIVIDE = '\\',
  POWER = '^' + 200, //just a text to see if it works
  SQRT = '@' + 200,
  TEXT = 't',
  CAT = '#' + 200,
  SUB = '_' + 200,
  PAREN = '(',
  EQUAL = '=',
  MORE = '>',
  LESS = '<',
  ABS = '|',
  SLASH = '/',
  ABOVE = 650, //something useless
  BELOW = 651,
  SYMBOL = 652
};

enum SymbolType {
  INTEGRAL = SYMBOL_ABOVE, // symbols have values above that
  SUM,
  PRODUCT,
  ARROW
};

//a box is a unit of a formula.
//it has up to two children, and routines
//for arranging and displaying them.

class box {
friend class KFormula;

private:
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

  static QRect symbolRect(SymbolType s, int size);
  static void drawSymbol(QPainter &p, SymbolType s, int size, int x, int y);

public:
  box(int setNum);
  box(QString setText);
  box(BoxType setType, box * setB1 = NULL, box * setB2 = NULL);

  virtual ~box();

  void setText(QString newText);
  void calculate(QPainter &p, int setFontsize = -1);
  QRect getRect() { return rect; }
  int getType() { return type; }
  box *getParent() { return parent; }
  QString getText() { return text; }
  void draw(QPainter &p, int x, int y);

  QRect getCursorPos(charinfo i, int x, int y);
  QRect getLastRect();
};


#endif // BOX_H_INCLUDED










