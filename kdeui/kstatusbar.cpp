/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
              (C) 1997,1998, 2000 Sven Radej (radej@kde.org)

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

#include <kstatusbar.h>

// $Id$
// $Log$
// Revision 1.27  2000/03/12 15:37:43  radej
// sven: New KStatusBar - inherits QStatusBar
//
// Revision 1.26  1999/12/13 22:44:37  kulow
// put some #ifdef __GNUC__ around the #warning
// removed the -DQT2_WORKAROUND. Sven says KMenubar is rewritten to be based
// on QMenuBar rsn anyway, so we can live with this til then
//
// Revision 1.25  1999/10/31 19:44:46  bero
// More template definitions for -frepo
//
// Revision 1.24  1999/10/08 23:13:42  bero
// ktopwidget.h no longer included
//
// Revision 1.23  1999/08/05 22:22:25  shausman
// - implemented KStatusBarItem *KStatusBar::item( int id );
//
// Revision 1.22  1999/06/09 21:52:26  cschlaeg
// serveral fixes for recently implemented layout management; removed segfault on close; removed segfault for no menubar apps; setFullWidth(false) is working again; floating a bar does not segfault any more but still does not work properly; I will look into this again.
//
// Revision 1.21  1999/06/06 17:29:44  cschlaeg
// New layout management implemented for KTMainWindow. This required
// updates for KToolBar, KMenuBar and KStatusBar. KTMainWindow::view_*
// public variables removed. Use mainViewGeometry() instead if you really
// have to. Added new classes in ktmlayout to handle the new layout
// management.
//
// Revision 1.20  1999/05/11 23:39:45  dfaure
// Added signal moved() to KStatusBar ; emit moved() in KStatusBar::enable ;
//  connected it to updateRects in KTMainWindow.
// Implemented KMenuBar::enable, for consistency with other *bars.
//
// Revision 1.19  1999/03/01 23:35:15  kulow
// CVS_SILENT ported to Qt 2.0
//
// Revision 1.18.2.2  1999/02/22 21:14:27  kulow
// extreme QStringing
//
// Revision 1.18.2.1  1999/02/21 20:56:12  kulow
// more porting to Qt 2.0. It compiles and links. Jucheisassa :)



#define FONT_Y_DELTA 3
#define DEFAULT_BORDERWIDTH 0

KStatusBarLabel::KStatusBarLabel( const QString& text, int _id,
                                 KStatusBar *parent, const char *name) :
  QLabel( parent, name) 
{   
  id = _id;

  // Commented out - not needed? (sven)
  //int w, h;
  //QFontMetrics fm = fontMetrics();
  //w = fm.width( text )+8;
  //h = fm.height() + FONT_Y_DELTA;
  //resize( w, h );


  setText( text );
  // umm... Mosfet? Can you help here?
  //if ( style() == MotifStyle )
  setLineWidth  (1);
  setFrameStyle (QFrame::StyledPanel | QFrame::Plain );
  
  setAlignment( AlignHCenter | AlignVCenter );

  connect (this, SIGNAL(itemPressed(int)), parent, SLOT(slotPressed(int)));
  connect (this, SIGNAL(itemReleased(int)), parent, SLOT(slotReleased(int)));
}

void KStatusBarLabel::mousePressEvent (QMouseEvent *)
{
  emit itemPressed (id);
}

void KStatusBarLabel::mouseReleaseEvent (QMouseEvent *)
{
  emit itemReleased (id);
}


KStatusBar::KStatusBar( QWidget *parent, const char *name )
  : QStatusBar( parent, name )
{
  items.setAutoDelete(true);
  setSizeGripEnabled(false);
}

KStatusBar::~KStatusBar ()
{
  // Umm... delete something;
  items.clear();
}

void KStatusBar::insertItem( const QString& text, int id, bool permanent)
{
  KStatusBarLabel *l = new KStatusBarLabel (text, id, this);
  items.insert(id, l);
  addWidget (l, 0, permanent);
}

void KStatusBar::removeItem (int id)
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    removeWidget (l);
    items.remove(id);
    // reformat (); // needed? (sven)
  }
  else
    debug ("KStatusBar::removeItem: bad id");
}

void KStatusBar::changeItem( const QString& text, int id )
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    l->setText(text);
    reformat();
  }
  else
    debug ("KStatusBar::changeItem: bad id");
}

void KStatusBar::slotPressed(int _id)
{
  emit pressed(_id);
}

void KStatusBar::slotReleased(int _id)
{
  emit released(_id);
}


#include "kstatusbar.moc"

//Eh!!!

