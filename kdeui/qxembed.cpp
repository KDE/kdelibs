/****************************************************************************
** $Id$
**
** Implementation of QXEmbed class
**
** Created :
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include <qapplication.h>
#include "qxembed.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>


// defined in qapplication_x11.cpp
extern Atom	qt_embedded_window;
extern Atom	qt_embedded_window_take_focus;
extern Atom	qt_embedded_window_focus_in;
extern Atom	qt_embedded_window_focus_out;
extern Atom	qt_embedded_window_tab_focus;
extern Atom	qt_embedded_window_support_tab_focus;
extern Atom	qt_wheel_event;
extern Atom	qt_unicode_key_press;
extern Atom	qt_unicode_key_release;
extern Atom	qt_wm_delete_window;

class QXEmbedData
{
public:
    QXEmbedData(){};
    ~QXEmbedData(){};
};


/*!
  \class QXEmbed qxembed.h

  \brief The QXEmbed class provides the base technology to embed
  windows of different applications on the X Window System

  An xembed widget serves as a container that can manage one single
  embedded X-window. These so-called client windows can be arbitrary
  QWidgets.

  Using xembed has a couple of significant advantages compared to a
  low-level call of XReparentWindow

  <ul>
  <li> The embedded window is integrated into the outer applications
  focus chain. With a plain XReparentWindow, it's unpredictable which
  application will get the focus and process key events.

  <li> The outer application always receives keyevents
  first. Therefore menu accelerators and other shortcuts continue to
  work.

  <li> The XDND drag and drop protocol is routed respectively. Plain
  XReparentWindow would break drag and drop for the embedded window.
  </ul>


  QXembed, however, also manages non-Qt windows. But as long as these
  windows do not support the Qt embedding protocol, the specific
  advantages are lost. This will result in completely broken and
  unpredictable focus handling, broken accelerators and disfunctional
  XDND drag and drop.

  Here's an overview of the Qt embedding protocol as implemented by
  QXEmbed and Qt:

  <ol>
  <li> The following X-Atoms are required:
          <ul>
	  <li> \c qew ("_QT_EMBEDDED_WINDOW")
	  <li> \c qew_take_focus ("_QT_EMBEDDED_WINDOW_TAKE_FOCUS")
	  <li> \c qew_focus_in ("_QT_EMBEDDED_WINDOW_FOCUS_IN")
	  <li> \c qew_focus_out ("_QT_EMBEDDED_WINDOW_FOCUS_OUT")
	  <li> \c qew_tab_focus ("_QT_EMBEDDED_WINDOW_TAB_FOCUS")
	  <li> \c qew_micro_focus_hint ("_QT_EMBEDDED_WINDOW_MICRO_FOCUS_HINT")
	  <li> \c qt_wheel_event ("_QT_WHEEL_EVENT")
	  <li> \c qt_unicode_key_press ("_QT_UNICODE_KEY_PRESS")
	  <li> \c qt_unicode_key_release ("_QT_UNICODE_KEY_RELEASE")
          <ul>

  <li> An embedded widget has the \c qew property set to 1. The \c qew
  property is of type \c XA_CARDINAL, format 32. It's the
  responsibility of the embedding widget to set this property
  respectively. When the window is released, the property should be
  set to 0.

  <li> If an embedded widget intents to take the focus (for example
  after it has been clicked when it supports the ClickFocus focus
  policy), it should send a \c qew_take_focus client message to its
  parent window.  The \window parameter of the client message has to
  be set to the window identifier of the parent window.

  <li> An embedded textinput widget that supports a micro focus hint
  may also want to inform the embedding application about this. For
  this purpose it can send \c qew_micro_focus_hint client messages to
  the parent window.  The \c window parameter should be set to the
  parent's window identifier, \c format is 32 and \c data.l[0] and \c
  data.l[1] contain the global x and y-coordinates.

  <li> The embedding widget serves as a focus proxy for the embedded
  window.  Therefore it has to route focus in/out message. Whenever it
  gets or looses the logical application focus, it shall send
  qew_focus_in or qew_focus_out to the embedded window.

  <li> keypress and keyrelease TODO

  <li> wheel events TODO

  <li> tab-focus chain TODO

  <li> Drag'n'drop XDND TODO

  </ol>

*/

/*!
  Constructs a xembed widget.

  The \e parent, \e name and \e f arguments are passed to the QFrame
  constructor.
 */
QXEmbed::QXEmbed(QWidget *parent, const char *name, WFlags f)
  : QWidget(parent, name, f)
{
    window = 0;
    setFocusPolicy(StrongFocus);

    //trick to create extraData();
    QCursor old = cursor();
    setCursor(Qt::blankCursor);
    setCursor(old);

    // we are interested int SubstructureNotify
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
}

/*!
  Destructor. Cleans up the focus if necessary.
 */
QXEmbed::~QXEmbed()
{
    static Atom wm_protocols = 0;
    if (!wm_protocols )
	wm_protocols = XInternAtom( qt_xdisplay(), "WM_PROTOCOLS", False );
//     if ( topLevelWidget()->isActiveWindow() ) {
// 	XEvent e;
// 	e.type = FocusIn;
// 	e.xfocus.window = topLevelWidget()->winId();
// 	e.xfocus.mode = NotifyNormal;
// 	e.xfocus.detail = NotifyDetailNone;
// 	XSendEvent(qt_xdisplay(), topLevelWidget()->winId(), 0, FALSE, &e);
//     }


    if ( window != 0 ) {
	XEvent ev;
	memset(&ev, 0, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = window;
	ev.xclient.message_type = wm_protocols;
	ev.xclient.format = 32;
	ev.xclient.data.s[0] = qt_wm_delete_window;
	XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
    }
    window = 0;
}


/*!
  Reimplimented to resize the embedded window respectively.
 */
void QXEmbed::resizeEvent(QResizeEvent*)
{
    if (window != 0)
	XResizeWindow(qt_xdisplay(), window, width(), height());
}

/*!
  Reimplimented to ensure the embedded window will be visible as well.
 */
void QXEmbed::showEvent(QShowEvent*)
{
    if (window != 0)
	XMapRaised(qt_xdisplay(), window);

}

/*!
  Reimplimented to route the keyevents to the embedded window.
 */
void QXEmbed::keyPressEvent( QKeyEvent *e )
{
    if (!window)
	return;

    XEvent ev;
    QString text = e->text();
    int i = 1;
    int m = QMAX(1, text.length());
    do{
	memset(&ev, 0, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = window;
	ev.xclient.message_type = qt_unicode_key_press;
	ev.xclient.format = 16;
	ev.xclient.data.s[0] = e->key();
	ev.xclient.data.s[1] = e->ascii();
	ev.xclient.data.s[2] = e->state();
	ev.xclient.data.s[3] = e->isAutoRepeat();
	ev.xclient.data.s[4] = !text.isEmpty()?1:e->count();
	ev.xclient.data.s[5] = !text.isEmpty()?text[i-1].row():QChar::null.row();
	ev.xclient.data.s[6] = !text.isEmpty()?text[i-1].cell():QChar::null.cell();
	ev.xclient.data.s[7] = i++;
	ev.xclient.data.s[8] = m;
	XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
    } while ( i <= m);
}

/*!
  Reimplimented to route the keyevents to the embedded window.
 */
void QXEmbed::keyReleaseEvent( QKeyEvent *e )
{
    if (!window)
	return;

    XEvent ev;
    QString text = e->text();
    int i = 1;
    int m = QMAX(1, text.length());
    do{
	memset(&ev, 0, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = window;
	ev.xclient.message_type = qt_unicode_key_release;
	ev.xclient.format = 16;
	ev.xclient.data.s[0] = e->key();
	ev.xclient.data.s[1] = e->ascii();
	ev.xclient.data.s[2] = e->state();
	ev.xclient.data.s[3] = e->isAutoRepeat();
	ev.xclient.data.s[4] = !text.isEmpty()?1:e->count();
	ev.xclient.data.s[5] = !text.isEmpty()?text[i-1].row():QChar::null.row();
	ev.xclient.data.s[6] = !text.isEmpty()?text[i-1].cell():QChar::null.cell();
	ev.xclient.data.s[7] = i++;
	ev.xclient.data.s[8] = m;
	XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
    } while ( i <= m);
}

/*!
  Reimplimented to route the focus events to the embedded window.
 */
void QXEmbed::focusInEvent( QFocusEvent * ){
    if (!window)
	return;
    sendFocusIn();
}

/*!
  Reimplimented to route the focus events to the embedded window.
 */
void QXEmbed::focusOutEvent( QFocusEvent * ){
    if (!window)
	return;
    sendFocusOut();
}


/*!
  Reimplimented to route the wheel events to the embedded window.
 */
void QXEmbed::wheelEvent( QWheelEvent * e)
{
    if (!window)
	return;

    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = window;
    ev.xclient.message_type = qt_wheel_event;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = e->globalX();
    ev.xclient.data.l[1] = e->globalY();
    ev.xclient.data.l[2] = e->delta();
    ev.xclient.data.l[3] = e->state();
    XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
}


/*!

  Embeds the window with the identifier \a w into this xembed widget.

  This function is useful if the server knows about the client window
  that should be embedded.  Often it is vice versa: the client knows
  about his target container. In that case, it is not necessary to
  call embed(). Instead, the client will call the static function
  embedClientIntoWindow().

  \sa embeddedWinId()
 */
void QXEmbed::embed(WId w)
{
    if (!w)
	return;

    bool has_window =  w == window;

    window = w;
    long a = 1;
    window_supports_tab_focus = FALSE;
    XChangeProperty(qt_xdisplay(), w,
		    qt_embedded_window, XA_CARDINAL, 32, PropModeReplace,
		    (const unsigned char*)&a, 1);
    if ( !has_window )
	XReparentWindow(qt_xdisplay(), w, winId(), 0, 0);
    QApplication::syncX();
    XResizeWindow(qt_xdisplay(), w, width(), height());
    XMapRaised(qt_xdisplay(), window);
    extraData()->xDndProxy = w;

    if ( parent() ) {
	QEvent * layoutHint = new QEvent( QEvent::LayoutHint );
	QApplication::postEvent( parent(), layoutHint );
    }
    windowChanged( window );

    if (this == qApp->focusWidget() )
	sendFocusIn();
    else
	sendFocusOut();
}


/*!
  Returns the window identifier of the embedded window, or 0 if no
  window is embedded yet.
 */
WId QXEmbed::embeddedWinId() const
{
    return window;
}

void QXEmbed::sendFocusIn()
{
    XClientMessageEvent client_message;
    client_message.type = ClientMessage;
    client_message.window = window;
    client_message.format = 32;
    client_message.message_type = qt_embedded_window_focus_in;
    XSendEvent( qt_xdisplay(), client_message.window, FALSE, NoEventMask,
		(XEvent*)&client_message );
}

void QXEmbed::sendFocusOut()
{
    XClientMessageEvent client_message;
    client_message.type = ClientMessage;
    client_message.window = window;
    client_message.format = 32;
    client_message.message_type = qt_embedded_window_focus_out;
    XSendEvent( qt_xdisplay(), client_message.window, FALSE, NoEventMask,
		(XEvent*)&client_message );
}


/*!\reimp
 */
bool QXEmbed::focusNextPrevChild( bool next )
{
    if ( window && window_supports_tab_focus )
	return FALSE;
    else
	return QWidget::focusNextPrevChild( next );
}


/*!
  Reimplemented to observe child window changes
 */
bool QXEmbed::x11Event( XEvent* e)
{
    switch ( e->type ) {
    case DestroyNotify:
	if ( e->xdestroywindow.window == window ) {
	    window = 0;
	    windowChanged( window );
	    emit embeddedWindowDestroyed();
	}
	break;
    case ReparentNotify:
	if ( window && e->xreparent.window == window &&
	     e->xreparent.parent != winId() ) {
	    // we lost the window
	    window = 0;
	    windowChanged( window );
	} else if ( e->xreparent.parent == winId() ){
	    // we got a window
	    window = e->xreparent.window;
	    embed( window );
	}
	break;
    case MapRequest:
	if ( window && e->xmaprequest.window == window )
	    XMapRaised(qt_xdisplay(), window );
	break;
    case ClientMessage:
	if ( e->xclient.format == 32 && e->xclient.message_type ) {
	    if  ( e->xclient.message_type == qt_embedded_window_support_tab_focus ) {
		window_supports_tab_focus = TRUE;
	    }
	    else if  ( e->xclient.message_type == qt_embedded_window_tab_focus ) {
		window_supports_tab_focus = TRUE;
		QWidget::focusNextPrevChild( e->xclient.data.l[0] );
	    }
	}
    default:
	break;
    }
    return FALSE;
}


/*!
  A change handler that indicates that the embedded window has been
  changed.  The window handle can also be retrieved with
  embeddedWinId().
 */
void QXEmbed::windowChanged( WId )
{
}


/*!
  A utility function for clients that embed theirselves. The widget \a
  client will be embedded in the window that is passed as
  \c -embed command line argument.

  The function returns TRUE on sucess or FALSE if no such command line
  parameter is specified.

  \sa embedClientIntoWindow()
 */
bool QXEmbed::processClientCmdline( QWidget* client, int& argc, char ** argv )
{
    int myargc = argc;
    WId window = 0;
    int i, j;

    j = 1;
    for ( i=1; i<myargc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	if ( strcmp(arg,"-embed") == 0 && i < myargc-1 ) {
	    QCString s = argv[++i];
	    window = s.toInt();
	} else
	    argv[j++] = argv[i];
    }
    argc = j;

    if ( window != 0 ) {
	embedClientIntoWindow( client, window );
	return TRUE;
    }

    return FALSE;
}


/*!
  A utility function for clients that embed theirselves. The widget \a
  client will be embedded in the window \a window. The application has
  to ensure that \a window is the handle of the window identifier of
  an QXEmbed widget.

  \sa processClientCmdline()
 */
void QXEmbed::embedClientIntoWindow(QWidget* client, WId window)
{
    XReparentWindow(qt_xdisplay(), client->winId(), window, 0, 0);
    client->show();
}



/*!
  Specifies that this widget can use additional space, and that it can
  survive on less than sizeHint().
*/

QSizePolicy QXEmbed::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}


/*!
  Returns a size sufficient for the embedded window
*/
QSize QXEmbed::sizeHint() const
{
    return minimumSizeHint();
}


/*!
  Returns a size sufficient for one character, and scroll bars.
*/

QSize QXEmbed::minimumSizeHint() const
{
    int minw = 0;
    int minh = 0;
    if ( window ) {
	XSizeHints size;
	long msize;
	if (XGetWMNormalHints(qt_xdisplay(), window, &size, &msize)
	    && ( size.flags & PMinSize) ) {
	    minw = size.min_width;
	    minh = size.min_height;
	}
    }

    return QSize( minw, minh );
}

// for KDE
#include "qxembed.moc"
