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
#include <kxmlgui.h>
#include <kaction.h>
#include <kstdaction.h>

#include <kparts/partmanager.h>
#include <kparts/mainwindow.h>

#include <X11/Xlib.h> //used to have XSetTransientForHint()

#define i18nop // a no-operation i18n(), only KWrite-internal and defined in Makefile.am

#include <kwrite/kwrite.h>
#include <kwrite/highlight.h>
#include <kwrite/kwrite_doc.h>
#include <kwrite/kwrite_part.h>
#include <kwrite/kwrite_factory.h>

#include "kwrite_keys.h"
#include "ktextprint.h"
#include "kwdialog.h"
#include "prefdialog.h"
#include "undohistory.h"


#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

struct BufferInfo {
  void *user;
  int   w;
  int   h;
};

QList<BufferInfo> *bufferInfoList = 0L;
QPixmap *buffer = 0;

QPixmap *getBuffer(void *user) {
  BufferInfo *info;

  if (!buffer)
    buffer = new QPixmap;

  info = new BufferInfo;
  info->user = user;
  info->w = 0;
  info->h = 0;
  bufferInfoList->append(info);
  return buffer;
}

void resizeBuffer(void *user, int w, int h) {
  int z;
  BufferInfo *info;
  int maxW, maxH;

  maxW = w;
  maxH = h;
  for (z = 0; z < (int) bufferInfoList->count(); z++) {
    info = bufferInfoList->at(z);
    if (info->user == user) {
      info->w = w;
      info->h = h;
    } else {
      if (info->w > maxW) maxW = info->w;
      if (info->h > maxH) maxH = info->h;
    }
  }
  if (maxW != buffer->width() || maxH != buffer->height()) {
    buffer->resize(maxW, maxH);
  }
}

void releaseBuffer(void *user) {
  int z;
  BufferInfo *info;

  for (z = (int) bufferInfoList->count() -1; z >= 0 ; z--) {
    info = bufferInfoList->at(z);
    if (info->user == user) bufferInfoList->remove(z);
  }
  resizeBuffer(0, 0, 0);
}


// KWBookmark

KWBookmark::KWBookmark(int xPos, int yPos, const KWCursor &cursor, KWrite *kWrite)
  : KWLineAttribute(cursor.y(), kWrite) {

  m_cursorX = cursor.x();
  m_xPos = xPos;
  m_yPos = yPos;
}

void KWBookmark::paint(QPainter &paint, int y, int height) {
//  paint.drawLine(0, y, 8, y + height);
  paint.setPen(Qt::green);
  paint.drawEllipse(1, y + 1, 7, height - 1);
}


// KWriteWidget

KWriteWidget::KWriteWidget(QWidget *parent) 
  : QWidget(parent, "KWriteWidget") {

  setFocusPolicy(ClickFocus);
}

void KWriteWidget::paintEvent(QPaintEvent *event) {
  int x, y;

  QRect updateR = event->rect();                    // update rectangle
//  debug("Update rect = (%i, %i, %i, %i)",
//    updateR.x(), updateR.y(), updateR.width(), updateR.height());

  int ux1 = updateR.x();
  int uy1 = updateR.y();
  int ux2 = ux1 + updateR.width();
  int uy2 = uy1 + updateR.height();

  QPainter paint;
  paint.begin(this);

  QColorGroup g = colorGroup();
  x = width();
  y = height();

  paint.setPen(g.dark());
  if (uy1 <= 0) paint.drawLine(0, 0, x-2, 0);
  if (ux1 <= 0) paint.drawLine(0, 1, 0, y-2);

  paint.setPen(black);
  if (uy1 <= 1) paint.drawLine(1, 1, x-3, 1);
  if (ux1 <= 1) paint.drawLine(1, 2, 1, y-3);

  paint.setPen(g.midlight());
  if (uy2 >= y-1) paint.drawLine(1, y-2, x-3, y-2);
  if (ux2 >= x-1) paint.drawLine(x-2, 1, x-2, y-2);

  paint.setPen(g.light());
  if (uy2 >= y) paint.drawLine(0, y-1, x-2, y-1);
  if (ux2 >= x) paint.drawLine(x-1, 0, x-1, y-1);

  x -= 2 + 16;
  y -= 2 + 16;
  if (ux2 > x && uy2 > y) {
    paint.fillRect(x, y, 16, 16, g.background());
  }
  paint.end();
}

void KWriteWidget::resizeEvent(QResizeEvent *) {
//  debug("Resize %d, %d", event->size().width(), event->size().height());
//  m_view->resize(width(), height());
  m_view->tagAll();
  m_view->updateView(0 /*ufNoScroll*/);
}


// KWrite

KWrite::KWrite(QWidget *parentWidget, QObject *parent, int flags, KWriteDoc *doc)
  : KParts::ReadWritePart(parent, "KWritePart") {
  
//KWrite::KWrite(KWriteDoc *doc, QWidget *parent, const QString &name, bool HandleOwnDND)
//  : QWidget(parent, name) {

  if (!bufferInfoList)
    bufferInfoList = new QList<BufferInfo>;

  setInstance( KWriteFactory::instance() );

  if (flags & kBrowser) {
    setXMLFile("kwrite_browser.rc");
    (void) new KWriteBrowserExtension(this);
  } else setXMLFile( "kwrite.rc" );

  // if no external doc given, create a new one
  if (doc == 0L) {
    doc = new KWriteDoc(HlManager::self());
  }
//  doc->incRefCount();
  m_doc = doc;
  m_widget = new KWriteWidget(parentWidget);
  m_view = new KWriteView(m_doc, m_widget, this, flags & kHandleOwnDND);
  m_widget->m_view = m_view;

  // some defaults
  m_configFlags = cfAutoIndent | cfSpaceIndent | cfBackspaceIndents
    | cfTabIndents | cfKeepIndentProfile
    | cfReplaceTabs | cfSpaceIndent | cfRemoveSpaces
    | cfDelOnInput | cfMouseAutoCopy
    | cfGroupUndo | cfShowTabs | cfSmartHome;
  m_wrapAt = 80;
  m_searchFlags = 0;
  m_replacePrompt = 0L;
  popup = 0L;
//  bookmarks.setAutoDelete(true);
  for (int z = 0; z < nBookmarks; z++) bookmark[z] = 0L;
  
  // KWrite commands
  m_dispatcher = new KWCommandDispatcher(m_view);
  KWCommandGroup *g;
  
  // cursor commands
  m_persistent = false;
  g = m_dispatcher->addGroup(i18nop("Cursor Movement"));
  g->setSelectModifiers(SHIFT, kSelectFlag, ALT, kMultiSelectFlag);
  g->addCommand(cmLeft,            i18nop("Left"), Key_Left, CTRL+Key_B);
  g->addCommand(cmRight,           i18nop("Right"), Key_Right, CTRL+Key_F);
  g->addCommand(cmWordLeft,        i18nop("Word Left"), CTRL+Key_Left);
  g->addCommand(cmWordRight,       i18nop("Word Right"), CTRL+Key_Right);
  g->addCommand(cmHome,            i18nop("Home"), Key_Home, CTRL+Key_A, Key_F27);
  g->addCommand(cmEnd,             i18nop("End"), Key_End, CTRL+Key_E, Key_F33);
  g->addCommand(cmUp,              i18nop("Up"), Key_Up, CTRL+Key_P);
  g->addCommand(cmDown,            i18nop("Down"), Key_Down, CTRL+Key_N);
  g->addCommand(cmScrollUp,        i18nop("Scroll Up"), CTRL+Key_Up);
  g->addCommand(cmScrollDown,      i18nop("Scroll Down"), CTRL+Key_Down);
  g->addCommand(cmTopOfView,       i18nop("Top Of View"), CTRL+Key_PageUp);
  g->addCommand(cmBottomOfView,    i18nop("Bottom Of View"), CTRL+Key_PageDown);
  g->addCommand(cmPageUp,          i18nop("Page Up"), Key_PageUp, Key_F29);
  g->addCommand(cmPageDown,        i18nop("Page Down"), Key_PageDown, Key_F35);
  g->addCommand(cmTop,             i18nop("Top"), CTRL+Key_Home);
  g->addCommand(cmBottom,          i18nop("Bottom"), CTRL+Key_End);
  g->addCommand(cmSelectLeft,      i18nop("Left + Select") , SHIFT+Key_F30);//, SHIFT+Key_4);
  g->addCommand(cmSelectRight,     i18nop("Right + Select") , SHIFT+Key_F32);//, SHIFT+Key_6);
  g->addCommand(cmSelectUp,        i18nop("Up + Select") , SHIFT+Key_F28);//, SHIFT+Key_8);
  g->addCommand(cmSelectDown,      i18nop("Down + Select") , SHIFT+Key_F34);//, SHIFT+Key_2);
  connect(g, SIGNAL(activated(int)), this, SLOT(doCursorCommand(int)));

  // edit commands (only handles extra keys)
  g = m_dispatcher->addGroup(i18nop("Edit Commands"));
  g->addCommand(cmReturn,          i18nop("Return"), Key_Return, Key_Enter);
  g->addCommand(cmBackspace,       i18nop("Backspace"), Key_Backspace, CTRL+Key_H);
  g->addCommand(cmBackspaceWord,   i18nop("Backspace Word"), CTRL+Key_Backspace);
  g->addCommand(cmDeleteChar,      i18nop("Delete Char"), Key_Delete, CTRL+Key_D);
  g->addCommand(cmDeleteWord,      i18nop("Delete Word"), CTRL+Key_Delete);
  g->addCommand(cmKillLine,        i18nop("Kill Line"), CTRL+Key_K);
  g->addCommand(cmUndo,            i18nop("Undo"), /*CTRL+Key_Z, */Key_F14);
  g->addCommand(cmRedo,            i18nop("Redo"), /*CTRL+Key_Y, */Key_F12);
  g->addCommand(cmCut,             i18nop("Cut"), /*CTRL+Key_X, */SHIFT+Key_Delete, Key_F20);
  g->addCommand(cmCopy,            i18nop("Copy"), /*CTRL+Key_C, */CTRL+Key_Insert, Key_F16);
  g->addCommand(cmPaste,           i18nop("Paste"), /*CTRL+Key_V, */SHIFT+Key_Insert, Key_F18);
  g->addCommand(cmDelete,          i18nop("Delete"), SHIFT+CTRL+Key_Delete);
  g->addCommand(cmIndent,          i18nop("Indent")/*, CTRL+Key_I*/);
  g->addCommand(cmUnindent,        i18nop("Unindent")/*, CTRL+Key_U*/);
  g->addCommand(cmCleanIndent,     i18nop("Clean Indent"));
  g->addCommand(cmSelectAll,       i18nop("Select All"));
  g->addCommand(cmDeselectAll,     i18nop("Deselect All"));
  g->addCommand(cmInvertSelection, i18nop("Invert Selection"));
  connect(g, SIGNAL(activated(int)), this, SLOT(doEditCommand(int)));

  // bookmark commands
  g = m_dispatcher->addGroup(i18nop("Bookmark Commands"));
  g->addCommand(cmSetBookmark,       i18nop("Set Bookmark")/*, ALT+Key_S*/);
  g->addCommand(cmAddBookmark,       i18nop("Add Bookmark")/*, ALT+Key_A*/);
  g->addCommand(cmClearBookmarks,    i18nop("Clear Bookmarks")/*, ALT+Key_C*/);
  g->addCommand(cmSetBookmarks +0,   i18nop("Set Bookmark 1"), CTRL+Key_1);
  g->addCommand(cmSetBookmarks +1,   i18nop("Set Bookmark 2"), CTRL+Key_2);
  g->addCommand(cmSetBookmarks +2,   i18nop("Set Bookmark 3"), CTRL+Key_3);
  g->addCommand(cmSetBookmarks +3,   i18nop("Set Bookmark 4"), CTRL+Key_4);
  g->addCommand(cmSetBookmarks +4,   i18nop("Set Bookmark 5"), CTRL+Key_5);
  g->addCommand(cmSetBookmarks +5,   i18nop("Set Bookmark 6"), CTRL+Key_6);
  g->addCommand(cmSetBookmarks +6,   i18nop("Set Bookmark 7"), CTRL+Key_7);
  g->addCommand(cmSetBookmarks +7,   i18nop("Set Bookmark 8"), CTRL+Key_8);
  g->addCommand(cmSetBookmarks +8,   i18nop("Set Bookmark 9"), CTRL+Key_9);
  g->addCommand(cmSetBookmarks +9,   i18nop("Set Bookmark 10"), CTRL+Key_0);
  g->addCommand(cmGotoBookmarks +0,  i18nop("Goto Bookmark 1"), ALT+Key_1);
  g->addCommand(cmGotoBookmarks +1,  i18nop("Goto Bookmark 2"), ALT+Key_2);
  g->addCommand(cmGotoBookmarks +2,  i18nop("Goto Bookmark 3"), ALT+Key_3);
  g->addCommand(cmGotoBookmarks +3,  i18nop("Goto Bookmark 4"), ALT+Key_4);
  g->addCommand(cmGotoBookmarks +4,  i18nop("Goto Bookmark 5"), ALT+Key_5);
  g->addCommand(cmGotoBookmarks +5,  i18nop("Goto Bookmark 6"), ALT+Key_6);
  g->addCommand(cmGotoBookmarks +6,  i18nop("Goto Bookmark 7"), ALT+Key_7);
  g->addCommand(cmGotoBookmarks +7,  i18nop("Goto Bookmark 8"), ALT+Key_8);
  g->addCommand(cmGotoBookmarks +8,  i18nop("Goto Bookmark 9"), ALT+Key_9);
  g->addCommand(cmGotoBookmarks +9,  i18nop("Goto Bookmark 10"), ALT+Key_0);
  connect(g, SIGNAL(activated(int)), this, SLOT(doBookmarkCommand(int)));


  //KSpell initial values
  kspell.kspell = 0;
  kspell.ksc = new KSpellConfig; //default KSpellConfig to start
  kspell.kspellon = FALSE;

//  m_view->setFocus();
  m_widget->resize(parentWidget->width(), parentWidget->height());
  

  // KPart initialisation
  setWidget(m_widget);

  m_cut = KStdAction::cut( this, SLOT( cut() ), actionCollection(), "edit_cut" );
  m_copy = KStdAction::copy( this, SLOT( copy() ), actionCollection(), "edit_copy" );
  m_paste = KStdAction::paste( this, SLOT( paste() ), actionCollection(), "edit_paste" );
  m_undo = KStdAction::undo( this, SLOT( undo() ), actionCollection(), "edit_undo" );
  m_redo = KStdAction::redo( this, SLOT( redo() ), actionCollection(), "edit_redo" );
  (void)new KAction( i18n( "Undo History" ), 0, this, SLOT( undoHistory() ), actionCollection(), "undo_history" );
  m_indent = new KAction( i18n( "&Indent"), 0, this, SLOT( indent() ), actionCollection(), "indent" );
  m_unindent = new KAction( i18n( "Unindent" ), 0, this, SLOT( unindent() ), actionCollection(), "unindent" );
  m_cleanIndent = new KAction( i18n( "Clean indent" ), 0, this, SLOT( cleanIndent() ), actionCollection(), "clean_indent" );
  KStdAction::selectAll( this, SLOT( selectAll() ), actionCollection(), "select_all" );
  (void)new KAction( i18n( "Unselect All" ), 0, this, SLOT( unselectAll() ), actionCollection(), "unselect_all" );
  (void)new KAction( i18n( "Invert Selection" ), 0, this, SLOT( invertSelection() ), actionCollection(), "invert_select" );
  m_spell = KStdAction::spelling( this, SLOT( spellCheck() ), actionCollection(), "spell_check" );

  // get list of supported programming languages
  QStringList langList;
  HlManager *hlManager = m_doc->hlManager();
  for (int z = 0; z < hlManager->highlights(); z++) {
    langList.append(hlManager->hlName(z));
  }

  m_langAction = new KSelectAction( i18n( "Set highlighting" ), 0, 
    actionCollection(), "highlight_select" );
  m_langAction->setItems( langList );
  m_langAction->setCurrentItem( 1 );
  connect( m_langAction, SIGNAL( activated( int ) ), this, SLOT( setHighlight( int ) ) );

  KStdAction::find( this, SLOT( find() ), actionCollection(), "find" );
  m_replace = KStdAction::replace( this, SLOT( replace() ), actionCollection(), "replace" );
  KStdAction::findNext( this, SLOT( findAgain() ), actionCollection(), "find_again" );
  KStdAction::gotoLine( this, SLOT( gotoLine() ), actionCollection(), "goto_line" );

  (void) new KAction( i18n( "Set bookmark" ), KStdAccel::addBookmark(), this, SLOT( setBookmark() ), 
    actionCollection(), "set_bookmark" );
  KStdAction::addBookmark( this, SLOT( addBookmark() ), actionCollection(), "add_bookmark" );
  (void) new KAction( i18n( "Clear bookmarks" ), 0, this, SLOT( clearBookmarks() ), 
    actionCollection(), "clear_bookmarks" );

  // TODO: add KListAction here for bookmark list.

//  connect( m_kwrite, SIGNAL( fileChanged() ), this, SLOT( newCaption() ) );
  //connect( m_kwrite->view(), SIGNAL( dropEventPass( QDropEvent* ) ), this, SLOT( slotDropEvent( QDropEvent* ) ) );

  m_cut->setEnabled( false );
  m_copy->setEnabled( false );
  m_paste->setEnabled( false );
  m_undo->setEnabled( false );
  m_redo->setEnabled( false );

  m_langAction->setCurrentItem( 0 );
}

KWrite::~KWrite() {
//  m_doc->decRefCount(); // deletes itself if RefCount becomes zero
  delete m_dispatcher;
  delete popup; // right mouse button popup
  clearBookmarks(); // deletes bookmark objects
  
  if (kspell.kspell) {
    kspell.kspell->setAutoDelete(true);
    kspell.kspell->cleanUp(); // need a way to wait for this to complete
  }
}

/*
void KWrite::addFindCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctFindCommands, i18nop("Find Commands"));
  cmdMngr.addCommand(cmFind,            i18nop("&Find...") , CTRL+Key_F, Key_F19);
  cmdMngr.addCommand(cmReplace,         i18nop("&Replace...") , CTRL+Key_R);
  cmdMngr.addCommand(cmFindAgain,       i18nop("Find &Again") , Key_F3);
  cmdMngr.addCommand(cmGotoLine,        i18nop("&Goto Line...") , CTRL+Key_G);
}

void KWrite::addStateCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctStateCommands, i18nop("State Commands"));
  cmdMngr.addCommand(cmToggleInsert,   i18nop("Insert Mode"), Key_Insert);
  cmdMngr.addCommand(cmToggleVertical, i18nop("&Vertical Selections"), Key_F5);
}
*/

int KWrite::currentLine() {
  return m_view->cursor.y();
}

int KWrite::currentColumn() {
  return m_doc->currentColumn(m_view->cursor);
}

int KWrite::currentCharNum() {
  return m_view->cursor.x();
}

void KWrite::setCursorPosition(int line, int col) {
  KWCursor cursor;

  cursor.set(col, line);
  m_view->updateCursor(cursor);
  m_view->center();
//  m_doc->unmarkFound();
//  m_view->updateView(ufPos, 0, line*m_doc->fontHeight() - height()/2);
//  m_doc->updateViews(m_view); //uptade all other views except this one
  m_doc->updateViews();
}

int KWrite::config() {
  int flags;

  flags = m_configFlags;
  if (m_doc->isSingleSelectMode()) flags |= cfSingleSelectMode;
  return flags;
}

void KWrite::setConfig(int flags) {
  bool updateView;

  // cfSingleSelectMode is a doc-property
  m_doc->setSingleSelectMode(flags & cfSingleSelectMode);
  flags &= ~cfSingleSelectMode;

  if (flags != m_configFlags) {
    // update the view if visibility of tabs has changed
    updateView = (flags ^ m_configFlags) & cfShowTabs;
    m_configFlags = flags;
    emit newConfig();
    if (updateView) m_view->update();
  }
}

void KWrite::setTabWidth(int w) {
  m_doc->setTabWidth(w);
  m_doc->updateViews();
}

int KWrite::tabWidth() {
  return m_doc->tabWidth();
}

void KWrite::setUndoSteps(int s) {
  m_doc->setUndoSteps(s);
}

int KWrite::undoSteps() {
  return m_doc->undoSteps();
}

void KWrite::setColors(QColor *colors) {
  m_doc->setColors(colors);
}

void KWrite::getColors(QColor *colors) {
  colors = m_doc->colors();
}

/*
bool KWrite::isOverwriteMode() {
  return(m_configFlags & cfOvr);
}
*/

void KWrite::setReadWrite(bool readWrite) {
  m_doc->setReadWrite(readWrite);
}

bool KWrite::isReadWrite() {
  return m_doc->isReadWrite();
}

void KWrite::setModified(bool modified) {
  m_doc->setModified(modified);
}

bool KWrite::isModified() {
  return m_doc->isModified();
}

bool KWrite::isLastView() {
  return m_doc->isLastView(1);
}

int KWrite::undoState() {
  return m_doc->undoState();
}

int KWrite::nextUndoType() {
  return m_doc->nextUndoType();
}

int KWrite::nextRedoType() {
  return m_doc->nextRedoType();
}

void KWrite::undoTypeList(QValueList<int>& lst) {
  m_doc->undoTypeList(lst);
}

void KWrite::redoTypeList(QValueList<int>& lst) {
  m_doc->redoTypeList(lst);
}

QString KWrite::undoTypeName(int type) {
  return KWActionGroup::typeName(type);
}

void KWrite::copySettings(KWrite *w) {
  m_wrapAt = w->m_wrapAt;
  m_configFlags = w->m_configFlags;
  m_searchFlags = w->m_searchFlags;
}

void KWrite::emitNewStatus() {
  bool rw = isReadWrite();
  bool mt = hasMarkedText();

  m_cut->setEnabled(rw && mt);
  m_copy->setEnabled(rw && mt);
  m_paste->setEnabled(rw);
  m_replace->setEnabled(rw);
  m_indent->setEnabled(rw);
  m_unindent->setEnabled(rw);
  m_cleanIndent->setEnabled(rw);
  m_spell->setEnabled(rw);

  emit newStatus();
}

void KWrite::emitNewUndo() {
  int state = undoState();

  m_undo->setEnabled(state & kUndoPossible);
  m_redo->setEnabled(state & kRedoPossible);

  emit newUndo();
}

void KWrite::optionsDialog() {
  PreferencesDlg::doSettings(this);
}

/*
void KWrite::colDlg() {
  ColorDialog *dlg = new ColorDialog(m_widget, m_doc->colors());

  if (dlg->exec() == QDialog::Accepted) {
    dlg->getColors(m_doc->colors());
    m_doc->tagAll();
    m_doc->updateViews();
  }

  delete dlg;
}
*/

void KWrite::doStateCommand(int cmdNum) {
  switch (cmdNum) {
    case cmToggleInsert:
      toggleInsert();
      return;
    case cmToggleVertical:
      toggleVertical();
      return;
  }
}


void KWrite::toggleInsert() {
  setConfig(m_configFlags ^ cfOvr);
}

void KWrite::toggleVertical() {
  setConfig(m_configFlags ^ cfVerticalSelect);
  emit statusMsg(m_configFlags & cfVerticalSelect ? i18n("Vertical Selections On") : i18n("Vertical Selections Off"));
}

int KWrite::numLines() {
  return m_doc->numLines();
}

QString KWrite::text() {
  return m_doc->text();
}

QString KWrite::currentTextLine() {
  TextLine *textLine;
  textLine = m_doc->textLine(m_view->cursor.y());
  return QString(textLine->getText(), textLine->length());
}

QString KWrite::textLine(int num) {
  TextLine *textLine;
  textLine = m_doc->textLine(num);
  return QString(textLine->getText(), textLine->length());
}

QString KWrite::currentWord() {
  return m_doc->getWord(m_view->cursor);
}

QString KWrite::word(int x, int y) {
  KWCursor cursor;

  cursor.setY((m_view->yPos + y) / m_doc->fontHeight());
  if (cursor.y() < 0 || cursor.y() > m_doc->lastLine())
    return QString();
  cursor.setX(m_doc->textPos(m_doc->textLine(cursor.y()), m_view->xPos - 2 + x));

  return m_doc->getWord(cursor);
}

void KWrite::setText(const QString &s) {
  m_doc->setText(s);
  m_doc->updateViews();
}

void KWrite::insertText(const QString &s) {
  VConfig c;
  m_view->getVConfig(c);
  m_doc->insert(c, s);
  m_doc->updateViews();
}

bool KWrite::hasMarkedText() {
  return m_doc->hasMarkedText();
}

QString KWrite::markedText() {
  return m_doc->markedText(m_configFlags);
}

void KWrite::loadFile(QIODevice &dev, bool insert) {
  VConfig c;

  if (!insert) {
    m_doc->loadFile(dev);
  } else {
    m_view->getVConfig(c);
    if (c.flags & cfDelOnInput) m_doc->delMarkedText(c);
    m_doc->insertFile(c, dev);
    m_doc->updateViews();
  }
  m_doc->updateViews();
}

void KWrite::writeFile(QIODevice &dev) {
  m_doc->writeFile(dev);
  m_doc->updateViews();
}

bool KWrite::loadFile(const QString &name, int flags) {
  QFileInfo info(name);

  if (!info.exists()) {
    if (flags & lfNewFile)
      return true;

    KMessageBox::sorry(m_widget, i18n("The specified File does not exist"));
    return false;
  }

  if (info.isDir()) {
    KMessageBox::sorry(m_widget, i18n("You have specified a directory"));
    return false;
  }

  if (!info.isReadable()) {
    KMessageBox::sorry(m_widget, i18n("You do not have read permission to this file"));
    return false;
  }

  QFile f(name);
  if (f.open(IO_ReadOnly)) {
    loadFile(f, flags & lfInsert);
    f.close();
    return true;
  }

  KMessageBox::sorry(m_widget, i18n("An Error occured while trying to open this Document"));
  return false;
}

bool KWrite::writeFile(const QString &name) {

  QFileInfo info(name);
  if (info.exists() && !info.isWritable()) {
    KMessageBox::sorry(m_widget, i18n("You do not have write permission to this file"));
    return false;
  }

  QFile f(name);
  if (f.open(IO_WriteOnly | IO_Truncate)) {
    writeFile(f);
    f.close();
    return true;//m_doc->setFileName(name);
  }
  KMessageBox::sorry(m_widget, i18n("An Error occured while trying to open this Document"));
  return false;
}

void KWrite::loadURL(const KURL &url, int flags) {
  QString tempFile;

  KIO::NetAccess::download(url, tempFile);

  QString name(url.path());

  if (loadFile(tempFile, flags)) {
    name = url.url();

    if (flags & lfInsert) {
      name.prepend(": ");
      name.prepend(i18n("Inserted"));
    } else {
      if (flags & lfNoAutoHl)
        m_doc->updateLines();
/* else
        m_doc->setFileName(name);
*/

      name.prepend(": ");
      name.prepend(i18n("Read"));
    }
  }

  KIO::NetAccess::removeTempFile(tempFile);
}

void KWrite::writeURL(const KURL &url, int flags) {
  if (url.isLocalFile()) {
    // usual local file
    emit statusMsg(i18n("Saving..."));

    QString name(url.path());

    if (writeFile(name)) {
      name = url.path();
      if (!(flags & lfNoAutoHl)) m_doc->setFileName(name);
      name.prepend(": ");
      name.prepend(i18n("Wrote"));
      emit statusMsg(name);
      setModified(false);
    }
  } else {
    // url
    emit statusMsg(i18n("Saving..."));
/*
#warning Saving disabled because of KIO problems...

    KIO::Job*iojob = new KIO::Job;
    iojob->setGUImode(KIO::Job::NONE);
    QString tmpFile;
    tmpFile = QString(_PATH_TMP"/kwrite%1").arg(time(0L));

    m_sNet.insert(iojob->id(), new QString(u.url()));
    m_sLocal.insert(iojob->id(), new QString(tmpFile));
    m_flags.insert(iojob->id(), new int(flags));

    connect(iojob, SIGNAL(sigFinished(int)), this, SLOT(slotPUTFinished(int)));
    connect(iojob, SIGNAL(sigError(int, const char *)), this, SLOT(slotIOJobError(int, const char *)));
    iojob->copy(tmpFile, url);

    if (!writeFile(tmpFile)) return;
*/
  }
}

void KWrite::slotGETFinished(int id) {
  QString *tmpFile = m_sLocal.find(id);
  QString *netFile = m_sNet.find(id);
  int flags = * m_flags.find(id);
//  QString string;
//  string = i18n("Finished '%1'").arg(tmpFile);
//  setGeneralStatusField(string);

  if (loadFile(*tmpFile, flags)) {
    QString msg = *netFile;

    if (flags & lfInsert) {
      msg.prepend(": ");
      msg.prepend(i18n("Inserted"));
    } else {
      if (!(flags & lfNoAutoHl)) {
        m_doc->setFileName(*netFile);
      } else {
        m_doc->updateLines();
        m_doc->updateViews();
      }

      msg.prepend(": ");
      msg.prepend(i18n("Read"));
    }

    emit statusMsg(msg);
  }

  //clean up
  unlink(tmpFile->data());
  m_sNet.remove(id);
  m_sLocal.remove(id);
  m_flags.remove(id);
}

void KWrite::slotPUTFinished(int id) {
  QString *tmpFile = m_sLocal.find(id);
  QString *netFile = m_sNet.find(id);
  int flags = * m_flags.find(id);
//  QString string;
//  string = i18n("Finished '%1'").arg(tmpFile);
//  setGeneralStatusField(string);

  if (!(flags & lfNoAutoHl)) m_doc->setFileName(*netFile);
  QString msg = *netFile;
  msg.prepend(": ");
  msg.prepend(i18n("Wrote"));
  emit statusMsg(msg);
  setModified(false);

  //clean up
  unlink(tmpFile->data());
  m_sNet.remove(id);
  m_sLocal.remove(id);
  m_flags.remove(id);
}

void KWrite::slotIOJobError(int e, const char *s) {
  printf("error %d = %s\n", e, s);
}

// TODO : why not ask the document directly ?
bool KWrite::hasFileName() {
  return m_doc->hasFileName();
}

// TODO : why not ask the document directly ?
const QString KWrite::fileName() {
  return m_doc->fileName();
}

void KWrite::setFileName(const QString &s) {
  m_doc->setFileName(s);
}

bool KWrite::canDiscard() {
  if (isModified()) {
    int query = KMessageBox::warningYesNoCancel(m_widget,
                i18n("The current Document has been modified.\nWould you like to save it?"));

    switch (query) {
      case KMessageBox::Yes: //yes
        if (save() == CANCEL)
	  return false;

	if (isModified()) {
	  query = KMessageBox::warningYesNo(m_widget,
	          i18n("Could not save the document.\nDiscard it and continue?"));

	  if (query == 1)
	    return false;
        }
        break;

      case KMessageBox::Cancel:
        return false;
    }
  }
  return true;
}

void KWrite::newDoc() {
  if (canDiscard())
    clear();
}

/*
void KWrite::open() {
  KURL url;

  if (!canDiscard())
    return;
//  if (m_doc->hasFileName()) s = QFileInfo(m_doc->fileName()).dirPath();
//    else s = QDir::currentDirPath();

  url = KFileDialog::getOpenURL(m_doc->fileName(), "*", this);

  if (url.isEmpty())
    return;

//  kapp->processEvents();
  loadURL(url);
}

void KWrite::insertFile() {
  if (isReadOnly())
    return;

  KURL url;

  url = KFileDialog::getOpenURL(m_doc->fileName(), "*", this);

  if (url.isEmpty())
    return;

//  kapp->processEvents();
  loadURL(url, lfInsert);
}

KWrite::fileResult KWrite::save() {
  if (isModified()) {
    if (m_doc->hasFileName() && ! isReadOnly()) {
      writeURL(m_doc->fileName(), lfNoAutoHl);
    } else return saveAs();
  } else emit statusMsg(i18n("No changes need to be saved"));
  return OK;
}

KWrite::fileResult KWrite::saveAs() {
  KURL url;
  int query;

  do {
    query = 0;
    url = KFileDialog::getSaveURL(m_doc->fileName(), "*", this);

    if (url.isEmpty())
      return CANCEL;

    if (url.isLocalFile()) {
      QFileInfo info;
      info.setFile(url.path());

      if (info.exists())
        query = KMessageBox::warningYesNo(m_widget,
	        i18n("A Document with this Name already exists.\nDo you want to overwrite it?"));
    }
  }
  while (query == 1);

//  kapp->processEvents();
  writeURL(url);
  return OK;
}
*/

bool KWrite::openFile() {
  loadFile(m_file);
  m_doc->setFileName(m_file);
  return true;
}

bool KWrite::saveFile() {
  return writeFile(m_file);
}


void KWrite::doCursorCommand(int cmdNum) {
  VConfig c;
  m_view->getVConfig(c);

  if (cmdNum & kSelectFlag) {
    c.flags |= cfMark;
    m_persistent = false;
  }    
  if (cmdNum & kMultiSelectFlag) {
    c.flags |= cfMark | cfKeepSelection;
    m_persistent = true;
  }
  if (m_persistent) c.flags |= cfPersistent;

  cmdNum &= ~(kSelectFlag | kMultiSelectFlag);
  m_view->doCursorCommand(c, cmdNum);
  m_doc->updateViews();
}

void KWrite::doEditCommand(int cmdNum) {
  VConfig c;
  m_view->getVConfig(c);
  m_view->doEditCommand(c, cmdNum);
  m_doc->updateViews();
}

void KWrite::doBookmarkCommand(int cmdNum) {
  if (cmdNum == cmSetBookmark)
    setBookmark(); 
  else if (cmdNum == cmAddBookmark)
    addBookmark();
  else if (cmdNum == cmClearBookmarks)
    clearBookmarks();
  else if (cmdNum >= cmSetBookmarks && cmdNum < cmSetBookmarks + 10)
    setBookmark(cmdNum - cmSetBookmarks); 
  else if (cmdNum >= cmGotoBookmarks && cmdNum < cmGotoBookmarks + 10)
    gotoBookmark(cmdNum - cmGotoBookmarks);
}

/*
void KWrite::configureKWriteKeys() {
  KWKeyData data;
  
  m_dispatcher->getData(data);
  
  KDialogBase *dlg = new KDialogBase(m_widget, "tabdialog", true, 
    i18n("KWrite Keys"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok);
  
  KWKeyConfigTab *keys = new KWKeyConfigTab(dlg, data);
  dlg->setMainWidget(keys);
  dlg->resize(450, 320);
  
  if (dlg->exec()) {
    m_dispatcher->setData(data);  
  }
  
  delete dlg;
}
*/

void KWrite::getKeyData(KWKeyData &data) {
  m_dispatcher->getData(data);
}

void KWrite::setKeyData(const KWKeyData &data) {
  m_dispatcher->setData(data);
}

void KWrite::clear() {
  if (!isReadWrite())
    return;

  m_doc->clear();
  m_doc->clearFileName();
  m_doc->updateViews();
}

/*
void KWrite::cut() {
  if (isReadOnly())
    return;

  VConfig c;
  m_view->getVConfig(c);
  m_doc->cut(m_view, c);
  m_doc->updateViews();
}

void KWrite::copy() {
  m_doc->copy(m_configFlags);
}

void KWrite::paste() {
  if (isReadOnly())
    return;

  VConfig c;
  m_view->getVConfig(c);
  m_doc->paste(m_view, c);
  m_doc->updateViews();
}

void KWrite::undo() {
  undoMultiple(1);
}

void KWrite::redo() {
  redoMultiple(1);
}
*/

void KWrite::undoMultiple(int count) {
  if (!isReadWrite())
    return;

  VConfig c;
  m_view->getVConfig(c);
  m_doc->undo(c, count);
  m_doc->updateViews();
}

void KWrite::redoMultiple(int count) {
  if (!isReadWrite())
    return;

  VConfig c;
  m_view->getVConfig(c);
  m_doc->redo(c, count);
  m_doc->updateViews();
}

void KWrite::undoHistory() {
  UndoHistory *undoH = new UndoHistory(this, m_widget, "UndoHistory", true);

  undoH->setCaption(i18n("Undo/Redo History"));
  connect(this, SIGNAL(newUndo()), undoH, SLOT(newUndo()));
  connect(undoH, SIGNAL(undo(int)), this, SLOT(undoMultiple(int)));
  connect(undoH, SIGNAL(redo(int)), this, SLOT(redoMultiple(int)));
  undoH->exec();

  delete undoH;
}
void KWrite::indent() {
/*
  if (isReadOnly())
    return;

  VConfig c;
  m_view->getVConfig(c);
  m_doc->indent(m_view, c);
  m_doc->updateViews();
*/
}

void KWrite::unindent() {
/*
  if (isReadOnly())
    return;

  VConfig c;
  m_view->getVConfig(c);
  m_doc->unIndent(m_view, c);
  m_doc->updateViews();
*/
}

void KWrite::cleanIndent() {
/*
  if (isReadOnly())
    return;

  VConfig c;
  m_view->getVConfig(c);
  m_doc->cleanIndent(m_view, c);
  m_doc->updateViews();
*/
}

/*
void KWrite::selectAll() {
  m_doc->selectAll();
  m_doc->updateViews();
}

void KWrite::unselectAll() {
  m_doc->deselectAll();
  m_doc->updateViews();
}
*/

void KWrite::invertSelection() {
  m_doc->invertSelection();
  m_doc->updateViews();
}

static void addToStrList(QStringList &list, const QString &str) {
  if (list.count() > 0) {
    if (list.first() == str) return;
    QStringList::Iterator it;
    it = list.find(str);
    if (*it != 0L) list.remove(it);
    if (list.count() >= 16) list.remove(list.fromLast());
  }
  list.prepend(str);
}

void KWrite::find() {
  if (!m_doc->hasMarkedText())
    m_searchFlags &= ~sfSelected;

  SearchDialog *searchDialog = new SearchDialog(m_widget, m_searchForList, m_replaceWithList, m_searchFlags & ~sfReplace);

  m_view->focusOutEvent(0L);// QT bug ?

  if (searchDialog->exec() == QDialog::Accepted) {
    addToStrList(m_searchForList, searchDialog->getSearchFor());
    m_searchFlags = searchDialog->getFlags() | (m_searchFlags & sfPrompt);
    initSearch(s, m_searchFlags);
    searchAgain(s);
  }

  delete searchDialog;
}

void KWrite::replace() {
  if (!isReadWrite())
    return;

  if (!m_doc->hasMarkedText())
    m_searchFlags &= ~sfSelected;

  SearchDialog *searchDialog = new SearchDialog(m_widget, m_searchForList, m_replaceWithList, m_searchFlags | sfReplace);

  m_view->focusOutEvent(0L);// QT bug ?

  if (searchDialog->exec() == QDialog::Accepted) {
    addToStrList(m_searchForList, searchDialog->getSearchFor());
    addToStrList(m_replaceWithList, searchDialog->getReplaceWith());
    m_searchFlags = searchDialog->getFlags();
    initSearch(s, m_searchFlags);
    replaceAgain();
  }

  delete searchDialog;
}

void KWrite::findAgain() {
  initSearch(s, m_searchFlags | sfFromCursor | sfPrompt | sfAgain);

  if (s.flags & sfReplace)
    replaceAgain(); 
  else
    searchAgain(s);
}

void KWrite::gotoLine() {
  GotoLineDialog *dlg = new GotoLineDialog(m_widget, m_view->cursor.y() + 1, m_doc->numLines());

  if (dlg->exec() == QDialog::Accepted) {
    KWCursor cursor(0, dlg->getLine() - 1);

    m_view->updateCursor(cursor);
    m_view->center();
    m_view->updateView(ufUpdateOnScroll);
    m_doc->updateViews(m_view);
  }

  delete dlg;
}

void KWrite::initSearch(SConfig &s, int flags) {
  QString searchFor = m_searchForList.first();

  s.flags = flags;

  if (s.flags & sfFromCursor)
    s.cursor = m_view->cursor; 
  else {
    if (!(s.flags & sfBackward))
      s.cursor.set(0, 0); 
    else
      s.cursor.set(-1, m_doc->lastLine());

    s.flags |= sfFinished;
  }

  if (!(s.flags & sfBackward)) {
    if (!(s.cursor.x() || s.cursor.y()))
      s.flags |= sfFinished;
  } else
    s.startCursor.moveX(-searchFor.length());

  s.startCursor = s.cursor;
}

void KWrite::continueSearch(SConfig &s) {
  if (!(s.flags & sfBackward))
    s.cursor.set(0, 0); 
  else
    s.cursor.set(-1, m_doc->lastLine());

  s.flags |= sfFinished;
  s.flags &= ~sfAgain;
}

void KWrite::searchAgain(SConfig &s) {
  int query;
  QString str;
  KWCursor cursor;
  QString searchFor = m_searchForList.first();
  int slen = searchFor.length();

  do {
    query = 1;

    if (m_doc->doSearch(s, searchFor)) {
      cursor = s.cursor;

      if (!(s.flags & sfBackward))
        s.cursor.moveX(slen);

      m_view->updateCursor(s.cursor); //does deselectAll()
      exposeFound(cursor, slen, (s.flags & sfAgain) ? 0 : ufUpdateOnScroll, false);
    } else {
      if (!(s.flags & sfFinished)) {
        // ask for continue
        if (!(s.flags & sfBackward)) {
          // forward search
          str = i18n("End of document reached.\nContinue from the beginning?");
          query = KMessageBox::questionYesNo(m_widget, str, i18n("Find"));
	} else {
          // backward search
          str = i18n("Beginning of document reached.\nContinue from the end");
          query = KMessageBox::questionYesNo(m_widget, str, i18n("Find"));
	}

	continueSearch(s);
      } else
        KMessageBox::sorry(m_widget, i18n("Search string not found!"), i18n("Find"));
    }
  }
  while (query == 0);
}

void KWrite::replaceAgain() {
  if (!isReadWrite())
    return;

  m_replaces = 0;

  if (s.flags & sfPrompt)
    doReplaceAction(-1); 
  else
    doReplaceAction(srAll);
}

void KWrite::doReplaceAction(int result, bool found) {
  KWCursor cursor;
  bool started;
  QString searchFor = m_searchForList.first();
  QString replaceWith = m_replaceWithList.first();
  int slen = searchFor.length();
  int rlen = replaceWith.length();

  switch (result) {
    case srYes: //yes
      m_doc->recordStart(m_view, s.cursor, m_configFlags, KWActionGroup::ugReplace, true);
      m_doc->recordReplace(s.cursor, slen, replaceWith);
      m_replaces++;
      if (s.cursor.y() == s.startCursor.y() && s.cursor.x() < s.startCursor.x())
        s.startCursor.moveX(rlen - slen);
      if (!(s.flags & sfBackward)) s.cursor.moveX(rlen);
      m_doc->recordEnd(m_view, s.cursor, m_configFlags | cfPersistent);
      break;
    case srNo: //no
      if (!(s.flags & sfBackward)) s.cursor.moveX(slen);
      break;
    case srAll: //replace all
      deleteReplacePrompt();
      do {
        started = false;
        while (found || m_doc->doSearch(s, searchFor)) {
          if (!started) {
            found = false;
            m_doc->recordStart(m_view, s.cursor, m_configFlags,
              KWActionGroup::ugReplace);
            started = true;
          }
          m_doc->recordReplace(s.cursor, slen, replaceWith);
          m_replaces++;
          if (s.cursor.y() == s.startCursor.y() && s.cursor.x() < s.startCursor.x())
            s.startCursor.moveX(rlen - slen);
          if (!(s.flags & sfBackward)) s.cursor.moveX(rlen);
        }
        if (started) m_doc->recordEnd(m_view, s.cursor,
          m_configFlags | cfPersistent);
      } while (!askReplaceEnd());
      return;
    case srCancel: //cancel
      deleteReplacePrompt();
      return;
    default:
      m_replacePrompt = 0L;
  }

  do {
    if (m_doc->doSearch(s, searchFor)) {
      //text found: highlight it, show replace prompt if needed and exit
      cursor = s.cursor;
      if (!(s.flags & sfBackward)) cursor.moveX(slen);
      m_view->updateCursor(cursor); //does deselectAll()
      exposeFound(s.cursor, slen, (s.flags & sfAgain) ? 0 : ufUpdateOnScroll, true);
      if (m_replacePrompt == 0L) {
        m_replacePrompt = new ReplacePrompt(m_widget);
        XSetTransientForHint(qt_xdisplay(), m_replacePrompt->winId(), m_widget->topLevelWidget()->winId());
        m_doc->setPseudoModal(m_replacePrompt);//disable();
        connect(m_replacePrompt, SIGNAL(clicked()), this, SLOT(replaceSlot()));
        m_replacePrompt->show(); //this is not modal
      }
      return; //exit if text found
    }
    //nothing found: repeat until user cancels "repeat from beginning" dialog
  }
  while (!askReplaceEnd());

  deleteReplacePrompt();
}

void KWrite::exposeFound(KWCursor &cursor, int slen, int flags, bool replace) {
  TextLine *textLine;
  int x1, x2, y1, y2, xPos, yPos;

  m_doc->markFound(cursor, slen);

  textLine = m_doc->textLine(cursor.y());
  x1 = m_doc->textWidth(textLine, cursor.x())        -10;
  x2 = m_doc->textWidth(textLine, cursor.x() + slen) +20;
  y1 = m_doc->fontHeight()*cursor.y()                 -10;
  y2 = y1 + m_doc->fontHeight()                     +30;

  xPos = m_view->xPos;
  yPos = m_view->yPos;

  if (x1 < 0) x1 = 0;
  if (replace) y2 += 90;

  if (x1 < xPos || x2 > xPos + m_view->width()) {
    xPos = x2 - m_view->width();
  }
  if (y1 < yPos || y2 > yPos + m_view->height()) {
    xPos = x2 - m_view->width();
    yPos = m_doc->fontHeight()*cursor.y() - m_widget->height()/3;
  }
  m_view->setPos(xPos, yPos);
  m_view->updateView(flags);// | ufPos, xPos, yPos);
  m_doc->updateViews(m_view);
//  m_doc->updateViews();
}

void KWrite::deleteReplacePrompt() {
  m_doc->setPseudoModal(0L);
}

bool KWrite::askReplaceEnd() {
  QString str;
  int query;

  m_doc->updateViews();

  if (s.flags & sfFinished) {
    // replace finished
    str = i18n("%1 replace(s) made").arg(m_replaces);
    KMessageBox::information(m_widget, str, i18n("Replace"));
    return true;
  }

  // ask for continue
  if (!(s.flags & sfBackward)) {
    // forward search
    str = i18n("%1 replace(s) made.\nEnd of document reached.\nContinue from the beginning?").arg(m_replaces);
    query = KMessageBox::questionYesNo(m_widget, str, i18n("Replace"));
  } else {
    // backward search
    str = i18n("%1 replace(s) made.\nBeginning of document reached.\nContinue from the end?").arg(m_replaces);
    query = KMessageBox::questionYesNo(m_widget, str, i18n("Replace"));
  }

  m_replaces = 0;
  continueSearch(s);

  return query;
}

void KWrite::replaceSlot() {
  doReplaceAction(m_replacePrompt->result(), true);
}

void KWrite::installRBPopup(QPopupMenu *p) {
  popup = p;
}

/*
void KWrite::installBMPopup(KGuiCmdPopup *p) {
  connect(p, SIGNAL(aboutToShow()), SLOT(updateBMPopup()));
}
*/

void KWrite::setBookmark() {
  int z;
  QPopupMenu *popup = new QPopupMenu(0L);

  for (z = 1; z <= nBookmarks; z++)
    popup->insertItem(QString::number(z), z);

  popup->move(m_widget->mapToGlobal(
    QPoint((m_widget->width() - 41) / 2, (m_widget->height() - 211) / 2)));
  z = popup->exec();

  delete popup;

  if (z > 0) setBookmark(z - 1);
}

void KWrite::addBookmark() {
  int z;

  for (z = 0; z < nBookmarks; z++) {
    if (bookmark[z] == 0L) {
      setBookmark(z);
      break;
    }
  }
}

void KWrite::clearBookmarks() {
  int z;
  for (z = 0; z < nBookmarks; z++) {
    if (bookmark[z] != 0L) {
      delete bookmark[z]; // removes itself from the doc
      bookmark[z] = 0L;
    }
  }
}

void KWrite::setBookmark(int n) {
  if (n >= nBookmarks) return;

  if (bookmark[n] != 0L) {
    delete bookmark[n]; // removes itself from the doc
  }  
  bookmark[n] = new KWBookmark(m_view->xPos, m_view->yPos, m_view->cursor, this);;  
  m_doc->addLineAttribute(bookmark[n]);
}

void KWrite::gotoBookmark(int n) {
  KWBookmark *b;

  if (n >= nBookmarks) return;
  if (bookmark[n] == 0L) return;

  b = bookmark[n];
  KWCursor cursor = b->cursor();
  m_view->updateCursor(cursor);
  m_view->setPos(b->xPos(), b->yPos());
  m_doc->updateViews();
}

void KWrite::updateBMPopup() {
/*
  KWBookmark *b;
  QString buf;
  int z, id;

  p = (KGuiCmdPopup *) sender();
  p->clear();
//  p->insertSeparator();
  for (z = 0; z < (int) bookmarks.count(); z++) {
    b = bookmarks.at(z);
    if (b->cursor.y() >= 0) {
      if (p->count() == 3) p->insertSeparator();
      id = p->addCommand(ctBookmarkCommands, cmGotoBookmarks + z);
      buf = i18n("Line %1").arg(b->cursor.y() +1);
      p->setText(buf, id);
//      p->insertItem(buf, z);
//      if (z < 9) p->setAccel(ALT+keys[z], z);
    }
  }

  while ((int) p->count() > bmEntries) {
    p->removeItemAt(p->count() - 1);
  }

  for (z = 0; z < (int) bookmarks.count(); z++) {
    b = bookmarks.at(z);
//  for (b = bookmarks.first(); b != 0L; b = bookmarks.next()) {
    if (b->cursor.y() >= 0) {
      if ((int) p->count() == bmEntries) p->insertSeparator();
      buf = i18n("Line %1").arg(b->cursor.y() +1);
      p->insertItem(buf, z);
      if (z < 9) p->setAccel(ALT+keys[z], z);
    }
  }
*/
}


void KWrite::readConfig(KConfig *config) {

  m_searchFlags = config->readNumEntry("SearchFlags", sfPrompt);
  m_configFlags = config->readNumEntry("ConfigFlags", m_configFlags) & ~cfMark;
  m_wrapAt = config->readNumEntry("WrapAt", m_wrapAt);
/*
  int flags;

  config->setGroup("Search Options");
  flags = 0;
  if (config->readNumEntry("CaseSensitive")) flags |= sfCaseSensitive;
  if (config->readNumEntry("WholeWordsOnly")) flags |= sfWholeWords;
  if (config->readNumEntry("FromCursor")) flags |= sfFromCursor;
  if (config->readNumEntry("FindBackwards")) flags |= sfBackward;
  if (config->readNumEntry("SelectedText")) flags |= sfSelected;
  if (config->readNumEntry("PromptOnReplace", 1)) flags |= sfPrompt;
  searchFlags = flags;

  config->setGroup("Settings");
  flags = 0;
  if (config->readNumEntry("AutoIndent")) flags |= cfAutoIndent;
  if (config->readNumEntry("BackspaceIndent")) flags |= cfBackspaceIndent;
  if (config->readNumEntry("WordWrap")) flags |= cfWordWrap;
  if (config->readNumEntry("ReplaceTabs")) flags |= cfReplaceTabs;
  if (config->readNumEntry("RemoveTrailingSpaces", 1)) flags |= cfRemoveSpaces;
  if (config->readNumEntry("WrapCursor")) flags |= cfWrapCursor;
  if (config->readNumEntry("AutoBrackets")) flags |= cfAutoBrackets;
  if (config->readNumEntry("PersistentSelections", 1)) flags |= cfPersistent;
  if (config->readNumEntry("MultipleSelections")) flags |= cfKeepSelection;
  if (config->readNumEntry("VerticalSelections")) flags |= cfVerticalSelect;
  if (config->readNumEntry("DeleteOnInput")) flags |= cfDelOnInput;
  if (config->readNumEntry("ToggleOld")) flags |= cfXorSelect;
  m_configFlags = flags;

  wrapAt = config->readNumEntry("WrapAt", 79);
  m_doc->setTabWidth(config->readNumEntry("TabWidth", 8));
  m_doc->setUndoSteps(config->readNumEntry("UndoSteps", 50));
*/
}

void KWrite::writeConfig(KConfig *config) {

  config->writeEntry("SearchFlags", m_searchFlags);
  config->writeEntry("ConfigFlags", m_configFlags);
  config->writeEntry("WrapAt", m_wrapAt);

/*
  int flags;

  config->setGroup("Search Options");
  flags = searchFlags;
  config->writeEntry("CaseSensitive", (flags & sfCaseSensitive) != 0);
  config->writeEntry("WholeWordsOnly", (flags & sfWholeWords) != 0);
  config->writeEntry("FromCursor", (flags & sfFromCursor) != 0);
  config->writeEntry("FindBackwards", (flags & sfBackward) != 0);
  config->writeEntry("SelectedText", (flags & sfSelected) != 0);
  config->writeEntry("PromptOnReplace", (flags & sfPrompt) != 0);

  config->setGroup("Settings");
  flags = m_configFlags;
  config->writeEntry("AutoIndent", (flags & cfAutoIndent) != 0);
  config->writeEntry("BackspaceIndent", (flags & cfBackspaceIndent) != 0);
  config->writeEntry("WordWrap", (flags & cfWordWrap) != 0);
  config->writeEntry("ReplaceTabs", (flags & cfReplaceTabs) != 0);
  config->writeEntry("RemoveTrailingSpaces", (flags & cfRemoveSpaces) != 0);
  config->writeEntry("WrapCursor", (flags & cfWrapCursor) != 0);
  config->writeEntry("AutoBrackets", (flags & cfAutoBrackets) != 0);
  config->writeEntry("PersistentSelections", (flags & cfPersistent) != 0);
  config->writeEntry("MultipleSelections", (flags & cfKeepSelection) != 0);
  config->writeEntry("VerticalSelections", (flags & cfVerticalSelect) != 0);
  config->writeEntry("DeleteOnInput", (flags & cfDelOnInput) != 0);
  config->writeEntry("ToggleOld", (flags & cfXorSelect) != 0);

  config->writeEntry("WrapAt", m_wrapAt);
  config->writeEntry("TabWidth", m_doc->tabChars);
  config->writeEntry("UndoSteps", m_doc->undoSteps());
*/
}

void KWrite::readSessionConfig(KConfig *config) {
  KWCursor cursor;
  int z;
  char s1[16];
  QString s2;

  readConfig(config);

  m_view->xPos = config->readNumEntry("XPos");
  m_view->yPos = config->readNumEntry("YPos");
  cursor.set(config->readNumEntry("CursorX"),
    config->readNumEntry("CursorY"));
  m_view->updateCursor(cursor);

//  count = config->readNumEntry("Bookmarks");
  for (z = 0; z < nBookmarks; z++) {
    sprintf(s1, "Bookmark%d", z + 1);
    s2 = config->readEntry(s1);
    if (!s2.isEmpty()) {
      int xPos, yPos, cursorX, line;

      sscanf(s2, "%d, %d, %d, %d", &cursorX, &line, &xPos, &yPos);
      bookmark[z] = new KWBookmark(xPos, yPos, KWCursor(cursorX, line), this);
    }
  }
}

void KWrite::writeSessionConfig(KConfig *config) {
  int z;
  char s1[16];
  char s2[64];
  KWBookmark *b;

  writeConfig(config);

  config->writeEntry("XPos", m_view->xPos);
  config->writeEntry("YPos", m_view->yPos);
  config->writeEntry("CursorX", m_view->cursor.x());
  config->writeEntry("CursorY", m_view->cursor.y());

//  config->writeEntry("Bookmarks", bookmarks.count());
  for (z = 0; z < nBookmarks; z++) {
    b = bookmark[z];
    if (b != 0L) {
      sprintf(s1, "Bookmark%d", z + 1);
      sprintf(s2, "%d, %d, %d, %d", b->cursorX(), b->line(), 
        b->xPos(), b->yPos());
      config->writeEntry(s1, s2);
    }
  }
}


/*
void KWrite::setHighlight(Highlight *hl) {
  if (hl) {
    m_doc->setHighlight(hl);
    m_doc->updateViews();
  }
}
*/

int KWrite::highlightNum() {
  return m_doc->highlightNum();
}

void KWrite::setHighlight(int n) {
  m_doc->setHighlight(n);
  m_doc->updateViews();
}

void KWrite::hlDef() {
  DefaultsDialog *dlg;
  HlManager *hlManager;
  ItemStyleList defaultStyleList;
  ItemFont defaultFont;
//  int count, z;

  hlManager = m_doc->hlManager();
  defaultStyleList.setAutoDelete(true);

  hlManager->getDefaults(defaultStyleList, defaultFont);
/*
  defItemStyleList = m_doc->defItemStyleList;
  count = defItemStyleList->count();
  for (z = 0; z < count ; z++) {
    itemStyleList.append(new ItemStyle(*defItemStyleList->at(z)));
  }
  */
  dlg = new DefaultsDialog(hlManager, &defaultStyleList, &defaultFont, m_widget);

  dlg->setCaption(i18n("Highlight Defaults"));

  if (dlg->exec() == QDialog::Accepted) {
    hlManager->setDefaults(defaultStyleList, defaultFont);
/*    for (z = 0; z < count; z++) {
      defItemStyleList->at(z)->setData(*itemStyleList.at(z));
    }
    m_doc->defFont->setData(defFont);*/
  }
  delete dlg;
}

void KWrite::hlDlg() {
  HighlightDialog *dlg;
  HlManager *hlManager;
  HlDataList hlDataList;


  hlManager = m_doc->hlManager();
  hlDataList.setAutoDelete(true);
  //this gets the data from the KConfig object
  hlManager->getHlDataList(hlDataList);
  dlg = new HighlightDialog(hlManager, &hlDataList,
    m_doc->highlightNum(), m_widget);
  dlg->setCaption(i18n("Highlight Settings"));
//  dlg->hlChanged(m_doc->highlightNum());
  if (dlg->exec() == QDialog::Accepted) {
    //this stores the data into the KConfig object
    hlManager->setHlDataList(hlDataList);
  }
  delete dlg;
}

int KWrite::eolMode() {
  return m_doc->eolMode();
}

void KWrite::setEolMode(int eol) {
  if (!isReadWrite())
    return;

  m_doc->setEolMode(eol);
  m_doc->setModified(true);
}

/*
void KWrite::paintEvent(QPaintEvent *event) {
  int x, y;

  QRect updateR = event->rect();                    // update rectangle
//  debug("Update rect = (%i, %i, %i, %i)",
//    updateR.x(), updateR.y(), updateR.width(), updateR.height());

  int ux1 = updateR.x();
  int uy1 = updateR.y();
  int ux2 = ux1 + updateR.width();
  int uy2 = uy1 + updateR.height();

  QPainter paint;
  paint.begin(this);

  QColorGroup g = colorGroup();
  x = width();
  y = height();

  paint.setPen(g.dark());
  if (uy1 <= 0) paint.drawLine(0, 0, x-2, 0);
  if (ux1 <= 0) paint.drawLine(0, 1, 0, y-2);

  paint.setPen(black);
  if (uy1 <= 1) paint.drawLine(1, 1, x-3, 1);
  if (ux1 <= 1) paint.drawLine(1, 2, 1, y-3);

  paint.setPen(g.midlight());
  if (uy2 >= y-1) paint.drawLine(1, y-2, x-3, y-2);
  if (ux2 >= x-1) paint.drawLine(x-2, 1, x-2, y-2);

  paint.setPen(g.light());
  if (uy2 >= y) paint.drawLine(0, y-1, x-2, y-1);
  if (ux2 >= x) paint.drawLine(x-1, 0, x-1, y-1);

  x -= 2 + 16;
  y -= 2 + 16;
  if (ux2 > x && uy2 > y) {
    paint.fillRect(x, y, 16, 16, g.background());
  }
  paint.end();
}

void KWrite::resizeEvent(QResizeEvent *) {

//  debug("Resize %d, %d", event->size().width(), event->size().height());

  m_view->resize(width(), height());
  m_view->tagAll();
  m_view->updateView(0); // ufNoScroll);
}
*/

//  Spellchecking methods

KSpellConfig *KWrite::ksConfig() {
  return kspell.ksc;
}

void KWrite::setKSConfig(const KSpellConfig _ksc) {
  *kspell.ksc = _ksc;
}

void KWrite::spellCheck() {
  if (!isReadWrite())
    return;

  kspell.kspell= new KSpell(m_widget, "KWrite: Spellcheck", this,
                      SLOT(spellCheck2(KSpell *)));

  connect(kspell.kspell, SIGNAL(death()),
          this, SLOT(spellCleanDone()));

  connect(kspell.kspell, SIGNAL(progress(unsigned int)),
          this, SIGNAL(spellcheck_progress(unsigned int)));
  connect(kspell.kspell, SIGNAL(misspelling(QString , QStringList *, unsigned)),
          this, SLOT(misspelling(QString, QStringList *, unsigned)));
  connect(kspell.kspell, SIGNAL(corrected(QString, QString, unsigned)),
          this, SLOT(corrected(QString, QString, unsigned)));
  connect(kspell.kspell, SIGNAL(done(const char *)),
          this, SLOT(spellResult(const char *)));
}

void KWrite::spellCheck2(KSpell *) {
  m_doc->setReadWrite(false);

  // this is a hack, setPseudoModal() has been hacked to recognize 0x01
  // as special(never tries to delete it)
  // this should either get improved(with a #define or something),
  // or kspell should provide access to the spell widget.
  m_doc->setPseudoModal((QWidget *)0x01);

  kspell.spell_tmptext = text();


  kspell.kspellon = TRUE;
  kspell.kspellMispellCount = 0;
  kspell.kspellReplaceCount = 0;
  kspell.kspellPristine = ! m_doc->isModified();

  kspell.kspell->setProgressResolution(1);

  kspell.kspell->check(kspell.spell_tmptext);
}

void KWrite::misspelling(QString origword, QStringList *, unsigned pos) {
  int line;
  unsigned int cnt;

  // Find pos  -- CHANGEME: store the last found pos's cursor
  //   and do these searched relative to that to
  //   (significantly) increase the speed of the spellcheck

  for (cnt = 0, line = 0 ; line <= m_doc->lastLine() && cnt <= pos ; line++)
    cnt += m_doc->textLine(line)->length()+1;

  // Highlight the mispelled word
  KWCursor cursor;
  line--;
  cursor.set(pos - (cnt - m_doc->textLine(line)->length()) + 1, line);
//  deselectAll(); // shouldn't the spell check be allowed within selected text?
  kspell.kspellMispellCount++;
  m_view->updateCursor(cursor); //this does deselectAll() if no persistent selections
  m_doc->markFound(cursor, origword.length());
  m_doc->updateViews();
}

void KWrite::corrected(QString originalword, QString newword, unsigned pos) {
  //we'll reselect the original word in case the user has played with
  //the selection

  int line;
  unsigned int cnt=0;

  if (newword != originalword) {

      // Find pos
      for (line = 0 ; line <= m_doc->lastLine() && cnt <= pos ; line++)
        cnt += m_doc->textLine(line)->length() + 1;

      // Highlight the mispelled word
      KWCursor cursor;
      line--;
      cursor.set(pos - (cnt-m_doc->textLine(line)->length()) + 1, line);
      m_view->updateCursor(cursor);
      m_doc->markFound(cursor, newword.length());

      m_doc->recordStart(m_view, cursor, m_configFlags,
        KWActionGroup::ugSpell, true, kspell.kspellReplaceCount > 0);
      m_doc->recordReplace(cursor, originalword.length(), newword);
      m_doc->recordEnd(m_view, cursor, m_configFlags | cfGroupUndo);

      kspell.kspellReplaceCount++;
    }

}

void KWrite::spellResult(const char *) {
  unselectAll(); //!!! this should not be done with persistent selections

//  if (kspellReplaceCount) m_doc->recordReset();

  // we know if the check was cancelled
  // we can safely use the undo mechanism to backout changes
  // in case of a cancel, because we force the entire spell check
  // into one group(record)
  if (kspell.kspell->dlgResult() == 0) {
    if (kspell.kspellReplaceCount) {
      // backout the spell check
      VConfig c;
      m_view->getVConfig(c);
      m_doc->undo(c);
      // clear the redo list, so the cancelled spell check can't be redo'ed <- say that word ;-)
      m_doc->clearRedo();
      // make sure the modified flag is turned back off
      // if we started with a clean buffer
      if (kspell.kspellPristine)
        m_doc->setModified(false);
    }
  }

  m_doc->setPseudoModal(0L);
  m_doc->setReadWrite(true);

  // if we marked up the text, clear it now
  if (kspell.kspellMispellCount)
    m_doc->unmarkFound();

  m_doc->updateViews();

  kspell.kspell->cleanUp();
}

void KWrite::spellCleanDone() {
  KSpell::spellStatus status = kspell.kspell->status();
  kspell.spell_tmptext = "";
  delete kspell.kspell;

  kspell.kspell = 0;
  kspell.kspellon = FALSE;

  if (status == KSpell::Error) {
     KMessageBox::sorry(m_widget, i18n("ISpell could not be started.\n"
     "Please make sure you have ISpell properly configured and in your PATH."));
  } else if (status == KSpell::Crashed) {
     m_doc->setPseudoModal(0L);
     m_doc->setReadWrite(true);

     // if we marked up the text, clear it now
     if (kspell.kspellMispellCount)
        m_doc->unmarkFound();

     m_doc->updateViews();
     KMessageBox::sorry(m_widget, i18n("ISpell seems to have crashed."));
  } else {
     emit spellcheck_done();
  }
}
