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
  KFormulaEdit(QWidget * parent=0, const char *name=0, WFlags f=0, bool r = 0);
  KFormulaEdit(bool r) {
    KFormulaEdit(0, 0, 0, r);
  }
  virtual ~KFormulaEdit();
  void setText(QString text);
  QString text() { return formText; }
  void redraw(int all = 1);
  KFormula *getFormula() { return form; }

  QSize sizeHint();
  QSizePolicy sizePolicy();

protected:
  QArray<_cursorInfo> cursorCache;
  bool restricted;  //whether for typesetting or evaluation
  int cacheState;
  QPixmap pm;       //double buffering
  KFormula *form;
  QString formText; //the text that gets converted into the formula
  static QString clipText; //clipboard
  int cursorPos;
  QArray<charinfo> info; //where each character of formText
                         //ends up in the formula
  QStack<QString> undo, redo;  //if there was a "diff" for strings it would
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
  int isInMatrix(int immed = 0); // am I inside (directly inside if immed)
                                 // of a matrix?
  int deleteAtCursor();
  int posAtPoint(QPoint p);
  int cursorDrawn;
  int isSelecting;
  int textSelected;
  int selectStart;
  int selectStartOrig;
  int isInString(int pos, const QString &str);

  QRect getCursorPos(int pos);

public slots:
  void insertChar(QChar c);

protected slots:
  void toggleCursor();
  void computeCache();   // computes one step

};


#endif // KFORMULAEDIT_H_INCLUDED




















