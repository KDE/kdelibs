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

#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <qlist.h>
#include <qstring.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qdropsite.h>
#include <qdragobject.h>

#include <kapp.h>
#include <kconfig.h>
#include <ktoolbar.h>
#include <kstatusbar.h>

#include <ktmainwindow.h>

#include <kwrite/kwrite_doc.h>
#include <kwrite/kwrite_view.h>

#include "ktextprint.h"
#include "kguicommand.h"

class TopLevel : public KTMainWindow {
    //Q_ OBJECT
  public:

    TopLevel(KWriteDoc * = 0L, const QString &path = QString::null);
    ~TopLevel();
    void init(); //initialize caption, status and show

    virtual bool queryClose();
    virtual bool queryExit();

    void loadURL(const QString &url, int flags = 0);

  protected:
    void setupEditWidget(KWriteDoc *);
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();

    virtual void dragEnterEvent( QDragEnterEvent * );
//  virtual void dragMoveEvent( QDragMoveEvent * );
//  virtual void dragLeaveEvent( QDragLeaveEvent * );
    virtual void dropEvent( QDropEvent * );

    KWrite *m_mainview;
    int menuInsert, menuSave;
    int menuUndo, menuRedo, menuUndoHist,
        menuIndent, menuUnindent, menuCleanIndent,
        menuSpell, menuCut, menuPaste, menuReplace;
    int menuVertical, menuShowTB, menuShowSB, menuShowPath;
//    int statusID, toolID, verticalID, indentID;

    KGuiCmdPopup *file, *edit, *options;
    QPopupMenu *recentPopup, *hlPopup, *eolPopup;

    QStrList recentFiles;

    bool hideToolBar;
    bool hideStatusBar;
    bool showPath;

    QTimer *statusbarTimer;

  public slots:
    void openRecent(int);
    void newWindow();
    void newView();
    void closeWindow();
    void quitEditor();

    void configure();
    void keys();
    void toggleToolBar();
    void toggleStatusBar();
    void togglePath();

  public:
    void print(bool dialog);
  public slots:
    void doPrint(KTextPrint &);
    void printNow();
    void printDlg();

    void helpSelected();

    void newCurPos();
    void newStatus();
    void statusMsg(const QString &);
    void timeout();
    void newCaption();
    void newUndo();

    void showHighlight();
    void showEol();

    void slotDropEvent(QDropEvent *);

  //config file functions
  public:
    //common config
    void readConfig(KConfig *);
    void writeConfig(KConfig *);
    //config file
    void readConfig();
  public slots:
    void writeConfig();
  //session management
  public:
    void restore(KConfig *,int);
  protected:
    virtual void readProperties(KConfig *);
    virtual void saveProperties(KConfig *);
    virtual void saveData(KConfig *);

};

#endif
