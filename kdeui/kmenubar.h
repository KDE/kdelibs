/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Sven Radej (sven@lisa.exp.univie.ac.at)
    Copyright (C) 1997 Matthias Ettrich (ettrich@kde.org)

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

//$Id$
//$Log$
//Revision 1.15.4.1  1999/03/24 16:48:57  ettrich
//workaround for qt-1.44 behaviour
//
//Revision 1.15  1998/12/16 01:27:14  ettrich
//fixed slightly broken macstyle removal
//
//Revision 1.14  1998/11/25 13:24:53  radej
//sven: Someone changed protected things to private (was it me?).
//
//Revision 1.13  1998/11/23 15:34:05  radej
//sven: Nicer sysmenu button
//
//Revision 1.12  1998/11/22 13:35:47  radej
//sven: IMPROVED Mac menubar: Accelerators, SystemMenu, look...
//
//Revision 1.11  1998/11/15 09:22:59  garbanzo
//Clean up kmenubar.h
//
//Revision 1.10  1998/11/11 14:32:12  radej
//sven: *Bars can be made flat by MMB (Like in Netscape, but this works)
//
//Revision 1.9  1998/08/31 00:52:12  torben
//Torben: One new function and made others virtual
//=> binary incompatible. Sorry. Please use virtual whenever it is
//not a performance problem.
//
//Revision 1.8  1998/05/07 23:13:23  radej
//Moving with KToolBoxManager
//
//
//Revision 1.17  1999/03/06 18:03:34  ettrich
//the nifty "flat" feature of kmenubar/ktoolbar is now more visible:
//It has its own menu entry and reacts on simple LMP clicks.

#ifndef _KMENUBAR_H
#define _KMENUBAR_H

class _menuBar : public QMenuBar
 {
   Q_OBJECT

 public:
   _menuBar(QWidget *parent=0, const char *name=0);
   ~_menuBar();
protected:
     void resizeEvent( QResizeEvent* );
 };

#include <qmenubar.h>
 * This is floatable toolbar. It can be set to float, Top, or Bottom
class KToolBoxManager;

/**
 * Floatable menu bar. It can be set to float, Top, or Bottom
 * of KTopLevelWidget. It can be used without KTopLevelWidget, but
 * then you should maintain items (toolbars, menubar, statusbar)
 * yourself.
 *
 * Interface is the same as QMenuBar, except that you can't
 * add pixmaps.
 * @short KDE floatable menubar
 * If you want to add other methods for 100% compatibility with QMenuBar
 * just add those methods, and pass all arguments ot menu bar.
 * see kmenubar.cpp for details. It is extremly simple.
 * @author Sven Radej <sven@kde.org>
 */
class KMenuBar : public QFrame
 {
   Q_OBJECT

 public:

   /**
    * Positions of menubar.
    */
   enum menuPosition{Top, Bottom, Floating, Flat, FloatingSystem};

   /**
    * Constructor. For all details about inserting items see
    * @ref QMenuBar
    */
   KMenuBar( QWidget *parent=0, const char *name=0);

   /**
    * Destructor. Embeds menubar back if floating. Delete menubar
    * in your destructor or closeEvent for 100% safety
    */
   ~KMenuBar();

   /**
    * Enable or disable moving. This only disables user moving
    * menubar can be moved wit @ref #setMenuBarPos.
    */
   void enableMoving(bool flag = TRUE);

   /**
    * Returns menubar position.
    */
   menuPosition menuBarPos() {return position;};

   /**
    * Enables/disables floating.
    */
    * This cannot be used to set toolbar flat. For that, use @ref setFlat .
   void enableFloating(bool flag = TRUE);

   /**
    * Sets position. Can be used when floating or moving is disabled.
    */
   void setMenuBarPos(menuPosition mpos);
   void setTitle(const char *_title) {title = _title;};
   /**
    * Sets title for floating menu bar. Default is Main widget title.
    */
   void setTitle(const QString&_title) {title = _title;};

   /**
   virtual int insertItem(const char *text,
    * details.
    */
   virtual uint count();
   virtual int insertItem(const char *text, int id=-1, int index=-1 );
   virtual int insertItem(const char *text, QPopupMenu *popup,
                  int accel=0 );

   virtual int insertItem(const QString& text, int id=-1, int index=-1 );
   virtual int insertItem(const QString& text, QPopupMenu *popup,
                  int id=-1, int index=-1 );

   virtual void insertSeparator(int index=-1 );
   virtual void removeItem(int id);
   virtual const char *text(int id);
   virtual void changeItem(const char *text, int id);
   virtual int accel(int id);
   virtual void setAccel(int key, int id );
   virtual QString text(int id);
   virtual void changeItem(const QString& text, int id);
   virtual void setItemChecked(int id , bool flag);
   virtual void setItemEnabled(int id, bool flag);
   /**
   * This method switches flat/unflat mode. Carefull: might not work
   * If menubar is floating.
   */
   virtual int idAt( int index );

   int heightForWidth ( int max_width ) const;

   void setFlat (bool);

 protected slots:
   void ContextCallback(int index);
   void slotActivated (int id);
   void slotHighlighted (int id);
   void slotHotSpot (int i);

 protected:

   void init();
   void mousePressEvent ( QMouseEvent *m );
   void resizeEvent( QResizeEvent *e );
   void paintEvent(QPaintEvent *);
   void closeEvent (QCloseEvent *e);
   void leaveEvent (QEvent *e);
   bool eventFilter(QObject *, QEvent *);

private:
   bool moving;
   const char *title;
   int oldX;
   int oldY;
   int oldWFlags;
   QString title;
   menuPosition position;
   menuPosition lastPosition;
   menuPosition movePosition;

   QPopupMenu *context;
   QMenuBar *menu;
   QFrame *frame;

signals:
    /**
     * This signal is connected to @ref KTopLevel::updateRects. It is
     * emited when menu bar changes its position.
     */
    void moved (menuPosition);

    /**
     * This signal is emited when item id is highlighted.
     */
    void highlighted(int id);

    /**
     * This signal is emited when item id is activated.
     */
    void activated(int id);

private:
   QFrame *handle;
   QPoint pointerOffset;
   QPoint parentOffset;
   KToolBoxManager *mgr;
   bool highlight;
   bool transparent;
   bool buttonDownOnHandle;
 };

#endif
