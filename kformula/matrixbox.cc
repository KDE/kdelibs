#include "matrixbox.h"
#include "kformula.h"
#include <stdio.h>


//-----------------------CONSTRUCTORS AND DESTRUCTORS-----------------

matrixbox::matrixbox()
{
  type = MATRIX;
  width = height = 0;
  b1 = b2 = NULL;
  parent = NULL;
  dirty = 1;

  relx = rely = 0;

  fontsize = DEFAULT_FONT_SIZE;
}

matrixbox::matrixbox(int w, int h)
{
  type = MATRIX;
  width = w;
  height = h;
  b1 = b2 = NULL;
  parent = NULL;
  dirty = 1;

  relx = rely = 0;

  fontsize = DEFAULT_FONT_SIZE;
}


matrixbox::~matrixbox()
{
  if(b1 != NULL) b1->parent = NULL;
  if(b2 != NULL) b2->parent = NULL;
  b1 = b2 = NULL;
  while(elems.size()) {
    if(elems[elems.size() - 1] != NULL) {
      elems[elems.size() - 1]->parent = NULL;
    }
    elems.resize(elems.size() - 1);
  }

  if(parent) {
    if(parent->b1 == this) parent->b1 = NULL;
    if(parent->b2 == this) parent->b2 = NULL;
    parent->makeDirty();
  }
}

//-----------------------------ADD ELEM---------------------------
//build up the matrix by successively adding elements.  Columns come first.
void matrixbox::addElem(box *e)
{
  if(e != NULL) {
    ASSERT((int)elems.size() < width * height);

    e->parent = this;
    elems.resize(elems.size() + 1);
    elems[elems.size() - 1] = e;

    dirty = 1;
  }
}

//----------------------------CALCULATE---------------------------
void matrixbox::calculate(QPainter &p, int setFontsize)
{
  if(!dirty) return;

  ASSERT((int)elems.size() == width * height);

  //reset everything:
  elem_pos.resize(width * height);

  relx = rely = 0;
  b1x = b1y = b2x = b2y = 0;
  if(setFontsize == -1) setFontsize = p.font().pointSize();
  fontsize = setFontsize;
  if(fontsize < MIN_FONT_SIZE) fontsize = MIN_FONT_SIZE;
  
  int i;
  QFont f; //temporary to preserve the current font of the painter.
  
  f = p.font();
  
  lastFont = f;
  lastFont.setPointSize(fontsize);
  p.setFont(lastFont);

  QFontMetrics fm = p.fontMetrics();

  //calculate all the elements
  for(i = 0; i < width * height; i++) {
    ASSERT(elems[i] != NULL);
    elem_pos[i] = QPoint(0, 0);
    elems[i]->calculate(p, fontsize);
  }

  //center them horizontally:
  for(i = 0; i < width * height; i++) {
    elem_pos[i] += QPoint(-(elems[i]->getRect().left() +
			    elems[i]->getRect().right()) / 2, 0);
  }

  //now we need to arrange them: first by columns, then by rows
  int cursize = 0;

  for(i = 0; i < width * height; i += height) {
    int j, maxsize;

    maxsize = 0;
    
    //find the widest one:
    for(j = 0; j < height; j++) {
      maxsize = QMAX(maxsize, elems[i + j]->getRect().width());
    }

    //and move them all accordingly:
    for(j = 0; j < height; j++) {
      elem_pos[i + j] += QPoint(cursize + maxsize / 2, 0);
    }

    //and after we are done with the column, update cursize:
    cursize += (maxsize + SPACE * 3);
  }

  rect.setX(-SPACE);
  rect.setWidth(cursize - SPACE);

  cursize = 0;

  for(i = 0; i < height; i++) {
    int j, maxsize;

    maxsize = 0;

    //find the one which is highest above the midline:
    for(j = 0; j < width * height; j += height) {
      maxsize = QMAX(maxsize, -elems[i + j]->getRect().top());
    }

    //and move them all accordingly:
    for(j = 0; j < width * height; j += height) {
      elem_pos[i + j] += QPoint(0, cursize + maxsize);
    }

    cursize += (maxsize + SPACE * 3); //update cursize the first time
    maxsize = 0;

    //find the one which is lowest below the midline:
    for(j = 0; j < width * height; j += height) {
      maxsize = QMAX(maxsize, elems[i + j]->getRect().bottom());
    }

    //and after we are done with the row, update cursize:
    cursize += maxsize;
  }
  
  rect.setY(-(cursize - SPACE * 3) / 2);
  rect.setHeight(cursize - SPACE * 3);

  elem_pos.translate(0, -(cursize - SPACE * 3) / 2);
  
  dirty = 0;

}


//------------------------------DRAW--------------------------
//now drawing them is easy.
void matrixbox::draw(QPainter &p, int x, int y)
{
  if(dirty) calculate(p); // just in case

  offsx = x;
  offsy = y;

  int i;

  for(i = 0; i < (int)elems.size(); i++) {
    elems[i]->draw(p, x + elem_pos[i].x(), y + elem_pos[i].y());
  }

  return;
}


//----------------------------GET CURSOR POS---------------------
QRect matrixbox::getCursorPos(charinfo i, int x, int y)
{
  int j;
  QRect tmp;

  QFontMetrics fm(lastFont);

  //set the height of the rectangle proportional to the font height:
  //and offset to the baseline
  tmp.setRect(x, -fm.height() * 2 / 3 + y -
	      fm.boundingRect("+").center().y(), 1,
	      fm.height() * 3 / 4);

  if(i.where == this) {
    if(i.posinbox == 0) tmp.setX(rect.x() + x - 1); 
    else tmp.setX(x + rect.right() + 1);

    tmp.setWidth(1); //make sure cursor is thin and not empty

    return tmp;
  }

  for(j = 0; j < (int)elems.size(); j++) {

    tmp = elems[j]->getCursorPos(i, x + elem_pos[j].x(),
				 y + elem_pos[j].y());

    if(!tmp.isEmpty()) {

      return tmp;
    }
  }

  return QRect();
}
