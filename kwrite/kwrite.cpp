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

#include <kwrite/kwrite.h>
#include <kwrite/highlight.h>
#include <kwrite/kwrite_doc.h>

#include "kwdialog.h"
#include "ktextprint.h"
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

QList<BufferInfo> bufferInfoList;
QPixmap *buffer = 0;

QPixmap *getBuffer(void *user) {
  BufferInfo *info;

  if (!buffer)
    buffer = new QPixmap;

  info = new BufferInfo;
  info->user = user;
  info->w = 0;
  info->h = 0;
  bufferInfoList.append(info);
  return buffer;
}

void resizeBuffer(void *user, int w, int h) {
  int z;
  BufferInfo *info;
  int maxW, maxH;

  maxW = w;
  maxH = h;
  for (z = 0; z < (int) bufferInfoList.count(); z++) {
    info = bufferInfoList.at(z);
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

  for (z = (int) bufferInfoList.count() -1; z >= 0 ; z--) {
    info = bufferInfoList.at(z);
    if (info->user == user) bufferInfoList.remove(z);
  }
  resizeBuffer(0, 0, 0);
}

KWrite::KWrite(KWriteDoc *doc, QWidget *parent, const QString &name, bool HandleOwnDND)
  : QWidget(parent, name) {
  m_doc = doc;
  m_view = new KWriteView(this, doc, HandleOwnDND);

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
  bookmarks.setAutoDelete(true);

  //KSpell initial values
  kspell.kspell = 0;
  kspell.ksc = new KSpellConfig; //default KSpellConfig to start
  kspell.kspellon = FALSE;

  m_view->setFocus();
  resize(parent->width(), parent->height());
}

KWrite::~KWrite() {
  if (kspell.kspell) {
    kspell.kspell->setAutoDelete(true);
    kspell.kspell->cleanUp(); // need a way to wait for this to complete
  }

  delete popup; //right mouse button popup
}

/*
void KWrite::addCursorCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctCursorCommands, I18N_NOOP("Cursor Movement"));
  cmdMngr.setSelectModifiers(Qt::SHIFT, selectFlag, Qt::ALT, multiSelectFlag);
  cmdMngr.addCommand(cmLeft,            I18N_NOOP("Left"), Qt::Key_Left, Qt::CTRL+Qt::Key_B);
  cmdMngr.addCommand(cmRight,           I18N_NOOP("Right"), Qt::Key_Right, Qt::CTRL+Qt::Key_F);
  cmdMngr.addCommand(cmWordLeft,        I18N_NOOP("Word Left"), Qt::CTRL+Qt::Key_Left);
  cmdMngr.addCommand(cmWordRight,       I18N_NOOP("Word Right"), Qt::CTRL+Qt::Key_Right);
  cmdMngr.addCommand(cmHome,            I18N_NOOP("Home"), Qt::Key_Home, Qt::CTRL+Qt::Key_A, Qt::Key_F27);
  cmdMngr.addCommand(cmEnd,             I18N_NOOP("End"), Qt::Key_End, Qt::CTRL+Qt::Key_E, Qt::Key_F33);
  cmdMngr.addCommand(cmUp,              I18N_NOOP("Up"), Qt::Key_Up, Qt::CTRL+Qt::Key_P);
  cmdMngr.addCommand(cmDown,            I18N_NOOP("Down"), Qt::Key_Down, Qt::CTRL+Qt::Key_N);
  cmdMngr.addCommand(cmScrollUp,        I18N_NOOP("Scroll Up"), Qt::CTRL+Qt::Key_Up);
  cmdMngr.addCommand(cmScrollDown,      I18N_NOOP("Scroll Down"), Qt::CTRL+Qt::Key_Down);
  cmdMngr.addCommand(cmTopOfView,       I18N_NOOP("Top Of View"), Qt::CTRL+Qt::Key_PageUp);
  cmdMngr.addCommand(cmBottomOfView,    I18N_NOOP("Bottom Of View"), Qt::CTRL+Qt::Key_PageDown);
  cmdMngr.addCommand(cmPageUp,          I18N_NOOP("Page Up"), Qt::Key_PageUp, Qt::Key_F29);
  cmdMngr.addCommand(cmPageDown,        I18N_NOOP("Page Down"), Qt::Key_PageDown, Qt::Key_F35);
//  cmdMngr.addCommand(cmCursorPageUp,    I18N_NOOP("Cursor Page Up"));
//  cmdMngr.addCommand(cmCursorPageDown,  I18N_NOOP("Cursor Page Down"));
  cmdMngr.addCommand(cmTop,             I18N_NOOP("Top"), Qt::CTRL+Qt::Key_Home);
  cmdMngr.addCommand(cmBottom,          I18N_NOOP("Bottom"), Qt::CTRL+Qt::Key_End);
  cmdMngr.addCommand(cmLeft | selectFlag, I18N_NOOP("Left + Select") , Qt::SHIFT+Qt::Key_F30);//, Qt::SHIFT+Qt::Key_4);
  cmdMngr.addCommand(cmRight | selectFlag, I18N_NOOP("Right + Select") , Qt::SHIFT+Qt::Key_F32);//, Qt::SHIFT+Qt::Key_6);
  cmdMngr.addCommand(cmUp | selectFlag,   I18N_NOOP("Up + Select") , Qt::SHIFT+Qt::Key_F28);//, Qt::SHIFT+Qt::Key_8);
  cmdMngr.addCommand(cmDown | selectFlag, I18N_NOOP("Down + Select") , Qt::SHIFT+Qt::Key_F34);//, Qt::SHIFT+Qt::Key_2);
}

void KWrite::addEditCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctEditCommands, I18N_NOOP("Edit Commands"));
  cmdMngr.addCommand(cmReturn,          I18N_NOOP("Return"), Qt::Key_Return, Qt::Key_Enter);
  cmdMngr.addCommand(cmDelete,          I18N_NOOP("Delete"), Qt::Key_Delete, Qt::CTRL+Qt::Key_D);
  cmdMngr.addCommand(cmBackspace,       I18N_NOOP("Backspace"), Qt::Key_Backspace, Qt::CTRL+Qt::Key_H);
  cmdMngr.addCommand(cmKillLine,        I18N_NOOP("Kill Line"), Qt::CTRL+Qt::Key_K);
  cmdMngr.addCommand(cmUndo,            I18N_NOOP("&Undo"), Qt::CTRL+Qt::Key_Z, Qt::Key_F14);
  cmdMngr.addCommand(cmRedo,            I18N_NOOP("R&edo"), Qt::CTRL+Qt::Key_Y, Qt::Key_F12);
  cmdMngr.addCommand(cmCut,             I18N_NOOP("C&ut"), Qt::CTRL+Qt::Key_X, Qt::SHIFT+Qt::Key_Delete, Qt::Key_F20);
  cmdMngr.addCommand(cmCopy,            I18N_NOOP("&Copy"), Qt::CTRL+Qt::Key_C, Qt::CTRL+Qt::Key_Insert, Qt::Key_F16);
  cmdMngr.addCommand(cmPaste,           I18N_NOOP("&Paste"), Qt::CTRL+Qt::Key_V, Qt::SHIFT+Qt::Key_Insert, Qt::Key_F18);
  cmdMngr.addCommand(cmIndent,          I18N_NOOP("&Indent"), Qt::CTRL+Qt::Key_I);
  cmdMngr.addCommand(cmUnindent,        I18N_NOOP("Uninden&t"), Qt::CTRL+Qt::Key_U);
  cmdMngr.addCommand(cmCleanIndent,     I18N_NOOP("Clean Indent"));
  cmdMngr.addCommand(cmSelectAll,       I18N_NOOP("&Select All"));
  cmdMngr.addCommand(cmDeselectAll,     I18N_NOOP("&Deselect All"));
  cmdMngr.addCommand(cmInvertSelection, I18N_NOOP("In&vert Selection"));
}

void KWrite::addFindCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctFindCommands, I18N_NOOP("Find Commands"));
  cmdMngr.addCommand(cmFind,            I18N_NOOP("&Find...") , Qt::CTRL+Qt::Key_F, Qt::Key_F19);
  cmdMngr.addCommand(cmReplace,         I18N_NOOP("&Replace...") , Qt::CTRL+Qt::Key_R);
  cmdMngr.addCommand(cmFindAgain,       I18N_NOOP("Find &Again") , Qt::Key_F3);
  cmdMngr.addCommand(cmGotoLine,        I18N_NOOP("&Goto Line...") , Qt::CTRL+Qt::Key_G);
}

void KWrite::addBookmarkCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctBookmarkCommands, I18N_NOOP("Bookmark Commands"));
  cmdMngr.addCommand(cmSetBookmark,       I18N_NOOP("&Set Bookmark..."), Qt::ALT+Qt::Key_X);
  cmdMngr.addCommand(cmAddBookmark,       I18N_NOOP("&Add Bookmark"), Qt::ALT+Qt::Key_A);
  cmdMngr.addCommand(cmClearBookmarks,    I18N_NOOP("&Clear Bookmarks"), Qt::ALT+Qt::Key_C);
  cmdMngr.addCommand(cmSetBookmarks +0,   I18N_NOOP("Set Bookmark 1"));
  cmdMngr.addCommand(cmSetBookmarks +1,   I18N_NOOP("Set Bookmark 2"));
  cmdMngr.addCommand(cmSetBookmarks +2,   I18N_NOOP("Set Bookmark 3"));
  cmdMngr.addCommand(cmSetBookmarks +3,   I18N_NOOP("Set Bookmark 4"));
  cmdMngr.addCommand(cmSetBookmarks +4,   I18N_NOOP("Set Bookmark 5"));
  cmdMngr.addCommand(cmSetBookmarks +5,   I18N_NOOP("Set Bookmark 6"));
  cmdMngr.addCommand(cmSetBookmarks +6,   I18N_NOOP("Set Bookmark 7"));
  cmdMngr.addCommand(cmSetBookmarks +7,   I18N_NOOP("Set Bookmark 8"));
  cmdMngr.addCommand(cmSetBookmarks +8,   I18N_NOOP("Set Bookmark 9"));
  cmdMngr.addCommand(cmSetBookmarks +9,   I18N_NOOP("Set Bookmark 10"));
  cmdMngr.addCommand(cmGotoBookmarks +0,  I18N_NOOP("Goto Bookmark 1"), Qt::ALT+Qt::Key_1);
  cmdMngr.addCommand(cmGotoBookmarks +1,  I18N_NOOP("Goto Bookmark 2"), Qt::ALT+Qt::Key_2);
  cmdMngr.addCommand(cmGotoBookmarks +2,  I18N_NOOP("Goto Bookmark 3"), Qt::ALT+Qt::Key_3);
  cmdMngr.addCommand(cmGotoBookmarks +3,  I18N_NOOP("Goto Bookmark 4"), Qt::ALT+Qt::Key_4);
  cmdMngr.addCommand(cmGotoBookmarks +4,  I18N_NOOP("Goto Bookmark 5"), Qt::ALT+Qt::Key_5);
  cmdMngr.addCommand(cmGotoBookmarks +5,  I18N_NOOP("Goto Bookmark 6"), Qt::ALT+Qt::Key_6);
  cmdMngr.addCommand(cmGotoBookmarks +6,  I18N_NOOP("Goto Bookmark 7"), Qt::ALT+Qt::Key_7);
  cmdMngr.addCommand(cmGotoBookmarks +7,  I18N_NOOP("Goto Bookmark 8"), Qt::ALT+Qt::Key_8);
  cmdMngr.addCommand(cmGotoBookmarks +8,  I18N_NOOP("Goto Bookmark 9"), Qt::ALT+Qt::Key_9);
  cmdMngr.addCommand(cmGotoBookmarks +9,  I18N_NOOP("Goto Bookmark 10"), Qt::ALT+Qt::Key_0);
}

void KWrite::addStateCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctStateCommands, I18N_NOOP("State Commands"));
  cmdMngr.addCommand(cmToggleInsert,   I18N_NOOP("Insert Mode"), Qt::Key_Insert);
  cmdMngr.addCommand(cmToggleVertical, I18N_NOOP("&Vertical Selections"), Qt::Key_F5);
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
  if (m_doc->singleSelection()) flags |= cfSingleSelection;
  return flags;
}

void KWrite::setConfig(int flags) {
  bool updateView;

  // cfSingleSelection is a doc-property
  m_doc->setSingleSelection(flags & cfSingleSelection);
  flags &= ~cfSingleSelection;

  if (flags != m_configFlags) {
    // update the view if visibility of tabs has changed
    updateView = (flags ^ m_configFlags) & cfShowTabs;
    m_configFlags = flags;
    emit newStatus();
    if (updateView) m_view->update();
  }
}

int KWrite::tabWidth() {
  return m_doc->tabChars();
}

void KWrite::setTabWidth(int w) {
  m_doc->setTabWidth(w);
  m_doc->updateViews();
}

int KWrite::undoSteps() {
  return m_doc->undoSteps();
}

void KWrite::setUndoSteps(int s) {
  m_doc->setUndoSteps(s);
}

/*
bool KWrite::isOverwriteMode() {
  return(m_configFlags & cfOvr);
}
*/

bool KWrite::isReadOnly() {
  return m_doc->readOnly();
}

bool KWrite::isModified() {
  return m_doc->modified();
}

void KWrite::setReadOnly(bool m) {
  m_doc->setReadOnly(m);
}

void KWrite::setModified(bool m) {
  m_doc->setModified(m);
}

bool KWrite::isLastView() {
  return m_doc->isLastView(1);
}

KWriteDoc *KWrite::doc() {
  return m_doc;
}

KWriteView *KWrite::view() {
  return m_view;
}

int KWrite::undoState() {
  if (isReadOnly())
    return 0; 
  else
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

void KWrite::colDlg() {
  ColorDialog *dlg = new ColorDialog(this, m_doc->colors());

  if (dlg->exec() == QDialog::Accepted) {
    dlg->getColors(m_doc->colors());
    m_doc->tagAll();
    m_doc->updateViews();
  }

  delete dlg;
}

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

void KWrite::setColors(QColor *colors) {
  m_doc->setColors(colors);
}

void KWrite::getColors(QColor *colors) {
  colors = m_doc->colors();
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

    KMessageBox::sorry(this, i18n("The specified File does not exist"));
    return false;
  }

  if (info.isDir()) {
    KMessageBox::sorry(this, i18n("You have specified a directory"));
    return false;
  }

  if (!info.isReadable()) {
    KMessageBox::sorry(this, i18n("You do not have read permission to this file"));
    return false;
  }

  QFile f(name);
  if (f.open(IO_ReadOnly)) {
    loadFile(f, flags & lfInsert);
    f.close();
    return true;
  }

  KMessageBox::sorry(this, i18n("An Error occured while trying to open this Document"));
  return false;
}

bool KWrite::writeFile(const QString &name) {

  QFileInfo info(name);
  if (info.exists() && !info.isWritable()) {
    KMessageBox::sorry(this, i18n("You do not have write permission to this file"));
    return false;
  }

  QFile f(name);
  if (f.open(IO_WriteOnly | IO_Truncate)) {
    writeFile(f);
    f.close();
    return true;//m_doc->setFileName(name);
  }
  KMessageBox::sorry(this, i18n("An Error occured while trying to open this Document"));
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
    int query = KMessageBox::warningYesNoCancel(this,
                i18n("The current Document has been modified.\nWould you like to save it?"));

    switch (query) {
      case KMessageBox::Yes: //yes
        if (save() == CANCEL)
	  return false;

	if (isModified()) {
	  query = KMessageBox::warningYesNo(this,
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
        query = KMessageBox::warningYesNo(this,
	        i18n("A Document with this Name already exists.\nDo you want to overwrite it?"));
    }
  }
  while (query == 1);

//  kapp->processEvents();
  writeURL(url);
  return OK;
}

void KWrite::doCursorCommand(int cmdNum) {
  VConfig c;
  m_view->getVConfig(c);

  if (cmdNum & selectFlag)
    c.flags |= cfMark;

  if (cmdNum & multiSelectFlag)
    c.flags |= cfMark | cfKeepSelection;

  cmdNum &= ~(selectFlag | multiSelectFlag);
  m_view->doCursorCommand(c, cmdNum);
  m_doc->updateViews();
}

void KWrite::doEditCommand(int cmdNum) {
  VConfig c;
  m_view->getVConfig(c);
  m_view->doEditCommand(c, cmdNum);
  m_doc->updateViews();
}


void KWrite::clear() {
  if (isReadOnly())
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
  if (isReadOnly())
    return;

  VConfig c;
  m_view->getVConfig(c);
  m_doc->undo(c, count);
  m_doc->updateViews();
}

void KWrite::redoMultiple(int count) {
  if (isReadOnly())
    return;

  VConfig c;
  m_view->getVConfig(c);
  m_doc->redo(c, count);
  m_doc->updateViews();
}

void KWrite::undoHistory() {
  UndoHistory *undoH = new UndoHistory(this, this, "UndoHistory", true);

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

void KWrite::unIndent() {
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

void KWrite::deselectAll() {
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

  SearchDialog *searchDialog = new SearchDialog(this, m_searchForList, m_replaceWithList, m_searchFlags & ~sfReplace);

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
  if (isReadOnly())
    return;

  if (!m_doc->hasMarkedText())
    m_searchFlags &= ~sfSelected;

  SearchDialog *searchDialog = new SearchDialog(this, m_searchForList, m_replaceWithList, m_searchFlags | sfReplace);

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
  GotoLineDialog *dlg = new GotoLineDialog(this, m_view->cursor.y() + 1, m_doc->numLines());

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
    s.startCursor.addX(-searchFor.length());

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
        s.cursor.addX(slen);

      m_view->updateCursor(s.cursor); //does deselectAll()
      exposeFound(cursor, slen, (s.flags & sfAgain) ? 0 : ufUpdateOnScroll, false);
    } else {
      if (!(s.flags & sfFinished)) {
        // ask for continue
        if (!(s.flags & sfBackward)) {
          // forward search
          str = i18n("End of document reached.\nContinue from the beginning?");
          query = KMessageBox::questionYesNo(this, str, i18n("Find"));
	} else {
          // backward search
          str = i18n("Beginning of document reached.\nContinue from the end");
          query = KMessageBox::questionYesNo(this, str, i18n("Find"));
	}

	continueSearch(s);
      } else
        KMessageBox::sorry(this, i18n("Search string not found!"), i18n("Find"));
    }
  }
  while (query == 0);
}

void KWrite::replaceAgain() {
  if (isReadOnly())
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
        s.startCursor.addX(rlen - slen);
      if (!(s.flags & sfBackward)) s.cursor.addX(rlen);
      m_doc->recordEnd(m_view, s.cursor, m_configFlags | cfPersistent);
      break;
    case srNo: //no
      if (!(s.flags & sfBackward)) s.cursor.addX(slen);
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
            s.startCursor.addX(rlen - slen);
          if (!(s.flags & sfBackward)) s.cursor.addX(rlen);
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
      if (!(s.flags & sfBackward)) cursor.addX(slen);
      m_view->updateCursor(cursor); //does deselectAll()
      exposeFound(s.cursor, slen, (s.flags & sfAgain) ? 0 : ufUpdateOnScroll, true);
      if (m_replacePrompt == 0L) {
        m_replacePrompt = new ReplacePrompt(this);
        XSetTransientForHint(qt_xdisplay(), m_replacePrompt->winId(), topLevelWidget()->winId());
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
    yPos = m_doc->fontHeight()*cursor.y() - height()/3;
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
    KMessageBox::information(this, str, i18n("Replace"));
    return true;
  }

  // ask for continue
  if (!(s.flags & sfBackward)) {
    // forward search
    str = i18n("%1 replace(s) made.\nEnd of document reached.\nContinue from the beginning?").arg(m_replaces);
    query = KMessageBox::questionYesNo(this, str, i18n("Replace"));
  } else {
    // backward search
    str = i18n("%1 replace(s) made.\nBeginning of document reached.\nContinue from the end?").arg(m_replaces);
    query = KMessageBox::questionYesNo(this, str, i18n("Replace"));
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

  for (z = 1; z <= 10; z++)
    popup->insertItem(QString::number(z), z);

  popup->move(mapToGlobal(QPoint((width() - 41) / 2, (height() - 211) / 2)));
  z = popup->exec();

  delete popup;

  if (z > 0)
    setBookmark(z - 1);
}

void KWrite::addBookmark() {
  int z;

  for (z = 0; z < (int) bookmarks.count(); z++) {
    if (bookmarks.at(z)->cursor.y() == -1)
      break;
  }

  setBookmark(z);
}

void KWrite::clearBookmarks() {
  bookmarks.clear();
}

void KWrite::setBookmark(int n) {
  KWBookmark *b;

  if (n >= 10)
    return;

  while ((int) bookmarks.count() <= n)
    bookmarks.append(new KWBookmark());

  b = bookmarks.at(n);
  b->xPos = m_view->xPos;
  b->yPos = m_view->yPos;
  b->cursor = m_view->cursor;
}

void KWrite::gotoBookmark(int n) {
  KWBookmark *b;

  if (n < 0 || n >= (int) bookmarks.count())
    return;

  b = bookmarks.at(n);
  if (b->cursor.y() == -1)
    return;

  m_view->updateCursor(b->cursor);
  m_view->setPos(b->xPos, b->yPos);
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
  int count, z;
  char s1[16];
  QString s2;
  KWBookmark *b;

/*
  m_searchFlags = config->readNumEntry("SearchFlags", sfPrompt);
  m_configFlags = config->readNumEntry("ConfigFlags");
  m_wrapAt = config->readNumEntry("WrapAt", 79);
*/
  readConfig(config);

  m_view->xPos = config->readNumEntry("XPos");
  m_view->yPos = config->readNumEntry("YPos");
  cursor.set(config->readNumEntry("CursorX"),
    config->readNumEntry("CursorY"));
  m_view->updateCursor(cursor);

  count = config->readNumEntry("Bookmarks");
  for (z = 0; z < count; z++) {
    b = new KWBookmark();
    bookmarks.append(b);
    sprintf(s1, "Bookmark%d", z+1);
    s2 = config->readEntry(s1);
    if (!s2.isEmpty()) {
      int x = b->cursor.x();
      int y = b->cursor.y();

      sscanf(s2, "%d, %d, %d, %d", &b->xPos, &b->yPos, &x, &y);

      b->cursor.set(x, y);
    }
  }
}

void KWrite::writeSessionConfig(KConfig *config) {
  int z;
  char s1[16];
  char s2[64];
  KWBookmark *b;

/*
  config->writeEntry("SearchFlags", m_searchFlags);
  config->writeEntry("ConfigFlags", m_configFlags);
  config->writeEntry("WrapAt", m_wrapAt);
*/
  writeConfig(config);

  config->writeEntry("XPos", m_view->xPos);
  config->writeEntry("YPos", m_view->yPos);
  config->writeEntry("CursorX", m_view->cursor.x());
  config->writeEntry("CursorY", m_view->cursor.y());

  config->writeEntry("Bookmarks", bookmarks.count());
  for (z = 0; z < (int) bookmarks.count(); z++) {
    b = bookmarks.at(z);
    if (b->cursor.y() != -1) {
      sprintf(s1, "Bookmark%d", z+1);
      sprintf(s2, "%d, %d, %d, %d", b->xPos, b->yPos, b->cursor.x(), b->cursor.y());
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
  dlg = new DefaultsDialog(hlManager, &defaultStyleList, &defaultFont, this);

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
    m_doc->highlightNum(), this);
  dlg->setCaption(i18n("Highlight Settings"));
//  dlg->hlChanged(m_doc->highlightNum());
  if (dlg->exec() == QDialog::Accepted) {
    //this stores the data into the KConfig object
    hlManager->setHlDataList(hlDataList);
  }
  delete dlg;
}

int KWrite::getHl() {
  return m_doc->highlightNum();
}

void KWrite::setHl(int n) {
  m_doc->setHighlight(n);
  m_doc->updateViews();
}

int KWrite::getEol() {
  return m_doc->eolMode();
}

void KWrite::setEol(int eol) {
  if (isReadOnly())
    return;

  m_doc->setEolMode(eol);
  m_doc->setModified(true);
}

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
  m_view->updateView(0 /*ufNoScroll*/);
}

//  Spellchecking methods

KSpellConfig *KWrite::ksConfig() {
  return kspell.ksc;
}

void KWrite::setKSConfig(const KSpellConfig _ksc) {
  *kspell.ksc = _ksc;
}

void KWrite::spellcheck() {
  if (isReadOnly())
    return;

  kspell.kspell= new KSpell(this, "KWrite: Spellcheck", this,
                      SLOT(spellcheck2(KSpell *)));

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

void KWrite::spellcheck2(KSpell *) {
  m_doc->setReadOnly(TRUE);

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
  deselectAll(); //!!! this should not be done with persistent selections

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
  m_doc->setReadOnly(FALSE);

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
     KMessageBox::sorry(this, i18n("ISpell could not be started.\n"
     "Please make sure you have ISpell properly configured and in your PATH."));
  } else if (status == KSpell::Crashed) {
     m_doc->setPseudoModal(0L);
     m_doc->setReadOnly(FALSE);

     // if we marked up the text, clear it now
     if (kspell.kspellMispellCount)
        m_doc->unmarkFound();

     m_doc->updateViews();
     KMessageBox::sorry(this, i18n("ISpell seems to have crashed."));
  } else {
     emit spellcheck_done();
  }
}
