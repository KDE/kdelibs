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
#include <sys/time.h>

#include <qfont.h>
#include <qobject.h>
#include <qpainter.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qtextstream.h>

#include <kcharsets.h>

#include <kwrite/highlight.h>
#include <kwrite/kwrite_doc.h>
#include <kwrite/kwrite_view.h>

//text attribute constants
const int taSelected = 0x40;
const int taFound = 0x80;
const int taSelectMask = taSelected | taFound;
const int taAttrMask = ~taSelectMask & 0xFF;
const int taShift = 6;

TextLine::TextLine(int attribute, int context)
  : len(0), size(0), text(0L), attribs(0L), attr(attribute), ctx(context) {
}

TextLine::~TextLine() {
  delete [] text;
  delete [] attribs;
}

void TextLine::replace(int pos, int delLen, const QChar *insText, int insLen,
  uchar *insAttribs) {

  int newLen, i, z;
  uchar newAttr;
  QChar *newText;
  uchar *newAttribs;

  //find new length
  newLen = len - delLen;
  if (newLen < pos) newLen = pos;
  newLen += insLen;
//printf("len %d, pos %d, delLen %d, insLen %d, newLen %d\n",
//  len, pos, delLen, insLen, newLen);

  newAttr = (pos < len) ? attribs[pos] : attr;

  if (newLen > size) {
    //reallocate data
    size = size*3 >> 1;
    if (size < newLen) size = newLen;
    size = (size + 15) & (~15);

    newText = new QChar[size];
    newAttribs = new uchar[size];
  ASSERT(newText);
  ASSERT(newAttribs);

    i = QMIN(len, pos);
    for (z = 0; z < i; z++) {
      newText[z] = text[z];
      newAttribs[z] = attribs[z];
    }
  } else {
    newText = text;
    newAttribs = attribs;
  }

  //fill up with spaces and attribute
  for (z = len; z < pos; z++) {
    newText[z] = ' ';
    newAttribs[z] = attr;
  }

  i = (insLen - delLen);
  if (i != 0) {
    if (i <= 0) {
      //text to replace longer than new text
      for (z = pos + delLen; z < len; z++) {
        newText[z + i] = text[z];
        newAttribs[z + i] = attribs[z];
      }
    } else {
      //text to replace shorter than new text
      for (z = len -1; z >= pos + delLen; z--) {
        newText[z + i] = text[z];
        newAttribs[z + i] = attribs[z];
      }
    }
  }

  if (newText != text) {//delete old stuff on realloc
    delete [] text;
    delete [] attribs;
    text = newText;
    attribs = newAttribs;
  }

  if (insAttribs == 0L) {
    for (z = 0; z < insLen; z++) {
      text[pos + z] = insText[z];
      attribs[pos + z] = newAttr;
    }
  } else {
    for (z = 0; z < insLen; z++) {
      text[pos + z] = insText[z];
      attribs[pos + z] = insAttribs[z];
    }
  }
  len = newLen;
}

void TextLine::wrap(TextLine *nextLine, int pos) {
  int l;

  l = len - pos;
  if (l > 0) {
    nextLine->replace(0, 0, &text[pos], l, &attribs[pos]);
    attr = attribs[pos];
    len = pos;
  }
}

void TextLine::unWrap(int pos, TextLine *nextLine, int len) {

  replace(pos, 0, nextLine->text, len, nextLine->attribs);
  attr = nextLine->getRawAttr(len);
  nextLine->replace(0, len, 0L, 0);
}

int TextLine::firstChar() const {
  int z = 0;

  while (z < len && text[z].isSpace()) z++;
  return(z < len) ? z : -1;
}

int TextLine::lastChar() const {
  int z = len;

  while (z > 0 && text[z - 1].isSpace()) z--;
  return z;
}

void TextLine::removeSpaces() {

  while (len > 0 && text[len - 1].isSpace()) len--;
}

QChar TextLine::getChar(int pos) const {
  if (pos < len) return text[pos];
  return ' ';
}

const QChar *TextLine::getString() {
  QChar ch = QChar('\0');  // this hack makes the string null terminated
  replace(len, 0, &ch, 1); //  only syntax highlight needs it
  len--;
//  resize(len+1);
//  text[len] = '\0';
  return text;
}

int TextLine::cursorX(int pos, int tabChars) const {
  int l, x, z;

  l = (pos < len) ? pos : len;
  x = 0;
  for (z = 0; z < l; z++) {
    if (text[z] == '\t') x += tabChars - (x % tabChars); else x++;
  }
  x += pos - l;
  return x;
}

void TextLine::setAttribs(int attribute, int start, int end) {
  int z;

  if (end > len) end = len;
  for (z = start; z < end; z++) attribs[z] = (attribs[z] & taSelectMask) | attribute;
}

void TextLine::setAttr(int attribute) {
  attr = (attr & taSelectMask) | attribute;
}

int TextLine::getAttr(int pos) const {
  if (pos < len) return attribs[pos] & taAttrMask;
  return attr & taAttrMask;
}

int TextLine::getAttr() const {
  return attr & taAttrMask;
}

int TextLine::getRawAttr(int pos) const {
  if (pos < len) return attribs[pos];
  return(attr & taSelectMask) ? attr : attr | 256;
}

int TextLine::getRawAttr() const {
  return attr;
}

void TextLine::setContext(int context) {
  ctx = context;
}

int TextLine::getContext() const {
  return ctx;
}


void TextLine::select(bool sel, int start, int end) {
  int z;

  if (end > len) end = len;
  if (sel) {
    for (z = start; z < end; z++) attribs[z] |= taSelected;
  } else {
    for (z = start; z < end; z++) attribs[z] &= ~taSelected;
  }
}

void TextLine::selectEol(bool sel, int pos) {
  int z;

  if (sel) {
    for (z = pos; z < len; z++) attribs[z] |= taSelected;
    attr |= taSelected;
  } else {
    for (z = pos; z < len; z++) attribs[z] &= ~taSelected;
    attr &= ~taSelected;
  }
}


void TextLine::toggleSelect(int start, int end) {
  int z;

  if (end > len) end = len;
  for (z = start; z < end; z++) attribs[z] = attribs[z] ^ taSelected;
}


void TextLine::toggleSelectEol(int pos) {
  int z;

  for (z = pos; z < len; z++) attribs[z] = attribs[z] ^ taSelected;
  attr = attr ^ taSelected;
}


int TextLine::numSelected() const {
  int z, n;

  n = 0;
  for (z = 0; z < len; z++) if (attribs[z] & taSelected) n++;
  return n;
}

bool TextLine::isSelected(int pos) const {
  if (pos < len) return(attribs[pos] & taSelected);
  return(attr & taSelected);
}

bool TextLine::isSelected() const {
  return(attr & taSelected);
}

int TextLine::findSelected(int pos) const {
  while (pos < len && attribs[pos] & taSelected) pos++;
  return pos;
}

int TextLine::findUnselected(int pos) const {
  while (pos < len && !(attribs[pos] & taSelected)) pos++;
  return pos;
}

int TextLine::findRevSelected(int pos) const {
  while (pos > 0 && attribs[pos - 1] & taSelected) pos--;
  return pos;
}

int TextLine::findRevUnselected(int pos) const {
  while (pos > 0 && !(attribs[pos - 1] & taSelected)) pos--;
  return pos;
}

void TextLine::markFound(int pos, int l) {
  int z;

  l += pos;
  if (l > len) l = len;
  for (z = pos; z < l; z++) attribs[z] |= taFound;
}

void TextLine::unmarkFound() {
  int z;
  for (z = 0; z < len; z++) attribs[z] &= ~taFound;
}


// Attribute

//Attribute::Attribute() : font(), fm(font) {
//}

void Attribute::setFont(const QFont &f) {
  font = f;
  fm = QFontMetrics(f);
//workaround for slow QFontMetrics::width(), QFont::fixedPitch() doesn't seem to work
  if ((fontWidth = fm.width('W')) != fm.width('i')) fontWidth = -1;
}



// KWAction

KWAction::KWAction(Action a, KWCursor &cursor, int len, const QString &text)
  : action(a), cursor(cursor), len(len), text(text) {
}

KWActionGroup::KWActionGroup(KWCursor &aStart, int type)
  : start(aStart), action(0L), undoType(type) {
}

KWActionGroup::~KWActionGroup() {
  KWAction *current, *next;

  current = action;
  while (current) {
    next = current->next;
    delete current;
    current = next;
  }
}

void KWActionGroup::insertAction(KWAction *a) {
  a->next = action;
  action = a;
}

QString KWActionGroup::typeName(int type) {
  // return a short text description of the given undo group type suitable for a menu
  // not the lack of i18n's, the caller is expected to handle translation
  switch (type) {
    case ugPaste :
      return "Paste Text";
    case ugDelBlock :
      return "Selection Overwrite";
    case ugIndent :
      return "Indent";
    case ugUnindent :
      return "Unindent";
    case ugReplace :
      return "Text Replace";
    case ugSpell :
      return "Spell Check";
    case ugInsChar :
      return "Typing";
    case ugDelChar :
       return "Delete Text";
    case ugInsLine :
      return "New Line";
    case ugDelLine :
      return "Delete Line";
  }
  return "";
}

KWriteDoc::KWriteDoc(HlManager *hlManager, const QString &path)
  : QObject(0L), m_hlManager(hlManager), m_fName(path) {

  m_refCount = 0;

  m_contents.setAutoDelete(true);

  m_colors[0] = white;
  m_colors[1] = darkBlue;
  m_colors[2] = black;
  m_colors[3] = black;
  m_colors[4] = white;

  m_highlight = 0L;
  m_tabChars = 8;

  m_singleSelectMode = false;

  m_newDocGeometry = false;
  m_readWrite = true;

  m_modified = false;

  m_undoList.setAutoDelete(true);
  m_undoState = 0;
  m_undoSteps = 50;

//  recordReset();

  m_pseudoModal = 0L;
  clear();
  clearFileName();

  setHighlight(0); //calls updateFontData()
  // if the user changes the highlight with the dialog, notify the doc
  connect(hlManager, SIGNAL(changed()), SLOT(hlChanged()));

  m_newDocGeometry = false;
}

KWriteDoc::~KWriteDoc() {
  m_highlight->release();
}

void KWriteDoc::decRefCount() {
  m_refCount--;
  if (m_refCount == 0) delete this;
}


void KWriteDoc::registerView(KWriteView *view) {
  m_views.append(view);
  incRefCount();
}

void KWriteDoc::removeView(KWriteView *view) {
  m_views.remove(view);
  decRefCount();
}

bool KWriteDoc::ownedView(KWriteView *view) {
  // do we own the given view?
  return (m_views.containsRef(view) > 0);
}

bool KWriteDoc::isLastView(int numViews) {
  return ((int) m_views.count() == numViews);
}


TextLine *KWriteDoc::textLine(int line) {
//  if (line < 0) line = 0;
  if (line >= (int) m_contents.count())
    return 0L;

  return m_contents.at(line);
}

int KWriteDoc::textLength(int line) {
  return m_contents.at(line)->length();
}

void KWriteDoc::setTabWidth(int chars) {
  TextLine *textLine;
  int len;

  if (m_tabChars == chars) return;
  if (chars < 1) chars = 1;
  if (chars > 16) chars = 16;
  m_tabChars = chars;
  updateFontData();

  m_maxLength = -1;
  for (textLine = m_contents.first(); textLine != 0L; textLine = m_contents.next()) {
    len = textWidth(textLine, textLine->length());
    if (len > m_maxLength) {
      m_maxLength = len;
      m_longestLine = textLine;
    }
  }
//  tagAll();
}

void KWriteDoc::setReadWrite(bool readWrite) {
  KWriteView *view;

  if (readWrite != m_readWrite) {
    m_readWrite = readWrite;
    for (view = m_views.first(); view != 0L; view = m_views.next()) {
      view->m_kWrite->emitNewStatus();
    }
    newUndo(); // undo/redo become disabled when readWrite is switched off
  }
}

void KWriteDoc::setModified(bool modified) {
  KWriteView *view;

  if (modified != m_modified) {
    m_modified = modified;
    for (view = m_views.first(); view != 0L; view = m_views.next()) {
      view->m_kWrite->emitNewStatus();
    }
  }
}

void KWriteDoc::readConfig(KConfig *config) {
  int z;
  char s[16];

  setTabWidth(config->readNumEntry("TabWidth", 8));
  setUndoSteps(config->readNumEntry("UndoSteps", 50));
  m_singleSelectMode = config->readBoolEntry("SingleSelectMode", false);
  for (z = 0; z < 5; z++) {
    sprintf(s, "Color%d", z);
    m_colors[z] = config->readColorEntry(s, &m_colors[z]);
  }
}

void KWriteDoc::writeConfig(KConfig *config) {
  int z;
  char s[16];

  config->writeEntry("TabWidth", m_tabChars);
  config->writeEntry("UndoSteps", m_undoSteps);
  config->writeEntry("SingleSelectMode", m_singleSelectMode);
  for (z = 0; z < 5; z++) {
    sprintf(s, "Color%d", z);
    config->writeEntry(s, m_colors[z]);
  }
}

void KWriteDoc::readSessionConfig(KConfig *config) {
  readConfig(config);
  m_fName = config->readEntry("URL");
  setHighlight(m_hlManager->nameFind(config->readEntry("Highlight")));
}

void KWriteDoc::writeSessionConfig(KConfig *config) {
  writeConfig(config);
  config->writeEntry("URL", m_fName);
  config->writeEntry("Highlight", m_highlight->name());
}

Highlight *KWriteDoc::highlight() {
  return m_highlight;
}

int KWriteDoc::highlightNum() {
  return m_hlManager->findHl(m_highlight);
}

int KWriteDoc::numAttribs() {
  return m_numAttribs;
}

void KWriteDoc::findHighlight(const QString &filename) {
  int hl;

  hl = m_hlManager->wildcardFind(filename);

  if (hl == -1) {
    // fill the detection buffer with the contents of the text
    const uint HOWMANY = 1024;
    QString buf;
    TextLine *textLine;

    for (textLine = m_contents.first(); textLine != 0L; textLine = m_contents.next()) {
      buf.insert(buf.length(), textLine->getText(), textLine->length());
      if (buf.length() >= HOWMANY) break;
    }

    hl = m_hlManager->mimeFind(buf, filename);
  }

  setHighlight(hl);
}

void KWriteDoc::setHighlight(int n) {
debug("highlight: %i", n);
  if (n < 0)
    return;

  Highlight *h;

//  hlNumber = n;

  h = m_hlManager->getHl(n);
  if (h == m_highlight) {
    updateLines();
  } else {
    if (m_highlight != 0L) m_highlight->release();
    h->use();
    m_highlight = h;
    makeAttribs();
  }
}

void KWriteDoc::makeAttribs() {
  m_numAttribs = m_hlManager->makeAttribs(m_highlight, m_attribs, MAX_ATTRIBS);
  updateFontData();
  updateLines();
}

void KWriteDoc::hlChanged() {
  makeAttribs();
  updateViews();
}


void KWriteDoc::updateFontData() {
  int maxAscent, maxDescent;
  int minTabWidth, maxTabWidth;
  int i, z;
  KWriteView *view;

  maxAscent = 0;
  maxDescent = 0;
  minTabWidth = 0xffffff;
  maxTabWidth = 0;

  for (z = 0; z < m_numAttribs; z++) {
    i = m_attribs[z].fm.ascent();
    if (i > maxAscent) maxAscent = i;
    i = m_attribs[z].fm.descent();
    if (i > maxDescent) maxDescent = i;
    i = m_attribs[z].fm.width('x');
    if (i < minTabWidth) minTabWidth = i;
    if (i > maxTabWidth) maxTabWidth = i;
  }

  m_fontHeight = maxAscent + maxDescent + 1;
  m_fontAscent = maxAscent;
  m_tabWidth = m_tabChars*(maxTabWidth + minTabWidth) / 2;

  for (view = m_views.first(); view != 0L; view = m_views.next()) {
    resizeBuffer(view, view->width(), m_fontHeight);
    view->tagAll();
    view->updateCursor();
  }
}

void KWriteDoc::addLineAttribute(KWLineAttribute *a) {
  KWriteView *view;

  m_lineAttribs.insert(a);

  for (view = m_views.first(); view != 0L; view = m_views.next()) {
    view->updateBorder();
  }
}

int KWriteDoc::textWidth(TextLine *textLine, int cursorX) {
  int x;
  int z;
  char ch;
  Attribute *a;

  x = 0;
  for (z = 0; z < cursorX; z++) {
    ch = textLine->getChar(z);
    a = &m_attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->width(ch);//a->fm.width(ch);
  }
  return x;
}

int KWriteDoc::textWidth(KWCursor &cursor) {
  if (cursor.x() < 0) cursor.setX(0);
  if (cursor.y() < 0) cursor.setY(0);
  if (cursor.y() >= (int) m_contents.count()) cursor.setY((int) m_contents.count() - 1);
  return textWidth(m_contents.at(cursor.y()), cursor.x());
}

int KWriteDoc::textWidth(bool wrapCursor, KWCursor &cursor, int xPos) {
  TextLine *textLine;
  int len;
  int x, oldX;
  int z;
  char ch;
  Attribute *a;

  if (cursor.y() < 0) cursor.setY(0);
  if (cursor.y() >= (int) m_contents.count()) cursor.setY((int) m_contents.count() - 1);
  textLine = m_contents.at(cursor.y());
  len = textLine->length();

  x = oldX = z = 0;
  while (x < xPos && (!wrapCursor || z < len)) {
    oldX = x;
    ch = textLine->getChar(z);
    a = &m_attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->width(ch);//a->fm.width(ch);
    z++;
  }
  if (xPos - oldX < x - xPos && z > 0) {
    z--;
    x = oldX;
  }
  cursor.setX(z);
  return x;
}


int KWriteDoc::textPos(TextLine *textLine, int xPos) {
/*  int newXPos;

  return textPos(textLine, xPos, newXPos);
}

int KWriteDoc::textPos(TextLine *textLine, int xPos, int &newXPos) {
*/
//  int len;
  int x, oldX;
  int z;
  char ch;
  Attribute *a;

//  len = textLine->length();

  x = oldX = z = 0;
  while (x < xPos) {// && z < len) {
    oldX = x;
    ch = textLine->getChar(z);
    a = &m_attribs[textLine->getAttr(z)];
    x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->width(ch);//a->fm.width(ch);
    z++;
  }
  if (xPos - oldX < x - xPos && z > 0) {
    z--;
   // newXPos = oldX;
  }// else newXPos = x;
  return z;
}

int KWriteDoc::textWidth() {
  return m_maxLength + 8;
}

int KWriteDoc::textHeight() {
  return m_contents.count()*m_fontHeight;
}

void KWriteDoc::insert(VConfig &c, const QString &s) {
  int pos;
  QChar ch;
  QString buf;

  if (s.isEmpty()) return;

  recordStart(c, KWActionGroup::ugPaste);

  pos = 0;
  if (!(c.flags & cfVerticalSelect)) {
    do {
      ch = s[pos];
      if (ch.isPrint() || ch == '\t') {
        buf += ch; // append char to buffer
      } else if (ch == '\n') {
        recordAction(KWAction::newLine, c.cursor); // wrap contents behind cursor to new line
        recordInsert(c, buf); // append to old line
//        c.cursor.x() += buf.length();
        buf.truncate(0); // clear buffer
	c.cursor.set(0, c.cursor.y() + 1);
      }
      pos++;
    } while (pos < (int) s.length());
  } else {
    int xPos;

    xPos = textWidth(c.cursor);
    do {
      ch = s[pos];
      if (ch.isPrint() || ch == '\t') {
        buf += ch;
      } else if (ch == '\n') {
        recordInsert(c, buf);
        buf.truncate(0);
	c.cursor.move(buf.length(), 1);
        if (c.cursor.y() >= numLines())
          recordAction(KWAction::insLine, c.cursor);
        c.cursor.setX(textPos(m_contents.at(c.cursor.y()), xPos));
      }
      pos++;
    } while (pos < (int) s.length());
  }
  recordInsert(c, buf);
  c.cursor.moveX(buf.length());
  recordEnd(c);
}

void KWriteDoc::insertFile(VConfig &c, QIODevice &dev) {
  char block[256];
  int len;
  char *s;
  QChar ch;
  QString buf;
  char last = '\0';

  recordStart(c, KWActionGroup::ugPaste);

  do {
    len = dev.readBlock(block, 256);
    s = block;
    while (len > 0) {
      ch = *s;
      if (ch.isPrint() || *s == '\t') {
        buf += ch;
      } else if (*s == '\n' || *s == '\r') {
        if (last != '\r' || *s != '\n') {
          recordAction(KWAction::newLine, c.cursor);
          recordInsert(c, buf);
          buf.truncate(0);
          c.cursor.incY();
          c.cursor.setX(0);
        }
        last = *s;
      }
      s++;
      len--;
    }
  } while (s != block);
  recordInsert(c, buf);
  recordEnd(c);
}

void KWriteDoc::loadFile(QIODevice &dev) {
  QTextStream t(&dev);
  TextLine *textLine;
  QChar ch;
  QChar last = '\0';

  clear();

  textLine = m_contents.getFirst();
  while (!t.atEnd()) {
    t >> ch;
    if (ch.isPrint() || ch == '\t') {
      textLine->append(&ch, 1);
    } else if (ch == '\n' || ch == '\r') {
      if (last != '\r' || ch != '\n') {
        textLine = new TextLine();
        m_contents.append(textLine);
        if (ch == '\r') m_eolMode = eolMacintosh;
      } else m_eolMode = eolDos;
      last = ch;
    }
  }
//  updateLines();
}

void KWriteDoc::writeFile(QIODevice &dev) {
  QTextStream t(&dev);
  TextLine *textLine;

  textLine = m_contents.first();
  do {
    QString str(textLine->getText(), textLine->length());
//    QConstString str((QChar *) textLine->getText(), textLine->length());
//    dev.writeBlock(str.string().latin1(), textLine->length());
    t << str;
    textLine = m_contents.next();
    if (!textLine) break;
    if (m_eolMode != eolUnix) t << '\r'; //dev.putch('\r');
    if (m_eolMode != eolMacintosh) t << '\n'; //dev.putch('\n');
  } while (true);
}

int KWriteDoc::currentColumn(KWCursor &cursor) {
  return m_contents.at(cursor.y())->cursorX(cursor.x(), m_tabChars);
}

bool KWriteDoc::insertChars(VConfig &c, const QString &chars) {
  TextLine *textLine;
  int z, pos, l;
  bool onlySpaces;
  QChar ch;
  QString buf;

  textLine = m_contents.at(c.cursor.y());

  pos = 0;
  onlySpaces = true;
  for (z = 0; z < (int) chars.length(); z++) {
    ch = chars[z];
    if (ch == '\t' && c.flags & cfReplaceTabs) {
      l = m_tabChars - (textLine->cursorX(c.cursor.x(), m_tabChars) % m_tabChars);
      while (l > 0) {
        buf.insert(pos, ' ');
        pos++;
        l--;
      }
    } else if (ch.isPrint() || ch == '\t') {
      buf.insert(pos, ch);
      pos++;
      if (ch != ' ') onlySpaces = false;
      if (c.flags & cfAutoBrackets) {
        if (ch == '(') buf.insert(pos, ')');
        if (ch == '[') buf.insert(pos, ']');
        if (ch == '{') buf.insert(pos, '}');
      }
      /*
      if (ch == 'ä') {
        strcpy(buf, "&auml;");
        l = z = 6;
      }
      if (ch == 'ö') {
        strcpy(buf, "&ouml;");
        l = z = 6;
      }
      if (ch == 'ü') {
        strcpy(buf, "&uuml;");
        l = z = 6;
      }
      if (ch == 'Ä') {
        strcpy(buf, "&Auml;");
        l = z = 6;
      }
      if (ch == 'Ö') {
        strcpy(buf, "&Ouml;");
        l = z = 6;
      }
      if (ch == 'Ü') {
        strcpy(buf, "&Uuml;");
        l = z = 6;
      }*/
      /*
      if (ch == 'ä') {
        strcpy(buf, "\"a");
        l = z = 2;
      }
      if (ch == 'ö') {
        strcpy(buf, "\"o");
        l = z = 2;
      }
      if (ch == 'ü') {
        strcpy(buf, "\"u");
        l = z = 2;
      }
      if (ch == 'Ä') {
        strcpy(buf, "\"A");
        l = z = 2;
      }
      if (ch == 'Ö') {
        strcpy(buf, "\"O");
        l = z = 2;
      }
      if (ch == 'Ü') {
        strcpy(buf, "\"U");
        l = z = 2;
      }
      if (ch == 'ß') {
        strcpy(buf, "\"s");
        l = z = 2;
      }*/
    }
  }
  //pos = cursor increment

  //return false if nothing has to be inserted
  if (buf.isEmpty()) return false;

  //auto deletion of the marked text occurs not very often and can therefore
  //  be recorded separately
  if (c.flags & cfDelOnInput) delMarkedText(c);

/*  //do nothing if spaces will be removed
  if (onlySpaces && c.flags & cfRemoveSpaces && c.cursor.x() >= textLine->length()) {
    //no trailing space, but move the cursor

    if (c.flags & cfGroupUndo) {
      //modify last undo step that it includes the cursor motion
      KWActionGroup *g = undoList.getLast();
      if (undoCount < 1024 && g != 0L && g->undoType == KWActionGroup::ugInsChar
        && g->end.x == c.cursor.x() && g->end.y == c.cursor.y()) {

        g->end.x += pos;
      }
    }

    c.cursor.x() += pos;
    c.view->updateCursor(c.cursor);
    return true;
  }*/

  recordStart(c, KWActionGroup::ugInsChar);
  recordReplace(c/*.cursor*/, (c.flags & cfOvr) ? buf.length() : 0, buf);
  c.cursor.moveX(pos);

  if (c.flags & cfWordWrap && c.wrapAt > 0) {
    int line;
    const QChar *s;
//    int pos;
    KWCursor actionCursor;

    line = c.cursor.y();
    do {
      textLine = m_contents.at(line);
      s = textLine->getText();
      l = textLine->length();
      for (z = c.wrapAt; z < l; z++) if (!s[z].isSpace()) break; //search for text to wrap
      if (z >= l) break; // nothing more to wrap
      pos = c.wrapAt;
      for (; z >= 0; z--) {//find wrap position
        if (s[z].isSpace()) {
          pos = z + 1;
          break;
        }
      }
      //pos = wrap position

      if (line == c.cursor.y() && pos <= c.cursor.x()) {
        //wrap cursor
	c.cursor.move(-pos, 1);
      }

      if (textLine == m_contents.getLast() || m_contents.next()->length() == 0) {
        //at end of doc: create new line
        actionCursor.set(pos, line);
        recordAction(KWAction::newLine, actionCursor);
      } else {
        //wrap
        actionCursor.setY(line + 1);
        if (!s[l - 1].isSpace()) {//add space in next line if necessary
          actionCursor.setX(0);
          recordInsert(actionCursor, " ");
        }
        actionCursor.setX(textLine->length() - pos);
        recordAction(KWAction::wordWrap, actionCursor);
      }
      line++;
    } while (true);
  }
  recordEnd(c);
  return true;
}

QString tabString(int pos, int tabChars) {
  QString s;
  while (pos >= tabChars) {
    s += '\t';
    pos -= tabChars;
  }
  while (pos > 0) {
    s += ' ';
    pos--;
  }
  return s;
}

void KWriteDoc::newLine(VConfig &c) {

  //auto deletion of marked text is done by the view to have a more
  // "low level" KWriteDoc::newLine method
  recordStart(c, KWActionGroup::ugInsLine);

  if (!(c.flags & cfAutoIndent)) {
    recordAction(KWAction::newLine, c.cursor);
    c.cursor.incY();
    c.cursor.setX(0);
  } else {
    TextLine *textLine;
    int pos;

    textLine = m_contents.at(c.cursor.y());
    pos = textLine->firstChar();
    if (c.cursor.x() < pos) c.cursor.setX(pos); // place cursor on first char if before

    while (pos < 0) {// search a not empty text line
      textLine = m_contents.prev();
      if (textLine == 0L) break;
      pos = textLine->firstChar();
    }
    recordAction(KWAction::newLine, c.cursor);
    c.cursor.incY();
    c.cursor.setX(0);
    if (pos > 0) {
      pos = textLine->cursorX(pos, m_tabChars);
      if (m_contents.at(c.cursor.y())->length() > 0) {
        QString s = tabString(pos, (c.flags & cfSpaceIndent) ? 0xffffff : m_tabChars);
        recordInsert(c.cursor, s);
        pos = s.length();
      }
//      recordInsert(c.cursor, QString(textLine->getText(), pos));
      c.cursor.setX(pos);
    }
  }

  recordEnd(c);
}

void KWriteDoc::killLine(VConfig &c) {

  recordStart(c, KWActionGroup::ugDelLine);
  c.cursor.setX(0);
  recordDelete(c.cursor, 0xffffff);
  if (c.cursor.y() < (int) m_contents.count() -1) {
    recordAction(KWAction::killLine, c.cursor);
  }
  recordEnd(c);
}

void KWriteDoc::backspace(VConfig &c) {
  if (c.cursor.x() <= 0 && c.cursor.y() <= 0)
    return;

  if (c.cursor.x() > 0) {
    recordStart(c, KWActionGroup::ugDelChar);

    if (!(c.flags & cfBackspaceIndents)) {
      // ordinary backspace
      c.cursor.decX();
      recordDelete(c.cursor, 1);
    } else {
      // backspace indents: erase to next indent position
      int l = 1; // del one char
      TextLine *textLine = m_contents.at(c.cursor.y());
      int pos = textLine->firstChar();

      if (pos < 0 || pos >= c.cursor.x()) {
        // only spaces on left side of cursor
        // search a line with less spaces
        while ((textLine = m_contents.prev()) != 0L) {
          pos = textLine->firstChar();

	  if (pos >= 0 && pos < c.cursor.x()) {
            l = c.cursor.x() - pos; // del more chars
            break;
          }
        }
      }
      // break effectively jumps here
      c.cursor.moveX(-l);
      recordDelete(c.cursor, l);
    }
  } else {
    // c.cursor.x() == 0: wrap to previous line
    recordStart(c, KWActionGroup::ugDelLine);
    int len = m_contents.at(c.cursor.y() - 1)->length();
    c.cursor.set(len, c.cursor.y() - 1);
    recordAction(KWAction::delLine, c.cursor);
  }

  recordEnd(c);
}

void KWriteDoc::backspaceWord(VConfig &c) {

  if (c.cursor.x() <= 0 && c.cursor.y() <= 0)
    return;

  if (c.cursor.x() > 0) {
    TextLine *textLine = m_contents.at(c.cursor.y());
    int x = c.cursor.x();

    // backspace to next word
    recordStart(c, KWActionGroup::ugDelChar);
    do {
      c.cursor.decX();
    } while (c.cursor.x() > 0 && !m_highlight->isInWord(textLine->getChar(c.cursor.x())));

    while (c.cursor.x() > 0 && m_highlight->isInWord(textLine->getChar(c.cursor.x() -1)))
      c.cursor.decX();

    recordDelete(c.cursor, x - c.cursor.x());
  } else {
    // c.cursor.x() == 0: wrap to previous line
    recordStart(c, KWActionGroup::ugDelLine);
    int len = m_contents.at(c.cursor.y() - 1)->length();
    c.cursor.set(len, c.cursor.y() - 1);
    recordAction(KWAction::delLine, c.cursor);
  }

  recordEnd(c);
}


void KWriteDoc::delChar(VConfig &c) {
  TextLine *textLine;
  int len;

  textLine = m_contents.at(c.cursor.y());
  len = (c.flags & cfRemoveSpaces) ? textLine->lastChar() : textLine->length();
  if (c.cursor.x() < len) {
    // delete one character
    recordStart(c, KWActionGroup::ugDelChar);
    recordDelete(c.cursor, 1);
    recordEnd(c);
  } else {
    if (c.cursor.y() < (int) m_contents.count() -1) {
      // wrap next line to this line
      textLine->truncate(c.cursor.x()); // truncate spaces
      recordStart(c, KWActionGroup::ugDelLine);
      recordAction(KWAction::delLine, c.cursor);
      recordEnd(c);
    }
  }
}

void KWriteDoc::delWord(VConfig &c) {
  TextLine *textLine;
  int len;

  textLine = m_contents.at(c.cursor.y());
  len = (c.flags & cfRemoveSpaces) ? textLine->lastChar() : textLine->length();
  if (c.cursor.x() < len) {
    int x = c.cursor.x();

    // delete one word
    recordStart(c, KWActionGroup::ugDelChar);
    while (x < len && m_highlight->isInWord(textLine->getChar(x)))
      x++;

    while (x < len && !m_highlight->isInWord(textLine->getChar(x)))
      x++;

    recordDelete(c.cursor, x - c.cursor.x());
    recordEnd(c);
  } else {
    if (c.cursor.y() < (int) m_contents.count() -1) {
      // wrap next line to this line
      textLine->truncate(c.cursor.x()); // truncate spaces
      recordStart(c, KWActionGroup::ugDelLine);
      recordAction(KWAction::delLine, c.cursor);
      recordEnd(c);
    }
  }
}

void KWriteDoc::clear() {
  KWCursor cursor;
  KWriteView *view;

  setPseudoModal(0L);
  cursor.set(0, 0);

  for (view = m_views.first(); view != 0L; view = m_views.next()) {
    view->updateCursor(cursor);
    view->tagAll();
  }

  m_eolMode = eolUnix;
  m_contents.clear();
  m_contents.append(m_longestLine = new TextLine());
  m_maxLength = 0;
  m_select.setX(-1);
  m_selectStart = 0xffffff;
  m_selectEnd = 0;
  m_oldMarkState = false;
  m_foundLine = -1;
  setModified(false);
  m_undoList.clear();
  m_currentUndo = 0;
  newUndo();
}

void KWriteDoc::cut(VConfig &c) {
  if (m_selectEnd < m_selectStart)
    return;

  copy(c.flags);
  delMarkedText(c);
}

void KWriteDoc::copy(int flags) {
  if (m_selectEnd < m_selectStart)
    return;

  QString s = markedText(flags);
  if (!s.isEmpty()) {
    if (m_singleSelectMode)
      disconnect(QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
    QApplication::clipboard()->setText(s);
    if (m_singleSelectMode) {
      connect(QApplication::clipboard(), SIGNAL(dataChanged()),
        this, SLOT(clipboardChanged()));
    }
  }
}

void KWriteDoc::paste(VConfig &c) {
  QString s = QApplication::clipboard()->text();

  if (!s.isEmpty()) {
    insert(c, s);
  }
}

void KWriteDoc::toggleRect(int start, int end, int x1, int x2) {
  int z, line;
  bool t;

  if (x1 > x2) {
    z = x1;
    x1 = x2;
    x2 = z;
  }
  if (start > end) {
    z = start;
    start = end;
    end = z;
  }

  t = false;
  for (line = start; line < end; line++) {
    int x, oldX, s, e, newX1, newX2;
    TextLine *textLine;
    char ch;
    Attribute *a;

    textLine = m_contents.at(line);

    //--- speed optimization
    //s = textPos(textLine, x1, newX1);
    x = oldX = z = 0;
    while (x < x1) {// && z < len) {
      oldX = x;
      ch = textLine->getChar(z);
      a = &m_attribs[textLine->getAttr(z)];
      x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->width(ch);//a->fm.width(ch);
      z++;
    }
    s = z;
    if (x1 - oldX < x - x1 && z > 0) {
      s--;
      newX1 = oldX;
    } else newX1 = x;
    //e = textPos(textLine, x2, newX2);
    while (x < x2) {// && z < len) {
      oldX = x;
      ch = textLine->getChar(z);
      a = &m_attribs[textLine->getAttr(z)];
      x += (ch == '\t') ? m_tabWidth - (x % m_tabWidth) : a->width(ch);//a->fm.width(ch);
      z++;
    }
    e = z;
    if (x2 - oldX < x - x2 && z > 0) {
      e--;
      newX2 = oldX;
    } else newX2 = x;
    //---

    if (e > s) {
      textLine->toggleSelect(s, e);
      tagLineRange(line, newX1, newX2);
      t = true;
    }
  }
  if (t) {
    end--;
//    tagLines(start, end);

    if (start < m_selectStart) m_selectStart = start;
    if (end > m_selectEnd) m_selectEnd = end;
  }
}

void KWriteDoc::selectTo(VConfig &c, KWCursor &cursor, int cXPos) {
  //c.cursor = old cursor position
  //cursor = new cursor position

  if (c.cursor != m_select) {
    //new selection

    if (!(c.flags & cfKeepSelection)) deselectAll();
//      else recordReset();

    m_anchor = c.cursor;
    m_aXPos = c.cXPos;
  }

  if (!(c.flags & cfVerticalSelect)) {
    //horizontal selections
    TextLine *textLine;
    int x, y, sXPos;
    int ex, ey, eXPos;
    bool sel;

//  if (cursor.y() > c.cursor.y() || (cursor.y() == c.cursor.y() && cursor.x() > c.cursor.x())) {
    if (cursor > c.cursor) {
      x = c.cursor.x();
      y = c.cursor.y();
      sXPos = c.cXPos;
      ex = cursor.x();
      ey = cursor.y();
      eXPos = cXPos;
      sel = true;
    } else {
      x = cursor.x();
      y = cursor.y();
      sXPos = cXPos;
      ex = c.cursor.x();
      ey = c.cursor.y();
      eXPos = c.cXPos;
      sel = false;
    }

//    tagLines(y, ye);
    if (y < ey) {
      //tagLineRange(y, sXPos, 0xffffff);
      tagLines(y, ey -1);
      tagLineRange(ey, 0, eXPos);
    } else tagLineRange(y, sXPos, eXPos);

    if (y < m_selectStart) m_selectStart = y;
    if (ey > m_selectEnd) m_selectEnd = ey;

    textLine = m_contents.at(y);

    if (c.flags & cfXorSelect) {
      //xor selection with old selection
      while (y < ey) {
        textLine->toggleSelectEol(x);
        x = 0;
        y++;
        textLine = m_contents.at(y);
      }
      textLine->toggleSelect(x, ex);
    } else {
      //set selection over old selection

//    if (m_anchor.y() > y || (m_anchor.y() == y && m_anchor.x() > x)) {
      if (m_anchor > KWCursor(x, y)) {
//      if (m_anchor.y() < ey || (m_anchor.y() == ey && m_anchor.x() < ex)) {
        if (KWCursor(ex, ey) > m_anchor) {
          sel = !sel;
          while (y < m_anchor.y()) {
            textLine->selectEol(sel, x);
            x = 0;
            y++;
            textLine = m_contents.at(y);
          }
          textLine->select(sel, x, m_anchor.x());
          x = m_anchor.x();
        }
        sel = !sel;
      }
      while (y < ey) {
        textLine->selectEol(sel, x);
        x = 0;
        y++;
        textLine = m_contents.at(y);
      }
      textLine->select(sel, x, ex);
    }
  } else {
    //vertical(block) selections
//    int ax, sx, ex;

//    ax = textWidth(anchor);
//    sx = textWidth(start);
//    ex = textWidth(end);

    toggleRect(c.cursor.y() + 1, cursor.y() + 1, m_aXPos, c.cXPos);
    toggleRect(m_anchor.y(), cursor.y() + 1, c.cXPos, cXPos);
  }
  m_select = cursor;
  optimizeSelection();
}

void KWriteDoc::selectAll() {
  int z;
  TextLine *textLine;

  m_select.setX(-1);

  unmarkFound();
//  if (selectStart != 0 || selectEnd != lastLine()) recordReset();

  m_selectStart = 0;
  m_selectEnd = lastLine();

  tagLines(m_selectStart, m_selectEnd);

  for (z = m_selectStart; z < m_selectEnd; z++) {
    textLine = m_contents.at(z);
    textLine->selectEol(true, 0);
  }
  textLine = m_contents.at(z);
  textLine->select(true, 0, textLine->length());
}

void KWriteDoc::deselectAll() {
  int z;
  TextLine *textLine;

  m_select.setX(-1);
  if (m_selectEnd < m_selectStart) return;

  unmarkFound();
//  recordReset();

  tagLines(m_selectStart, m_selectEnd);

  for (z = m_selectStart; z <= m_selectEnd; z++) {
    textLine = m_contents.at(z);
    textLine->selectEol(false, 0);
  }
  m_selectStart = 0xffffff;
  m_selectEnd = 0;
}

void KWriteDoc::invertSelection() {
  int z;
  TextLine *textLine;

  m_select.setX(-1);

  unmarkFound();
//  if (selectStart != 0 || selectEnd != lastLine()) recordReset();

  m_selectStart = 0;
  m_selectEnd = lastLine();

  tagLines(m_selectStart, m_selectEnd);

  for (z = m_selectStart; z < m_selectEnd; z++) {
    textLine = m_contents.at(z);
    textLine->toggleSelectEol(0);
  }
  textLine = m_contents.at(z);
  textLine->toggleSelect(0, textLine->length());
  optimizeSelection();
}

void KWriteDoc::selectWord(KWCursor &cursor, int flags) {
  TextLine *textLine;
  int start, end, len;

  textLine = m_contents.at(cursor.y());
  len = textLine->length();
  start = end = cursor.x();
  while (start > 0 && m_highlight->isInWord(textLine->getChar(start - 1))) start--;
  while (end < len && m_highlight->isInWord(textLine->getChar(end))) end++;
  if (end <= start) return;
  if (!(flags & cfKeepSelection)) deselectAll();
//    else recordReset();

  textLine->select(true, start, end);

  m_anchor.set(start, cursor.y());
  m_select.set(end, cursor.y());

  tagLines(cursor.y(), cursor.y());
  if (cursor.y() < m_selectStart) m_selectStart = cursor.y();
  if (cursor.y() > m_selectEnd) m_selectEnd = cursor.y();
}

void KWriteDoc::doIndent(VConfig &c, int change) {

  c.cursor.setX(0);

  recordStart(c, (change < 0) ? KWActionGroup::ugUnindent
    : KWActionGroup::ugIndent);

  if (m_selectEnd < m_selectStart) {
    // single line
    optimizeLeadingSpace(c.cursor.y(), c.flags, change);
  } else {
    // entire selection
    TextLine *textLine;
    int line, z;
    QChar ch;

    if (c.flags & cfKeepIndentProfile && change < 0) {
      // unindent so that the existing indent profile doesn´t get screwed
      // if any line we may unindent is already full left, don't do anything
      for (line = m_selectStart; line <= m_selectEnd; line++) {
        textLine = m_contents.at(line);
        if (textLine->isSelected() || textLine->numSelected()) {
          for (z = 0; z < tabWidth(); z++) {
            ch = textLine->getChar(z);
            if (ch == '\t') break;
            if (ch != ' ') {
              change = 0;
              goto jumpOut;
            }
          }
        }
      }
      jumpOut:;
    }

    for (line = m_selectStart; line <= m_selectEnd; line++) {
      textLine = m_contents.at(line);
      if (textLine->isSelected() || textLine->numSelected()) {
        optimizeLeadingSpace(line, c.flags, change);
      }
    }
  }
  // recordEnd now removes empty undo records
  recordEnd(c.view, c.cursor, c.flags | cfPersistent);
}

/*
  Optimize the leading whitespace for a single line.
  If change is > 0, it adds indentation units(tabChars)
  if change is == 0, it only optimizes
  If change is < 0, it removes indentation units
  This will be used to indent, unindent, and optimal-fill a line.
  If excess space is removed depends on the flag cfKeepExtraSpaces
  which has to be set by the user
*/
void KWriteDoc::optimizeLeadingSpace(int line, int flags, int change) {
  TextLine *textLine;
  int len;
  int chars, space, okLen;
  QChar ch;
  int extra;
  QString s;
  KWCursor cursor;

  textLine = m_contents.at(line);
  len = textLine->length();
  space = 0; // length of space at the beginning of the textline
  okLen = 0; // length of space which does not have to be replaced
  for (chars = 0; chars < len; chars++) {
    ch = textLine->getChar(chars);
    if (ch == ' ') {
      space++;
      if (flags & cfSpaceIndent && okLen == chars) okLen++;
    } else if (ch == '\t') {
      space += m_tabChars - space % m_tabChars;
      if (!(flags & cfSpaceIndent) && okLen == chars) okLen++;
    } else break;
  }

  space += change*m_tabChars; // modify space width
  // if line contains only spaces it will be cleared
  if (space < 0 || chars == len) space = 0;

  extra = space % m_tabChars; // extra spaces which don´t fit the indentation pattern
  if (flags & cfKeepExtraSpaces) chars -= extra;

  if (flags & cfSpaceIndent) {
    space -= extra;
    ch = ' ';
  } else {
    space /= m_tabChars;
    ch = '\t';
  }

  // don´t replace chars which are already ok
  cursor.setX(QMIN(okLen, QMIN(chars, space)));
  chars -= cursor.x();
  space -= cursor.x();
  if (chars == 0 && space == 0) return; //nothing to do

  s.fill(ch, space);

//printf("chars %d insert %d cursor.x() %d\n", chars, insert, cursor.x());
  cursor.setY(line);
  recordReplace(cursor, chars, s);
}


QString KWriteDoc::text() {
  TextLine *textLine;
  QString s;

  textLine = m_contents.first();
  do {
    s.insert(s.length(), textLine->getText(), textLine->length());
    textLine = m_contents.next();
    if (!textLine) break;
    s.append('\n');
  } while (true);
  return s;
}

QString KWriteDoc::getWord(KWCursor &cursor) {
  TextLine *textLine;
  int start, end, len;

  textLine = m_contents.at(cursor.y());
  len = textLine->length();
  start = end = cursor.x();
  while (start > 0 && m_highlight->isInWord(textLine->getChar(start - 1))) start--;
  while (end < len && m_highlight->isInWord(textLine->getChar(end))) end++;
  len = end - start;
  return QString(&textLine->getText()[start], len);
}

void KWriteDoc::setText(const QString &s) {
  TextLine *textLine;
  int pos;
  QChar ch;

  clear();

  textLine = m_contents.getFirst();
  for (pos = 0; pos <= (int) s.length(); pos++) {
    ch = s[pos];
    if (ch.isPrint() || ch == '\t') {
      textLine->append(&ch, 1);
    } else if (ch == '\n') {
      textLine = new TextLine();
      m_contents.append(textLine);
    }
  }
  updateLines();
}


QString KWriteDoc::markedText(int flags) {
  TextLine *textLine;
  int len, z, start, end, i;

  len = 1;
  if (!(flags & cfVerticalSelect)) {
    for (z = m_selectStart; z <= m_selectEnd; z++) {
      textLine = m_contents.at(z);
      len += textLine->numSelected();
      if (textLine->isSelected()) len++;
    }
    QString s;
    len = 0;
    for (z = m_selectStart; z <= m_selectEnd; z++) {
      textLine = m_contents.at(z);
      end = 0;
      do {
        start = textLine->findUnselected(end);
        end = textLine->findSelected(start);
        for (i = start; i < end; i++) {
          s[len] = textLine->getChar(i);
          len++;
        }
      } while (start < end);
      if (textLine->isSelected()) {
        s[len] = '\n';
        len++;
      }
    }
    s[len] = '\0';
    return s;
  } else {
    for (z = m_selectStart; z <= m_selectEnd; z++) {
      textLine = m_contents.at(z);
      len += textLine->numSelected() + 1;
    }
    QString s;
    len = 0;
    for (z = m_selectStart; z <= m_selectEnd; z++) {
      textLine = m_contents.at(z);
      end = 0;
      do {
        start = textLine->findUnselected(end);
        end = textLine->findSelected(start);
        for (i = start; i < end; i++) {
          s[len] = textLine->getChar(i);
          len++;
        }
      } while (start < end);
      s[len] = '\n';
      len++;
    }
    s[len] = '\0';
    return s;
  }
}

void KWriteDoc::delMarkedText(VConfig &c/*, bool undo*/) {
  TextLine *textLine;
  int end = 0;

  if (m_selectEnd < m_selectStart) return;

  // the caller may have already started an undo record for the current action
//  if (undo)

  //auto deletion of the marked text occurs not very often and can therefore
  //  be recorded separately
  recordStart(c, KWActionGroup::ugDelBlock);

  for (c.cursor.setY(m_selectEnd); c.cursor.y() >= m_selectStart; c.cursor.decY()) {
    textLine = m_contents.at(c.cursor.y());

    c.cursor.setX(textLine->length());
    do {
      end = textLine->findRevUnselected(c.cursor.x());
      if (end == 0) break;
      c.cursor.setX(textLine->findRevSelected(end));
      recordDelete(c.cursor, end - c.cursor.x());
    } while (true);
    end = c.cursor.x();
    c.cursor.setX(textLine->length());
    if (textLine->isSelected()) recordAction(KWAction::delLine, c.cursor);
  }
  c.cursor.incY();
  /*if (end < c.cursor.x())*/ c.cursor.setX(end);

  m_selectEnd = -1;
  m_select.setX(-1);

  /*if (undo)*/ recordEnd(c);
}

void KWriteDoc::tagLineRange(int line, int x1, int x2) {
  int z;

  for (z = 0; z < (int) m_views.count(); z++) {
    m_views.at(z)->tagLines(line, line, x1, x2);
  }
}

void KWriteDoc::tagLines(int start, int end) {
  int z;

  for (z = 0; z < (int) m_views.count(); z++) {
    m_views.at(z)->tagLines(start, end, 0, 0xffffff);
  }
}

void KWriteDoc::tagAll() {
  int z;

  for (z = 0; z < (int) m_views.count(); z++) {
    m_views.at(z)->tagAll();
  }
}

void KWriteDoc::updateLines(int startLine, int endLine, int flags, int cursorY) {
  TextLine *textLine;
  int line, lastLine;
  int ctxNum, endCtx;

  lastLine = (int) m_contents.count() -1;
  if (endLine >= lastLine) endLine = lastLine;

  line = startLine;
  ctxNum = 0;
  if (line > 0) ctxNum = m_contents.at(line - 1)->getContext();
  do {
    textLine = m_contents.at(line);
    if (line <= endLine && line != cursorY) {
      if (flags & cfRemoveSpaces) textLine->removeSpaces();
      updateMaxLength(textLine);
    }
    endCtx = textLine->getContext();
    ctxNum = m_highlight->doHighlight(ctxNum, textLine);
    textLine->setContext(ctxNum);
    line++;
  } while (line <= lastLine && (line <= endLine || endCtx != ctxNum));
  tagLines(startLine, line - 1);
}


void KWriteDoc::updateMaxLength(TextLine *textLine) {
  int len;

  len = textWidth(textLine, textLine->length());

  if (len > m_maxLength) {
    m_longestLine = textLine;
    m_maxLength = len;
    m_newDocGeometry = true;
  } else {
    if (!m_longestLine || (textLine == m_longestLine && len <= m_maxLength*3/4)) {
      m_maxLength = -1;
      for (textLine = m_contents.first(); textLine != 0L; textLine = m_contents.next()) {
        len = textWidth(textLine, textLine->length());
        if (len > m_maxLength) {
          m_maxLength = len;
          m_longestLine = textLine;
        }
      }
      m_newDocGeometry = true;
    }
  }
}

void KWriteDoc::updateViews(KWriteView *exclude) {
  bool markState = hasMarkedText();
  KWriteView *view = m_views.first();
  int flags = m_newDocGeometry ? ufDocGeometry : 0;

  for (; view != 0L; view = m_views.next()) {
    if (view != exclude) view->updateView(flags);

    // notify every view about the changed mark state....
    if (m_oldMarkState != markState) view->m_kWrite->emitNewStatus();
  }
  m_oldMarkState = markState;
  m_newDocGeometry = false;
}

QColor &KWriteDoc::cursorCol(int x, int y) {
  TextLine *textLine;
  int attr;
  Attribute *a;

  textLine = m_contents.at(y);
  attr = textLine->getRawAttr(x);
  a = &m_attribs[attr & taAttrMask];
  if (attr & taSelectMask) return a->selCol; else return a->col;
}

void KWriteDoc::paintTextLine2(QPainter *p, int line, int xStart, int xEnd, bool /*showTabs*/) {
    //int z;
    int len;
    QString str;
    const QChar *s;
    TextLine *textLine;
    Attribute *a = 0L;
    //int xc, yc;
    int yPos = line*m_fontHeight;


    if (line >= (int) m_contents.count()) {
	p->fillRect(0, 0, xEnd - xStart, m_fontHeight, m_colors[4]);
	return;
    }

    textLine = m_contents.at(line);
    len = textLine->length();
    s =  textLine->getText();

    a = &m_attribs[textLine->getAttr(0)];
    p->setFont(a->font);

    str = QString(s, len);
    p->drawText(0, yPos + m_fontHeight, str);
}

void KWriteDoc::paintTextLine(QPainter &paint, int line, int xStart, int xEnd, bool showTabs) {
  int y;
  TextLine *textLine;
  int len;
  const QChar *s;
  int z, x;
  char ch;
  Attribute *a = 0L;
  int attr, nextAttr;
  int xs;
  int xc, zc;

//  struct timeval tv1, tv2, tv3; //for profiling
//  struct timezone tz;

  y = 0;//line*m_fontHeight - yPos;
  if (line >= (int) m_contents.count()) {
    paint.fillRect(0, y, xEnd - xStart, m_fontHeight, m_colors[4]);
    return;
  }

  textLine = m_contents.at(line);
  len = textLine->length();
  s = textLine->getText();

  // skip to first visible character
  x = 0;
  z = 0;
  do {
    xc = x;
    zc = z;
    if (z == len) break;
    ch = s[z];//textLine->getChar(z);
    if (ch == '\t') {
      x += m_tabWidth - (x % m_tabWidth);
    } else {
      a = &m_attribs[textLine->getAttr(z)];
      x += a->width(ch);//a->fm.width(ch);
    }
    z++;
  } while (x <= xStart);

//gettimeofday(&tv1, &tz);

  // draw background
  xs = xStart;
  attr = textLine->getRawAttr(zc);
  while (x < xEnd) {
    nextAttr = textLine->getRawAttr(z);
    if ((nextAttr ^ attr) & (taSelectMask | 256)) {
      paint.fillRect(xs - xStart, y, x - xs, m_fontHeight, m_colors[attr >> taShift]);
      xs = x;
      attr = nextAttr;
    }
    if (z == len) break;
    ch = s[z];//textLine->getChar(z);
    if (ch == '\t') {
      x += m_tabWidth - (x % m_tabWidth);
    } else {
      a = &m_attribs[attr & taAttrMask];
      x += a->width(ch);//a->fm.width(ch);
    }
    z++;
  }
  paint.fillRect(xs - xStart, y, xEnd - xs, m_fontHeight, m_colors[attr >> taShift]);
  len = z; //reduce length to visible length

//gettimeofday(&tv2, &tz);

  // draw text
  x = xc;
  z = zc;
  y += m_fontAscent -1;
  attr = -1;
  while (z < len) {
    ch = s[z];//textLine->getChar(z);
    if (ch == '\t') {
      if (z > zc) {
        //this should cause no copy at all
        QConstString str((QChar *) &s[zc], z - zc);
        QString s = str.string();
        paint.drawText(x - xStart, y, s);
        x += a->width(s);//a->fm.width(str);//&s[zc], z - zc);
      }
      zc = z +1;

      if (showTabs) {
        nextAttr = textLine->getRawAttr(z);
        if (nextAttr != attr) {
          attr = nextAttr;
          a = &m_attribs[attr & taAttrMask];

          if (attr & taSelectMask) paint.setPen(a->selCol); else paint.setPen(a->col);
          paint.setFont(a->font);
        }
        // visualize tabs
//        paint.drawLine(x - xStart, y -2, x - xStart, y);
//        paint.drawLine(x - xStart, y, x - xStart + 2, y);
        paint.drawPoint(x - xStart, y);
        paint.drawPoint(x - xStart +1, y);
        paint.drawPoint(x - xStart, y -1);
      }
      x += m_tabWidth - (x % m_tabWidth);
    } else {
      nextAttr = textLine->getRawAttr(z);
      if (nextAttr != attr) {
        if (z > zc) {
          QConstString str((QChar *) &s[zc], z - zc);
          //QConstString str((QChar *) &s[zc], z - zc +1);
          QString s = str.string();
          paint.drawText(x - xStart, y, s);
          x += a->width(s);//a->fm.width(str);//&s[zc], z - zc);
          zc = z;
        }
        attr = nextAttr;
        a = &m_attribs[attr & taAttrMask];

        if (attr & taSelectMask) paint.setPen(a->selCol); else paint.setPen(a->col);
        paint.setFont(a->font);
      }
    }
    z++;
  }
  if (z > zc) {
    QConstString str((QChar *) &s[zc], z - zc);
    paint.drawText(x - xStart, y, str.string());
  }
}

/*
void KWriteDoc::printTextLine(QPainter &paint, int line, int xEnd, int y) {
  TextLine *textLine;
  int z, x;
  Attribute *a = 0L;
  int attr, nextAttr;
  char ch;
  char buf[256];
  int bufp;

  if (line >= (int) m_contents.count()) return;
  textLine = m_contents.at(line);

  z = 0;
  x = 0;
  y += fontAscent -1;
  attr = -1;
  bufp = 0;
  while (x < xEnd && z < textLine->length()) {
    ch = textLine->getChar(z);
    if (ch == '\t') {
      if (bufp > 0) {
        paint.drawText(x, y, buf, bufp);
        x += paint.fontMetrics().width(buf, bufp);
        bufp = 0;
      }
      x += tabWidth - (x % tabWidth);
    } else {
      nextAttr = textLine->getAttr(z);
      if (nextAttr != attr || bufp >= 256) {
        if (bufp > 0) {
          paint.drawText(x, y, buf, bufp);
          x += paint.fontMetrics().width(buf, bufp);
          bufp = 0;
        }
        attr = nextAttr;
        a = &attribs[attr];
        paint.setFont(a->font);
      }
      buf[bufp] = ch;
      bufp++;
    }
    z++;
  }
  if (bufp > 0) paint.drawText(x, y, buf, bufp);
}
*/

void KWriteDoc::paintBorder(KWrite *kWrite, QPainter &paint, int line,
  int yStart, int yEnd) {

  KWLineAttribute *current;
  int y;

  current = m_lineAttribs.first();

  // attribs are ordered by line number, find the first attribute to paint
  while (current != 0L) {
    if (current->line() >= line) break;
    current = m_lineAttribs.next(current);
  }

  // paint attributes
  while (current != 0L) {
    y = yStart + (current->line() - line)*fontHeight();
    if (y > yEnd) break;

    // paint all doc-attribs (m_kWrite == 0L), but only the correct view-attribs
    if (current->m_kWrite == 0L || current->m_kWrite == kWrite)
      current->paint(paint, y, fontHeight());
    current = m_lineAttribs.next(current);
  }

  // paint line numbers;
  if (kWrite->numbersDigits() > 0) {
    Attribute *a;
    QString numberString;
    int number, z;

    a = &m_attribs[0];
    paint.setFont(a->font);
    paint.setPen(a->col);

    numberString.fill(' ', kWrite->numbersDigits());

    y = yStart;
    while (y < yEnd) {
      line++;

      // do number to string by hand to make it fast (no heap-operations)
      number = line;
      z = kWrite->numbersDigits();
      do {
        z--;
        numberString[z] = '0' + number % 10;
	number /= 10;
      } while (number > 0 && z > 0);

      paint.drawText(kWrite->numbersX(), y + m_fontAscent - 1, numberString);
      y += fontHeight();
    }
  }
}


bool KWriteDoc::hasFileName() {
  return m_fName.findRev('/') +1 < (int) m_fName.length();
}

const QString KWriteDoc::fileName() {
  return m_fName;
}

void KWriteDoc::setFileName(const QString &s) {
  int pos;
  KWriteView *view;
  m_fName = s;

  for (view = m_views.first(); view != 0L; view = m_views.next()) {
    emit view->m_kWrite->fileChanged();
  }

  //highlight detection
  pos = m_fName.findRev('/') + 1;

  if (pos >= (int) m_fName.length())
    return; //no filename

  findHighlight(m_fName.right(m_fName.length() - pos));

  updateViews();
}

void KWriteDoc::clearFileName() {
  KWriteView *view;

  // keep current path
  m_fName.truncate(m_fName.findRev('/') + 1);

  // notify views
  for (view = m_views.first(); view != 0L; view = m_views.next()) {
    emit view->m_kWrite->fileChanged();
  }
}

bool KWriteDoc::doSearch(SConfig &sc, const QString &searchFor) {
  int line, col;
  int searchEnd;
//  int slen, blen, tlen;
//  char *s, *b, *t;
  QString str;
  int slen, smlen, bufLen, tlen;
  const QChar *s;
  QChar *t;
  TextLine *textLine;
  int z, pos, newPos;

  if (searchFor.isEmpty()) return false;
  str = (sc.flags & sfCaseSensitive) ? searchFor : searchFor.lower();

  s = str.unicode();
  slen = str.length();
  smlen = slen*sizeof(QChar);
  bufLen = 0;
  t = 0L;

  line = sc.cursor.y();
  col = sc.cursor.x();
  if (!(sc.flags & sfBackward)) {
    //forward search
    if (sc.flags & sfSelected) {
      if (line < m_selectStart) {
        line = m_selectStart;
        col = 0;
      }
      searchEnd = m_selectEnd;
    } else searchEnd = lastLine();

    while (line <= searchEnd) {
      textLine = m_contents.at(line);
      tlen = textLine->length();
      if (tlen > bufLen) {
        delete t;
        bufLen = (tlen + 255) & (~255);
        t = new QChar[bufLen];
      }
      memcpy(t, textLine->getText(), tlen*sizeof(QChar));
      if (sc.flags & sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(pos);
          newPos = textLine->findUnselected(pos);
          memset(&t[pos], 0, (newPos - pos)*sizeof(QChar));
          pos = newPos;
        } while (pos < tlen);
      }
      if (!(sc.flags & sfCaseSensitive)) for (z = 0; z < tlen; z++) t[z] = t[z].lower();

      tlen -= slen;
      if (sc.flags & sfWholeWords && tlen > 0) {
        //whole word search
        if (col == 0) {
          if (!m_highlight->isInWord(t[slen])) {
            if (memcmp(t, s, smlen) == 0) goto found;
          }
          col++;
        }
        while (col < tlen) {
          if (!m_highlight->isInWord(t[col -1]) && !m_highlight->isInWord(t[col + slen])) {
            if (memcmp(&t[col], s, smlen) == 0) goto found;
          }
          col++;
        }
        if (!m_highlight->isInWord(t[col -1]) && memcmp(&t[col], s, smlen) == 0)
          goto found;
      } else {
        //normal search
        while (col <= tlen) {
          if (memcmp(&t[col], s, smlen) == 0) goto found;
          col++;
        }
      }
      col = 0;
      line++;
    }
  } else {
    // backward search
    if (sc.flags & sfSelected) {
      if (line > m_selectEnd) {
        line = m_selectEnd;
        col = -1;
      }
      searchEnd = m_selectStart;
    } else searchEnd = 0;;

    while (line >= searchEnd) {
      textLine = m_contents.at(line);
      tlen = textLine->length();
      if (tlen > bufLen) {
        delete t;
        bufLen = (tlen + 255) & (~255);
        t = new QChar[bufLen];
      }
      memcpy(t, textLine->getText(), tlen*sizeof(QChar));
      if (sc.flags & sfSelected) {
        pos = 0;
        do {
          pos = textLine->findSelected(pos);
          newPos = textLine->findUnselected(pos);
          memset(&t[pos], 0, (newPos - pos)*sizeof(QChar));
          pos = newPos;
        } while (pos < tlen);
      }
      if (!(sc.flags & sfCaseSensitive)) for (z = 0; z < tlen; z++) t[z] = t[z].lower();

      if (col < 0 || col > tlen) col = tlen;
      col -= slen;
      if (sc.flags & sfWholeWords && tlen > slen) {
        //whole word search
        if (col + slen == tlen) {
          if (!m_highlight->isInWord(t[col -1])) {
            if (memcmp(&t[col], s, smlen) == 0) goto found;
          }
          col--;
        }
        while (col > 0) {
          if (!m_highlight->isInWord(t[col -1]) && !m_highlight->isInWord(t[col+slen])) {
            if (memcmp(&t[col], s, smlen) == 0) goto found;
          }
          col--;
        }
        if (!m_highlight->isInWord(t[slen]) && memcmp(t, s, smlen) == 0)
          goto found;
      } else {
        //normal search
        while (col >= 0) {
          if (memcmp(&t[col], s, slen) == 0) goto found;
          col--;
        }
      }
      line--;
    }
  }
  sc.flags |= sfWrapped;
  return false;
found:
  if (sc.flags & sfWrapped) {
    if ((line > sc.startCursor.y() || (line == sc.startCursor.y() && col >= sc.startCursor.x()))
      ^ ((sc.flags & sfBackward) != 0)) return false;
  }
  sc.cursor.set(col, line);
  return true;
}

void KWriteDoc::unmarkFound() {
  if (m_pseudoModal) return;
  if (m_foundLine != -1) {
    m_contents.at(m_foundLine)->unmarkFound();
    tagLines(m_foundLine, m_foundLine);
    m_foundLine = -1;
  }
}

void KWriteDoc::markFound(KWCursor &cursor, int len) {
//  unmarkFound();
//  recordReset();
  if (m_foundLine != -1) {
    m_contents.at(m_foundLine)->unmarkFound();
    tagLines(m_foundLine, m_foundLine);
  }
  m_contents.at(cursor.y())->markFound(cursor.x(), len);
  m_foundLine = cursor.y();
  tagLines(m_foundLine, m_foundLine);
}


void KWriteDoc::tagLine(int line) {
  if (m_tagStart > line) m_tagStart = line;
  if (m_tagEnd < line) m_tagEnd = line;
}

void KWriteDoc::insLine(int line) {
  if (m_selectStart >= line) m_selectStart++;
  if (m_selectEnd >= line) m_selectEnd++;
  if (m_tagStart >= line) m_tagStart++;
  if (m_tagEnd >= line) m_tagEnd++;

  m_newDocGeometry = true;

  // notify the views about the new line
  KWriteView *view;
  for (view = m_views.first(); view != 0L; view = m_views.next()) {
    view->insLine(line);
  }

  // notify the line attributes
  KWLineAttribute *current = m_lineAttribs.first();
  while (current != 0L) {
    current->insLine(line);
    current = m_lineAttribs.next(current);
  }
}

void KWriteDoc::delLine(int line) {
  if (m_selectStart >= line && m_selectStart > 0) m_selectStart--;
  if (m_selectEnd >= line) m_selectEnd--;
  if (m_tagStart >= line && m_tagStart > 0) m_tagStart--;
  if (m_tagEnd >= line) m_tagEnd--;

  m_newDocGeometry = true;

  // notify the views about the deleted line
  KWriteView *view;
  for (view = m_views.first(); view != 0L; view = m_views.next()) {
    view->delLine(line);
  }

  // notify the line attributes
  KWLineAttribute *current = m_lineAttribs.first();
  while (current != 0L) {
    current->delLine(line);
    current = m_lineAttribs.next(current);
  }
}

void KWriteDoc::optimizeSelection() {
  TextLine *textLine;

  while (m_selectStart <= m_selectEnd) {
    textLine = m_contents.at(m_selectStart);
    if (textLine->isSelected() || textLine->numSelected() > 0) break;
    m_selectStart++;
  }
  while (m_selectEnd >= m_selectStart) {
    textLine = m_contents.at(m_selectEnd);
    if (textLine->isSelected() || textLine->numSelected() > 0) break;
    m_selectEnd--;
  }
  if (m_selectStart > m_selectEnd) {
    m_selectStart = 0xffffff;
    m_selectEnd = 0;
  }
}

void KWriteDoc::doAction(KWAction *a) {

  switch (a->action) {
    case KWAction::replace:
      doReplace(a);
      break;
    case KWAction::wordWrap:
      doWordWrap(a);
      break;
    case KWAction::wordUnWrap:
      doWordUnWrap(a);
      break;
    case KWAction::newLine:
      doNewLine(a);
      break;
    case KWAction::delLine:
      doDelLine(a);
      break;
    case KWAction::insLine:
      doInsLine(a);
      break;
    case KWAction::killLine:
      doKillLine(a);
      break;
/*    case KWAction::doubleLine:
      break;
    case KWAction::removeLine:
      break;*/
  }
}

void KWriteDoc::doReplace(KWAction *a) {
  TextLine *textLine;
  int l;

  //exchange current text with stored text in KWAction *a

  textLine = m_contents.at(a->cursor.y());
  l = textLine->length() - a->cursor.x();
  if (l > a->len) l = a->len;

  QString oldText(&textLine->getText()[a->cursor.x()], (l < 0) ? 0 : l);
  textLine->replace(a->cursor.x(), a->len, a->text.unicode(), a->text.length());

  a->len = a->text.length();
  a->text = oldText;

  tagLine(a->cursor.y());
}

void KWriteDoc::doWordWrap(KWAction *a) {
  TextLine *textLine;

  textLine = m_contents.at(a->cursor.y() - 1);
  a->len = textLine->length() - a->cursor.x();
  textLine->wrap(m_contents.next(), a->len);

  tagLine(a->cursor.y() - 1);
  tagLine(a->cursor.y());
  if (m_selectEnd == a->cursor.y() - 1) m_selectEnd++;

  a->action = KWAction::wordUnWrap;
}

void KWriteDoc::doWordUnWrap(KWAction *a) {
  TextLine *textLine;

  textLine = m_contents.at(a->cursor.y() - 1);
//  textLine->setLength(a->len);
  textLine->unWrap(a->len, m_contents.next(), a->cursor.x());

  tagLine(a->cursor.y() - 1);
  tagLine(a->cursor.y());

  a->action = KWAction::wordWrap;
}

void KWriteDoc::doNewLine(KWAction *a) {
  TextLine *textLine, *newLine;

  textLine = m_contents.at(a->cursor.y());
  newLine = new TextLine(textLine->getRawAttr(), textLine->getContext());
  textLine->wrap(newLine, a->cursor.x());
  m_contents.insert(a->cursor.y() + 1, newLine);

  insLine(a->cursor.y() + 1);
  tagLine(a->cursor.y());
  tagLine(a->cursor.y() + 1);
  if (m_selectEnd == a->cursor.y()) m_selectEnd++;//addSelection(a->cursor.y() + 1);

  a->action = KWAction::delLine;
}

void KWriteDoc::doDelLine(KWAction *a) {
  TextLine *textLine, *nextLine;

  textLine = m_contents.at(a->cursor.y());
  nextLine = m_contents.next();
//  textLine->setLength(a->cursor.x());
  textLine->unWrap(a->cursor.x(), nextLine, nextLine->length());
  textLine->setContext(nextLine->getContext());
  if (m_longestLine == nextLine) m_longestLine = 0L;
  m_contents.remove();

  tagLine(a->cursor.y());
  delLine(a->cursor.y() + 1);

  a->action = KWAction::newLine;
}

void KWriteDoc::doInsLine(KWAction *a) {
  m_contents.insert(a->cursor.y(), new TextLine());

  insLine(a->cursor.y());

  a->action = KWAction::killLine;
}

void KWriteDoc::doKillLine(KWAction *a) {
  TextLine *textLine;

  textLine = m_contents.at(a->cursor.y());
  if (m_longestLine == textLine) m_longestLine = 0L;
  m_contents.remove();

  delLine(a->cursor.y());
  tagLine(a->cursor.y());

  a->action = KWAction::insLine;
}

void KWriteDoc::newUndo() {
  KWriteView *view;
  int state = 0;

  if (m_readWrite) {
    if (m_currentUndo > 0) state |= KWrite::kUndoPossible;
    if (m_currentUndo < (int) m_undoList.count()) state |= KWrite::kRedoPossible;
  }
  // only emit signals if undo state has changed
  if (state != m_undoState) {
    m_undoState = state;
    for (view = m_views.first(); view != 0L; view = m_views.next()) {
      view->m_kWrite->emitNewUndo();
    }
  }
}

void KWriteDoc::recordStart(VConfig &c, int newUndoType) {
  recordStart(c.view, c.cursor, c.flags, newUndoType);
}

void KWriteDoc::recordStart(KWriteView *, KWCursor &cursor, int flags,
  int newUndoType, bool keepModal, bool mergeUndo) {

  KWActionGroup *g;

  if (!keepModal) setPseudoModal(0L);

  // try to append to last action
  g = m_undoList.getLast();
  if (g != 0L && ((m_undoCount < 1024 && flags & cfGroupUndo
    && g->end == cursor) || mergeUndo)) {

    //undo grouping : same actions are put into one undo step
    //precondition : new action starts where old stops or mergeUndo flag
    if (g->undoType == newUndoType
      || (g->undoType == KWActionGroup::ugInsChar
        && newUndoType == KWActionGroup::ugInsLine)
      || (g->undoType == KWActionGroup::ugDelChar
        && newUndoType == KWActionGroup::ugDelLine)) {

      m_undoCount++;
      if (g->undoType != newUndoType) m_undoCount = 0xffffff;
      return;
    }
  }
  m_undoCount = 0;

  while ((int) m_undoList.count() > m_currentUndo) m_undoList.removeLast();
  while ((int) m_undoList.count() > m_undoSteps) {
    m_undoList.removeFirst();
    m_currentUndo--;
  }

  g = new KWActionGroup(cursor, newUndoType);
  m_undoList.append(g);

  unmarkFound();
  m_tagEnd = 0;
  m_tagStart = 0xffffff;
}

void KWriteDoc::recordAction(KWAction::Action action, KWCursor &cursor) {
  KWAction *a;

  a = new KWAction(action, cursor);
  doAction(a);
  m_undoList.getLast()->insertAction(a);
}

void KWriteDoc::recordInsert(VConfig &c, const QString &text) {
  recordReplace(c, 0, text);
}

void KWriteDoc::recordReplace(VConfig &c, int len, const QString &text) {
  TextLine *textLine;

  if (c.cursor.x() > 0 && !(c.flags & cfSpaceIndent)) {
    textLine = m_contents.at(c.cursor.y());
    if (textLine->length() == 0) {
      QString s = tabString(c.cursor.x(), tabWidth());
      int len = s.length();
      s += text;
      c.cursor.setX(0);
      recordReplace(c.cursor, len, s);
      c.cursor.setX(len);
      return;
    }
  }
  recordReplace(c.cursor, len, text);
}

void KWriteDoc::recordInsert(KWCursor &cursor, const QString &text) {
  recordReplace(cursor, 0, text);
}

void KWriteDoc::recordDelete(KWCursor &cursor, int len) {
  recordReplace(cursor, len, QString::null);
}

void KWriteDoc::recordReplace(KWCursor &cursor, int len, const QString &text) {
  KWAction *a;
  TextLine *textLine;
  int l;

  if (len == 0 && text.isEmpty()) return;

  //try to append to last replace action
  a = m_undoList.getLast()->action;
  if (a == 0L || a->action != KWAction::replace
    || a->cursor.x() + a->len != cursor.x() || a->cursor.y() != cursor.y()) {

//if (a != 0L) printf("new %d %d\n", a->cursor.x() + a->len, cursor.x());
    a = new KWAction(KWAction::replace, cursor);
    m_undoList.getLast()->insertAction(a);
  }

  //replace
  textLine = m_contents.at(cursor.y());
  l = textLine->length() - cursor.x();
  if (l > len) l = len;
  a->text.insert(a->text.length(), &textLine->getText()[cursor.x()], (l < 0) ? 0 : l);
  textLine->replace(cursor.x(), len, text.unicode(), text.length());
  a->len += text.length();

  tagLine(a->cursor.y());
}

void KWriteDoc::recordEnd(VConfig &c) {
  recordEnd(c.view, c.cursor, c.flags);
}

void KWriteDoc::recordEnd(KWriteView *view, KWCursor &cursor, int flags) {
  KWActionGroup *g;

  // clear selection if option "persistent selections" is off
//  if (!(flags & cfPersistent)) deselectAll();

  g = m_undoList.getLast();
  if (g->action == 0L) {
    // no action has been done: remove empty undo record
    m_undoList.removeLast();
    return;
  }
  // store end cursor position for redo
  g->end = cursor;
  m_currentUndo = m_undoList.count();

  if (m_tagStart <= m_tagEnd) {
    optimizeSelection();
    updateLines(m_tagStart, m_tagEnd, flags, cursor.y());
    setModified(true);
  }

  view->updateCursor(cursor, flags);

  newUndo(); // only emits signals if undo state has changed
}


void KWriteDoc::doActionGroup(KWActionGroup *g, int flags) {
  KWAction *a, *next;

  setPseudoModal(0L);
  if (!(flags & cfPersistent)) deselectAll();
  unmarkFound();
  m_tagEnd = 0;
  m_tagStart = 0xffffff;

  a = g->action;
  g->action = 0L;
  while (a) {
    doAction(a);
    next = a->next;
    g->insertAction(a);
    a = next;
  }
  optimizeSelection();
  if (m_tagStart <= m_tagEnd) updateLines(m_tagStart, m_tagEnd, flags);

  // these functions now only emit signals if state really changes
  setModified(true);
  newUndo();
}

int KWriteDoc::nextUndoType() {
  KWActionGroup *g;

  if (m_currentUndo <= 0) return KWActionGroup::ugNone;
  g = m_undoList.at(m_currentUndo - 1);
  return g->undoType;
}

int KWriteDoc::nextRedoType() {
  KWActionGroup *g;

  if (m_currentUndo >= (int) m_undoList.count()) return KWActionGroup::ugNone;
  g = m_undoList.at(m_currentUndo);
//  if (!g) return KWActionGroup::ugNone;
  return g->undoType;
}

void KWriteDoc::undoTypeList(QValueList<int>& lst) {
  KWActionGroup *g;
  int currUndo = m_currentUndo;

  lst.clear();

  while (true) {
    if (currUndo <= 0)
      return;

    currUndo--;
    g = m_undoList.at(currUndo);
    lst.append(g->undoType);
  }
}

void KWriteDoc::redoTypeList(QValueList<int>& lst) {
  KWActionGroup *g;
  int currUndo = m_currentUndo;

  lst.clear();

  while (true) {
    if (currUndo + 1 > (int) m_undoList.count())
      return;

    g = m_undoList.at(currUndo);
    currUndo++;
    lst.append(g->undoType);
  }
}

void KWriteDoc::undo(VConfig &c, int count) {
  KWActionGroup *g = 0L;
  int num;

  for (num = 0 ; num < count ; num++) {
    if (m_currentUndo <= 0) break;
    m_currentUndo--;
    g = m_undoList.at(m_currentUndo);
    doActionGroup(g, c.flags);
  }
  if (num > 0) { // only update the cursor if doActionGroup was called
    c.view->updateCursor(g->start);
  }
}

void KWriteDoc::redo(VConfig &c, int count) {
  KWActionGroup *g = 0L;
  int num;

  for (num = 0 ; num < count ; num++) {
    if (m_currentUndo+1 > (int) m_undoList.count()) break;
    g = m_undoList.at(m_currentUndo);
    m_currentUndo++;
    doActionGroup(g, c.flags);
  }
  if (num > 0) { // only update the cursor if doActionGroup was called
    c.view->updateCursor(g->end);
  }
}

void KWriteDoc::clearRedo() {
  // disable redos
  // this was added as an assist to the spell checker
  bool deleted = false;

  while ((int) m_undoList.count() > m_currentUndo) {
    deleted = true;
    m_undoList.removeLast();
  }

  if (deleted) newUndo();
}

void KWriteDoc::setUndoSteps(int steps) {
  if (steps < 5) steps = 5;
  m_undoSteps = steps;
}

void KWriteDoc::setPseudoModal(QWidget *w) {
//  QWidget *old = pseudoModal;

  // (glenebob)
  // this is a temporary hack to make the spell checker work a little
  // better - as kspell progresses, this sort of thing should become
  // obsolete or worked around more cleanly
  // this is relied upon *only *by the spell-check code
  if (m_pseudoModal && m_pseudoModal != (QWidget *)1L)
    delete m_pseudoModal;

//  pseudoModal = 0L;
//  if (old || w) recordReset();

 m_pseudoModal = w;
}


void KWriteDoc::newBracketMark(KWCursor &cursor, BracketMark &bm) {
  TextLine *textLine;
  int x, line, count, attr;
  char bracket, opposite, ch;
  Attribute *a;

  bm.eXPos = -1; //mark bracked mark as invalid

  x = cursor.x() -1; // -1 to look at left side of cursor
  if (x < 0) return;
  line = cursor.y(); //current line
  count = 0; //bracket counter for nested brackets
  textLine = m_contents.at(line);
  bracket = textLine->getChar(x);
  attr = textLine->getAttr(x);
  if (bracket == '(' || bracket == '[' || bracket == '{') {
    //get opposite bracket
    opposite = ')';
    if (bracket == '[') opposite = ']';
    if (bracket == '{') opposite = '}';
    //get attribute of bracket(opposite bracket must have the same attribute)
    x++;
    while (line - cursor.y() < 40) {
      //go to next line on end of line
      while (x >= textLine->length()) {
        line++;
        if (line > lastLine()) return;
        textLine = m_contents.at(line);
        x = 0;
      }
      if (textLine->getAttr(x) == attr) {
        //try to find opposite bracked
        ch = textLine->getChar(x);
        if (ch == bracket) count++; //same bracket : increase counter
        if (ch == opposite) {
          count--;
          if (count < 0) goto found;
        }
      }
      x++;
    }
  } else if (bracket == ')' || bracket == ']' || bracket == '}') {
    opposite = '(';
    if (bracket == ']') opposite = '[';
    if (bracket == '}') opposite = '{';
    x--;
    while (cursor.y() - line < 20) {

      while (x < 0) {
        line--;
        if (line < 0) return;
        textLine = m_contents.at(line);
        x = textLine->length() -1;
      }
      if (textLine->getAttr(x) == attr) {
        ch = textLine->getChar(x);
        if (ch == bracket) count++;
        if (ch == opposite) {
          count--;
          if (count < 0) goto found;
        }
      }
      x--;
    }
  }
  return;
found:
  //cursor position of opposite bracket
  bm.cursor.set(x, line);

  //x position(start and end) of related bracket
  bm.sXPos = textWidth(textLine, x);
  a = &m_attribs[attr];
  bm.eXPos = bm.sXPos + a->width(bracket);//a->fm.width(bracket);
}

void KWriteDoc::clipboardChanged() {//slot
  if (m_singleSelectMode) {
    disconnect(QApplication::clipboard(), SIGNAL(dataChanged()),
      this, SLOT(clipboardChanged()));
    deselectAll();
    updateViews();
  }
}

void KWriteDoc::setColors(QColor *colors) {
  m_colors[0] = colors[0];
  m_colors[1] = colors[1];
  m_colors[2] = colors[2];
  m_colors[3] = colors[3];
  m_colors[4] = colors[4];
}

#include "kwrite_doc.moc"
