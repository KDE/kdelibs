#ifndef KFORMULA_H_INCLUDED
#define KFORMULA_H_INCLUDED

#include <qarray.h>
#include <qpainter.h>
#include <qcstring.h>
#include <qrect.h>

#define SPECIAL "+-*/@^#{}_(=<>|\\"

class box;

struct charinfo { // used to determine where each character gets
                  // sent during parsing
  box * where;    // points to box where it ends up
  int posinbox;   // where in that box is it
  int posinstr;   // where is it in the parsed string af parenthesize
};

int operator==(struct charinfo a, struct charinfo b);

//this class is for parsing and storing the actual boxes that
//make up a formula

class KFormula {

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
};


#endif // KFORMULA_H_INCLUDED


