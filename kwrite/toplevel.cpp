/*
  $Id$

    Copyright (C) 1998, 1999 Jochen Wilhelmy
                             digisnap@cs.tu-berlin.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qkeycode.h>
#include <qdropsite.h>
#include <qtabdialog.h>
#include <qdragobject.h>

#include <kwm.h>
#include <kapp.h>
#include <kurl.h>
#include <kglobal.h>
#include <klocale.h>

#include <ksconfig.h>
#include <kmenubar.h>
#include <kstdaccel.h>
#include <kiconloader.h>

// needed because of QVariant used in kconfig.h
#ifdef Bool
#undef Bool
#endif

#include <kconfig.h>

#include <kwrite/highlight.h>

#include "toplevel.h"
#include "kwdialog.h"
#include "kguicommand.h"

// StatusBar field IDs
#define ID_LINE_COLUMN 1
#define ID_INS_OVR 2
#define ID_MODIFIED 3
#define ID_GENERAL 4

const int toolUndo = 1;
const int toolRedo = 2;
const int toolCut = 3;
const int toolPaste = 4;

//command categories
const int ctFileCommands = 10;

//file commands
const int cmNew             = 1;
const int cmOpen            = 2;
const int cmInsert          = 3;
const int cmSave            = 4;
const int cmSaveAs          = 5;
const int cmPrint           = 6;
const int cmNewWindow       = 7;
const int cmNewView         = 8;
const int cmClose           = 9;

QList<KWriteDoc> docList; //documents
KGuiCmdManager cmdMngr; //manager for user -> gui commands

TopLevel::TopLevel (KWriteDoc *doc, const QString &path)
  : KTMainWindow("KWrite") {

  setMinimumSize(180,120);

  recentFiles.setAutoDelete(TRUE);

  statusbarTimer = new QTimer(this);
  connect(statusbarTimer,SIGNAL(timeout()),this,SLOT(timeout()));

//  connect(kapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(set_colors()));

  if (!doc) {
    doc = new KWriteDoc(HlManager::self(), path); //new doc with default path
    docList.append(doc);
  }
  setupEditWidget(doc);
  setupMenuBar();
  setupToolBar();
  setupStatusBar();

//  readConfig();

  setAcceptDrops(true);
}

TopLevel::~TopLevel() {

//  delete file;
//  delete edit;
//  delete help;
//  delete options;
//  delete recentpopup;
//  delete toolbar;
  if (m_mainview->isLastView()) docList.remove(m_mainview->doc());
}

void TopLevel::init() {

  hideToolBar = !hideToolBar;
  toggleToolBar();
  hideStatusBar = !hideStatusBar;
  toggleStatusBar();
  showPath = !showPath;
  togglePath();
  newCurPos();
  newStatus();
//  newCaption();
  newUndo();

  show();
}

bool TopLevel::queryClose() {
  if (!m_mainview->isLastView()) return true;
  return m_mainview->canDiscard();
//  writeConfig();
}

bool TopLevel::queryExit() {
  writeConfig();
  kapp->config()->sync();

  return true;
}

void TopLevel::loadURL(const QString &url, int flags) {
  m_mainview->loadURL(url,flags);
}


void TopLevel::setupEditWidget(KWriteDoc *doc) {

  m_mainview = new KWrite(doc,this,QString::null,false);

  connect(m_mainview,SIGNAL(newCurPos()),this,SLOT(newCurPos()));
  connect(m_mainview,SIGNAL(newStatus()),this,SLOT(newStatus()));
  connect(m_mainview,SIGNAL(statusMsg(const QString &)),this,SLOT(statusMsg(const QString &)));
  connect(m_mainview,SIGNAL(fileChanged()),this,SLOT(newCaption()));
  connect(m_mainview,SIGNAL(newUndo()),this,SLOT(newUndo()));
  connect(m_mainview->view(),SIGNAL(dropEventPass(QDropEvent *)),this,SLOT(slotDropEvent(QDropEvent *)));

  setView(m_mainview,FALSE);
}

void TopLevel::setupMenuBar() {
  KMenuBar *menubar;
  KGuiCmdPopup *find, *bookmarks;
  QPopupMenu *help, *popup;
  QPixmap pixmap;
  int z;

  KGuiCmdDispatcher *dispatcher;
  dispatcher = new KGuiCmdDispatcher(this, &cmdMngr);
  dispatcher->connectCategory(ctCursorCommands, m_mainview, SLOT(doCursorCommand(int)));
  dispatcher->connectCategory(ctEditCommands, m_mainview, SLOT(doEditCommand(int)));
  dispatcher->connectCategory(ctBookmarkCommands, m_mainview, SLOT(doBookmarkCommand(int)));
  dispatcher->connectCategory(ctStateCommands, m_mainview, SLOT(doStateCommand(int)));

  file =        new KGuiCmdPopup(dispatcher);
  edit =        new KGuiCmdPopup(dispatcher);
  find =        new KGuiCmdPopup(dispatcher);
  bookmarks =   new KGuiCmdPopup(dispatcher);
  options =     new KGuiCmdPopup(dispatcher);
  help =        new QPopupMenu();
  recentPopup = new QPopupMenu();

//    int addCommand(int catNum, int cmdNum, QPixmap &pixmap,
//      const QObject *receiver, const char *member, int id = -1, int index = -1);

  pixmap = BarIcon("filenew");
  file->addCommand(ctFileCommands, cmNew, pixmap, m_mainview, SLOT(newDoc()));
  pixmap = BarIcon("fileopen");
  file->addCommand(ctFileCommands, cmOpen, pixmap, m_mainview, SLOT(open()));
  menuInsert = file->addCommand(ctFileCommands, cmInsert, m_mainview, SLOT(insertFile()));
  file->insertItem(i18n("Open &Recent"), recentPopup);
  connect(recentPopup, SIGNAL(activated(int)), SLOT(openRecent(int)));
  file->insertSeparator ();
  pixmap = BarIcon("filefloppy");
  menuSave = file->addCommand(ctFileCommands, cmSave, pixmap, m_mainview, SLOT(save()));
  file->addCommand(ctFileCommands, cmSaveAs, m_mainview, SLOT(saveAs()));
  file->insertSeparator ();
  pixmap = BarIcon("fileprint");
  file->addCommand(ctFileCommands, cmPrint, pixmap, this, SLOT(printDlg()));
  file->insertSeparator ();
  file->addCommand(ctFileCommands, cmNewWindow, this, SLOT(newWindow()));
  file->addCommand(ctFileCommands, cmNewView, this, SLOT(newView()));
  file->insertSeparator ();
  pixmap = BarIcon("exit");
  file->addCommand(ctFileCommands, cmClose, pixmap, this, SLOT(closeWindow()));

/*
  file->insertItem(i18n("&New..."),m_mainview,SLOT(newDoc()),keys.openNew());
  file->insertItem(i18n("&Open..."),m_mainview,SLOT(open()),keys.open());
  menuInsert = file->insertItem(i18n("&Insert..."),m_mainview,SLOT(insertFile()));
  file->insertItem(i18n("Open Recen&t"), recentPopup);
  connect(recentPopup,SIGNAL(activated(int)),SLOT(openRecent(int)));
  file->insertSeparator ();
  menuSave = file->insertItem(i18n("&Save"),m_mainview,SLOT(save()),keys.save());
  file->insertItem(i18n("S&ave as..."),m_mainview,SLOT(saveAs()));
  file->insertSeparator ();
  file->insertItem(i18n("&Print..."), m_mainview,SLOT(print()),keys.print());
  file->insertSeparator ();
//  file->insertItem (i18n("&Mail..."),this,SLOT(mail()));
//  file->insertSeparator ();
  file->insertItem (i18n("New &Window"),this,SLOT(newWindow()));
  file->insertItem (i18n("New &View"),this,SLOT(newView()));
  file->insertSeparator ();
  file->insertItem(i18n("&Close"),this,SLOT(closeWindow()),keys.close());
//  file->insertItem (i18n("E&xit"),this,SLOT(quitEditor()),keys.quit());
*/

  pixmap = BarIcon("undo");
  menuUndo = edit->addCommand(ctEditCommands, cmUndo, pixmap);
  pixmap = BarIcon("redo");
  menuRedo = edit->addCommand(ctEditCommands, cmRedo, pixmap);
  menuUndoHist = edit->insertItem(i18n("Undo/Redo &History..."),m_mainview,SLOT(undoHistory()));
  edit->insertSeparator();
  pixmap = BarIcon("editcut");
  menuCut = edit->addCommand(ctEditCommands, cmCut, pixmap);
  pixmap = BarIcon("editcopy");
  edit->addCommand(ctEditCommands, cmCopy, pixmap);
  pixmap = BarIcon("editpaste");
  menuPaste = edit->addCommand(ctEditCommands, cmPaste, pixmap);
//  edit->insertSeparator();
//  pixmap = BarIcon("search");
//  edit->addCommand(ctFindCommands, cmFind, pixmap, m_mainview, SLOT(search()));
//  edit->addCommand(ctFindCommands, cmReplace, m_mainview, SLOT(replace()));
//  edit->addCommand(ctFindCommands, cmFindAgain, m_mainview, SLOT(searchAgain()));
//  edit->addCommand(ctFindCommands, cmGotoLine, m_mainview, SLOT(gotoLine()));
  edit->insertSeparator();
  edit->addCommand(ctEditCommands, cmIndent);
  edit->addCommand(ctEditCommands, cmUnindent);
  edit->addCommand(ctEditCommands, cmCleanIndent);
  edit->insertSeparator();
  edit->addCommand(ctEditCommands, cmSelectAll);
  edit->addCommand(ctEditCommands, cmDeselectAll);
  edit->addCommand(ctEditCommands, cmInvertSelection);
  edit->insertSeparator();
  pixmap = BarIcon("spellcheck");
  menuSpell = edit->insertItem(pixmap, i18n("Check Spe&lling..."), m_mainview,SLOT(spellcheck()));


  // find dialog
  pixmap = BarIcon("search");
  find->addCommand(ctFindCommands, cmFind, pixmap, m_mainview, SLOT(find()));
  find->addCommand(ctFindCommands, cmReplace, m_mainview, SLOT(replace()));
  find->addCommand(ctFindCommands, cmFindAgain, m_mainview, SLOT(findAgain()));
  find->addCommand(ctFindCommands, cmGotoLine, m_mainview, SLOT(gotoLine()));

/*
  menuCut = edit->insertItem(i18n("C&ut"),m_mainview,SLOT(cut()),keys.cut());
  edit->insertItem(i18n("&Copy"),m_mainview,SLOT(copy()),keys.copy());
  menuPaste = edit->insertItem(i18n("&Paste"),m_mainview,SLOT(paste()),keys.paste());
  edit->insertSeparator();
  edit->insertItem(i18n("C&ut"),m_mainview,SLOT(cut()), keys.cut());
  edit->insertItem(i18n("&Copy"),m_mainview,SLOT(copy()), keys.copy());
  edit->insertItem(i18n("&Paste"),m_mainview,SLOT(paste()), keys.paste());
  edit->insertSeparator();
  edit->insertItem(i18n("&Find..."),m_mainview,SLOT(search()),keys.find());
  menuReplace = edit->insertItem(i18n("&Replace..."),m_mainview,SLOT(replace()),keys.replace());
  edit->insertItem(i18n("Find &Again"),m_mainview,SLOT(searchAgain()),Key_F3);
  edit->insertItem(i18n("&Goto Line..."),m_mainview,SLOT(gotoLine()),CTRL+Key_G);
  edit->insertSeparator();
  menuUndo = edit->insertItem(i18n("U&ndo"),m_mainview,SLOT(undo()),keys.undo());
  menuRedo = edit->insertItem(i18n("R&edo"),m_mainview,SLOT(redo()),CTRL+Key_Y);
  menuUndoHist = edit->insertItem(i18n("Undo/Redo &History..."),m_mainview,SLOT(undoHistory()));
  edit->insertSeparator();
  menuIndent = edit->insertItem(i18n("&Indent"),m_mainview,SLOT(indent()),CTRL+Key_I);
  menuUnindent = edit->insertItem(i18n("Uninden&t"),m_mainview,SLOT(unIndent()),CTRL+Key_U);
  menuCleanIndent = edit->insertItem(i18n("C&lean Indentation"),m_mainview,SLOT(cleanIndent()));
  edit->insertSeparator();
//  edit->insertItem(i18n("Format..."),m_mainview,SLOT(format()));
//  edit->insertSeparator();
  edit->insertItem(i18n("&Select All"),m_mainview,SLOT(selectAll()));
  edit->insertItem(i18n("&Deselect All"),m_mainview,SLOT(deselectAll()));
  edit->insertItem(i18n("In&vert Selection"),m_mainview,SLOT(invertSelection()));
  edit->insertSeparator();
  menuSpell = edit->insertItem(i18n("Spe&llcheck..."), m_mainview,SLOT(spellcheck()));
//  edit->insertSeparator();
//  edit->insertItem(i18n("Insert &Date"),this,SLOT(insertDate()));
//  edit->insertItem(i18n("Insert &Time"),this,SLOT(insertTime()));
*/

//  bookmarks->insertItem(i18n("&Set Bookmark..."),m_mainview,SLOT(setBookmark()),ALT+Key_S);
//  bookmarks->insertItem(i18n("&Add Bookmark"),m_mainview,SLOT(addBookmark()));
//  bookmarks->insertItem(i18n("&Clear Bookmarks"),m_mainview,SLOT(clearBookmarks()),ALT+Key_C);
  //m_mainview->installBMPopup(bookmarks);

  //highlight selector
  hlPopup = new QPopupMenu();
  hlPopup->setCheckable(true);
  HlManager * hlManager = HlManager::self();
  for (z = 0; z < hlManager->highlights(); z++) {
    hlPopup->insertItem(i18n(hlManager->hlName(z)),z);
  }
  connect(hlPopup,SIGNAL(activated(int)),m_mainview,SLOT(setHl(int)));

  // end of line selector
  eolPopup = new QPopupMenu();
  eolPopup->setCheckable(true);
  eolPopup->insertItem("Unix", eolUnix);
  eolPopup->insertItem("Macintosh", eolMacintosh);
  eolPopup->insertItem("Windows/Dos", eolDos);
  connect(eolPopup,SIGNAL(activated(int)),m_mainview,SLOT(setEol(int)));

  options->setCheckable(TRUE);
  options->insertItem(i18n("Set Highlight"),hlPopup);
  connect(hlPopup,SIGNAL(aboutToShow()),this,SLOT(showHighlight()));
  options->insertItem(i18n("&Defaults..."),m_mainview,SLOT(hlDef()));
  options->insertItem(i18n("&Highlight..."),m_mainview,SLOT(hlDlg()));
//  indentID = options->insertItem(i18n("Auto &Indent"),this,SLOT(toggle_indent_mode()));
//  options->insertSeparator();
//  options->insertItem(i18n("&Options..."),m_mainview,SLOT(optDlg()));
  options->insertItem(i18n("&Colors..."),m_mainview,SLOT(colDlg()));
  options->insertSeparator();
  options->insertItem(i18n("&Configure..."), this, SLOT(configure()));
  options->insertItem(i18n("&Keys..."), this, SLOT(keys()));
  options->insertItem(i18n("End Of Line"),eolPopup);
  connect(eolPopup,SIGNAL(aboutToShow()),this,SLOT(showEol()));
//  options->insertItem(i18n("&Keys..."), this, SLOT(keyDlg()));
  options->insertSeparator();
  menuVertical = options->addCommand(ctStateCommands, cmToggleVertical);
    //Item(i18n("&Vertical Selections"),m_mainview,SLOT(toggleVertical()),Key_F5);
  menuShowTB = options->insertItem(i18n("Show &Toolbar"),this,SLOT(toggleToolBar()));
  menuShowSB = options->insertItem(i18n("Show &Statusbar"),this,SLOT(toggleStatusBar()));
  menuShowPath = options->insertItem(i18n("Show &Path"),this,SLOT(togglePath()));
  options->insertItem(i18n("Save Config"),this,SLOT(writeConfig()));
//  options->insertSeparator();
//  options->insertItem(i18n("Save Options"),this,SLOT(save_options()));

  help = helpMenu( i18n("KWrite 1.0.0\n\nCopyright 1998, 1999\nJochen Wilhelmy\ndigisnap@cs.tu-berlin.de"));

  //right mouse button popup
  popup = new QPopupMenu();

  pixmap = BarIcon("fileopen");
  popup->insertItem(QIconSet(pixmap), i18n("&Open..."),m_mainview,SLOT(open()));
  pixmap = BarIcon("filefloppy");
  popup->insertItem(QIconSet(pixmap), i18n("&Save"),m_mainview,SLOT(save()));
  popup->insertItem(i18n("S&ave as..."),m_mainview,SLOT(saveAs()));
  popup->insertSeparator();
/*  pixmap = BarIcon("undo");
  popup->insertItem(QIconSet(pixmap), i18n("&Undo"),m_mainview,SLOT(undo()));
  pixmap = BarIcon("redo");
  popup->insertItem(QIconSet(pixmap), i18n("R&edo"),m_mainview,SLOT(redo()));
  popup->insertSeparator();*/
  pixmap = BarIcon("editcut");
  popup->insertItem(QIconSet(pixmap), i18n("C&ut"),m_mainview,SLOT(cut()));
  pixmap = BarIcon("editcopy");
  popup->insertItem(QIconSet(pixmap), i18n("&Copy"),m_mainview,SLOT(copy()));
  pixmap = BarIcon("editpaste");
  popup->insertItem(QIconSet(pixmap), i18n("&Paste"),m_mainview,SLOT(paste()));
  m_mainview->installRBPopup(popup);

  menubar = menuBar();
  menubar->insertItem(i18n("&File"), file);
  menubar->insertItem(i18n("&Edit"), edit);
  menubar->insertItem(i18n("&Search"), find);
  menubar->insertItem(i18n("&Bookmarks"), bookmarks);
  menubar->insertItem(i18n("&Options"), options);
  menubar->insertSeparator();
  menubar->insertItem(i18n("&Help"), help);
}

void TopLevel::setupToolBar(){
  KToolBar *toolbar;

  toolbar = toolBar();//new KToolBar(this);

  QPixmap pixmap;

  pixmap = BarIcon("filenew");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    m_mainview,SLOT(newDoc()),TRUE,i18n("New"));

  pixmap = BarIcon("fileopen");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    m_mainview,SLOT(open()),TRUE,i18n("Open"));

  pixmap = BarIcon("filefloppy");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    m_mainview,SLOT(save()),TRUE,i18n("Save"));

  toolbar->insertSeparator();

  pixmap = BarIcon("editcut");
  toolbar->insertButton(pixmap,toolCut,SIGNAL(clicked()),
    m_mainview,SLOT(cut()),TRUE,i18n("Cut"));

  pixmap = BarIcon("editcopy");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    m_mainview,SLOT(copy()),TRUE,i18n("Copy"));

  pixmap = BarIcon("editpaste");
  toolbar->insertButton(pixmap,toolPaste,SIGNAL(clicked()),
    m_mainview,SLOT(paste()),TRUE,i18n("Paste"));

  toolbar->insertSeparator();

  pixmap = BarIcon("undo");
  toolbar->insertButton(pixmap,toolUndo,SIGNAL(clicked()),
    m_mainview,SLOT(undo()),TRUE,i18n("Undo"));

  pixmap = BarIcon("redo");
  toolbar->insertButton(pixmap,toolRedo,SIGNAL(clicked()),
    m_mainview,SLOT(redo()),TRUE,i18n("Redo"));

  toolbar->insertSeparator();

  pixmap = BarIcon("fileprint");
  toolbar->insertButton(pixmap, 0, SIGNAL(clicked()),
    this, SLOT(printNow()), TRUE, i18n("Print Document"));

/*
  pixmap = BarIcon("send");
  toolbar->insertButton(pixmap, 0,
                      SIGNAL(clicked()), this,
                      SLOT(mail()), TRUE, i18n("Mail Document"));

*/
  toolbar->insertSeparator();
  pixmap = BarIcon("help");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    this,SLOT(helpSelected()),TRUE,i18n("Help"));

  toolbar->setBarPos(KToolBar::Top);
}

void TopLevel::setupStatusBar(){
    KStatusBar *statusbar;
    statusbar = statusBar();//new KStatusBar( this );
    statusbar->insertItem("Line:000000 Col: 000", ID_LINE_COLUMN);
    statusbar->insertItem("XXX", ID_INS_OVR);
    statusbar->insertItem("*", ID_MODIFIED);
    statusbar->insertItem("", ID_GENERAL);

    //statusbar->setInsertOrder(KStatusBar::RightToLeft);
    //statusbar->setAlignment(ID_INS_OVR, AlignCenter);
    //statusbar->setAlignment(ID_MODIFIED, AlignCenter);

    //    statusbar->setInsertOrder(KStatusBar::LeftToRight);
    //    statusbar->setBorderWidth(1);

//    setStatusBar( statusbar );
}


void TopLevel::openRecent(int id) {
  if (m_mainview->canDiscard()) m_mainview->loadURL(recentPopup->text(id));
}

void TopLevel::newWindow() {

  TopLevel *t = new TopLevel(0L, m_mainview->fileName());
  t->readConfig();
  t->init();
//  t->m_mainview->doc()->inheritFileName(m_mainview->doc());
}

void TopLevel::newView() {

  TopLevel *t = new TopLevel(m_mainview->doc());
  t->readConfig();
  t->m_mainview->copySettings(m_mainview);
  t->init();
}


void TopLevel::closeWindow() {
  close();
}


void TopLevel::quitEditor() {

//  writeConfig();
  kapp->quit();
}

void TopLevel::configure() {
  KWM kwm;
  // I read that no widgets should be created on the stack
  QTabDialog *qtd = new QTabDialog(this, "tabdialog", TRUE);

  qtd->setCaption(i18n("Configure KWrite"));

  // indent options
  IndentConfigTab *indentConfig = new IndentConfigTab(qtd, m_mainview);
  qtd->addTab(indentConfig, i18n("Indent"));

  // select options
  SelectConfigTab *selectConfig = new SelectConfigTab(qtd, m_mainview);
  qtd->addTab(selectConfig, i18n("Select"));

  // edit options
  EditConfigTab *editConfig = new EditConfigTab(qtd, m_mainview);
  qtd->addTab(editConfig, i18n("Edit"));

  // spell checker
  KSpellConfig *ksc = new KSpellConfig(qtd, 0L, m_mainview->ksConfig());
  qtd->addTab(ksc, i18n("Spellchecker"));

  // keys
  //this still lacks layout management, so the tabdialog does not
  //make it fit
//  KGuiCmdConfigTab *keys = new KGuiCmdConfigTab(qtd, &cmdMngr);
//  qtd->addTab(keys, i18n("Keys"));

  qtd->setOkButton(i18n("OK"));
  qtd->setCancelButton(i18n("Cancel"));

  // Is there a _right_ way to do this?
  // yes: don´t do it :)
//  qtd->setMinimumSize (ksc.sizeHint().width() + qtd->sizeHint().width(),
//          ksc.sizeHint().height() + qtd->sizeHint().height());
  kwm.setMiniIcon(qtd->winId(), kapp->miniIcon());

  if (qtd->exec()) {
    // indent options
    indentConfig->getData(m_mainview);
    // select options
    selectConfig->getData(m_mainview);
    // edit options
    editConfig->getData(m_mainview);
    // spell checker
    ksc->writeGlobalSettings();
    m_mainview->setKSConfig(*ksc);
    // keys
//    cmdMngr.changeAccels();
//    cmdMngr.writeConfig(kapp->config());
//  } else {
//    // cancel keys
//    cmdMngr.restoreAccels();
  }

  delete qtd;
}

void TopLevel::keys() {

  KDialogBase *dlg = new KDialogBase(this, "keys", true,
    i18n("Configure Keybindings"), KDialogBase::Ok | KDialogBase::Cancel,
    KDialogBase::Ok);

  // keys
  //this still lacks layout management, so the tabdialog does not
  //make it fit
  KGuiCmdConfigTab *keys = new KGuiCmdConfigTab(dlg, &cmdMngr);
  keys->resize(450, 290);
  dlg->setMainWidget(keys);
  dlg->resize(450, 315);

  if (dlg->exec()) {
    // change keys
    cmdMngr.changeAccels();
    cmdMngr.writeConfig(kapp->config());
  } else {
    // cancel keys
    cmdMngr.restoreAccels();
  }

  delete dlg;
}



void TopLevel::toggleToolBar() {

  options->setItemChecked(menuShowTB,hideToolBar);
  if (hideToolBar) {
    hideToolBar = FALSE;
    enableToolBar(KToolBar::Show);
  } else {
    hideToolBar = TRUE;
    enableToolBar(KToolBar::Hide);
  }
}

/*
void TopLevel::keyDlg() {
  QDialog *dlg;

//  cmdMngr.saveAccels();
  dlg = new KGuiCmdConfig(&cmdMngr, this);
  dlg->setCaption(i18n("Key Bindings"));
  if (dlg->exec() == QDialog::Accepted) {
    cmdMngr.changeAccels();
    cmdMngr.writeConfig(kapp->config());
  } else cmdMngr.restoreAccels();
  delete dlg;
}
*/

void TopLevel::toggleStatusBar() {

  options->setItemChecked(menuShowSB, hideStatusBar);
  if (hideStatusBar) {
    hideStatusBar = FALSE;
    enableStatusBar(KStatusBar::Show);
  } else {
    hideStatusBar = TRUE;
    enableStatusBar(KStatusBar::Hide);
  }
}

void TopLevel::togglePath() {

  showPath = !showPath;
  options->setItemChecked(menuShowPath, showPath);
  newCaption();
}


void TopLevel::print(bool dialog) {
  QString title = m_mainview->fileName();
  if (!showPath) {
    int pos = title.findRev('/');
    if (pos != -1) {
      title = title.right(title.length() - pos - 1);
    }
  }

  KTextPrintConfig::print(this, kapp->config(), dialog, title,
    m_mainview->numLines(), this, SLOT(doPrint(KTextPrint &)));
}

void TopLevel::doPrint(KTextPrint &printer) {
  KWriteDoc *doc = m_mainview->doc();

  int z, numAttribs;
  Attribute *a;
  int line, attr, nextAttr, oldZ;
  TextLine *textLine;
  const QChar *s;

//  printer.setTitle(m_doc->fileName());
  printer.setTabWidth(doc->tabWidth());

  numAttribs = doc->numAttribs();
  a = doc->attribs();
  for (z = 0; z < numAttribs; z++) {
    printer.defineColor(z, a[z].col.red(), a[z].col.green(), a[z].col.blue());
  }

  printer.begin();

  line = 0;
  attr = -1;
  while (true) {
    textLine = doc->textLine(line);
    s = textLine->getText();
//    printer.print(s, textLine->length());
    oldZ = 0;
    for (z = 0; z < textLine->length(); z++) {
      nextAttr = textLine->getAttr(z);
      if (nextAttr != attr) {
        attr = nextAttr;
        printer.print(&s[oldZ], z - oldZ);
        printer.setColor(attr);
        int fontStyle = 0;
        if (a[attr].font.bold()) fontStyle |= KTextPrint::Bold;
        if (a[attr].font.italic()) fontStyle |= KTextPrint::Italics;
        printer.setFontStyle(fontStyle);
        oldZ = z;
      }
    }
    printer.print(&s[oldZ], z - oldZ);

    line++;
    if (line == doc->numLines()) break;
    printer.newLine();
  }

  printer.end();
}

void TopLevel::printNow() {
  print(false);
}

void TopLevel::printDlg() {
  print(true);
}

void TopLevel::helpSelected() {
  kapp->invokeHTMLHelp( "" , "" );
}

void TopLevel::newCurPos() {
  statusBar()->changeItem(QString("%1: %2 %3: %4")
    .arg(i18n("Line"))
    .arg(m_mainview->currentLine() +1)
    .arg(i18n("Col"))
    .arg(m_mainview->currentColumn() +1),
    ID_LINE_COLUMN);
}

void TopLevel::newStatus() {
  int config;
  bool readOnly;

  readOnly = m_mainview->isReadOnly();

  config = m_mainview->config();
  options->setItemChecked(menuVertical,config & cfVerticalSelect);

  if (readOnly)
    statusBar()->changeItem("R/O",ID_INS_OVR);
  else
    statusBar()->changeItem(config & cfOvr ? "OVR" : "INS",ID_INS_OVR);

  statusBar()->changeItem(m_mainview->isModified() ? "*" : "",ID_MODIFIED);

  file->setItemEnabled(menuInsert,!readOnly);
  file->setItemEnabled(menuSave,!readOnly);

  edit->setItemEnabled(menuIndent,!readOnly);
  edit->setItemEnabled(menuUnindent,!readOnly);
  edit->setItemEnabled(menuCleanIndent,!readOnly);
  edit->setItemEnabled(menuSpell,!readOnly);
  edit->setItemEnabled(menuCut,!readOnly);
  edit->setItemEnabled(menuPaste,!readOnly);
  edit->setItemEnabled(menuReplace,!readOnly);

  toolBar()->setItemEnabled(toolCut,!readOnly);
  toolBar()->setItemEnabled(toolPaste,!readOnly);

  newUndo();
}

void TopLevel::statusMsg(const QString &msg) {
  statusbarTimer->stop();
  statusBar()->changeItem(msg, ID_GENERAL);
  statusbarTimer->start(10000, true); //single shot
}

void TopLevel::timeout() {
  statusBar()->changeItem("", ID_GENERAL);
}

void TopLevel::newCaption() {
  QString caption;
  int z;

  if (m_mainview->hasFileName()) {
    caption = m_mainview->fileName();
    //set recent files popup menu
    z = (int) recentPopup->count();
    while (z > 0) {
      z--;
      if (caption == recentPopup->text(recentPopup->idAt(z)))
      recentPopup->removeItemAt(z);
    }
    recentPopup->insertItem(caption, 0, 0);
    if (recentPopup->count() > 5) recentPopup->removeItemAt(5);
    for (z = 0; z < 5; z++) recentPopup->setId(z, z);

    //set caption
    if (!showPath) {
      int pos = caption.findRev('/');
      if (pos != -1) {
        caption = caption.right( caption.length() - pos - 1);
      }
    }
    setCaption(caption);
  } else {
    setCaption(kapp->caption());
  }
}

void TopLevel::newUndo() {
  int state, uType, rType;
  QString t;

  state = m_mainview->undoState();

  edit->setItemEnabled(menuUndoHist,(state & 1 || state & 2));

  t = cmdMngr.getCommand(ctEditCommands, cmUndo)->getName();
  if (state & 1) {
    uType = m_mainview->nextUndoType();
    edit->setItemEnabled(menuUndo,true);
    toolBar()->setItemEnabled(toolUndo,true);

    t += ' ';
    t += i18n(m_mainview->undoTypeName(uType));
  } else {
    edit->setItemEnabled(menuUndo,false);
    toolBar()->setItemEnabled(toolUndo,false);
  }
  edit->setText(t, menuUndo);

  t = cmdMngr.getCommand(ctEditCommands, cmRedo)->getName();
  if (state & 2) {
    rType = m_mainview->nextRedoType();
    edit->setItemEnabled(menuRedo,true);
    toolBar()->setItemEnabled(toolRedo,true);

    t += ' ';
    t += i18n(m_mainview->undoTypeName(rType));
  } else {
    edit->setItemEnabled(menuRedo,false);
    toolBar()->setItemEnabled(toolRedo,false);
  }
  edit->setText(t, menuRedo);
}

void TopLevel::dragEnterEvent( QDragEnterEvent *event )
{
  event->accept(QUriDrag::canDecode(event));
}

void TopLevel::dropEvent( QDropEvent *event )
{
  slotDropEvent(event);
}

void TopLevel::slotDropEvent( QDropEvent *event )
{
  QStrList  urls;

  if (QUriDrag::decode(event, urls)) {
    debug("TopLevel:Handling QUriDrag...");
    char *s;
    for (s = urls.first(); s != 0L; s = urls.next()) {
      // Load the first file in this window
      if (s == urls.getFirst() && !m_mainview->isModified() && !m_mainview->isReadOnly()) {
        loadURL(s);
      } else {
        TopLevel *t = new TopLevel();
        t->readConfig();
        t->loadURL(s);
        t->init();
      }
    }
  }
}

void TopLevel::showHighlight() {
  int hl = m_mainview->getHl();

  for (int index = 0; index < (int) hlPopup->count(); index++)
    hlPopup->setItemChecked(index, hl == index);
}

void TopLevel::showEol() {
  int eol = m_mainview->getEol();

  for (int index = 0; index < (int) eolPopup->count(); index++)
    eolPopup->setItemChecked(index, eol == index);
}

//common config
void TopLevel::readConfig(KConfig *config) {
  int z;
  char name[16];
  QString s;

  hideToolBar = config->readNumEntry("HideToolBar");
  hideStatusBar = config->readNumEntry("HideStatusBar");
  showPath = config->readNumEntry("ShowPath");

  for (z = 0; z < 5; z++) {
    sprintf(name, "Recent%d", z + 1);
    s = config->readEntry(name);
    if (!s.isEmpty()) recentPopup->insertItem(s);
  }
}

void TopLevel::writeConfig(KConfig *config) {
  int z;
  char name[16];

  config->writeEntry("HideToolBar",hideToolBar);
  config->writeEntry("HideStatusBar",hideStatusBar);
  config->writeEntry("ShowPath",showPath);

  for (z = 0; z < (int) recentPopup->count(); z++) {
    sprintf(name, "Recent%d", z + 1);
    config->writeEntry(name, recentPopup->text(recentPopup->idAt(z)));
  }
}

//config file
void TopLevel::readConfig() {
  KConfig *config;
  int w, h;

  config = kapp->config();

  config->setGroup("General Options");
  w = config->readNumEntry("Width", 550);
  h = config->readNumEntry("Height", 400);
  resize(w, h);

  readConfig(config);
//  hideToolBar = config->readNumEntry("HideToolBar");
//  hideStatusBar = config->readNumEntry("HideStatusBar");

  m_mainview->readConfig(config);
  m_mainview->doc()->readConfig(config);
}

void TopLevel::writeConfig() {
  KConfig *config;

  config = kapp->config();

  config->setGroup("General Options");
  config->writeEntry("Width", width());
  config->writeEntry("Height", height());

  writeConfig(config);
//  config->writeEntry("HideToolBar",hideToolBar);
//  config->writeEntry("HideStatusBar",hideStatusBar);

  m_mainview->writeConfig(config);
  m_mainview->doc()->writeConfig(config);
}

// session management
void TopLevel::restore(KConfig *config, int n) {

  if (m_mainview->isLastView() && m_mainview->hasFileName()) { //in this case first view
    loadURL(m_mainview->fileName(), lfNoAutoHl);
  }
  readPropertiesInternal(config, n);
  init();
//  show();
}

void TopLevel::readProperties(KConfig *config) {

  readConfig(config);
  m_mainview->readSessionConfig(config);
}

void TopLevel::saveProperties(KConfig *config) {

  writeConfig(config);
  config->writeEntry("DocumentNumber",docList.find(m_mainview->doc()) + 1);
  m_mainview->writeSessionConfig(config);
#warning fix session management
#if 0
  setUnsavedData(m_mainview->isModified());
#endif
}

void TopLevel::saveData(KConfig *config) { //save documents
  int z;
  char buf[16];
  KWriteDoc *doc;

  config->setGroup("Number");
  config->writeEntry("NumberOfDocuments",docList.count());

  for (z = 1; z <= (int) docList.count(); z++) {
     sprintf(buf,"Document%d",z);
     config->setGroup(buf);
     doc = docList.at(z - 1);
     doc->writeSessionConfig(config);
  }
}

//restore session
void restore() {
  KConfig *config;
  int docs, windows, z;
  char buf[16];
  KWriteDoc *doc;
  TopLevel *t;

  config = kapp->sessionConfig();
  if (!config) return;

  config->setGroup("Number");
  docs = config->readNumEntry("NumberOfDocuments");
  windows = config->readNumEntry("NumberOfWindows");

  for (z = 1; z <= docs; z++) {
     sprintf(buf,"Document%d",z);
     config->setGroup(buf);
     doc = new KWriteDoc(HlManager::self());
     doc->readSessionConfig(config);
     docList.append(doc);
  }

  for (z = 1; z <= windows; z++) {
    sprintf(buf,"%d",z);
    config->setGroup(buf);
    t = new TopLevel(docList.at(config->readNumEntry("DocumentNumber") - 1));
    t->restore(config,z);
  }
}

/*
static void addCursorCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctCursorCommands, I18N_NOOP("Cursor Movement"));
  cmdMngr.setSelectModifiers(Qt::SHIFT, selectFlag, Qt::ALT, multiSelectFlag);
  cmdMngr.addCommand(cmLeft,            I18N_NOOP("Left"            ), Qt::Key_Left, Qt::CTRL+Qt::Key_B);
  cmdMngr.addCommand(cmRight,           I18N_NOOP("Right"           ), Qt::Key_Right, Qt::CTRL+Qt::Key_F);
  cmdMngr.addCommand(cmWordLeft,        I18N_NOOP("Word Left"       ), Qt::CTRL+Qt::Key_Left);
  cmdMngr.addCommand(cmWordRight,       I18N_NOOP("Word Right"      ), Qt::CTRL+Qt::Key_Right);
  cmdMngr.addCommand(cmHome,            I18N_NOOP("Home"            ), Qt::Key_Home, Qt::CTRL+Qt::Key_A, Qt::Key_F27);
  cmdMngr.addCommand(cmEnd,             I18N_NOOP("End"             ), Qt::Key_End, Qt::CTRL+Qt::Key_E, Qt::Key_F33);
  cmdMngr.addCommand(cmUp,              I18N_NOOP("Up"              ), Qt::Key_Up, Qt::CTRL+Qt::Key_P);
  cmdMngr.addCommand(cmDown,            I18N_NOOP("Down"            ), Qt::Key_Down, Qt::CTRL+Qt::Key_N);
  cmdMngr.addCommand(cmScrollUp,        I18N_NOOP("Scroll Up"       ), Qt::CTRL+Qt::Key_Up);
  cmdMngr.addCommand(cmScrollDown,      I18N_NOOP("Scroll Down"     ), Qt::CTRL+Qt::Key_Down);
  cmdMngr.addCommand(cmTopOfView,       I18N_NOOP("Top Of View"     ), Qt::CTRL+Qt::Key_PageUp);
  cmdMngr.addCommand(cmBottomOfView,    I18N_NOOP("Bottom Of View"  ), Qt::CTRL+Qt::Key_PageDown);
  cmdMngr.addCommand(cmPageUp,          I18N_NOOP("Page Up"         ), Qt::Key_PageUp, Qt::Key_F29);
  cmdMngr.addCommand(cmPageDown,        I18N_NOOP("Page Down"       ), Qt::Key_PageDown, Qt::Key_F35);
//  cmdMngr.addCommand(cmCursorPageUp,    I18N_NOOP("Cursor Page Up"  ));
//  cmdMngr.addCommand(cmCursorPageDown,  I18N_NOOP("Cursor Page Down"));
  cmdMngr.addCommand(cmTop,             I18N_NOOP("Top"             ), Qt::CTRL+Qt::Key_Home);
  cmdMngr.addCommand(cmBottom,          I18N_NOOP("Bottom"          ), Qt::CTRL+Qt::Key_End);
  cmdMngr.addCommand(cmLeft | selectFlag, I18N_NOOP("Left + Select"  ) , Qt::SHIFT+Qt::Key_F30);//, Qt::SHIFT+Qt::Key_4);
  cmdMngr.addCommand(cmRight | selectFlag,I18N_NOOP("Right + Select" ) , Qt::SHIFT+Qt::Key_F32);//, Qt::SHIFT+Qt::Key_6);
  cmdMngr.addCommand(cmUp | selectFlag,   I18N_NOOP("Up + Select"    ) , Qt::SHIFT+Qt::Key_F28);//, Qt::SHIFT+Qt::Key_8);
  cmdMngr.addCommand(cmDown | selectFlag, I18N_NOOP("Down + Select"  ) , Qt::SHIFT+Qt::Key_F34);//, Qt::SHIFT+Qt::Key_2);
}

static void addEditCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctEditCommands, I18N_NOOP("Edit Commands"));
  cmdMngr.addCommand(cmReturn,          I18N_NOOP("Return"           ), Qt::Key_Return, Qt::Key_Enter);
  cmdMngr.addCommand(cmDelete,          I18N_NOOP("Delete"           ), Qt::Key_Delete, Qt::CTRL+Qt::Key_D);
  cmdMngr.addCommand(cmBackspace,       I18N_NOOP("Backspace"        ), Qt::Key_Backspace, Qt::CTRL+Qt::Key_H);
  cmdMngr.addCommand(cmKillLine,        I18N_NOOP("Kill Line"        ), Qt::CTRL+Qt::Key_K);
  cmdMngr.addCommand(cmUndo,            I18N_NOOP("&Undo"            ), Qt::CTRL+Qt::Key_Z, Qt::Key_F14);
  cmdMngr.addCommand(cmRedo,            I18N_NOOP("R&edo"            ), Qt::CTRL+Qt::Key_Y, Qt::Key_F12);
  cmdMngr.addCommand(cmCut,             I18N_NOOP("C&ut"             ), Qt::CTRL+Qt::Key_X, Qt::SHIFT+Qt::Key_Delete, Qt::Key_F20);
  cmdMngr.addCommand(cmCopy,            I18N_NOOP("&Copy"            ), Qt::CTRL+Qt::Key_C, Qt::CTRL+Qt::Key_Insert, Qt::Key_F16);
  cmdMngr.addCommand(cmPaste,           I18N_NOOP("&Paste"           ), Qt::CTRL+Qt::Key_V, Qt::SHIFT+Qt::Key_Insert, Qt::Key_F18);
  cmdMngr.addCommand(cmIndent,          I18N_NOOP("&Indent"          ), Qt::CTRL+Qt::Key_I);
  cmdMngr.addCommand(cmUnindent,        I18N_NOOP("Uninden&t"        ), Qt::CTRL+Qt::Key_U);
  cmdMngr.addCommand(cmCleanIndent,     I18N_NOOP("Clean Indent"     ));
  cmdMngr.addCommand(cmSelectAll,       I18N_NOOP("&Select All"      ));
  cmdMngr.addCommand(cmDeselectAll,     I18N_NOOP("&Deselect All"    ));
  cmdMngr.addCommand(cmInvertSelection, I18N_NOOP("In&vert Selection"));
}

static void addFindCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctFindCommands, I18N_NOOP("Find Commands"));
  cmdMngr.addCommand(cmFind,            I18N_NOOP("&Find..."        ) , Qt::CTRL+Qt::Key_F, Qt::Key_F19);
  cmdMngr.addCommand(cmReplace,         I18N_NOOP("&Replace..."     ) , Qt::CTRL+Qt::Key_R);
  cmdMngr.addCommand(cmFindAgain,       I18N_NOOP("Find &Again"     ) , Qt::Key_F3);
  cmdMngr.addCommand(cmGotoLine,        I18N_NOOP("&Goto Line..."   ) , Qt::CTRL+Qt::Key_G);
}

static void addBookmarkCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctBookmarkCommands, I18N_NOOP("Bookmark Commands"));
  cmdMngr.addCommand(cmSetBookmark,       I18N_NOOP("&Set Bookmark..."), Qt::ALT+Qt::Key_X);
  cmdMngr.addCommand(cmAddBookmark,       I18N_NOOP("&Add Bookmark"   ), Qt::ALT+Qt::Key_A);
  cmdMngr.addCommand(cmClearBookmarks,    I18N_NOOP("&Clear Bookmarks" ), Qt::ALT+Qt::Key_C);
  cmdMngr.addCommand(cmSetBookmarks +0,   I18N_NOOP("Set Bookmark 1"  ));
  cmdMngr.addCommand(cmSetBookmarks +1,   I18N_NOOP("Set Bookmark 2"  ));
  cmdMngr.addCommand(cmSetBookmarks +2,   I18N_NOOP("Set Bookmark 3"  ));
  cmdMngr.addCommand(cmSetBookmarks +3,   I18N_NOOP("Set Bookmark 4"  ));
  cmdMngr.addCommand(cmSetBookmarks +4,   I18N_NOOP("Set Bookmark 5"  ));
  cmdMngr.addCommand(cmSetBookmarks +5,   I18N_NOOP("Set Bookmark 6"  ));
  cmdMngr.addCommand(cmSetBookmarks +6,   I18N_NOOP("Set Bookmark 7"  ));
  cmdMngr.addCommand(cmSetBookmarks +7,   I18N_NOOP("Set Bookmark 8"  ));
  cmdMngr.addCommand(cmSetBookmarks +8,   I18N_NOOP("Set Bookmark 9"  ));
  cmdMngr.addCommand(cmSetBookmarks +9,   I18N_NOOP("Set Bookmark 10" ));
  cmdMngr.addCommand(cmGotoBookmarks +0,  I18N_NOOP("Goto Bookmark 1" ), Qt::ALT+Qt::Key_1);
  cmdMngr.addCommand(cmGotoBookmarks +1,  I18N_NOOP("Goto Bookmark 2" ), Qt::ALT+Qt::Key_2);
  cmdMngr.addCommand(cmGotoBookmarks +2,  I18N_NOOP("Goto Bookmark 3" ), Qt::ALT+Qt::Key_3);
  cmdMngr.addCommand(cmGotoBookmarks +3,  I18N_NOOP("Goto Bookmark 4" ), Qt::ALT+Qt::Key_4);
  cmdMngr.addCommand(cmGotoBookmarks +4,  I18N_NOOP("Goto Bookmark 5" ), Qt::ALT+Qt::Key_5);
  cmdMngr.addCommand(cmGotoBookmarks +5,  I18N_NOOP("Goto Bookmark 6" ), Qt::ALT+Qt::Key_6);
  cmdMngr.addCommand(cmGotoBookmarks +6,  I18N_NOOP("Goto Bookmark 7" ), Qt::ALT+Qt::Key_7);
  cmdMngr.addCommand(cmGotoBookmarks +7,  I18N_NOOP("Goto Bookmark 8" ), Qt::ALT+Qt::Key_8);
  cmdMngr.addCommand(cmGotoBookmarks +8,  I18N_NOOP("Goto Bookmark 9" ), Qt::ALT+Qt::Key_9);
  cmdMngr.addCommand(cmGotoBookmarks +9,  I18N_NOOP("Goto Bookmark 10"), Qt::ALT+Qt::Key_0);
}

static void addStateCommands(KGuiCmdManager &cmdMngr) {
  cmdMngr.addCategory(ctStateCommands, I18N_NOOP("State Commands"));
  cmdMngr.addCommand(cmToggleInsert,   I18N_NOOP("Insert Mode"), Qt::Key_Insert);
  cmdMngr.addCommand(cmToggleVertical, I18N_NOOP("&Vertical Selections"), Qt::Key_F5);
}

int main(int argc, char **argv) {
  KGuiCmdApp *a = new KGuiCmdApp( argc, argv, "kwrite" );
  //KApplication a(argc,argv);

  //list that contains all documents
  docList.setAutoDelete(false);

  //init commands
  addCursorCommands(cmdMngr);

  cmdMngr.addCategory(ctFileCommands, I18N_NOOP("File Commands"));
  cmdMngr.addCommand(cmNew,             I18N_NOOP("&New..."    ));
  cmdMngr.addCommand(cmOpen,            I18N_NOOP("&Open..."   ), Qt::CTRL+Qt::Key_O, Qt::Key_F17);
  cmdMngr.addCommand(cmInsert,          I18N_NOOP("&Insert..." ));
  cmdMngr.addCommand(cmSave,            I18N_NOOP("&Save"      ), Qt::CTRL+Qt::Key_S);
  cmdMngr.addCommand(cmSaveAs,          I18N_NOOP("Save &As..."));
  cmdMngr.addCommand(cmPrint,           I18N_NOOP("&Print..."  ), Qt::CTRL+Qt::Key_P);
  cmdMngr.addCommand(cmNewWindow,       I18N_NOOP("New &Window"));
  cmdMngr.addCommand(cmNewView,         I18N_NOOP("New &View"  ));
  cmdMngr.addCommand(cmClose,           I18N_NOOP("&Close"     ), Qt::CTRL+Qt::Key_W, Qt::Key_Escape);
//  cmdMngr.addCommand(cmClose,           "&Quit"      );

  addEditCommands(cmdMngr);
  addFindCommands(cmdMngr);
  addBookmarkCommands(cmdMngr);
  addStateCommands(cmdMngr);


  //todo: insert reading of kde-global keybinding file here
  cmdMngr.makeDefault(); //make keybindings default
  cmdMngr.readConfig(kapp->config());

  if (kapp->isRestored()) {
    restore();
  } else {
    TopLevel *t = new TopLevel();
    t->readConfig();
    if (argc > 1) t->loadURL(argv[1],lfNewFile);
    t->init();
  }
  return a->exec();
}
*/
