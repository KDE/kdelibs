#include "kclipboard.h"

#include <qdatetime.h>
#include <qapplication.h>
#define  GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>  

#include <assert.h>
#include <iostream.h>
#include <ctype.h>
#include <string.h>

// Here we depend on Qt not to change their implementation!!!
extern Time qt_x_clipboardtime;                 // def. in qapp_x11.cpp   
extern QObject *qt_clipboard;                   // defined in qapp_xyz.cpp

KClipboard* KClipboard::s_pSelf = 0L;

KClipboard* KClipboard::self()
{
  if ( s_pSelf == 0L )
    s_pSelf = new KClipboard;
  
  return s_pSelf;
}

KClipboard::KClipboard()
{
  if ( s_pSelf )
  {
    cerr << "You may only open one KClipboard at once" << endl;
    assert( 0 );
  }
  
  if ( qt_clipboard != 0L )
  {    
    cerr << "KClipboard::KClipboard There is already a clipboard registered\n" << endl;
    assert( 0 );
  }
    
  qt_clipboard = this;
    
  m_bOwner = false;
  m_pOwner = 0L;
  m_bEmpty = true;    
  m_mimeTypeLen = 0;
}

KClipboard::~KClipboard()
{
}
   
bool KClipboard::open( int _mode )
{
  return open( _mode, "application/octet-stream" );
}

bool KClipboard::open( int _mode, const QString& _format )
{
  if ( _mode != IO_ReadOnly && _mode != ( IO_WriteOnly | IO_Truncate ) && _mode != IO_WriteOnly )
  {    
    cerr << "KClipboard: Wrong flags in call for Ken" << endl;
    assert( 0 );
  }
    
  if ( _mode == IO_WriteOnly )
    _mode |= IO_Truncate;

  if ( _mode == ( IO_WriteOnly | IO_Truncate ) )
  {
    m_bEmpty = false;
    m_strFormat = _format;

    cerr << "Fuck ya too" << endl;

    QBuffer::open( _mode );
    if ( _format != "application/octet-stream" && 
	 _format != "text/plain" ) 
    {
      m_mimeTypeLen = _format.length() + 1;
      writeBlock( _format.ascii(), m_mimeTypeLen );
    }
    else
      m_mimeTypeLen = 0;
    
    cerr << "2 Fuck ya too" << endl;

    return true;
  }
  else if ( ( _mode & IO_ReadOnly ) == IO_ReadOnly )
  {
    // printf("isOwner %i\n", ( isOwner() ? 1:0 ) );
    // printf("isEmpty %i\n", ( isEmpty() ? 1:0 ) );
    if ( !isOwner() )
      fetchData();
      
    if ( m_strFormat != _format && _format != "application/octet-stream" )
      return false;

    if ( !QBuffer::open( _mode ) )
      return false;
    at( m_mimeTypeLen );

    return true;
  }
  else
    assert( 0 );
}

const QString KClipboard::format()
{
  if ( !isOwner() )
    fetchData();

  return m_strFormat;  
}
  
void KClipboard::close()
{
  if ( ( mode() & IO_WriteOnly ) == IO_WriteOnly )
    setOwner();    
 
  QBuffer::close();
 
  if ( ( mode() & IO_ReadOnly ) == IO_ReadOnly && !isOwner() )
    clear();
}
    
void KClipboard::clear()
{
  m_bEmpty = true;
  m_strFormat = "";
  
  buffer().resize( 0 );
  
  if ( isOwner() )
    setOwner();
}

bool KClipboard::isEmpty()
{
  return m_bEmpty;
}

bool KClipboard::isOwner()
{
  return m_bOwner;
}

void KClipboard::setOwner()
{
  if ( isOwner() )
    return;

  // printf("Setting owner\n");
    
  QWidget *owner = makeOwner();
  Window win = owner->winId();
  Display *dpy = owner->x11Display();
    
  XSetSelectionOwner( dpy, XA_PRIMARY, win, qt_x_clipboardtime );
  if ( XGetSelectionOwner( dpy, XA_PRIMARY ) != win )
  {
    cerr <<  "KClipboard::setOwner: Cannot set X11 selection owner" << endl;
    return;
  }                            

  m_bOwner = true;
}

void KClipboard::fetchData()
{
  // printf("Getting data\n");
    
  if ( isOwner() )
    return;

  // printf("Doing it really!\n");
    
  clear();
    
  QWidget *owner = makeOwner();
  Window   win   = owner->winId();
  Display *display   = owner->x11Display();

  if ( XGetSelectionOwner(display,XA_PRIMARY) == None )
    return;

  Atom prop = XInternAtom( display, "QT_SELECTION", FALSE );
  XConvertSelection( display, XA_PRIMARY, XA_STRING, prop, win, CurrentTime );

  /** DEBUG code */
  XFlush( display );
  /** End DEBUG code */

  XEvent xevent;

  /** DEBUG code */
  QTime started = QTime::currentTime();
  while ( TRUE )
  {
    if ( XCheckTypedWindowEvent(display,win,SelectionNotify,&xevent) )
      break;
    QTime now = QTime::currentTime();
    if ( started > now )
      started = now;
    if ( started.msecsTo(now) > 5000 )
    {
      return;
    }
  }
  /** End Debug code */

  win  = xevent.xselection.requestor;
  prop = xevent.xselection.property;

  int nread = 0;
  Atom type;
  ulong nitems, bytes_after;
  int format;
  uchar *result;

  QBuffer::open( IO_WriteOnly | IO_Truncate );

  bool first = true;
  
  do 
  {
    int n = XGetWindowProperty( display, win, prop, nread/4, 1024, TRUE,
				AnyPropertyType, &type, &format, &nitems,
				&bytes_after, &result );
    if ( n != Success || type != XA_STRING )
      break;

    if ( first )
    {
      first = false;

      unsigned int i;
      for( i = 0; i < nitems; i++ )
	if ( result[i] == 0 )
	  break;
      
      if ( i < nitems )
      {
	m_mimeTypeLen = i + 1;
	m_strFormat = reinterpret_cast<const char*>(result);
      }
      else
      {  
	m_strFormat = "";
      }
    }
    
    writeBlock( (const char*)result, nitems );
    nread += nitems;

    XFree( (char *)result );
    
  } while ( bytes_after > 0 );

  // printf("#################### READ %i bytes\n", nread );
    
  QBuffer::close();

  if ( m_strFormat.isEmpty() )
  {
    // Find non printable characters
    QByteArray ba = buffer();
    QString d(ba);
    int len = ba.size();
    for( int j = 0; j < len; j++ )
    {
      if (
	    d[j].unicode() < ' '  // #### Can use isPrintable()
	&& d[j] != '\n' && d[j] != '\r' && d[j] != '\t' )
      {
	m_strFormat = "application/octet-stream";
	return;
      }
    }
    
    m_strFormat = "text/plain";  
  }
  
  return;
}

bool KClipboard::event( QEvent *e )
{
  if ( e->type() != QEvent::Clipboard )
    return false;

  Display *display = qt_xdisplay();
  XEvent *xevent = static_cast<XEvent*>(static_cast<QCustomEvent*>(e)->data());

  switch ( xevent->type )
  {
  case SelectionNotify:
    // printf("NOTIFY\n");
    m_bOwner = false;
    clear();
    break;

  case SelectionRequest:
    // printf("REQUEST\n");
    {
      // printf("Sending %i bytes\n",size());
	    
      XEvent xev;
      XSelectionRequestEvent *xreqev = &xevent->xselectionrequest;
      xev.xselection.type = SelectionNotify;
      xev.xselection.display = xreqev->display;
      xev.xselection.requestor = xreqev->requestor;
      xev.xselection.selection = xreqev->selection;
      xev.xselection.target = xreqev->target;
      xev.xselection.property = None;
      xev.xselection.time = xreqev->time;
      
      if ( xreqev->target == XA_STRING )
      {
	XChangeProperty ( display, xreqev->requestor, xreqev->property, XA_STRING, 8,
                                  PropModeReplace, (uchar *)buffer().data(), buffer().size() );
	xev.xselection.property = xreqev->property;
      }
      XSendEvent( display, xreqev->requestor, False, 0, &xev );
    }
  break;

  case SelectionClear:                    // new selection owner
    // printf("CLEAR\n");
    m_bOwner = false;
    clear();
    emit ownerChanged();
    break;

  }

  return true;
}                                              

QWidget* KClipboard::makeOwner()
{
  // Fake some clipboard owner
  if ( m_pOwner )  
    return m_pOwner;
  if ( qApp->mainWidget() )
    m_pOwner = qApp->mainWidget();
  else                     
    m_pOwner = new QWidget( 0L );
  return m_pOwner;
}            

void KClipboard::setURLList( QStrList& _urls )
{
  open( IO_WriteOnly | IO_Truncate, "url/url" );
  
  const char* s;
  for( s = _urls.first(); s != 0L; s = _urls.next() )
  {
    if ( s == _urls.getLast() )
      writeBlock( s, strlen( s ) );
    else
      writeBlock( s, strlen( s ) + 1 );
  }
  
  close();
}

bool KClipboard::urlList( QStrList& _urls)
{
  if ( !isOwner() )
    fetchData();

  if ( m_strFormat != "url/url" )
    return false;

  QByteArray ba = octetStream();
  char* d = ba.data();
  unsigned int c = 0;

  while ( c < ba.size() )
  {
    unsigned int start = c;
    // Find zero or end
    while ( d[c] != 0 && c < ba.size() )
      c++;

    if ( c < ba.size() )
    {
      _urls.append( d + start );
      // Skip zero
      c++;
    }
    else
    {
      char* s = new char[ c - start + 1 ];
      memcpy( s, d + start, c - start );
      s[ c - start ] = '\0';
      _urls.append( s );
      delete []s;
    }
  }

  return true;
}

void KClipboard::setText( const QString& _text )
{
  open( IO_WriteOnly | IO_Truncate, "text/plain" );  

  writeBlock( _text.ascii(), _text.length() );
  
  close();
}

const QString KClipboard::text()
{
  if ( !isOwner() )
    fetchData();

 if ( m_strFormat != "text/plain" )
    return QString::null;

  QByteArray ba = buffer();
  return ba.data() + m_mimeTypeLen;
}

void KClipboard::setOctetStream( QByteArray& _arr )
{
  open( IO_WriteOnly | IO_Truncate );  

  writeBlock( _arr.data(), _arr.size() );
  
  close();
}

QByteArray KClipboard::octetStream()
{
  if ( !isOwner() )
    fetchData();

  QByteArray ba;
  ba.duplicate( buffer().data() + m_mimeTypeLen, buffer().size() - m_mimeTypeLen );
  
  return ba;
}

#include "kclipboard.moc"

