/*
  $Id$

   Copyright (C) 1998, 1999 Jochen Wilhelmy
                            digisnap@cs.tu-berlin.de

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

#ifndef __KWRITE_VIEV_H__
#define __KWRITE_VIEV_H__

#include <qstring.h>
#include <qdialog.h>
#include <qkeycode.h>
#include <qintdict.h>
#include <qiodevice.h>
#include <qdropsite.h>
#include <qscrollbar.h>
#include <qpopupmenu.h>
#include <qdragobject.h>

#include <kurl.h>
#include <kconfig.h>

#include <kparts/part.h>

#include <kwrite/kwrite.h>

class KWriteDoc;
class KTextPrint;
class KSpell;
class KSpellConfig;

void resizeBuffer(void *user, int w, int h);

class KWriteView : public QWidget {
    Q_OBJECT

    friend KWriteDoc;
    friend KWrite;

  public:

    // a drop-aware container should set HandleOwnURIDrops = false and handle all URI drops
    // KWriteView will otherwise handle URI drops, but is slightly limited
    // KWriteView always handles text drops
    KWriteView(KWrite *, KWriteDoc *, bool HandleOwnURIDrops);
    ~KWriteView();

    virtual void doCursorCommand(VConfig &, int cmdNum);
    virtual void doEditCommand(VConfig &, int cmdNum);

    void cursorLeft(VConfig &);
    void cursorRight(VConfig &);
    void wordLeft(VConfig &);
    void wordRight(VConfig &);
    void home(VConfig &);
    void end(VConfig &);
    void cursorUp(VConfig &);
    void cursorDown(VConfig &);
    void scrollUp(VConfig &);
    void scrollDown(VConfig &);
    void topOfView(VConfig &);
    void bottomOfView(VConfig &);
    void pageUp(VConfig &);
    void pageDown(VConfig &);
    void cursorPageUp(VConfig &);
    void cursorPageDown(VConfig &);
    void top(VConfig &);
    void bottom(VConfig &);
    void top_home(VConfig &c);
    void bottom_end(VConfig &c);

  protected slots:

    void changeXPos(int);
    void changeYPos(int);

  protected:

    virtual void drawContents(QPainter *, int cx, int cy, int cw, int ch);

    void getVConfig(VConfig &);
    void changeState(VConfig &);
    void insLine(int line);
    void delLine(int line);
    void updateCursor();
    void updateCursor(KWCursor &newCursor);
    void updateCursor(KWCursor &newCursor, int flags);

    void clearDirtyCache(int height);
    void tagLines(int start, int end, int x1, int x2);
    void tagAll();
    void setPos(int x, int y);
    void center();

    void updateView(int flags);

    void paintTextLines(int xPos, int yPos);
    void paintCursor();
    void paintBracketMark();

    void placeCursor(int x, int y, int flags = 0);
    bool isTargetSelected(int x, int y);

    void doDrag();

    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void timerEvent(QTimerEvent *);

    virtual void dragEnterEvent(QDragEnterEvent *);
//  virtual void dragMoveEvent(QDragMoveEvent *);
//  virtual void dragLeaveEvent(QDragLeaveEvent *);
    virtual void dropEvent(QDropEvent *);

    KWrite *m_mainview;

  protected:

    KWriteDoc *m_doc;

    QScrollBar *xScroll;
    QScrollBar *yScroll;

    int xPos;
    int yPos;

    int mouseX;
    int mouseY;
    int scrollX;
    int scrollY;
    int scrollTimer;

    KWCursor cursor;
    bool cursorOn;
    int cursorTimer;
    int cXPos;
    int cOldXPos;

    int startLine;
    int endLine;

    bool exposeCursor;
    int updateState;
  //  int updateLines[2];
    int numLines;
    LineRange *lineRanges;
    int newXPos;
    int newYPos;

    QPixmap *drawBuffer;

    BracketMark bm;

    bool HandleURIDrops;

    enum DragState {diNone, diPending, diDragging};

    struct _dragInfo {
      DragState  state;
      QPoint     start;
      QTextDrag *dragObject;
    } dragInfo;

  signals:

    // emitted when KWriteView is not handling its own URI drops
    void dropEventPass(QDropEvent *);
};

#endif // __KWRITE_VIEW_H__
