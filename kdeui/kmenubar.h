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
//Revision 1.20  1999/05/11 23:39:48  dfaure
//Added signal moved() to KStatusBar ; emit moved() in KStatusBar::enable ;
// connected it to updateRects in KTMainWindow.
//Implemented KMenuBar::enable, for consistency with other *bars.
//
//Revision 1.19  1999/05/08 18:05:57  ssk
//Apparently moc doesn't like inner Q_OBJECTs. Moved KChildMenu (used to be
//_menuBar) to kmenubar.cpp
//
//Revision 1.18  1999/05/08 11:42:31  ssk
//Nested real menubar class, to clean up kdeui namespace.
//Minor doc update.
//
//Revision 1.17  1999/03/06 18:03:34  ettrich
//the nifty "flat" feature of kmenubar/ktoolbar is now more visible:
//It has its own menu entry and reacts on simple LMP clicks.

#ifndef _KMENUBAR_H
#define _KMENUBAR_H

#include <qmenubar.h>

class KToolBoxManager;

/**
 * Floatable menu bar. It can be set to float, Top, or Bottom
 * of KTopLevelWidget. It can be used without KTopLevelWidget, but
 * then you should maintain items (toolbars, menubar, statusbar)
 * yourself.
 *
 * Interface is the same as QMenuBar, except that you can't
 * add pixmaps.
 *
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
    * Argument for enable().
    */
   enum BarStatus{ Toggle, Show, Hide }; 

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
   void enableFloating(bool flag = TRUE);

   /**
    * Sets position. Can be used when floating or moving is disabled.
    */
   void setMenuBarPos(menuPosition mpos);

   /**
    * Sets title for floating menu bar. Default is Main widget title.
    */
   void setTitle(const QString&_title) {title = _title;};

   /**
    * Enable disable status bar. You can get the same effect with show
    * or hide, but if you do that the signal @ref #moved won't be emitted.
    */
   bool enable( BarStatus stat );

   /**
	* Reimplemented to return some useful measures.
	*/
   QSize sizeHint(void) const;

   /**
    * The rest is standard QMenuBar interface. See Qt docs for
    * details.
    */
   virtual uint count();
   virtual int insertItem(const QString& text,
                  const QObject *receiver, const char *member,
                  int accel=0 );

   virtual int insertItem(const QString& text, int id=-1, int index=-1 );
   virtual int insertItem(const QString& text, QPopupMenu *popup,
                  int id=-1, int index=-1 );

   virtual void insertSeparator(int index=-1 );
   virtual void removeItem(int id);
   virtual void removeItemAt(int index);
   virtual void clear();
   virtual int accel(int id);
   virtual void setAccel(int key, int id );
   virtual QString text(int id);
   virtual void changeItem(const QString& text, int id);
   virtual void setItemChecked(int id , bool flag);
   virtual void setItemEnabled(int id, bool flag);
   virtual int idAt( int index );

   int heightForWidth ( int max_width ) const;

   void setFlat (bool);

 protected slots:
   void ContextCallback(int index);
   void slotActivated (int id);
   void slotHighlighted (int id);
   void slotReadConfig ();
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
   QWidget *Parent;
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
   int oldMenuFrameStyle;
   KToolBoxManager *mgr;
   bool highlight;
   bool transparent;
   bool buttonDownOnHandle;
 };

#endif
