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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif

#include "kquickhelp.h"

#include <kapp.h>
#include <klocale.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qregexp.h>

// this selects wheter we want shapes or not
#define QH_SHAPE
#ifdef QH_SHAPE
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
  #include <X11/Xos.h>
  #include <X11/extensions/shape.h>
  #define X_SHADOW 6
  #define Y_SHADOW 6
#else
  #define X_SHADOW 0
  #define Y_SHADOW 0
#endif

// text attribute flags
#define F_BOLD 1
#define F_ITALIC 2
#define F_UNDERLINE 4

// predefined border and shadow width
#define X_BORDER 10
#define Y_BORDER 6

// numeric representation of allowed tokens
const int TOK_BOLD_ON = 1;
const int TOK_BOLD_OFF = 2;
const int TOK_ITALIC_ON = 3;
const int TOK_ITALIC_OFF = 4;
const int TOK_UNDERLINE_ON = 5;
const int TOK_UNDERLINE_OFF = 6;
const int TOK_FONTSIZE_PLUS = 7;
const int TOK_FONTSIZE_MINUS = 8;
const int TOK_COLOR_BY_RGB = 9;
const int TOK_COLOR_RED = 10;
const int TOK_COLOR_GREEN = 11;
const int TOK_COLOR_BLUE = 12;
const int TOK_COLOR_WHITE = 13;
const int TOK_COLOR_YELLOW = 14;
const int TOK_COLOR_BLACK = 15;
const int TOK_COLOR_BROWN = 16;
const int TOK_COLOR_MAGENTA = 17;
const int TOK_COLOR_CYAN = 18;
const int TOK_FONT_DEFAULT = 19;
const int TOK_FONT_FIXED = 20;
const int TOK_INDENT_PLUS = 21;
const int TOK_INDENT_MINUS = 22;
const int TOK_NEWLINE = 23;
const int TOK_LINK = 24;
const int TOK_ENDLINK = 25;


QPopupMenu *KQuickHelp::menu = 0;
KQuickHelp *KQuickHelp::instance = 0;
QList<KQuickTip> *KQuickHelp::tips;
KQuickHelpWindow *KQuickHelp::window = 0;


// the standard colors
QColor *KQuickHelpWindow::stdColors = 0; 
/*
  red,
  green,
  blue,
  white,
  yellow,
  black,
  QColor(165, 42, 42),
  magenta,
  cyan
};
*/

KQuickHelp_Token KQuickHelpWindow::tokens[] = {
  // font attributes
  {"<b>", TOK_BOLD_ON},
  {"</b>", TOK_BOLD_OFF},
  {"<i>", TOK_ITALIC_ON},
  {"</i>", TOK_ITALIC_OFF},
  {"<u>", TOK_UNDERLINE_ON},
  {"</u>", TOK_UNDERLINE_OFF},

  // font attributes (long)
  {"<bold>", TOK_BOLD_ON},
  {"</bold>", TOK_BOLD_OFF},
  {"<italic>", TOK_ITALIC_ON},
  {"</italic>", TOK_ITALIC_OFF},
  {"<underline>", TOK_UNDERLINE_ON},
  {"</underline>", TOK_UNDERLINE_OFF},

  // font size
  {"<FONT SIZE +>", TOK_FONTSIZE_PLUS},
  {"<FONT SIZE ->", TOK_FONTSIZE_MINUS},
  {"<+>", TOK_FONTSIZE_PLUS},
  {"<->", TOK_FONTSIZE_MINUS},

  // indentation
  {"<INDENT +>", TOK_INDENT_PLUS},
  {"<INDENT ->", TOK_INDENT_MINUS},
  {"<i+>", TOK_INDENT_PLUS},
  {"<i->", TOK_INDENT_MINUS},

  // color selection by RGB value
  {"<COLOR #", TOK_COLOR_BY_RGB},

  // color selection by color name
  {"<COLOR RED>", TOK_COLOR_RED},
  {"<COLOR GREEN>", TOK_COLOR_GREEN},
  {"<COLOR BLUE>", TOK_COLOR_BLUE},
  {"<COLOR WHITE>", TOK_COLOR_WHITE},
  {"<COLOR YELLOW>", TOK_COLOR_YELLOW},
  {"<COLOR BLACK>", TOK_COLOR_BLACK},
  {"<COLOR BROWN>", TOK_COLOR_BROWN},
  {"<COLOR MAGENTA>", TOK_COLOR_MAGENTA},
  {"<COLOR CYAN>", TOK_COLOR_CYAN},

  // color selection by color name (short form)
  {"<RED>", TOK_COLOR_RED},
  {"<GREEN>", TOK_COLOR_GREEN},
  {"<BLUE>", TOK_COLOR_BLUE},
  {"<WHITE>", TOK_COLOR_WHITE},
  {"<YELLOW>", TOK_COLOR_YELLOW},
  {"<BLACK>", TOK_COLOR_BLACK},
  {"<BROWN>", TOK_COLOR_BROWN},
  {"<MAGENTA>", TOK_COLOR_MAGENTA},
  {"<CYAN>", TOK_COLOR_CYAN},

  // font selection
  {"<FONT DEFAULT>", TOK_FONT_DEFAULT},
  {"<FONT FIXED>", TOK_FONT_FIXED},
  {"<DEFAULT>", TOK_FONT_DEFAULT},
  {"<FIXED>", TOK_FONT_FIXED},

  // newline
  {"<br>", TOK_NEWLINE},

  // hyperlinks
  {"<link ", TOK_LINK},
  {"</link>", TOK_ENDLINK},
  
  // token table terminator
  {0, 0}
};


KQuickHelp::KQuickHelp() : QObject(0) {
}


const QString& KQuickHelp::add(QWidget *w, const QString& s) {
    if (s.isNull())
	return QString::null;

  if(w != 0) {
    // make sure we have  a class instance running
    if(instance == 0) {
      tips = new QList<KQuickTip>;
      instance = new KQuickHelp();
      window = new KQuickHelpWindow();
      menu = new QPopupMenu;
      menu->insertItem(i18n("Quickhelp"));
      connect(menu, SIGNAL(activated(int)),
	      instance, SLOT(getKQuickHelp(int)));
      connect(window, SIGNAL(hyperlink(QString)),
	      instance, SLOT(hyperlinkRequested(QString)));
      tips->setAutoDelete(true);
    }

    KQuickTip *qt = new KQuickTip;
    qt->widget = w;
    qt->txt = s;
    tips->append(qt);
    connect(w, SIGNAL(destroyed()),
	    instance, SLOT(widgetDestroyed()));

    w->installEventFilter(instance);
  }

  return s;
}


void KQuickHelp::hyperlinkRequested(QString link) {
  // check protocol
  int pos = link.find(':');
  QString protocol;

  if(pos != -1 &&
    (protocol = link.left(pos).lower() == "http" ||
     protocol == "info" ||
     protocol == "ftp"  ||
     protocol == "file" ||
     protocol == "mailto"))
    {
      // lets give this URL to kfm, he knows better what
      // to do with it
      if(vfork() > 0) {
	// drop setuid, setgid
	setgid(getgid());
	setuid(getuid());
	
	execlp("kfmclient", "kfmclient", "exec", link.ascii(), 0);
	_exit(0);
      }
    } else {
      // hmm, seems I have to do it myself :-(
      QString fname = "", anchor = "";
      int idx;

      if((idx = link.find('#')) == -1)
	fname = link;
      else {
	if(link.at(0) == '#') {
	  anchor = link.mid(1, 255);
	  fname = QString(kapp->name()) + ".html";
	} else {
	  fname = link.left(idx);
	  anchor = link.mid(idx+1, 1024);
	}
      }
      
      kapp->invokeHTMLHelp(QString(kapp->name()) +  "/" + fname, anchor);
    }
}


void KQuickHelp::remove(QWidget *w) {
  for(unsigned i = 0; i < tips->count(); i++)
    if(tips->at(i)->widget == w) {
      tips->remove(i);
      return;
    }      
}


void KQuickHelp::widgetDestroyed() {
  remove((QWidget *)sender());
}


bool KQuickHelp::eventFilter(QObject *o, QEvent *e) {
  if(e->type() == QEvent::MouseButtonPress && ((QMouseEvent *)e)->button() == RightButton) {
    for(unsigned i = 0; i < tips->count(); i++) {
      if(tips->at(i)->widget == (QWidget *)o) {
	current = tips->at(i)->widget;
	currentText = tips->at(i)->txt;
	currentPos = QCursor::pos();
	menu->popup(currentPos);
	return true;
      }
    }
  }
  
  return false;
}


void KQuickHelp::getKQuickHelp(int) {
  window->popup(currentText, currentPos.x(), currentPos.y());
}


KQuickHelpWindow::KQuickHelpWindow() : QFrame(0, 0, WStyle_Customize|WStyle_Tool) {
   static QColor _stdColors[] = {
       red,
       green,
       blue,
      white,
	 yellow,
       black,
     QColor(165, 42, 42),
   magenta,
	 cyan
   };

   if (stdColors == 0)
     stdColors = _stdColors;

  setBackgroundColor(QColor(255, 255, 225));
  defaultCursor = cursor();
  linkAreas.setAutoDelete(true);
}


void KQuickHelpWindow::newText() {
  QPixmap pm(1, 1);
  QPainter p;
  p.begin(&pm);

  int w = 0, h = 0;
  paint(&p, w, h);
  p.end();

  // HACK: there may have been link-areas defined. Remove them
  linkAreas.clear();

  // resize to appropr. size
  resize(w, h);
}


void KQuickHelpWindow::mousePressEvent(QMouseEvent *e) {
  if(e->button() == LeftButton && activeLink.length())
    emit hyperlink(activeLink);
  hide();
}


void KQuickHelpWindow::mouseMoveEvent(QMouseEvent *e) {
  QPoint p = e->pos();

  for(uint i = 0; i < linkAreas.count(); i++)
    if(linkAreas.at(i)->area.contains(p)) {
      setCursor(upArrowCursor);
      activeLink = linkAreas.at(i)->link.copy();      
      return;
    } 

  activeLink = "";
  setCursor(defaultCursor);
}


void KQuickHelpWindow::keyPressEvent(QKeyEvent *e) {
  e->accept();
  hide();
}


void KQuickHelpWindow::show() {
  QFrame::show();
  grabMouse();
  grabKeyboard();
  setMouseTracking(true);
}


void KQuickHelpWindow::hide() {
  QFrame::hide();
  releaseMouse();
  releaseKeyboard();
  setMouseTracking(false);

  // clean up
  activeLink = "";
  linkAreas.clear();
}


void KQuickHelpWindow::popup(QString text, int x, int y) {
  txt = text.copy();
  newText();

  if(x + width() > QApplication::desktop()->width() - 8)
    x = QApplication::desktop()->width() - 8 - width();
  if(y + height() > QApplication::desktop()->height() - 4)
    y = QApplication::desktop()->height() - 4 - height();
  move(x, y);
  show();
}
  
  
void KQuickHelpWindow::paintEvent(QPaintEvent *) {
  QPainter p;

  p.begin(this);
  int x, y;
  paint(&p, x, y);
  p.end();

#ifdef QH_SHAPE
  // repaint, so we can apply the Shape (don't know if necessary)
  // kapp->flushX();

  QBitmap bm(width(), height());
  p.begin(&bm);
  p.setPen(color0);
  bm.fill(color1);
  
  p.fillRect(X_SHADOW, height() - Y_SHADOW,
	     width() - X_SHADOW, Y_SHADOW, Dense4Pattern);
  p.fillRect(width()-X_SHADOW, Y_SHADOW,
	     X_SHADOW, height()-Y_SHADOW, Dense4Pattern);
  p.fillRect(width()-X_SHADOW, 0,
	     X_SHADOW, Y_SHADOW, SolidPattern);
  p.fillRect(0, height()-Y_SHADOW,
	     X_SHADOW, Y_SHADOW, SolidPattern);
  p.end();
  
  XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0, bm.handle(), ShapeSet );
#endif
}


QString KQuickHelpWindow::token() {
  QString t = "";
  bool  backslash = false;

  while(tokIndex < (int)txt.length()) {
    QChar c = txt[tokIndex++];

    if(backslash) {
      backslash = false;
      t += c;
    } else {
      switch((char)c) {
      case '\\':
	backslash = true;
	break;

      case '\n':
	if(t.length() == 0)
	  t = "<br>";
	else
	  tokIndex--;
	return t;

      case '<':
	if(t.length() == 0)
	  t += c;
	else {
	  tokIndex--;
	  return t;
	}
	break;

      case '>':
	if(t.left(1) == "<") // bad token
	  t += c;
	return t;

      default:
	t += c;
      }
    }
  }

  return t;
}


void KQuickHelpWindow::paint(QPainter *p, int &w, int &h) {
  int posx = X_BORDER, posy = Y_BORDER;
  int maxy = 0, maxx = 0, idx;
  int indent = 0;
  QFont f;
  QColor txtColor = black;
  QColor savedColor;
  int txtFlags = 0;
  bool wasUnderline = false;

  QString link;

  int pointsize = p->font().pointSize();
  p->setPen(black);

  tokIndex = 0;
  while(tokIndex < (int)txt.length()) {
    QString t = token();
    if(t.left(1) == "<") { // a token      
      // find token
      int tokenID = -1;
      t = t.simplifyWhiteSpace();

      for(int i = 0; tokens[i].token != 0; i++)
	if(strncasecmp(t.data(), tokens[i].token, strlen(tokens[i].token)) == 0) {
	  tokenID = tokens[i].tokenID;
	  break;
	}
      
      switch(tokenID) {
      case -1:
	// ignore
	fprintf(stderr, "KQuickHelp: ignoring unknown token \"%s\"!\n", t.ascii());
	break;

	// font attributes
      case TOK_BOLD_ON:
	txtFlags |= F_BOLD;
	break;
      case TOK_BOLD_OFF:
	txtFlags &= ~F_BOLD;
	break;
      case TOK_ITALIC_ON:
	txtFlags |= F_ITALIC;
	break;
      case TOK_ITALIC_OFF:
	txtFlags &= ~F_ITALIC;
	break;
      case TOK_UNDERLINE_ON:
	txtFlags |= F_UNDERLINE;
	break;
      case TOK_UNDERLINE_OFF:
	txtFlags &= ~F_UNDERLINE;
	break;
	
	// font size
      case TOK_FONTSIZE_PLUS:
	f = p->font();
	while(QFontInfo(f).pointSize() == pointsize)
	  f.setPointSize(f.pointSize() + 1);
	pointsize = QFontInfo(f).pointSize();
	break;
      case TOK_FONTSIZE_MINUS:
	f = p->font();
	while(QFontInfo(f).pointSize() == pointsize)
	  f.setPointSize(f.pointSize() - 1);
	pointsize = QFontInfo(f).pointSize();
	break;

	// indentation
      case TOK_INDENT_PLUS:
	posx += fontMetrics().width('H') * 2;
	indent += fontMetrics().width('H') * 2;
	break;
      case TOK_INDENT_MINUS:
	if(indent > 0)
	  indent -= fontMetrics().width('H') * 2;
	break;

	// color by RGB value
      case TOK_COLOR_BY_RGB:
	idx = t.find(QRegExp("#[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]"));
	if(idx != -1) {
	  int cval, r, g, b;
	  sscanf(t.data() + idx + 1, "%x", &cval);
	  r = cval >> 16;
	  g = (cval >> 8) & 0xff;
	  b = cval & 0xff;
	  txtColor = QColor(r, g, b);
	}
	break;
      
	// newline
      case TOK_NEWLINE:
	posx = X_BORDER + indent;
	posy += maxy;
	maxy = 10;
	break;

	// standard colors
      case TOK_COLOR_RED:
      case TOK_COLOR_GREEN:
      case TOK_COLOR_BLUE:
      case TOK_COLOR_WHITE:
      case TOK_COLOR_YELLOW:
      case TOK_COLOR_BLACK:
      case TOK_COLOR_BROWN:
      case TOK_COLOR_MAGENTA:
      case TOK_COLOR_CYAN:
	txtColor = stdColors[tokenID-TOK_COLOR_RED];
	break;

	// hyperlinks
      case TOK_LINK:
	idx = t.find(QRegExp(" .*>", false));
	if(idx != -1) {
	  link = t.mid(idx+1, t.length()-idx-2);
	  savedColor = txtColor;
	  wasUnderline = (txtFlags & F_UNDERLINE) != 0;
	  txtColor = blue;
	  txtFlags |= F_UNDERLINE;
	}
	break;

      case TOK_ENDLINK:
	txtColor = savedColor;
	if(!wasUnderline)
	  txtFlags &= ~F_UNDERLINE;
	link = "";
	break;

      default:
	printf("TOKEN: \"%s\", ID=%d\n", t.ascii(), tokenID);
      }
    } else { // plain text
      p->setPen(txtColor);
      QFont f = font();
      f.setPointSize(pointsize);
      f.setBold((txtFlags & F_BOLD) != 0);
      f.setItalic((txtFlags & F_ITALIC) != 0);
      f.setUnderline((txtFlags & F_UNDERLINE) != 0);
      p->setFont(f);

      QRect r;
      p->drawText(posx, posy, 1024, 1024, AlignLeft|AlignTop, t, -1, &r, 0);
      if(link.length() > 0) {
	KQuickHelp_Link *l = new KQuickHelp_Link;
	l->area = r;
	l->link = link;
	linkAreas.append(l);
      }
      posx += r.width();
      maxy = QMAX(maxy, p->fontMetrics().lineSpacing());
      maxx = QMAX(maxx, r.right());
    }
  }

  w = maxx + X_BORDER + X_SHADOW;
  h = posy + maxy + Y_BORDER + Y_SHADOW;

  // draw frame
  p->setPen(QPen(black, 1));
  p->drawRect(0, 0, w - X_SHADOW, h - Y_SHADOW);
  p->fillRect(0, h - Y_SHADOW, w, h, QBrush(QColor(black)));
  p->fillRect(w - X_SHADOW, 0, w, h, QBrush(QColor(black)));
}
#include "kquickhelp.moc"

