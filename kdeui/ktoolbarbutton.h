/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)

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

// $Id$
// $Log$

#ifndef _KTOOLBARBUTTON_H
#define _KTOOLBARBUTTON_H

#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qbutton.h>
#include <qintdict.h>
#include <qstring.h>
#include <qpainter.h>
#include <qfont.h>
#include <qevent.h>

class KToolBar;

/**
 * A toolbar button. This is used internally by @ref KToolBar, use the 
 * KToolBar methods instead.
 * @internal
 */

class KToolBarButton : public QButton
 {
   Q_OBJECT

 public:
   KToolBarButton(const QPixmap& pixmap, int id, QWidget *parent,
                  const char *name=0L, int item_size = 26, const QString &txt=QString::null,
                  bool _mb = false);
   KToolBarButton(QWidget *parent=0L, const char *name=0L);
   ~KToolBarButton() {};
   void setEnabled(bool enable);

   virtual void setPixmap( const QPixmap & );
   virtual void setText ( const QString& text);
   void on(bool flag);
   void toggle();
   void beToggle(bool);
   bool ImASeparator () {return sep;};
   void youreSeparator () {sep = true;};
   QPopupMenu *popup () {return myPopup;};
   void setPopup (QPopupMenu *p);
   void setDelayedPopup (QPopupMenu *p);
   void setRadio(bool f);

 public slots:
   void modeChange();

 protected:
   void paletteChange(const QPalette &);
   void leaveEvent(QEvent *e);
   void enterEvent(QEvent *e);
   void drawButton(QPainter *p);
   bool eventFilter (QObject *o, QEvent *e);
   void showMenu();
   //void setIconSet (const QPixmap &);
   void makeDisabledPixmap();

 private:
   bool toolBarButton;
   bool sep;
   QPixmap enabledPixmap;
   QPixmap disabledPixmap;
   int icontext;
   int highlight;
   bool raised;
   int id;
   int _size;
   KToolBar *parentWidget;
   QString btext;
   QFont buttonFont;
   QPopupMenu *myPopup;
   bool delayPopup;
   QTimer *delayTimer;
   bool radio;

 protected slots:
     void ButtonClicked();
     void ButtonPressed();
     void ButtonReleased();
     void ButtonToggled();
     void slotDelayTimeout();

 signals:
     void clicked(int);
     void doubleClicked(int);
     void pressed(int);
     void released(int);
     void toggled(int);
     void highlighted (int, bool);
 };

class KToolBarButtonList : public QIntDict<KToolBarButton>
{
public:
   KToolBarButtonList();
   ~KToolBarButtonList() {}
};

#endif
