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

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <qdir.h>
#include <qfile.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qfileinfo.h>
#include <qprintdialog.h>
#include <qpaintdevicemetrics.h>

#include <kapp.h>
#include <kdebug.h>
#include <kspell.h>
#include <klocale.h>
#include <ksconfig.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>

#include <X11/Xlib.h> //used to have XSetTransientForHint()

#include <kwrite/highlight.h>
#include <kwrite/kwrite_doc.h>
#include <kwrite/kwrite_view.h>

#include "kwdialog.h"
#include "ktextprint.h"
#include "undohistory.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

#ifndef KDE_USE_FINAL
struct BufferInfo {
  void *user;
  int   w;
  int   h;
};
#endif

extern QPixmap *getBuffer(void *user);
extern void releaseBuffer(void *user);

KWriteView::KWriteView(KWrite *write, KWriteDoc *doc, bool HandleOwnDND)
  : QWidget( write )
{
  m_mainview = write;
  m_doc = doc;

  QWidget::setCursor(ibeamCursor);
  setBackgroundMode(NoBackground);

  setFocusPolicy(StrongFocus);
  move(2, 2);

  xScroll = new QScrollBar(QScrollBar::Horizontal, write);
  yScroll = new QScrollBar(QScrollBar::Vertical, write);
  connect(xScroll, SIGNAL(valueChanged(int)), SLOT(changeXPos(int)));
  connect(yScroll, SIGNAL(valueChanged(int)), SLOT(changeYPos(int)));

  xPos = 0;
  yPos = 0;
  scrollTimer = 0;
  cursor.set(0, 0);
  cursorOn = true;
  cursorTimer = 0;
  cXPos = 0;
  cOldXPos = 0;
  startLine = 0;
  endLine = -1;
  exposeCursor = false;
  updateState = 0;
  numLines = 0;
  lineRanges = 0L;
  newXPos = -1;
  newYPos = -1;
  drawBuffer = getBuffer(this);
  bm.sXPos = 0;
  bm.eXPos = -1;
  doc->registerView(this);
  setAcceptDrops(true);
  HandleURIDrops = HandleOwnDND;
  dragInfo.state = diNone;
}

KWriteView::~KWriteView() {
  m_doc->removeView(this);
  delete [] lineRanges;
  releaseBuffer(this);
}

void KWriteView::doCursorCommand(VConfig &c, int cmdNum) {
  switch (cmdNum) {
    case cmLeft:
      cursorLeft(c);
      break;
    case cmRight:
      cursorRight(c);
      break;
    case cmWordLeft:
      wordLeft(c);
      break;
    case cmWordRight:
      wordRight(c);
      break;
    case cmHome:
      home(c);
      break;
    case cmEnd:
      end(c);
      break;
    case cmUp:
      cursorUp(c);
      break;
    case cmDown:
      cursorDown(c);
      break;
    case cmScrollUp:
      scrollUp(c);
      break;
    case cmScrollDown:
      scrollDown(c);
      break;
    case cmTopOfView:
      topOfView(c);
      break;
    case cmBottomOfView:
      bottomOfView(c);
      break;
    case cmPageUp:
      pageUp(c);
      break;
    case cmPageDown:
      pageDown(c);
      break;
    case cmTop:
      top_home(c);
      break;
    case cmBottom:
      bottom_end(c);
      break;
  }
}

void KWriteView::doEditCommand(VConfig &c, int cmdNum) {
  // read-commands
  switch (cmdNum) {
    case cmCopy:
      m_doc->copy(c.flags);
      return;
    case cmSelectAll:
      m_doc->selectAll();
      return;
    case cmDeselectAll:
      m_doc->deselectAll();
      return;
    case cmInvertSelection:
      m_doc->invertSelection();
      return;
  }
  if (m_mainview->isReadOnly()) return;

  // write-commands
  switch (cmdNum) {
    case cmReturn:
      if (c.flags & cfDelOnInput) m_doc->delMarkedText(c);
      m_doc->newLine(c);
      //emit returnPressed();
      //event->ignore();
      return;
    case cmCut:
      m_doc->cut(c);
      return;
    case cmPaste:
      if (c.flags & cfDelOnInput) m_doc->delMarkedText(c);
      m_doc->paste(c);
      return;
    case cmUndo:
      m_doc->undo(c);
      return;
    case cmRedo:
      m_doc->redo(c);
      return;
    case cmIndent:
      m_doc->indent(c);
      return;
    case cmUnindent:
      m_doc->unIndent(c);
      return;
    case cmCleanIndent:
      m_doc->cleanIndent(c);
      return;
  }

  // commands that delete marked text
  if ((c.flags & cfDelOnInput) && m_doc->hasMarkedText()) {
    m_doc->delMarkedText(c); 
    return;
  }
  switch (cmdNum) {
    case cmBackspace:
      m_doc->backspace(c);
      return;
    case cmBackspaceWord:
      m_doc->backspaceWord(c);
      return;
    case cmDelete:
      m_doc->del(c);
      return;
    case cmDeleteWord:
      m_doc->delWord(c);
      return;
    case cmKillLine:
      m_doc->killLine(c);
      return;
  }
}

void KWriteView::cursorLeft(VConfig &c) {
  cursor.decX();

  if (c.flags & cfWrapCursor && cursor.x() < 0 && cursor.y() > 0) {
    cursor.decY();
    cursor.setX(m_doc->textLength(cursor.y()));
  }

  cOldXPos = cXPos = m_doc->textWidth(cursor);
  changeState(c);
}

void KWriteView::cursorRight(VConfig &c) {
  if (c.flags & cfWrapCursor) {
    if (cursor.x() >= m_doc->textLength(cursor.y())) {
      if (cursor.y() == m_doc->lastLine())
        return;

      cursor.set(-1, cursor.y() + 1);
    }
  }

  cursor.incX();
  cOldXPos = cXPos = m_doc->textWidth(cursor);
  changeState(c);
}

void KWriteView::wordLeft(VConfig &c) {
  TextLine *textLine;
  Highlight *highlight;

  highlight = m_doc->highlight();
  textLine = m_doc->textLine(cursor.y());

  if (cursor.x() > 0) {
    do {
      cursor.decX();
    } while (cursor.x() > 0 && !highlight->isInWord(textLine->getChar(cursor.x())));

    while (cursor.x() > 0 && highlight->isInWord(textLine->getChar(cursor.x() -1)))
      cursor.decX();
  } else {
    if (cursor.y() > 0) {
      cursor.decY();
      textLine = m_doc->textLine(cursor.y());
      cursor.setX(textLine->length());
    }
  }

  cOldXPos = cXPos = m_doc->textWidth(cursor);
  changeState(c);
}

void KWriteView::wordRight(VConfig &c) {
  Highlight *highlight = m_doc->highlight();
  TextLine *textLine = m_doc->textLine(cursor.y());
  int len = textLine->length();

  if (cursor.x() < len) {
    while (cursor.x() < len && highlight->isInWord(textLine->getChar(cursor.x())))
      cursor.incX();

    while (cursor.x() < len && !highlight->isInWord(textLine->getChar(cursor.x())))
      cursor.incX();
  } else {
    if (cursor.y() < m_doc->lastLine()) {
      cursor.incY();
      textLine = m_doc->textLine(cursor.y());
      cursor.setX(0);
    }
  }

  cOldXPos = cXPos = m_doc->textWidth(cursor);
  changeState(c);
}

void KWriteView::home(VConfig &c) {
  int lc = (c.flags & cfSmartHome) ? m_doc->textLine(cursor.y())->firstChar() : 0;

  if (lc <= 0 || cursor.x() == lc) {
    cursor.setX(0);
    cOldXPos = cXPos = 0;
  } else {
    cursor.setX(lc);
    cOldXPos = cXPos = m_doc->textWidth(cursor);
  }

  changeState(c);
}

void KWriteView::end(VConfig &c) {
  cursor.setX(m_doc->textLength(cursor.y()));
  cOldXPos = cXPos = m_doc->textWidth(cursor);
  changeState(c);
}

void KWriteView::cursorUp(VConfig &c) {
  cursor.decY();
  cXPos = m_doc->textWidth(c.flags & cfWrapCursor, cursor, cOldXPos);
  changeState(c);
}

void KWriteView::cursorDown(VConfig &c) {
  if (cursor.y() == m_doc->lastLine()) {
    int x = m_doc->textLength(cursor.y());

    if (cursor.x() >= x)
      return;

    cursor.setX(x);
    cXPos = m_doc->textWidth(cursor);
  } else {
    cursor.incY();
    cXPos = m_doc->textWidth(c.flags & cfWrapCursor, cursor, cOldXPos);
  }

  changeState(c);
}

void KWriteView::scrollUp(VConfig &c) {
  if (!yPos)
    return;

  newYPos = yPos - m_doc->fontHeight();

  if (cursor.y() == (yPos + height()) / m_doc->fontHeight() - 1) {
    cursor.decY();
    cXPos = m_doc->textWidth(c.flags & cfWrapCursor, cursor, cOldXPos);
    changeState(c);
  }
}

void KWriteView::scrollDown(VConfig &c) {
  if (endLine >= m_doc->lastLine())
    return;

  newYPos = yPos + m_doc->fontHeight();

  if (cursor.y() == (yPos + m_doc->fontHeight() - 1) / m_doc->fontHeight()) {
    cursor.incY();
    cXPos = m_doc->textWidth(c.flags & cfWrapCursor, cursor, cOldXPos);
    changeState(c);
  }
}

void KWriteView::topOfView(VConfig &c) {
  cursor.set(0, (yPos + m_doc->fontHeight() - 1) / m_doc->fontHeight());
  cOldXPos = cXPos = 0;
  changeState(c);
}

void KWriteView::bottomOfView(VConfig &c) {

  cursor.set(0, (yPos + height()) / m_doc->fontHeight() - 1);

  if (cursor.y() < 0)
    cursor.setY(0);

  cOldXPos = cXPos = 0;
  changeState(c);
}

void KWriteView::pageUp(VConfig &c) {
  int lines = endLine - startLine - 1;

  if (lines <= 0)
    lines = 1;

  if (!(c.flags & cfPageUDMovesCursor) && yPos > 0) {
    newYPos = yPos - lines*m_doc->fontHeight();

    if (newYPos < 0)
      newYPos = 0;
  }

  cursor.moveY(-lines);
  cXPos = m_doc->textWidth(c.flags & cfWrapCursor, cursor, cOldXPos);
  changeState(c);
}

void KWriteView::pageDown(VConfig &c) {
  int lines = endLine - startLine - 1;

  if (!(c.flags & cfPageUDMovesCursor) && endLine < m_doc->lastLine()) {
    if (lines < m_doc->lastLine() - endLine)
      newYPos = yPos + lines*m_doc->fontHeight(); 
    else
      newYPos = yPos + (m_doc->lastLine() - endLine)*m_doc->fontHeight();
  }

  cursor.moveY(lines);
  cXPos = m_doc->textWidth(c.flags & cfWrapCursor, cursor, cOldXPos);
  changeState(c);
}

// go to the top, same X position
void KWriteView::top(VConfig &c) {
  cursor.setY(0);
  cXPos = m_doc->textWidth(c.flags & cfWrapCursor, cursor, cOldXPos);
  changeState(c);
}

// go to the bottom, same X position
void KWriteView::bottom(VConfig &c) {
  cursor.setY(m_doc->lastLine());
  cXPos = m_doc->textWidth(c.flags & cfWrapCursor, cursor, cOldXPos);
  changeState(c);
}

// go to the top left corner
void KWriteView::top_home(VConfig &c) {
  cursor.set(0, 0);
  cOldXPos = cXPos = 0;
  changeState(c);
}

// go to the bottom right corner
void KWriteView::bottom_end(VConfig &c) {
  cursor.set(m_doc->textLength(cursor.y()), m_doc->lastLine());
  cOldXPos = cXPos = m_doc->textWidth(cursor);
  changeState(c);
}

void KWriteView::changeXPos(int p) {
  int dx = xPos - p;
  xPos = p;

  if (QABS(dx) < width())
    scroll(dx, 0); 
  else
    update();
}

void KWriteView::changeYPos(int p) {
  int dy = yPos - p;
  yPos = p;
  clearDirtyCache(height());

  if (QABS(dy) < height())
    scroll(0, dy); 
  else
    update();
}

void KWriteView::getVConfig(VConfig &c) {
  c.view = this;
  c.cursor = cursor;
  c.cXPos = cXPos;
  c.flags = m_mainview->m_configFlags;
  c.wrapAt = m_mainview->m_wrapAt;
}

void KWriteView::changeState(VConfig &c) {
  /*
   * we need to be sure to kill the selection on an attempted cursor
   * movement even if the cursor doesn't physically move,
   * but we need to be careful not to do some other things in this case,
   * like we don't want to expose the cursor
   */

  bool nullMove = (cursor == c.cursor);

  if (!nullMove) {
    m_doc->unmarkFound();

    exposeCursor = true;

    // mark old position of cursor as dirty
    if (cursorOn) {
      tagLines(c.cursor.y(), c.cursor.y(), c.cXPos - 2, c.cXPos + 3);
      cursorOn = false;
    }

    // mark old bracket mark position as dirty
    if (bm.sXPos < bm.eXPos) {
      tagLines(bm.cursor.y(), bm.cursor.y(), bm.sXPos, bm.eXPos);
    }

    // make new bracket mark
    m_doc->newBracketMark(cursor, bm);

    // remove trailing spaces when leaving a line
    if (c.flags & cfRemoveSpaces && cursor.y() != c.cursor.y()) {
      TextLine *textLine = m_doc->textLine(c.cursor.y());
      int newLen = textLine->lastChar();

      if (newLen != textLine->length()) {
        textLine->truncate(newLen);
        // if some spaces are removed, tag the line as dirty
        m_doc->tagLines(c.cursor.y(), c.cursor.y());
      }
    }
  }

  if (c.flags & cfMark) {
    if (!nullMove)
      m_doc->selectTo(c, cursor, cXPos);
  } else {
    if (!(c.flags & cfPersistent))
      m_doc->deselectAll();
  }
}

void KWriteView::insLine(int line) {
  if (line <= cursor.y())
    cursor.incY();

  if (line < startLine) {
    startLine++;
    endLine++;
    yPos += m_doc->fontHeight();
  } else if (line <= endLine)
    tagAll();

  //bookmarks
  KWBookmark *b;

  for (b = m_mainview->bookmarks.first(); b != 0L; b = m_mainview->bookmarks.next()) {
    if (b->cursor.y() >= line) {
      b->cursor.incY();
      b->yPos += m_doc->fontHeight();
    }
  }
}

void KWriteView::delLine(int line) {
  if (line <= cursor.y() && cursor.y() > 0)
    cursor.decY();

  if (line < startLine) {
    startLine--;
    endLine--;
    yPos -= m_doc->fontHeight();
  } else if (line <= endLine)
    tagAll();

  //bookmarks
  KWBookmark *b;
  for (b = m_mainview->bookmarks.first(); b != 0L; b = m_mainview->bookmarks.next()) {
    if (b->cursor.y() > line) {
      b->cursor.decY();
      b->yPos -= m_doc->fontHeight();
    }
  }
}

void KWriteView::updateCursor() {
  cOldXPos = cXPos = m_doc->textWidth(cursor);
}

void KWriteView::updateCursor(KWCursor &newCursor) {
  updateCursor(newCursor, m_mainview->config());
}

void KWriteView::updateCursor(KWCursor &newCursor, int flags) {
  if (!(flags & cfPersistent))
    m_doc->deselectAll();

  m_doc->unmarkFound();
  exposeCursor = true;

  if (cursorOn) {
    tagLines(cursor.y(), cursor.y(), cXPos - 2, cXPos + 3);
    cursorOn = false;
  }

  if (bm.sXPos < bm.eXPos)
    tagLines(bm.cursor.y(), bm.cursor.y(), bm.sXPos, bm.eXPos);

  m_doc->newBracketMark(newCursor, bm);
  cursor = newCursor;
  cOldXPos = cXPos = m_doc->textWidth(cursor);
}

// init the line dirty cache
void KWriteView::clearDirtyCache(int height) {
  int lines, z;

  // calc start and end line of visible part
  startLine = yPos/m_doc->fontHeight();
  endLine = (yPos + height -1)/m_doc->fontHeight();
  updateState = 0;
  lines = endLine - startLine +1;

  if (lines > numLines) {// resize the dirty cache
    numLines = lines*2;
    delete [] lineRanges;
    lineRanges = new LineRange[numLines];
  }

  for (z = 0; z < lines; z++) {// clear all lines
    lineRanges[z].start = 0xffffff;
    lineRanges[z].end = -2;
  }
  newXPos = newYPos = -1;
}

void KWriteView::tagLines(int start, int end, int x1, int x2) {
  LineRange *r;
  int z;

  start -= startLine;

  if (start < 0)
    start = 0;

  end -= startLine;

  if (end > endLine - startLine)
    end = endLine - startLine;

  if (x1 <= 0)
    x1 = -2;

  if (x1 < xPos-2)
    x1 = xPos-2;
  if (x2 > width() + xPos - 2)
    x2 = width() + xPos - 2;

  if (x1 >= x2)
    return;

  r = &lineRanges[start];

  for (z = start; z <= end; z++) {
    if (x1 < r->start)
      r->start = x1;

    if (x2 > r->end)
      r->end = x2;

    r++;
    updateState |= 1;
  }
}

void KWriteView::tagAll() {
  updateState = 3;
}

void KWriteView::setPos(int x, int y) {
  newXPos = x;
  newYPos = y;
}

void KWriteView::center() {
  newXPos = 0;
  newYPos = cursor.y()*m_doc->fontHeight() - height() / 2;

  if (newYPos < 0)
    newYPos = 0;
}

void KWriteView::updateView(int flags) {
  int fontHeight;
  int oldXPos, oldYPos;
  int w, h;
  int z;
  bool b;
  int xMax, yMax;
  int cYPos;
  int cXPosMin, cXPosMax, cYPosMin, cYPosMax;
  int dx, dy;
  int pageScroll;

//debug("upView %d %d %d %d %d", exposeCursor, updateState, flags, newXPos, newYPos);
  if (exposeCursor || flags & ufDocGeometry)
    emit m_mainview->newCurPos(); 
  else {
    if (updateState == 0 && newXPos < 0 && newYPos < 0)
      return;
  }

  if (cursorTimer) {
    killTimer(cursorTimer);
    cursorTimer = startTimer(KApplication::cursorFlashTime() / 2);
    cursorOn = true;
  }

  oldXPos = xPos;
  oldYPos = yPos;

  if (newXPos >= 0)
    xPos = newXPos;

  if (newYPos >= 0)
    yPos = newYPos;

  fontHeight = m_doc->fontHeight();
  cYPos = cursor.y()*fontHeight;
  z = 0;

  do {
    w = m_mainview->width() - 4;
    h = m_mainview->height() - 4;

    xMax = m_doc->textWidth() - w;
    b = (xPos > 0 || xMax > 0);

    if (b)
      h -= 16;

    yMax = m_doc->textHeight() - h;

    if (yPos > 0 || yMax > 0) {
      w -= 16;
      xMax += 16;

      if (!b && xMax > 0) {
        h -= 16;
        yMax += 16;
      }
    }

    if (!exposeCursor)
      break;

    cXPosMin = xPos + 4;
    cXPosMax = xPos + w - 8;
    cYPosMin = yPos;
    cYPosMax = yPos + (h - fontHeight);

    if (cXPos < cXPosMin)
      xPos -= cXPosMin - cXPos;

    if (xPos < 0)
      xPos = 0;

    if (cXPos > cXPosMax)
      xPos += cXPos - cXPosMax;

    if (cYPos < cYPosMin)
      yPos -= cYPosMin - cYPos;

    if (yPos < 0)
      yPos = 0;

    if (cYPos > cYPosMax)
      yPos += cYPos - cYPosMax;

    z++;
  }
  while (z < 2);

  if (xMax < xPos)
    xMax = xPos;

  if (yMax < yPos)
    yMax = yPos;

  if (xMax > 0) {
    pageScroll = w - (w % fontHeight) - fontHeight;

    if (pageScroll <= 0)
      pageScroll = fontHeight;

    xScroll->blockSignals(true);
    xScroll->setGeometry(2, h + 2, w, 16);
    xScroll->setRange(0, xMax);
    xScroll->setValue(xPos);
    xScroll->setSteps(fontHeight, pageScroll);
    xScroll->blockSignals(false);
    xScroll->show();
  } else
    xScroll->hide();

  if (yMax > 0) {
    pageScroll = h - (h % fontHeight) - fontHeight;

    if (pageScroll <= 0)
      pageScroll = fontHeight;

    yScroll->blockSignals(true);
    yScroll->setGeometry(w + 2, 2, 16, h);
    yScroll->setRange(0, yMax);
    yScroll->setValue(yPos);
    yScroll->setSteps(fontHeight, pageScroll);
    yScroll->blockSignals(false);
    yScroll->show();
  } else
    yScroll->hide();

  if (w != width() || h != height()) {
    clearDirtyCache(h);
    resize(w, h);
  } else {
    dx = oldXPos - xPos;
    dy = oldYPos - yPos;
    b = updateState == 3;

    if (flags & ufUpdateOnScroll)
      b |= dx || dy; 
    else
      b |= QABS(dx)*3 > w*2 || QABS(dy)*3 > h*2;

    if (b) {
      clearDirtyCache(h);
      update();
    } else {
      if (updateState > 0)
        paintTextLines(oldXPos, oldYPos);

      clearDirtyCache(h);

      if (dx || dy)
        scroll(dx, dy);

      if (cursorOn)
        paintCursor();

      if (bm.eXPos > bm.sXPos)
        paintBracketMark();
    }
  }

  exposeCursor = false;
}

void KWriteView::paintTextLines(int xPos, int yPos) {
//  int xStart, xEnd;
  int line;//, z;
  int h;
  LineRange *r;

  QPainter paint;
  paint.begin(drawBuffer);

  h = m_doc->fontHeight();
  r = lineRanges;
  for (line = startLine; line <= endLine; line++) {
    if (r->start < r->end) {
//debug("painttextline %d %d %d", line, r->start, r->end);
      m_doc->paintTextLine(&paint, line, r->start, r->end, m_mainview->m_configFlags & cfShowTabs);
      bitBlt(this, r->start - (xPos-2), line*h - yPos, drawBuffer, 0, 0,
        r->end - r->start, h);
    }
    r++;
  }
/*
  xStart = xPos-2;
  xEnd = xStart + width();
  h = m_doc->fontHeight();
  for (z = 0; z < updateState; z++) {
    line = updateLines[z];
    m_doc->paintTextLine(paint, line, xStart, xEnd);
    bitBlt(this, 0, line*h - yPos, drawBuffer, 0, 0, width(), h);
  }*/
  paint.end();
}

void KWriteView::paintCursor() {
  QPainter paint;
  int h = m_doc->fontHeight();
  int y = h*cursor.y() - yPos;
  int x = cXPos - (xPos-2);

  if (cursorOn) {
    paint.begin(this);
    paint.setClipping(false);
    paint.setPen(m_doc->cursorCol(cursor.x(), cursor.y()));
    h += y - 1;
    paint.drawLine(x, y, x, h);
    paint.drawLine(x - 2, y, x + 2, y);
    paint.drawLine(x - 2, h, x + 2, h);
  } else {
    paint.begin(drawBuffer);
    m_doc->paintTextLine(&paint, cursor.y(), cXPos - 2, cXPos + 3, m_mainview->m_configFlags & cfShowTabs);
    bitBlt(this, x - 2, y, drawBuffer, 0, 0, 5, h);
  }

  paint.end();
}

void KWriteView::paintBracketMark() {
  QPainter paint;
  int y = m_doc->fontHeight()*(bm.cursor.y() + 1) - yPos - 1;

  paint.begin(this);
  paint.setPen(m_doc->cursorCol(bm.cursor.x(), bm.cursor.y()));
  paint.drawLine(bm.sXPos - (xPos - 2), y, bm.eXPos - (xPos - 2) - 1, y);
  paint.end();
}

void KWriteView::placeCursor(int x, int y, int flags) {
  VConfig c;

  getVConfig(c);
  c.flags |= flags;
  cursor.setY((yPos + y) / m_doc->fontHeight());
  cXPos = cOldXPos = m_doc->textWidth(c.flags & cfWrapCursor, cursor, xPos - 2 + x);
  changeState(c);
}

bool KWriteView::isTargetSelected(int x, int y) {
  TextLine *line;

  y = (yPos + y) / m_doc->fontHeight();
  line = m_doc->textLine(y);

  if (!line)
    return false;

  x = m_doc->textPos(line, x);

  return line->isSelected(x);
}

void KWriteView::focusInEvent(QFocusEvent *) {
  if (!cursorTimer) {
    cursorTimer = startTimer(KApplication::cursorFlashTime() / 2);
    cursorOn = true;
    paintCursor();
  }
}

void KWriteView::focusOutEvent(QFocusEvent *) {
  if (cursorTimer) {
    killTimer(cursorTimer);
    cursorTimer = 0;
  }

  if (cursorOn) {
    cursorOn = false;
    paintCursor();
  }
}

void KWriteView::keyPressEvent(QKeyEvent *event) {
  VConfig c;

  getVConfig(c);
//  ascii = event->ascii();

  if (!m_mainview->isReadOnly()) {
    if (c.flags & cfTabIndents && m_doc->hasMarkedText()) {
      if (event->key() == Qt::Key_Tab) {
        m_doc->indent(c);
        m_doc->updateViews();
        return;
      }
      if (event->key() == Qt::Key_Backtab) {
        m_doc->unIndent(c);
        m_doc->updateViews();
        return;
      }
    }
    if (m_doc->insertChars(c, event->text())) {
      m_doc->updateViews();
      event->accept();
      return;
    }
  }
  event->ignore();
}
/*
  if (!m_mainview->isReadOnly()) {
    if (c.flags & cfTabIndents && m_doc->hasMarkedText()) {
      if (event->key() == Qt::Key_Tab) {
        m_doc->indent(c);
        m_doc->updateViews();
        return;
      }

      if (event->key() == Qt::Key_Backtab) {
        m_doc->unIndent(c);
        m_doc->updateViews();
        return;
      }
    }

    if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
      if (c.flags & cfDelOnInput)
        m_doc->delMarkedText(c);

      m_doc->newLine(c);
      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_Backspace) {
      if ((c.flags & cfDelOnInput) && m_doc->hasMarkedText())
        m_doc->delMarkedText(c); 
      else
        m_doc->backspace(c);

      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_Delete) {
      if ((c.flags & cfDelOnInput) && m_doc->hasMarkedText())
        m_doc->delMarkedText(c); 
      else
        m_doc->del(c);

      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_Left) {
      cursorLeft(c);
      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_Right) {
      cursorRight(c);
      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_Up) {
      cursorUp(c);
      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_Down) {
      cursorDown(c);
      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_Home) {
      home(c);
      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_End) {
      end(c);
      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_PageUp) {
      pageUp(c);
      m_doc->updateViews();
      event->accept();
      return;
    }
    if (event->key() == Qt::Key_PageDown) {
      pageDown(c);
      m_doc->updateViews();
      event->accept();
      return;
    }
    if (m_doc->insertChars(c, event->text())) {
      m_doc->updateViews();
      event->accept();
      return;
    }
  }

  event->ignore();
}
*/

void KWriteView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == LeftButton) {
    if (isTargetSelected(event->x(), event->y())) {
      // we have a mousedown on selected text
      // we initialize the drag info thingy as pending from this position

      dragInfo.state = diPending;
      dragInfo.start = event->pos();
    } else {
      // we have no reason to ever start a drag from here
      dragInfo.state = diNone;

      int flags = 0;

      if (event->state() & ShiftButton) {
        flags |= cfMark;

	if (event->state() & ControlButton)
	  flags |= cfMark | cfKeepSelection;
      }

      placeCursor(event->x(), event->y(), flags);
      scrollX = 0;
      scrollY = 0;

      if (!scrollTimer)
        scrollTimer = startTimer(50);

      m_doc->updateViews();
    }
  }

  if (event->button() == MidButton) {
    placeCursor(event->x(), event->y());

    if (!m_mainview->isReadOnly())
      m_mainview->paste();
  }

  if (m_mainview->popup && event->button() == RightButton)
    m_mainview->popup->popup(mapToGlobal(event->pos()));

  m_mainview->mousePressEvent(event); // this doesn't do anything, does it?
  // it does :-), we need this for KDevelop, so please don't uncomment it again -Sandy
}

void KWriteView::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() == LeftButton) {
    VConfig c;
    getVConfig(c);
    m_doc->selectWord(c.cursor, c.flags);
    m_doc->updateViews();
  }
}

void KWriteView::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == LeftButton) {
    if (dragInfo.state == diPending) {
      // we had a mouse down in selected area, but never started a drag
      // so now we kill the selection
      placeCursor(event->x(), event->y(), 0);
      m_doc->updateViews();
    } else if (dragInfo.state == diNone) {
      if (m_mainview->config() & cfMouseAutoCopy)
        m_mainview->copy();

      killTimer(scrollTimer);
      scrollTimer = 0;
    }

    dragInfo.state = diNone;
  }
}

void KWriteView::mouseMoveEvent(QMouseEvent *event) {
  if (event->state() & LeftButton) {
    int flags;
    int d;
    int x = event->x(),
        y = event->y();

    if (dragInfo.state == diPending) {
      // we had a mouse down, but haven't confirmed a drag yet
      // if the mouse has moved sufficiently, we will confirm

      if (x > dragInfo.start.x() + 4 || x < dragInfo.start.x() - 4 ||
          y > dragInfo.start.y() + 4 || y < dragInfo.start.y() - 4) {
        // we've left the drag square, we can start a real drag operation now
        doDrag();
      }
      return;
    } else if (dragInfo.state == diDragging) {
      // this isn't technically needed because mouseMoveEvent is suppressed during
      // Qt drag operations, replaced by dragMoveEvent
      return;
    }

    mouseX = event->x();
    mouseY = event->y();
    scrollX = 0;
    scrollY = 0;
    d = m_doc->fontHeight();
    if (mouseX < 0) {
      mouseX = 0;
      scrollX = -d;
    }
    if (mouseX > width()) {
      mouseX = width();
      scrollX = d;
    }
    if (mouseY < 0) {
      mouseY = 0;
      scrollY = -d;
    }
    if (mouseY > height()) {
      mouseY = height();
      scrollY = d;
    }
//debug("modifiers %d", ((KGuiCmdApp *) kapp)->getModifiers());
    flags = cfMark;
    if (event->state() & ControlButton) flags |= cfKeepSelection;
    placeCursor(mouseX, mouseY, flags);
    m_doc->updateViews(/*ufNoScroll*/);
  }
}

void KWriteView::wheelEvent(QWheelEvent *event) {
  if (yScroll->isVisible() == true) {
    QApplication::sendEvent(yScroll, event);
  }
}

void KWriteView::drawContents(QPainter *p, int /*cx*/, int cy, int /*cw*/, int ch) {
    // draw contents here
    int fontHeight;
    int line, endLine;
    int yPos;

    fontHeight = m_doc->fontHeight();
    line = cy / fontHeight;
    endLine = (cy + ch) / fontHeight + 1;
    yPos = line*fontHeight;

    if (endLine >= (int) m_doc->contents().count())
	endLine = m_doc->contents().count() - 1;

    while (line <= endLine) {
	m_doc->paintTextLine2(p, line, 0, width(), m_mainview->m_configFlags & cfShowTabs);

	line++;
	yPos += fontHeight;
    }

    if (cursorOn)
	paintCursor();

    if (bm.eXPos > bm.sXPos)
	paintBracketMark();
}

void KWriteView::paintEvent(QPaintEvent *event) {
  int xStart, xEnd;
  int h;
  int line, y, yEnd;

  QRect updateR = event->rect();

  //debug("update rect  = (%i, %i, %i, %i)", updateR.x(), updateR.y(), updateR.width(), updateR.height());

  QPainter paint;
  paint.begin(drawBuffer);

  xStart = xPos - 2 + updateR.x();
  xEnd = xStart + updateR.width();

  h = m_doc->fontHeight();
  line = (yPos + updateR.y()) / h;
  y = line*h - yPos;
  yEnd = updateR.y() + updateR.height();

  while (y < yEnd) {
    m_doc->paintTextLine(&paint, line, xStart, xEnd, m_mainview->m_configFlags & cfShowTabs);
    bitBlt(this, updateR.x(), y, drawBuffer, 0, 0, updateR.width(), h);
    line++;
    y += h;
  }

  paint.end();

  if (cursorOn)
    paintCursor();

  if (bm.eXPos > bm.sXPos)
    paintBracketMark();
}

void KWriteView::resizeEvent(QResizeEvent *) {
  resizeBuffer(this, width(), m_doc->fontHeight());
//  update();
}

void KWriteView::timerEvent(QTimerEvent *event) {
  if (event->timerId() == cursorTimer) {
    cursorOn = !cursorOn;
    paintCursor();
  }

  if (event->timerId() == scrollTimer && (scrollX | scrollY)) {
    xScroll->setValue(xPos + scrollX);
    yScroll->setValue(yPos + scrollY);
    placeCursor(mouseX, mouseY, cfMark);
    m_doc->updateViews(/*ufNoScroll*/);
  }
}

/////////////////////////////////////
// Drag and drop handlers
//

// call this to start a drag from this view
void KWriteView::doDrag() {
  dragInfo.state = diDragging;
  dragInfo.dragObject = new QTextDrag(m_doc->markedText(0), this);

  if (m_mainview->isReadOnly())
    dragInfo.dragObject->dragCopy();
  else {
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   drag() is broken for move operations in Qt - dragCopy() is the only safe way
   to go right now

    if (dragInfo.dragObject->drag()) {
      // the drag has completed and it turned out to be a move operation
      if (! m_doc->ownedView((KWriteView *)(QDragObject::target()))) {
        // the target is not me - we need to delete our selection
        VConfig c;
        getVConfig(c);
        m_doc->delMarkedText(c);
        m_doc->updateViews();
      }
    }
*/
    dragInfo.dragObject->dragCopy();
  }
}

void KWriteView::dragEnterEvent(QDragEnterEvent *event) {
  event->accept((QTextDrag::canDecode(event) && ! m_mainview->isReadOnly()) || QUriDrag::canDecode(event));
}

/*
void KWriteView::dragMoveEvent(QDragMoveEvent *) {
}
void KWriteView::dragLeaveEvent(QDragLeaveEvent *) {
  // we should implement a shadow cursor here
}
*/

void KWriteView::dropEvent(QDropEvent *event) {
  if (QUriDrag::canDecode(event)) {
    QStrList urls;

    if (!HandleURIDrops) {
      // the container should handle this one for us...
      emit dropEventPass(event);
    } else {
      // we can only load one url
      // this is why a smarter container should do this if possible
      if (QUriDrag::decode(event, urls)) {
        char *s;
        s = urls.first();
        if (s) {
          // Load the first file in this window
          if (s == urls.getFirst()) {
            if (m_mainview->canDiscard())
	      m_mainview->loadURL(KURL(s));
          }
        }
      }
    }
  } else if (QTextDrag::canDecode(event) && ! m_mainview->isReadOnly()) {
    QString text;

    if (QTextDrag::decode(event, text)) {
      bool priv, selected;

      // is the source our own document?
      priv = m_doc->ownedView((KWriteView *) (event->source()));
      // dropped on a text selection area?
      selected = isTargetSelected(event->pos().x(), event->pos().y());

      if (priv && selected) {
        // this is a drag that we started and dropped on our selection
        // ignore this case
        return;
      }

      VConfig c;
      KWCursor cursor;

      getVConfig(c);
      cursor = c.cursor;

      if (priv) {
        // this is one of mine(this document), not dropped on the selection
        if (event->action() == QDropEvent::Move) {
          m_doc->delMarkedText(c);
          getVConfig(c);
          cursor = c.cursor;
        }

	placeCursor(event->pos().x(), event->pos().y());
        getVConfig(c);
        cursor = c.cursor;
      } else {
        // this did not come from this document
        if (!selected) {
          placeCursor(event->pos().x(), event->pos().y());
          getVConfig(c);
          cursor = c.cursor;
        }
      }

      m_doc->insert(c, text);
      cursor = c.cursor;
      updateCursor(cursor);
      m_doc->updateViews();
    }
  }
}

KWBookmark::KWBookmark() {
  cursor.setY(-1); //mark bookmark as invalid
}
