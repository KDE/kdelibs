/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
              (C) 1997 Sven Radej (sven.radej@iname.com)

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
#include <ktopwidget.h>
#include <kstatusbar.moc>
#include <qpainter.h>
// Revision 1.10  1998/04/21 20:37:48  radej
// Added insertWidget and some reorganisation - BINARY INCOMPATIBLE


/*************************************************************************/
// We want a statusbar-fields to be by this amount heigher than fm.height().

// This does NOT include the border width which we set separately for the statusbar.


KStatusBarItem::KStatusBarItem( const char *text, int ID,
				QWidget *parent, const char *) : 
  QLabel( parent ) 
KStatusBarLabel::KStatusBarLabel( const QString& text, int _id,
  id = ID;
  QLabel( parent, name) 
{   
  id = _id;

  QFontMetrics fm = fontMetrics();
  w = fm.width( text )+8;
  h = fm.height() + FONT_Y_DELTA;
  setFrameStyle( QFrame::Panel | QFrame::Sunken );
  setText( text );
  if ( style() == MotifStyle )
      setFrameStyle( QFrame::Panel | QFrame::Sunken );
  setAlignment( AlignLeft | AlignVCenter );
int KStatusBarItem::ID()
{
  return id;
}

void KStatusBarItem::mousePressEvent (QMouseEvent *)


void KStatusBarLabel::mousePressEvent (QMouseEvent *)
{
void KStatusBarItem::mouseReleaseEvent (QMouseEvent *)
}

void KStatusBarLabel::mouseReleaseEvent (QMouseEvent *)
{
}

/***********************************************************************/

KStatusBar::KStatusBar( QWidget *parent, const char *name )
  : QFrame( parent, name )
{

  delete tmpTimer; // What do I have to notice!?
};

void KStatusBar::drawContents( QPainter * )
{
}


void KStatusBar::init()
{
  borderwidth = DEFAULT_BORDERWIDTH;
  fieldheight = fontMetrics().height() + FONT_Y_DELTA;

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
KStatusBar::~KStatusBar()
{
  for ( KStatusBarItem *b = labels.first(); b; b=labels.next() ) 
	delete b;
}

bool KStatusBar::enable( BarStatus stat )
{
  bool mystat = isVisible();
  if ( (stat == Toggle && mystat) || stat == Hide )
    hide();
  else if (tempWidget)
    show();
  return ( isVisible() == mystat );
}	

int KStatusBar::insertItem( const char *text, int id )
{
  KStatusBarItem *label = new KStatusBarItem( text, id, this );
  labels.append( label );	
  updateRects( TRUE );
  connect (label, SIGNAL(Pressed(int)), this, SLOT(slotPressed(int)));
  connect (label, SIGNAL(Released(int)), this, SLOT(slotReleased(int)));
  return labels.at();
}

void KStatusBar::resizeEvent( QResizeEvent * ) {
  updateRects( );
                            width()-2*borderwidth, fieldheight);
  else
    updateRects( ); // False? You wouldn't sell that to toolbar... (sven)
}

void KStatusBar::setInsertOrder(InsertOrder order){

  insert_order = order;
void KStatusBar::updateRects( bool res )
{  

void KStatusBar::updateRects( bool res ){
  
    for ( KStatusBarItem *b = labels.first(); b; b=labels.next() ) {

    int offset= borderwidth;	
    for ( KStatusBarItem *b = items.first(); b; b=items.next() ) {

      b->setGeometry( offset, borderwidth, b->width(), fieldheight );	
      offset+=b->width() + borderwidth;
      KStatusBarItem *l = labels.getLast();
    
    if ( !res ) {
      KStatusBarItem *l = items.getLast();
      if( l ) {
		offset-=l->width() + borderwidth;
		l->setGeometry( offset ,borderwidth, width() - offset - borderwidth, fieldheight );
      }
    }
  }
    for ( KStatusBarItem *b = labels.first(); b; b=labels.next() ) {
    int offset = width();

    for ( KStatusBarItem *b = items.first(); b; b=items.next() ) {
      offset -=b->width() + borderwidth;
      b->setGeometry(offset,borderwidth,b->width(),fieldheight );

      KStatusBarItem *l = labels.getLast();
    
    if ( !res ) {
      KStatusBarItem *l = items.getLast();
      if( l != 0L ) {
		offset+=l->width() - borderwidth;
		l->setGeometry(borderwidth,borderwidth,offset,fieldheight);
      }
    }
void KStatusBar::changeItem( const char *text, int id )
}
  for ( KStatusBarItem *b = labels.first(); b; b=labels.next() ) 
void KStatusBar::changeItem( const QString& text, int id )
	  b->setText( text );
  for ( KStatusBarItem *b = items.first(); b; b=items.next() ) 
	if ( b->ID() == id )
	  ((KStatusBarLabel *)b->getItem())->setText( text );
}
  for ( KStatusBarItem *b = labels.first(); b; b=labels.next() ) 
void KStatusBar::setAlignment( int id, int align)
	  b->setAlignment( align | AlignVCenter );
  for ( KStatusBarItem *b = items.first(); b; b=items.next() ) 
    if ( b->ID() == id ){
	  ((KStatusBarLabel *)b->getItem())->setAlignment(align|AlignVCenter);
  tempWidget=0;
}

void KStatusBar::slotPressed(int _id)
{
  emit pressed(_id);
}

void KStatusBar::slotReleased(int _id)

//Eh!!!

