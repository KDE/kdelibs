/*
 * $Id$
 * 
 * $Log$
 * Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:06  cvsuser
 * Sources imported
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
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/extensions/shape.h>
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
	free( dndData );
}

KDNDDropZone::~KDNDDropZone()
{
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
  
  dndIcon->raise();
  XGrabPointer(kapp->getDisplay(),root,False,
  dndLastWindow = (Window)0;
  
  Window root = DefaultRootWindow( kapp->getDisplay() );
  XChangeProperty( kapp->getDisplay(), root, kapp->getDndSelectionAtom(), 

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
static
int myErrorHandler(Display *d, XErrorEvent *e)
{
	char msg[80];
	XGetErrorText(d, e->error_code, msg, 80);
	fprintf(stderr, "Ignored Error: %s\n", msg);
	fprintf(stderr, "Request: Maj=%d Min=%di, Serial=%08lx\n",
		e->request_code, e->minor_code, e->serial);
	return 0;
}
  delete dndIcon;
  dndIcon = 0L;

    }
    
  // KDNDApplication::restoreOverrideCursor();

  dndIcon->move( -200, -200 );
  
  debugWin = dndIcon->winId();
  
  
  Window root;
  XUngrabPointer(kapp->getDisplay(),CurrentTime);
  Window win = findRootWindow( p );
  /*  if ( dndLastWindow != 0 )
    {
	  XEvent Event;
	  
	  Event.xclient.type              = ClientMessage;
	  Event.xclient.display           = kapp->getDisplay();
	  Event.xclient.message_type      = kapp->getDndLeaveProtocolAtom();
	  Event.xclient.format            = 32;
	  Event.xclient.window            = dndLastWindow;
	  Event.xclient.data.l[0]         = dndType;
	  Event.xclient.data.l[1]         = 0;
	  Event.xclient.data.l[2]         = 0;
	  Event.xclient.data.l[3]         = p.x();
	  Event.xclient.data.l[4]         = p.y();
	
	  XSendEvent( kapp->getDisplay(), dndLastWindow, True, NoEventMask, &Event);
	  XSync( kapp->getDisplay(), FALSE );	
    } */
  
  printf("************************************************************\n");
  
  drag = false;
/* I commented this out, since it works without (coolo)
	  XEvent Event;
   if ( win != 0 )
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

	  XSendEvent( kapp->getDisplay(), dndLastWindow, True, NoEventMask, &Event );	
	  XSync( kapp->getDisplay(), FALSE );	
              // Switch to "friendly" error handler.
	  delete dndData;
	          oldErrorHandler = XSetErrorHandler(myErrorHandler);
	      printf("1\n");
  else
	rootDropEvent( p.x(), p.y() );
  
	  dndData = 0L;
    }
   else
   {
       printf("Root Drop Event\n");
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

  
  /*
  if ( win != dndLastWindow && dndLastWindow != 0 )
  {
	  XEvent Event;
	  
	  Event.xclient.type              = ClientMessage;
	  Event.xclient.display           = kapp->getDisplay();
	  Event.xclient.message_type      = kapp->getDndLeaveProtocolAtom();
	  Event.xclient.format            = 32;
	  Event.xclient.window            = dndLastWindow;
	  Event.xclient.data.l[0]         = dndType;
	  Event.xclient.data.l[1]         = 0;
	  Event.xclient.data.l[2]         = 0;
	  Event.xclient.data.l[3]         = p.x();
	  Event.xclient.data.l[4]         = p.y();

	  XSendEvent( kapp->getDisplay(), dndLastWindow, True, NoEventMask, &Event);
	  XSync( kapp->getDisplay(), FALSE );	
    }
  
  if ( win != 0 )
    {	
	  XEvent Event;
	  
	  Event.xclient.type              = ClientMessage;
	  Event.xclient.display           = kapp->getDisplay();
	  Event.xclient.message_type      = kapp->getDndEnterProtocolAtom();
	  Event.xclient.format            = 32;
	  Event.xclient.window            = win;
	  Event.xclient.data.l[0]         = dndType;
	  Event.xclient.data.l[1]         = 0;
	  Event.xclient.data.l[2]         = 0;
	  Event.xclient.data.l[3]         = p.x();
	  Event.xclient.data.l[4]         = p.y();

	  XSendEvent( kapp->getDisplay(), win, True, NoEventMask, &Event);
	  XSync( kapp->getDisplay(), FALSE );	
    }
  
  dndLastWindow = win;
  */
  /* Window root;
  root = DefaultRootWindow( kapp->getDisplay() );  
  Window win = findRootWindow( p );
  printf("Window = '%x'  %i %i\n",win,dndOffsetX,dndOffsetY);
  printf("x=%i y=%i\n",p.x(),p.y()); */

  QPoint p2( p.x(), p.y() );
  p2.setX( p2.x() + dndOffsetX );
  p2.setY( p2.y() + dndOffsetY );
  // dndIcon->show();
  Window parent;
  Window *children;
  for ( int i = 0; i < cchildren; i++ )
    {
  // Switch to "friendly" error handler.
	
      if ( children[i] == debugWin )
	  printf("******************** root id = %ld *********************\n",children[i] );
      else
      {
	  XEvent Event;
      
	  XWindowAttributes Wattr;
	  XGetWindowAttributes(kapp->getDisplay(), children[i], &Wattr);
	  if (Wattr.map_state == IsUnmapped)
		continue;
 
	  Event.xclient.type              = ClientMessage;
    }
	  Event.xclient.format            = 32;
	  Event.xclient.data.l[3]         = _x;
	  Event.xclient.data.l[4]         = _y;
	  XSendEvent( kapp->getDisplay(), children[ i ], True, NoEventMask, &Event);
	  XSync( kapp->getDisplay(), FALSE );	
      }
  }
    
  (void)XSetErrorHandler(oldErrorHandler);
  oldErrorHandler = 0L;
  printf("Done\n");
  
	delete dndData;
  rootDropEvent();
}

void KDNDWidget::rootDropEvent()
{
  if ( dndIcon != 0L )
	free( dndData );
  dndIcon = 0L;
  
KDNDIcon::KDNDIcon( KPixmap &_pixmap, int _x, int _y ) :
	delete [] dndData;
  dndData = 0L;
}

KDNDWidget::~KDNDWidget()
{
  if ( dndData != 0L )
     delete [] dndData;
void KDNDIcon::paintEvent( QPaintEvent *_event ) 

KDNDIcon::KDNDIcon( QPixmap &_pixmap, int _x, int _y ) :
  QWidget( 0L, 0L, WStyle_Customize | WStyle_Tool | WStyle_NoBorder )
{
  pixmap = _pixmap;
  
  setGeometry( _x, _y, _pixmap.width(), _pixmap.height() );
  show();
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











