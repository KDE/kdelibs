#ifndef MATRIXBOX_H_INCLUDED
#define MATRIXBOX_H_INCLUDED

#include "box.h"

class matrixbox : public box
{
protected:
  int width, height; // number of elements in each direction

  QArray<box *> elems;
  QPointArray elem_pos; // the relative positions of all the elements

public:
  matrixbox();
  matrixbox(int w, int h);

  ~matrixbox();

  void addElem(box *e);

  void calculate(QPainter &p, int setFontsize = -1);
  void draw(QPainter &p, int x, int y);

  QRect getCursorPos(charinfo i, int x, int y);
};

#endif // MATRIXBOX_H_INCLUDED
