// $Id$

#ifndef KFORMULA_H_INCLUDED
#define KFORMULA_H_INCLUDED

#include <qarray.h>
#include <qpainter.h>
#include <qcstring.h>
#include <qrect.h>
#include <qstrlist.h>
#include "box.h"

struct charinfo { // used to determine where each character gets
                  // sent during parsing
  box * where;    // points to box where it ends up
  int posinbox;   // where in that box is it
  int posinstr;   // where is it in the parsed string af parenthesize
  bool left;      // whether it's to the left or right of its char
};

int operator==(struct charinfo a, struct charinfo b);

//this class is for parsing and storing the actual boxes that
//make up a formula

#define L_GROUP QChar('{')
#define R_GROUP QChar('}')
#define L_BRACE_UNSEEN QChar(129)
#define R_BRACE_UNSEEN QChar(130)

enum ErrorType {
  NO_ERROR = 0,
  DIVISION_BY_ZERO,
  ROOT_OF_NEGATIVE,
  UNDEFINED_VARIABLE,
  EMPTY_BOX,
  PARSE_ERROR
};

class KFormula {
private:

  static QString *SPECIAL; //all the special characters
  static QString *INTEXT;  //plus minus slash times relationals
  static QString *LOC;     //powers, subscripts, above, below
  static QString *DELIM;   //parentheses, absolute value, etc
  static QString *BIGOP;   //sums, products, integrals
  static QString *EVAL;    //things we can evaluate

  static void initStrings(void);

  bool restricted; // if will be evaluated

public:
  KFormula(bool r = FALSE);
  KFormula(int x, int y, bool r = FALSE);
  virtual ~KFormula();

  void redraw(QPainter &p);
  void setBoxes(QArray<box *> newBoxes);
  QArray<box *> getBoxes(void);
  void setPos(int x, int y); //sets the position for the center of the formula
  void parse(QString text, QArray<charinfo> *info = NULL);
  QString unparse(box *b = NULL);
  QRect getCursorPos(charinfo i);
  double evaluate(QStrList &vars, const QArray<double> &vals,
		  int *error = NULL, box *b = NULL);
  QSize size();

protected:
  QArray<box *> boxes;
  int posx, posy;

private:
  void parenthesize(QString &temp, int &i, QArray<charinfo> *info);
  box * makeBoxes(QString str, int offset, int maxlen, QArray<charinfo> *info);

public:
  static QString special() { initStrings(); return *SPECIAL; }
  static QString intext() { initStrings();  return *INTEXT; }
  static QString loc() { initStrings(); return *LOC; }
  static QString delim() { initStrings(); return *DELIM; }
  static QString bigop() { initStrings(); return *BIGOP; }
  static QString eval() { initStrings(); return *EVAL; }
};



#endif // KFORMULA_H_INCLUDED


