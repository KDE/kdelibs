/****************************************************************************
    Implementation of QXEmbed class

   Copyright (C) 1999-2000 Troll Tech AS

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
*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#if HAVE_USLEEP
#include <unistd.h>
#endif // HAVE_USLEEP
#endif // HAVE_CONFIG_H
#include "javaembed.h"

#include <kdebug.h>
#include <klocale.h>

#include <qapplication.h>
#include <qevent.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
// avoid name clashes between X and Qt
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyRelease
#undef KeyPress
#undef FocusOut
#undef FocusIn

#include "kqeventutil.h"
#include "kxeventutil.h"
class KJavaEmbedPrivate
{
friend class KJavaEmbed;
public:
  QPoint lastPos;
};

/*!\reimp
 */
bool KJavaEmbed::eventFilter( QObject* o, QEvent* e)
{
    QEvent::Type t = e->type();

    if( t != QEvent::MouseMove && t != QEvent::Timer && t <= QEvent::User )
    {
        //kdDebug(6100) << "KJavaEmbed::eventFilter, event = " << KQEventUtil::getQtEventName( e ) << endl;
        if( o == this )
        {
            //kdDebug(6100) << "event is for me:)" << endl;
        }
        switch ( e->type() )
        {
            
            case QEvent::FocusIn:
                break;

            case QEvent::FocusOut:
                break;
                
            case QEvent::Leave:
                /* check to see if we are entering the applet somehow... */
                break;

            case QEvent::Enter:
                break;

            case QEvent::WindowActivate:
    	        break;

            case QEvent::WindowDeactivate:
    	        break;
                
            case QEvent::Move:
               {
                    // if the browser window is moved or the page scrolled,
                    // AWT buttons do not respond anymore (although they are
                    // visually updated!) and subframes of select boxes appear
                    // on completely wrong (old) positions.
                    // This fixes the problem by notifying the applet.
                    QPoint globalPos = mapToGlobal(QPoint(0,0));
                    if (globalPos != d->lastPos) {
                        d->lastPos = globalPos;
                        sendSyntheticConfigureNotifyEvent();
                    }
                }                    
                break;
                
            default:
                break;
        }

    }

    return FALSE;
}

/*!
  Constructs a xembed widget.

  The \e parent, \e name and \e f arguments are passed to the QFrame
  constructor.
 */
KJavaEmbed::KJavaEmbed( QWidget *parent, const char *name, WFlags f )
  : QWidget( parent, name, f )
{
    d = new KJavaEmbedPrivate;

    setFocusPolicy( StrongFocus );
    setKeyCompression( FALSE );
    setAcceptDrops( TRUE );

    window = 0;
    // we are interested in SubstructureNotify
    XSelectInput(qt_xdisplay(), winId(),
                 KeyPressMask | KeyReleaseMask |
                 ButtonPressMask | ButtonReleaseMask |
                 KeymapStateMask |
                 ButtonMotionMask |
                 PointerMotionMask | // may need this, too
                 EnterWindowMask | LeaveWindowMask |
                 FocusChangeMask |
                 ExposureMask |
                 StructureNotifyMask |
                 SubstructureRedirectMask |
                 SubstructureNotifyMask
                 );

    topLevelWidget()->installEventFilter( this );
    qApp->installEventFilter( this );
}

/*!
  Destructor. Cleans up the focus if necessary.
 */
KJavaEmbed::~KJavaEmbed()
{
    //kdDebug(6100) << "~KJavaEmbed() window=" << window << endl;
    if ( window != 0 )
    {
        XUnmapWindow( qt_xdisplay(), window );
        QApplication::flushX();
    }

    delete d;
}

/*!\reimp
 */
void KJavaEmbed::resizeEvent( QResizeEvent* e )
{
    //kdDebug(6100) << "KJavaEmbed::resizeEvent width=" << e->size().width() << " height=" << e->size().height() <<endl;
    QWidget::resizeEvent( e );

    if ( window != 0 )
        XResizeWindow( qt_xdisplay(), window, e->size().width(), e->size().height() );
}

bool  KJavaEmbed::event( QEvent* e)
{
    //kdDebug(6100) << "KJavaEmbed::event, event type = " << KQEventUtil::getQtEventName( e ) << endl;
    switch( e->type() )
    {
        case QEvent::ShowWindowRequest:
        case QEvent::WindowActivate:
            //kdDebug(6100) << "XMapRaised window=" << window << endl;
            if (window != 0) {
                XMapRaised( qt_xdisplay(), window );
                QApplication::syncX();
            }
            break;
        default:
            break;
    }
    return QWidget::event( e );
}

/*!\reimp
 */
void KJavaEmbed::focusInEvent( QFocusEvent* )
{
    //kdDebug(6100) << "KJavaEmbed::focusInEvent" << endl;

    if ( !window )
        return;

    XEvent ev;
    memset( &ev, 0, sizeof( ev ) );
    ev.xfocus.type = XFocusIn;
    ev.xfocus.window = window;

    XSendEvent( qt_xdisplay(), window, true, NoEventMask, &ev );
}

/*!\reimp
 */
void KJavaEmbed::focusOutEvent( QFocusEvent* )
{
    //kdDebug(6100) << "KJavaEmbed::focusOutEvent" << endl;

    if ( !window )
        return;

    XEvent ev;
    memset( &ev, 0, sizeof( ev ) );
    ev.xfocus.type = XFocusOut;
    ev.xfocus.window = window;
    XSendEvent( qt_xdisplay(), window, true, NoEventMask, &ev );
}

static bool wstate_withdrawn( WId winid )
{
   // defined in qapplication_x11.cpp
   extern Atom qt_wm_state;
   Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;
    int r = XGetWindowProperty( qt_xdisplay(), winid, qt_wm_state, 0, 2,
                                FALSE, AnyPropertyType, &type, &format,
                                &length, &after, &data );
    bool withdrawn = TRUE;
    if ( r == Success && data && format == 32 ) {
        Q_UINT32 *wstate = (Q_UINT32*)data;
        withdrawn  = (*wstate == WithdrawnState );
        XFree( (char *)data );
    }
    return withdrawn;
}

/*!
  Embeds the window with the identifier \a w into this xembed widget.

  This function is useful if the server knows about the client window
  that should be embedded.  Often it is vice versa: the client knows
  about its target embedder. In that case, it is not necessary to call
  embed(). Instead, the client will call the static function
  embedClientIntoWindow().

  \sa embeddedWinId()
 */
void KJavaEmbed::embed( WId w )
{
    //kdDebug(6100) << "KJavaEmbed::embed " << w << endl;

    if ( w == 0 )
        return;

    window = w;

    //first withdraw the window
    QApplication::flushX();
    if (!wstate_withdrawn(window)) {
        int status = XWithdrawWindow( qt_xdisplay(), window, qt_xscreen() );
        if (status > 0) {
            unsigned long cnt = 0;
            unsigned long max = 1000;
            for (cnt = 0; !wstate_withdrawn(window) && cnt < max; cnt++) {
#if HAVE_USLEEP
                usleep(1000); // 1 ms 
#endif
            }
            if (cnt < max) { 
                //kdDebug(6100) 
                //    << "KJavaEmbed::embed: window withdrawn after " 
                //    << cnt << " loops" << endl;
            } else {
                kdDebug(6100) 
                    << "KJavaEmbed::embed: window still not withdrawn after " 
                    << cnt << " loops " << endl;
            }
        } else {
            kdDebug(6100) << "KJavaEmbed::embed: XWithdrawWindow returned status=" << status << endl;
        }
    }

    //now reparent the window to be swallowed by the KJavaEmbed widget
    //kdDebug(6100) << "++++++++++++++ Reparent embedder=" << winId() << " applet=" << window << endl;
    XReparentWindow( qt_xdisplay(), window, winId(), 0, 0 );
    QApplication::syncX();
    
    //now resize it
    XResizeWindow( qt_xdisplay(), window, width(), height() );
    XMapRaised( qt_xdisplay(), window );
        
}

/*!\reimp
 */
bool KJavaEmbed::focusNextPrevChild( bool next )
{
    if ( window )
        return FALSE;
    else
        return QWidget::focusNextPrevChild( next );
}

/*!\reimp
 */
bool KJavaEmbed::x11Event( XEvent* e)
{
    // kdDebug(6100) << "KJavaEmbed::x11Event " << KXEventUtil::getX11EventInfo( e ) << endl;
    switch ( e->type )
    {
        case DestroyNotify:
            if ( e->xdestroywindow.window == window )
            {
                window = 0;
            }
            break;
        case ConfigureRequest:
            if (e->xconfigurerequest.window == window 
                && e->xconfigurerequest.value_mask == (CWX|CWY)) 
            {
                    /*
                    XMoveResizeWindow(e->xconfigurerequest.display, window,
                        0,
                        0,
                        width(),
                        height());
                    */
                    sendSyntheticConfigureNotifyEvent();
            }
            break;
        default:
	        break;
    }

    return false;
}

void KJavaEmbed::sendSyntheticConfigureNotifyEvent() {
    QPoint globalPos = mapToGlobal(QPoint(0,0));
    if (window) {
        // kdDebug(6100) << "*************** sendSyntheticConfigureNotify ******************" << endl;
        XConfigureEvent c;
        memset(&c, 0, sizeof(c));
        c.type = ConfigureNotify;
        c.display = qt_xdisplay();
        c.send_event = True;
        c.event = window;
        c.window = winId();
        c.x = globalPos.x();
        c.y = globalPos.y();
        c.width = width();
        c.height = height();
        c.border_width = 0;
        c.above = None;
        c.override_redirect = 0;
        XSendEvent( qt_xdisplay(), c.event, TRUE, StructureNotifyMask, (XEvent*)&c );
        //kdDebug(6100) << "SENT " << KXEventUtil::getX11EventInfo((XEvent*)&c) << endl;
    }
}

/*!
  Specifies that this widget can use additional space, and that it can
  survive on less than sizeHint().
*/

QSizePolicy KJavaEmbed::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

/*!
  Returns a size sufficient for the embedded window
*/
QSize KJavaEmbed::sizeHint() const
{
    return minimumSizeHint();
}

/*!
  Returns the minimum size specified by the embedded window.
*/
QSize KJavaEmbed::minimumSizeHint() const
{
    if ( window )
    {
        kdDebug(6100) << "KJavaEmbed::minimumSizeHint, getting hints from window" << endl;

        XSizeHints size;
        long msize;
        if( XGetWMNormalHints( qt_xdisplay(), window, &size, &msize ) &&
            ( size.flags & PMinSize) )
        {
            kdDebug(6100) << "XGetWMNormalHints succeeded, width = " << size.min_width
                          << ", height = " << size.min_height << endl;

            return QSize( size.min_width, size.min_height );
        }
    }

    return QSize( 0, 0 );
}

// for KDE
#include "javaembed.moc"
