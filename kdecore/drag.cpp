/* This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)

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
/*
 * $Id$
 * 
 * $Log$
 * Revision 1.20  1998/06/16 06:03:12  kalle
 * Implemented copy constructors and assignment operators or disabled them
 *
 * Revision 1.19  1998/03/09 20:21:52  kulow
 * just remove the dragZone from the dragZoneList, if kapp is still exist.
 * If this happens in the end of the application, big problems accour (the
 * reason, for many segfaults of applications at exit)
 *
 * Revision 1.18  1998/01/18 14:38:28  kulow
 * reverted the changes, Jacek commited.
 * Only the RCS comments were affected, but to keep them consistent, I
 * thought, it's better to revert them.
 * I checked twice, that only comments are affected ;)
 *
 * Revision 1.16  1997/11/04 11:00:30  kulow
 * fixed some free mismatch errors
 *
 * Revision 1.15  1997/10/21 20:44:40  kulow
 * removed all NULLs and replaced it with 0L or "".
 * There are some left in mediatool, but this is not C++
 *
 * Revision 1.14  1997/10/16 11:14:26  torben
 * Kalle: Copyright headers
 * kdoctoolbar removed
 *
 * Revision 1.13  1997/10/12 11:05:32  kulow
 * don't export debugWin. kmenuedit used the same name
 *
 * Revision 1.12  1997/10/04 14:51:31  kulow
 * added some prototypes in case the shape.h is not found. I'm not sure,
 * if this works, but it helps for compiling
 *
 * Revision 1.11  1997/09/18 12:47:15  kulow
 * uups. The error message is not valid for the cpp file
 *
 * Revision 1.10  1997/09/18 12:16:03  kulow
 * corrected some header dependencies. Removed most of them in drag.h and put
 * them in drag.cpp. Now it should compile even under SunOS 4.4.1 ;)
 *
 * Revision 1.9  1997/08/31 08:03:47  kdecvs
 * Heureka: Fritz fixed the dragging of icons on the root window (Kalle)
 *
 * Revision 1.8  1997/07/31 19:52:31  kalle
 * Root-drop bug hopefully fixed (courtesy of Fritz Elfert of ISDN4Linux fame)
 *
 * Revision 1.7  1997/05/09 15:10:10  kulow
 * Coolo: patched ltconfig for FreeBSD
 * removed some stupid warnings
 *
 * Revision 1.6  1997/05/09 08:23:40  kulow
 * Coolo: included X11 headers again in drag.cpp (my fault)
 *
 * Revision 1.5  1997/05/08 22:53:15  kalle
 * Kalle:
 * KPixmap gone for good
 * Eliminated static objects from KApplication and HTML-Widget
 *
 * Revision 1.4  1997/05/08 21:09:44  kulow
 * Coolo: I forgot to include Torben's changes
 *
 * Revision 1.4  1997/03/10 19:59:12  kalle
 * Merged changes from 0.6.3
 *
 * Revision 1.3  1997/01/15 20:01:58  kalle
 * merged changes from 0.52
 *
 * Revision 1.2  1996/12/07 22:21:18  kalle
 * RCS header, include kapp.h
 *
 *
 * Drag 'n Drop implementation, taken from kfm
 *
 * Author: Torben Weis
 */

#include "drag.h"

#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#endif

#include <qlist.h>
#include <qcursor.h>
#include <qbitmap.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif    

#include "drag.moc"
#include "kapp.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#ifdef HAVE_X11_EXTENSTIONS_SHAPE_H
#include <X11/extensions/shape.h>
#else
extern "C" {
void XShapeCombineMask(Display*, Window, int, int, int, Pixmap, int);
#define ShapeBounding                   0 
#define ShapeSet                        0 
}
#endif

static Window debugWin = 0;

KDNDDropZone::KDNDDropZone( QWidget* _parent, int _accept ) : QObject()
{
  widget = _parent;
  acceptType = _accept;
  
  kapp->addDropZone( this );
}

QWidget* KDNDDropZone::getWidget()
{
  return widget;
}

QStrList & KDNDDropZone::getURLList()
{
    return urlList;
}

void KDNDDropZone::parseURLList()
{
    urlList.clear();

    if ( dndType != DndURL )
	return;

    QString s = dndData;
    s.detach();
    int i;
    
    while ( ( i = s.find( "\n" ) ) != -1 )
    {
	QString t = s.left( i );
	urlList.append( t.data() );
	s = s.mid( i + 1, s.length() );
    }
    
    urlList.append( s.data() );
}

void KDNDDropZone::drop( char *_data, int _size, int _type, int _x, int _y )
{
  dndSize = _size;
  dndType = _type;
  dndData = _data;
  
  dndX = _x;
  dndY = _y;
  
  parseURLList();

  emit dropAction( this );
}

void KDNDDropZone::enter( char *_data, int _size, int _type, int _x, int _y )
{
  dndSize = _size;
  dndType = _type;
  dndData = _data;
  
  dndX = _x;
  dndY = _y;
  
  parseURLList();

  emit dropEnter( this );
}

void KDNDDropZone::leave()
{
  emit dropLeave( this );
}

KDNDDropZone::~KDNDDropZone()
{
  if (kapp)
     kapp->removeDropZone( this );
}


void KDNDWidget::startDrag( KDNDIcon *_icon, char *_data, int _size, int _type, int _dx, int _dy )
{
  if ( dndData != 0 )
     delete [] dndData ;
  
  drag = true;
    
  dndData = new char[ _size + 1 ];
  memcpy( dndData, _data, _size );
  dndData[ _size ] = 0;
  
  dndSize = _size + 1;
  dndType = _type;
  dndIcon = _icon;
  dndOffsetX = _dx;
  dndOffsetY = _dy;
  
  dndIcon->raise();
  
  dndLastWindow = (Window)0;
  
  Window root = DefaultRootWindow( kapp->getDisplay() );
  XChangeProperty( kapp->getDisplay(), root, kapp->getDndSelectionAtom(), 
				   XA_STRING, 8,
				   PropModeReplace, (const unsigned char*)dndData, dndSize);
    
/* I commented this out, like mentioned by Paul Kendal (coolo)
  // Commenting this out will only work with click on focus window managers
  // grabMouse();
  XGrabPointer(kapp->getDisplay(),root,False,
			   ButtonMotionMask|ButtonPressMask|ButtonReleaseMask,
			   GrabModeSync,GrabModeAsync,root,
			   None,
			   CurrentTime);
*/

  // KDNDApplication::setOverrideCursor( waitCursor );
}

Window KDNDWidget::findRootWindow( QPoint & p )
{
    int x,y;
    Window root = DefaultRootWindow( kapp->getDisplay() );
    Window win;
    XTranslateCoordinates( kapp->getDisplay(), root, root, p.x(), p.y(), &x, &y, &win );
    /*
  Window root;
  Window parent;
  Window *children;
  unsigned int cchildren;
  
  int win = -1;
  int win_x,win_y,win_h,win_w;
  
  root = DefaultRootWindow( kapp->getDisplay() );
  
  XQueryTree( kapp->getDisplay(), root, &root, &parent, 
			  &children, &cchildren );
  
  XWindowAttributes *attribs = new XWindowAttributes;
  
  for ( int i = 0; i < cchildren; i++ )
    {
	  int x,y;
	  unsigned int w,h,b,d;
	  
	  XGetGeometry( kapp->getDisplay(), children[i], &root, 
					&x, &y, &w, &h, &b, &d );

	  QRect r( x,y,w,h );
	  
	  if ( r.contains( p ) && children[i] != dndIcon->winId() )
		{   
		  XGetWindowAttributes( kapp->getDisplay(), children[i], attribs );
		  
		  if ( attribs->map_state != 0 )
			{
			  win = children[i];
			  win_x = x;
			  win_y = y;
			  win_h = h;
			  win_w = w;
			}
		}
    }     
  
  delete attribs;
  */
  return win;
}


/**
 * @oldErrorHandler stores the original error handler.
 */
static
int (* oldErrorHandler)(Display *, XErrorEvent *) = 0L;

/**
 * Errorhandler to be used during DND
 */
static
int myErrorHandler(Display *d, XErrorEvent *e)
{
	char msg[80];
	XGetErrorText(d, e->error_code, msg, 80);
	//	fprintf(stderr, "Ignored Error: %s\n", msg);
	//	fprintf(stderr, "Request: Maj=%d Min=%di, Serial=%08lx\n",
	//		e->request_code, e->minor_code, e->serial);
	return 0;
}

void KDNDWidget::mouseReleaseEvent( QMouseEvent * _mouse )
{
  if ( !drag )
    {
	  dndMouseReleaseEvent( _mouse );
	  return;
    }
    
  // KDNDApplication::restoreOverrideCursor();

  dndIcon->move( -200, -200 );
  
//   printf("************************************************************\n");
//   printf("*** id = %i ****\n", dndIcon->winId());
//   printf("************************************************************\n");
  debugWin = dndIcon->winId();
  
  QPoint p = mapToGlobal( _mouse->pos() );
  
  Window root;
  root = DefaultRootWindow( kapp->getDisplay() );
  
  Window win = findRootWindow( p );
//   printf("************************************************************\n");
//   printf("*** win = %ld **** dndLastWindow = %ld ****\n", win, dndLastWindow );
//   printf("************************************************************\n");
  
  drag = false;
/* I commented this out, since it works without (coolo)
  // Commenting this out will only work with click on focus window managers
  // releaseMouse();
  XUngrabPointer(kapp->getDisplay(),CurrentTime);
*/

//   printf("Ungarbbed\n");
  
  // If we found a destination for the drop
   if ( win != 0 )
  // if ( dndLastWindow != 0 )
    {	
          XWindowAttributes Wattr;

          XGetWindowAttributes(kapp->getDisplay(), win, &Wattr);
          if (Wattr.map_state != IsUnmapped)
          { 
// 	printf("Sending event\n");
	
	      XEvent Event;
	  
	      Event.xclient.type              = ClientMessage;
	      Event.xclient.display           = kapp->getDisplay();
	      Event.xclient.message_type      = kapp->getDndProtocolAtom();
	      Event.xclient.format            = 32;
	      Event.xclient.window            = dndLastWindow;
	      Event.xclient.data.l[0]         = dndType;
	      Event.xclient.data.l[1]         = 0;
	      Event.xclient.data.l[2]         = 0;
	      Event.xclient.data.l[3]         = p.x();
	      Event.xclient.data.l[4]         = p.y();

              // Switch to "friendly" error handler.
              if (oldErrorHandler == 0L)
	          oldErrorHandler = XSetErrorHandler(myErrorHandler);
// 	      printf("1\n");
	      XSendEvent( kapp->getDisplay(), dndLastWindow, True, NoEventMask, &Event );	
// 	      printf("2\n");
	      XSync( kapp->getDisplay(), FALSE );	
// 	      printf("3\n");
	      (void) XSetErrorHandler(oldErrorHandler);
              oldErrorHandler = 0L;
          }

	  delete [] dndData;
	  dndData = 0L;
    }
   else
   {
//        printf("Root Drop Event\n");
       rootDropEvent( p.x(), p.y() );
   }

  delete dndIcon;
  dndIcon = 0L;
   
  dragEndEvent();
}

void KDNDWidget::mouseMoveEvent( QMouseEvent * _mouse )
{
  if ( !drag )
    {
	  dndMouseMoveEvent( _mouse );
	  return;
    }

    
  // dndIcon->hide();
  
  // QPoint p = mapToGlobal( _mouse->pos() );
  QPoint p = QCursor::pos();

  /* Window root;
  root = DefaultRootWindow( kapp->getDisplay() );  
  Window win = findRootWindow( p );
  printf("Window = '%x'  %i %i\n",win,dndOffsetX,dndOffsetY);
  printf("x=%i y=%i\n",p.x(),p.y()); */

  QPoint p2( p.x(), p.y() );
  p2.setX( p2.x() + dndOffsetX );
  p2.setY( p2.y() + dndOffsetY );
  dndIcon->move( p2 );
  // dndIcon->show();
}

void KDNDWidget::rootDropEvent( int _x, int _y )
{
  Window root;
  Window parent;
  Window *children;
  unsigned int cchildren;
    
//   printf("Root Window\n");
  root = DefaultRootWindow( kapp->getDisplay() );
//   printf("Query root tree\n");

  // Switch to "friendly" error handler.
  if (oldErrorHandler == 0L)
      oldErrorHandler = XSetErrorHandler(myErrorHandler);
  XQueryTree( kapp->getDisplay(), root, &root, &parent, &children, &cchildren );
    
  for ( uint i = 0; i < cchildren; i++ )
  {
	if ( children[i] == debugWin ) {}
// 	  printf("******************** root id = %ld *********************\n",children[i] );
      else
      {
	  XEvent Event;
      
	  XWindowAttributes Wattr;
	  XGetWindowAttributes(kapp->getDisplay(), children[i], &Wattr);
	  if (Wattr.map_state == IsUnmapped)
		continue;
 
	  Event.xclient.type              = ClientMessage;
	  Event.xclient.display           = kapp->getDisplay();
	  Event.xclient.message_type      = kapp->getDndRootProtocolAtom();
	  Event.xclient.format            = 32;
	  Event.xclient.window            = children[ i ];
	  Event.xclient.data.l[0]         = dndType;
	  Event.xclient.data.l[1]         = (long) time( 0L );
	  Event.xclient.data.l[2]         = 0;
	  Event.xclient.data.l[3]         = _x;
	  Event.xclient.data.l[4]         = _y;
	  XSendEvent( kapp->getDisplay(), children[ i ], True, NoEventMask, &Event);
	  XSync( kapp->getDisplay(), FALSE );	
      }
  }
    
  (void)XSetErrorHandler(oldErrorHandler);
  oldErrorHandler = 0L;
//   printf("Done\n");
  
  // Clean up.
  rootDropEvent();
}

void KDNDWidget::rootDropEvent()
{
  if ( dndIcon != 0L )
	delete dndIcon;
  dndIcon = 0L;
  
  if ( dndData != 0L )
	delete [] dndData;
  dndData = 0L;
}

KDNDWidget::~KDNDWidget()
{
  if ( dndData != 0L )
     delete [] dndData;
}

KDNDIcon::KDNDIcon( QPixmap &_pixmap, int _x, int _y ) :
  QWidget( 0L, 0L, WStyle_Customize | WStyle_Tool | WStyle_NoBorder )
{
  pixmap = _pixmap;
  
  setGeometry( _x, _y, _pixmap.width(), _pixmap.height() );
  show();
}


KDNDIcon::KDNDIcon( const KDNDIcon& icon )
{
  pixmap = icon.pixmap; // implicitly ref-counted
}


KDNDIcon& KDNDIcon::operator= ( const KDNDIcon& icon )
{
  if( this != &icon )
	pixmap = icon.pixmap;

  return *this;
}


void KDNDIcon::paintEvent( QPaintEvent * ) 
{
  bitBlt( this, 0,0, &pixmap );
}

void KDNDIcon::resizeEvent( QResizeEvent * )
{
  if ( pixmap.mask() != 0L )
	XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0, 
					   pixmap.mask()->handle(), ShapeSet );
}

KDNDIcon::~KDNDIcon()
{
}











