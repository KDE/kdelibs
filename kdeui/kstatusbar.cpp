/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
              (C) 1997,1998 Sven Radej (sven@lisa.exp.univie.ac.at)

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

#include <qpainter.h>

#include <ktopwidget.h>
#include <kstatusbar.h>
// $Id$
// $Log$
// Revision 1.18  1999/01/18 10:57:04  kulow
// .moc files are back in kdelibs. Built fine here using automake 1.3
//
// Revision 1.17  1999/01/15 09:31:23  kulow
// it's official - kdelibs builds with srcdir != builddir. For this I
// automocifized it, the generated rules are easier to maintain than
// selfwritten rules. I have to fight with some bugs of this tool, but
// generally it's better than keeping them updated by hand.
//
// Revision 1.16  1998/10/05 10:14:25  radej
// sven: Bugfix: SEGV when 10000 messages come in the same time
//
// Revision 1.15  1998/09/23 14:40:22  radej
// sven: connected timeout to clear. Thanks to Taj for hint, and no-thanks to
//       me - alzheimer for forgeting it
//
// Revision 1.14  1998/09/04 22:58:09  radej
// sven: added sizeHint. Very primitive - width is ?, but height is set
//       according to fontMetrics
//
// Revision 1.13  1998/08/05 09:58:47  radej
// sven: fixed removeItem SEGV; thanks to Frans van Dorsselaer for report.
//
// Revision 1.12  1998/05/08 16:25:55  radej
// Fixed timer ftr temp messages.
//
// Revision 1.11  1998/05/05 10:04:40  radej
// Not sunken in windows style
//
// Revision 1.10  1998/04/21 20:37:48  radej
// Added insertWidget and some reorganisation - BINARY INCOMPATIBLE
//


/*************************************************************************/
// We want a statusbar-fields to be by this amount heigher than fm.height().
// This does NOT include the border width which we set separately for the statusbar.

#define FONT_Y_DELTA 3
#define DEFAULT_BORDERWIDTH 0

KStatusBarLabel::KStatusBarLabel( const char *text, int _id,
                                 QWidget *parent, const char *) :
  QLabel( parent ) 
{   
  id = _id;

  QFontMetrics fm = fontMetrics();
  w = fm.width( text )+8;
  h = fm.height() + FONT_Y_DELTA;
  resize( w, h );
    
  setText( text );
  if ( style() == MotifStyle )
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
  setAlignment( AlignLeft | AlignVCenter );
}


void KStatusBarLabel::mousePressEvent (QMouseEvent *)
{
  emit Pressed(id);
}

void KStatusBarLabel::mouseReleaseEvent (QMouseEvent *)
{
  emit Released (id);
}

/***********************************************************************/

KStatusBar::KStatusBar( QWidget *parent, const char *name )
  : QFrame( parent, name )
{
  init();
}

KStatusBar::~KStatusBar ()
{
  tmpTimer->stop();
  for ( KStatusBarItem *b = items.first(); b; b=items.next() )
    delete b;
  if (tempMessage)
    delete tempMessage;

  delete tmpTimer; // What do I have to notice!?
};

void KStatusBar::drawContents( QPainter * )
{
}

void KStatusBar::init()
{
  borderwidth = DEFAULT_BORDERWIDTH;
  fieldheight = fontMetrics().height() + FONT_Y_DELTA;

  insert_order = KStatusBar::LeftToRight;
  setFrameStyle( QFrame::NoFrame );
  resize( width(), fieldheight + 2* borderwidth);
  tempWidget=0;
  tempMessage=0;

  tmpTimer = new QTimer(this);
  connect (tmpTimer, SIGNAL(timeout()), this, SLOT(clear()));

}

void KStatusBar::setHeight(int h){

  fieldheight = h - 2* borderwidth;
  resize( width(),h);
  
}


void KStatusBar::setBorderWidth(int b){
  
  borderwidth = b;
  resize( width(),height() + 2* borderwidth);
  
}

void KStatusBar::resizeEvent( QResizeEvent * ) {
  if (tempMessage)
    tempMessage->setGeometry(borderwidth, borderwidth,
                             width()-2*borderwidth, fieldheight);
  else if (tempWidget)
    tempWidget->setGeometry(borderwidth, borderwidth,
                            width()-2*borderwidth, fieldheight);
  else
    updateRects( ); // False? You wouldn't sell that to toolbar... (sven)
}

void KStatusBar::setInsertOrder(InsertOrder order){

  insert_order = order;

}

void KStatusBar::updateRects( bool res ){
  
  if( insert_order == KStatusBar::LeftToRight){

    int offset= borderwidth;	
    for ( KStatusBarItem *b = items.first(); b; b=items.next() ) {

      b->setGeometry( offset, borderwidth, b->width(), fieldheight );	
      offset+=b->width() + borderwidth;
    }
    
    if ( !res ) {
      KStatusBarItem *l = items.getLast();
      if( l ) {
		offset-=l->width() + borderwidth;
		l->setGeometry( offset ,borderwidth, width() - offset - borderwidth, fieldheight );
      }
    }
  }
  else{ // KStatusBar::RightToLeft
    int offset = width();

    for ( KStatusBarItem *b = items.first(); b; b=items.next() ) {
      offset -=b->width() + borderwidth;
      b->setGeometry(offset,borderwidth,b->width(),fieldheight );

    }
    
    if ( !res ) {
      KStatusBarItem *l = items.getLast();
      if( l != 0L ) {
		offset+=l->width() - borderwidth;
		l->setGeometry(borderwidth,borderwidth,offset,fieldheight);
      }
    }
  }	
}

bool KStatusBar::enable( BarStatus stat )
{
  bool mystat = isVisible();
  if ( (stat == Toggle && mystat) || stat == Hide )
    hide();
  else
    show();
  return ( isVisible() == mystat );
}	

int KStatusBar::insertItem( const char *text, int id )
{
  KStatusBarLabel *label = new KStatusBarLabel( text, id, this );
  KStatusBarItem *item = new KStatusBarItem(label, id, true);
/*
  // resize last item to default
  KStatusBarItem *l = items.getLast();
  if( l )
  {
    QFontMetrics fm = fontMetrics();
    int w = fm.width(((QLabel *) l->getItem())->text() )+8;
    int h = fm.height() + FONT_Y_DELTA;
    l->getItem()->resize( w, h );
  }
*/
  items.append( item );
  updateRects( TRUE );
  connect (label, SIGNAL(Pressed(int)), this, SLOT(slotPressed(int)));
  connect (label, SIGNAL(Released(int)), this, SLOT(slotReleased(int)));
  return items.at();
}

int KStatusBar::insertWidget(QWidget *_widget, int size, int id)
{
   KStatusBarItem *item = new KStatusBarItem(_widget, id, false);

   items.append( item );
   _widget->resize(size, fieldheight);
   updateRects( TRUE );
   return items.at();
}

void KStatusBar::removeItem (int id)
{
  for ( KStatusBarItem *b = items.first(); b; b=items.next() ) 
    if ( b->ID() == id )
    {
      items.remove();
      delete b;
      updateRects(false );
    }
  
}

void KStatusBar::replaceItem(int /* _id */, const char * /* _text */ )
{
 debug ("Not yet implemented. Sorry.");
}

void KStatusBar::replaceItem(int /* _id */ , QWidget * /* _widget */)
{
 debug ("Not yet implemented. Sorry.");
}

void KStatusBar::changeItem( const char *text, int id )
{
  for ( KStatusBarItem *b = items.first(); b; b=items.next() ) 
	if ( b->ID() == id )
	  ((KStatusBarLabel *)b->getItem())->setText( text );
}

void KStatusBar::setAlignment( int id, int align)
{
  for ( KStatusBarItem *b = items.first(); b; b=items.next() ) 
    if ( b->ID() == id ){
	  ((KStatusBarLabel *)b->getItem())->setAlignment(align|AlignVCenter);
    }
}

void KStatusBar::message(const char *text, int time)
{
  if (tmpTimer->isActive())
    tmpTimer->stop();
  
  if (tempMessage)
  {
    delete tempMessage;
    tempMessage = 0;
  }
  else if (tempWidget)
  {
    tempWidget->hide();
    tempWidget=0;
  }
  else
    for ( KStatusBarItem *b = items.first(); b; b=items.next() )
      b->hide();

  
  tempMessage = new KStatusBarLabel( text, -1, this );
  tempMessage->setGeometry(borderwidth, borderwidth,
                           width()-2*borderwidth, fieldheight);
  tempMessage->show();
  if (time >0)
    tmpTimer->start(time, true);
}

void KStatusBar::message(QWidget *widget, int time)
{
  if (tmpTimer->isActive())
    tmpTimer->stop();
  
  if (tempMessage)
  {
    delete tempMessage;
    tempMessage = 0;
  }
  else if (tempWidget)
  {
    tempWidget->hide();
    tempWidget=0;
  }
  else
    for ( KStatusBarItem *b = items.first(); b; b=items.next() )
      b->hide();

  // Hi, Trolls
  
  tempWidget = widget;
  tempWidget->setGeometry(borderwidth, borderwidth,
                          width()-2*borderwidth, fieldheight);
  tempWidget->show();
  if (time >0)
    tmpTimer->start(time, true);
}


void KStatusBar::clear()
{
  tmpTimer->stop();
  if (tempMessage)
    delete tempMessage;
  if (tempWidget)
    tempWidget->hide();
  
  for ( KStatusBarItem *b = items.first(); b; b=items.next() )
    b->show();

  tempMessage=0;
  tempWidget=0;
}

void KStatusBar::slotPressed(int _id)
{
  emit pressed(_id);
}

void KStatusBar::slotReleased(int _id)
{
  emit released(_id);
}

QSize KStatusBar::sizeHint()
{
  return QSize(width(), height());
}


#include "kstatusbar.moc"

//Eh!!!

