#ifndef KFORMULAEDIT_H_INCLUDED
#define KFORMULAEDIT_H_INCLUDED

#include "kformula.h"
#include <qwidget.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qstack.h>

class box;
class KFormula;
struct charinfo;

struct _cursorInfo { //private
  int dirty;
  QRect pos;
};


class KFormulaEdit : public QWidget
{
  Q_OBJECT
public:
  KFormulaEdit(QWidget * parent=0, const char *name=0, WFlags f=0);
  virtual ~KFormulaEdit();
  void setText(QCString text);
  QCString text() { return formText.copy(); }
  void redraw(int all = 1);
  KFormula *getFormula() { return form; }

protected:
  QArray<_cursorInfo> cursorCache;
  int cacheState;
  QPixmap pm;       //double buffering
  KFormula *form;
  QCString formText; //the text that gets converted into the formula
  QCString clipText; //clipboard
  int cursorPos;
  QArray<charinfo> info; //where each character of formText
                         //ends up in the formula
  QStack<QCString> undo, redo;  //if there was a "diff" for strings it would
                               //save space
  QTimer t, fast; //t is for the cursor, fast for the cache

  void keyPressEvent(QKeyEvent *e);
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void paintEvent(QPaintEvent *e);
  void resizeEvent(QResizeEvent *e);
  void focusInEvent(QFocusEvent *e);
  void focusOutEvent(QFocusEvent *e);
  void expandSelection();

  int isValidCursorPos(int pos);
  int deleteAtCursor();
  int posAtPoint(QPoint p);
  int cursorDrawn;
  int isSelecting;
  int textSelected;
  int selectStart;
  int selectStartOrig;

  QRect getCursorPos(int pos);

protected slots:
  void toggleCursor();
  void computeCache();   // computes one step

};


#endif // KFORMULAEDIT_H_INCLUDED




















