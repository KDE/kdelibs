// ---------------------------------------------------------------------------
//
//   QuickHelp: an improved mini-help system for KDE
//
// ---------------------------------------------------------------------------
//
//   This file is part of the KDE libraries
//
//   Copyright (C) 1997 Mario Weilguni <mweilguni@kde.org>
//
//   This library is free software; you can redistribute it and/or
//   modify it under the terms of the GNU Library General Public
//   License as published by the Free Software Foundation; either
//   version 2 of the License, or (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   Library General Public License for more details.
//
//   You should have received a copy of the GNU Library General Public License
//   along with this library; see the file COPYING.LIB.  If not, write to
//   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//   Boston, MA 02111-1307, USA.
//
// ---------------------------------------------------------------------------

#ifndef __QUICKHELP__H__
#define __QUICKHELP__H__

#include <qframe.h>
#include <qlist.h>
#include <qpopupmenu.h>


/**
 * Provides a helpsystem for KDE. It's mainly intended for those parts of the
 * program, where tooltips (@see QToolTip) give too little advice and calling
 * the HTML help viewer is overkill (or just too slow). For example, some 
 * programs have quite complex dialogs, and if you want to provide help you
 * can use a full-blown HTML-helpsystem (and typically nobody really reads it),
 * or tooltips (which provide too less information, and it's nerving to move
 * the mouse over a widget and to wait if a tooltip pops up. The solution to
 * this is QuickHelp (at least for me :-)
 * 
 * Normally, a Quickhelp just looks like a simple QToolTip, but it has some 
 * differences:
 *
 * 1. Tooltips only pop up when you leave the mouse over a widget for a short
 *    period of time. It's not possible to pop up upon request, as it is with
 *    QuickHelp
 *
 * 2. Tooltips do not allow formatting text.
 *
 *
 * Here are the highlights of QuickHelp:
 * o Requires few memory. The amount depends on the length of the help text you
 *   provide, plus a few hundreds of bytes of memory for QuickHelp itself and
 *   a few bytes for each widget you want to use QuickHelp with (8 bytes)
 *
 * o Text formatting: bold, italic, underline, reduce and enlarge font size, basic
 *   colors like red, green... as well as colors provided as RGB values
 *
 * o Hyperlinks, which calls the HTML-viewer to provider more information or 
 *   related information
 *
 * o Shadows. A QuickHelp has a shadow (at least when the underlying X-server 
 *   supports that feature. It is planned to make this configureable.
 *
 * o Font selection: QuickHelp allows to select basic fonts: the default fonts,
 *   the fixed system font currently.
 *
 * o Easy to use: just add a line like this:
 *     QuickHelp::add(myWidget, "text text text");
 *
 * o Web-integration: hyperlinks to ftp:, http:, file:, info:, man: and 
 *   mailto: protokolls are available
 * 
 *
 * 
 * TODO LIST:
 *
 * o make shadow configurable instead of compile time option (easy)
 * o faster painting, avoid to much QFont stuff (fairly easy)
 * o better event handling. Clear the mouse event that closed the
 *   KQuickHelp to avoid the underlaying widgets to get that event.
 * o Maybe all KQuickHelp text in one file?
 * o kdoc compliant documentation
 * o purify
 * 
 */

struct KQuickTip {
  QWidget *widget;
  QString txt;
};

struct KQuickHelp_Token {
  const char *token;
  int tokenID;
};

class KQuickHelp_Link {
public:
  QRect area;
  QString link;
};

class KQuickHelpWindow : public QFrame {
  Q_OBJECT
public:
  KQuickHelpWindow();
  ~KQuickHelpWindow() {}

  void newText();
  void popup(QString text, int atX, int atY);
  
  virtual void mouseMoveEvent(QMouseEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void keyPressEvent(QKeyEvent *);
  virtual void paintEvent(QPaintEvent *);
  void paint(QPainter *p, int &, int &);  
  virtual void show();
  virtual void hide();

signals:
  void hyperlink(QString);

private:
  QString token();

  QString activeLink;
  QCursor defaultCursor;
  int tokIndex;
  QString txt;

  QList<KQuickHelp_Link> linkAreas;
  static QColor stdColors[];
  static KQuickHelp_Token tokens[];
};


class KQuickHelp : public QObject {
  Q_OBJECT
public:  
  KQuickHelp();
  
  static const char *add(QWidget *, const char *);
  static void remove(QWidget *);

private:
  virtual bool eventFilter(QObject *, QEvent *);
  
private slots:
  void getKQuickHelp(int);
  void widgetDestroyed();
  void hyperlinkRequested(QString);

private:
  QWidget *current;
  QString currentText;
  QPoint currentPos;

  static QList<KQuickTip> *tips;
  static KQuickHelp *instance;
  static QPopupMenu *menu;
  static KQuickHelpWindow *window;  
};


#endif
