#ifndef KFORMULA_H_INCLUDED
#define KFORMULA_H_INCLUDED

#include <qarray.h>
#include <qpainter.h>
#include <qcstring.h>
#include <qrect.h>
#include "box.h"

struct charinfo { // used to determine where each character gets
                  // sent during parsing
  box * where;    // points to box where it ends up
  int posinbox;   // where in that box is it
  int posinstr;   // where is it in the parsed string af parenthesize
};

int operator==(struct charinfo a, struct charinfo b);

//this class is for parsing and storing the actual boxes that
//make up a formula

#define L_GROUP QChar('{')
#define R_GROUP QChar('}')
#define L_BRACE_UNSEEN QChar(129)
#define R_BRACE_UNSEEN QChar(130)

class KFormula {
private:

  static QString *SPECIAL; //all the special characters
  static QString *INTEXT;  //plus minus slash times relationals
  static QString *LOC;     //powers, subscripts, above, below
  static QString *DELIM;   //parentheses, absolute value, etc

  static void initStrings(void) {
    if(SPECIAL) return;
    SPECIAL = new QString();
    DELIM = new QString();
    LOC = new QString();
    INTEXT = new QString();
    
    SPECIAL->append('{');
    SPECIAL->append('}');
    SPECIAL->append(PLUS);
    SPECIAL->append(MINUS);
    SPECIAL->append(TIMES);
    SPECIAL->append(DIVIDE);
    SPECIAL->append(POWER);
    SPECIAL->append(SQRT);
    SPECIAL->append(ABS);
    SPECIAL->append(SUB);
    SPECIAL->append(PAREN);
    SPECIAL->append(EQUAL);
    SPECIAL->append(MORE);
    SPECIAL->append(LESS);
    SPECIAL->append(CAT);
    SPECIAL->append(SLASH);
    SPECIAL->append(ABOVE);
    SPECIAL->append(BELOW);
    INTEXT->append(PLUS);
    INTEXT->append(MINUS);
    INTEXT->append(TIMES);
    INTEXT->append(EQUAL);
    INTEXT->append(MORE);
    INTEXT->append(LESS);
    INTEXT->append(SLASH);
    LOC->append(POWER);
    LOC->append(SUB);
    LOC->append(ABOVE);
    LOC->append(BELOW);
    DELIM->append(ABS);
    DELIM->append(PAREN);
  }

public:
  KFormula();
  KFormula(int x, int y);
  virtual ~KFormula();

  void redraw(QPainter &p);
  void setBoxes(QArray<box *> newBoxes);
  QArray<box *> getBoxes(void);
  void setPos(int x, int y); //sets the position for the center of the formula
  void parse(QString text, QArray<charinfo> *info = NULL);
  QString unparse(box *b = NULL);
  QRect getCursorPos(charinfo i);

protected:
  QArray<box *> boxes;
  int posx, posy;

private:
  void parenthesize(QString &temp, int i, QArray<charinfo> *info);
  box * makeBoxes(QString str, int offset, int maxlen, QArray<charinfo> *info);

public:
  static QString special() { initStrings(); return *SPECIAL; }
  static QString intext() { initStrings();  return *INTEXT; }
  static QString loc() { initStrings(); return *LOC; }
  static QString delim() { initStrings(); return *DELIM; }
};



#endif // KFORMULA_H_INCLUDED


