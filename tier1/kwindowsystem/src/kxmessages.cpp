/****************************************************************************

 Copyright (C) 2001-2003 Lubos Lunak        <l.lunak@kde.org>
 Copyright 2012 David Faure <faure@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#include "kxmessages.h"

#if HAVE_X11

#include <qcoreapplication.h>
#include <QDebug>
#include <QWidget> // WId
#include <QAbstractNativeEventFilter>

#include <xcb/xcb.h>
#include <qx11info_x11.h>
#include <X11/Xlib.h>

class KXMessagesPrivate
    : public QAbstractNativeEventFilter
{
public:
    QWidget* handle;
    Atom accept_atom2;
    Atom accept_atom1;
    QMap< WId, QByteArray > incoming_messages;
    KXMessages* q;

    bool nativeEventFilter(const QByteArray& eventType, void *message, long *result) {
        Q_UNUSED(result);
        if (eventType != "xcb_generic_event_t")
            return false;
        xcb_generic_event_t* event = reinterpret_cast<xcb_generic_event_t *>(message);
        uint response_type = event->response_type & ~0x80;
        if (response_type != XCB_CLIENT_MESSAGE)
            return false;
        xcb_client_message_event_t * cm_event = reinterpret_cast<xcb_client_message_event_t *>(event);
        if (cm_event->format != 8)
            return false;
        if (cm_event->type != accept_atom1 && cm_event->type != accept_atom2)
            return false;
        char buf[ 21 ]; // can't be longer
        // Copy the data in order to null-terminate it
        qstrncpy(buf, reinterpret_cast<char *>(cm_event->data.data8), 21);
        //qDebug() << cm_event->window << "buf=\"" << buf << "\" atom=" << (cm_event->type == accept_atom1 ? "atom1" : "atom2");
        if (incoming_messages.contains(cm_event->window)) {
            if (cm_event->type == accept_atom1)
                // two different messages on the same window at the same time shouldn't happen anyway
                incoming_messages[cm_event->window] = QByteArray();
            incoming_messages[cm_event->window] += buf;
        } else {
            if (cm_event->type == accept_atom2) {
                return false; // middle of message, but we don't have the beginning
            }
            incoming_messages[cm_event->window] = buf;
        }
        if (strlen(buf) < 20) { // last message fragment
            emit q->gotMessage(QString::fromUtf8(incoming_messages[cm_event->window].constData()));
            incoming_messages.remove(cm_event->window);
        }
        return false; // lets other KXMessages instances get the event too
    }
};

static void send_message_internal(WId w_P, const QString& msg_P, long mask_P,
                                  Display* disp, Atom atom1_P, Atom atom2_P, Window handle_P);


// for broadcasting
static const long BROADCAST_MASK = PropertyChangeMask;
// CHECKME

KXMessages::KXMessages( const char* accept_broadcast_P, QObject* parent_P )
    : QObject( parent_P )
    , d( new KXMessagesPrivate )
{
    d->q = this;
    if( accept_broadcast_P != NULL ) {
        QCoreApplication::instance()->installNativeEventFilter(d);
        d->accept_atom1 = XInternAtom(QX11Info::display(), QByteArray(QByteArray( accept_broadcast_P ) + "_BEGIN").constData(), false);
        d->accept_atom2 = XInternAtom(QX11Info::display(), accept_broadcast_P, false);
    } else {
        d->accept_atom1 = d->accept_atom2 = None;
    }
    d->handle = new QWidget;
}

KXMessages::~KXMessages()
{
    delete d->handle;
    delete d;
}

void KXMessages::broadcastMessage(const char* msg_type_P, const QString& message_P, int screen_P)
{
    Atom a2 = XInternAtom( QX11Info::display(), msg_type_P, false );
    Atom a1 = XInternAtom( QX11Info::display(), QByteArray(QByteArray( msg_type_P ) + "_BEGIN").constData(), false );
    Window root = screen_P == -1 ? QX11Info::appRootWindow() : QX11Info::appRootWindow( screen_P );
    send_message_internal(root, message_P, BROADCAST_MASK, QX11Info::display(),
                          a1, a2, d->handle->winId());
}

bool KXMessages::broadcastMessageX(Display* disp, const char* msg_type_P,
                                   const QString& message_P, int screen_P)
{
    if( disp == NULL )
        return false;
    Atom a2 = XInternAtom( disp, msg_type_P, false );
    Atom a1 = XInternAtom( disp, QByteArray(QByteArray( msg_type_P ) + "_BEGIN").constData(), false );
    Window root = screen_P == -1 ? DefaultRootWindow( disp ) : RootWindow( disp, screen_P );
    Window win = XCreateSimpleWindow( disp, root, 0, 0, 1, 1,
                                      0, BlackPixel( disp, screen_P == -1 ? DefaultScreen( disp ) : screen_P ),
                                      BlackPixel( disp, screen_P == -1 ? DefaultScreen( disp ) : screen_P ));
    send_message_internal( root, message_P, BROADCAST_MASK, disp,
                           a1, a2, win );
    XDestroyWindow( disp, win );
    return true;
}

#if 0 // currently unused
void KXMessages::sendMessage(WId w_P, const char* msg_type_P, const QString& message_P)
{
    Atom a2 = XInternAtom( QX11Info::display(), msg_type_P, false );
    Atom a1 = XInternAtom( QX11Info::display(), QByteArray(QByteArray( msg_type_P ) + "_BEGIN").constData(), false );
    send_message_internal( w_P, message_P, 0, QX11Info::display(), a1, a2, d->handle->winId());
}

bool KXMessages::sendMessageX(Display* disp, WId w_P, const char* msg_type_P,
                              const QString& message_P)
{
    if( disp == NULL )
        return false;
    Atom a2 = XInternAtom( disp, msg_type_P, false );
    Atom a1 = XInternAtom( disp, QByteArray(QByteArray( msg_type_P ) + "_BEGIN").constData(), false );
    Window win = XCreateSimpleWindow( disp, DefaultRootWindow( disp ), 0, 0, 1, 1,
                                      0, BlackPixelOfScreen( DefaultScreenOfDisplay( disp )),
                                      BlackPixelOfScreen( DefaultScreenOfDisplay( disp )));
    send_message_internal( w_P, message_P, 0, disp, a1, a2, win );
    XDestroyWindow( disp, win );
    return true;
}
#endif

static void send_message_internal(WId w_P, const QString& msg_P, long mask_P,
                                  Display* disp, Atom atom1_P, Atom atom2_P, Window handle_P)
{
    //qDebug() << "send_message_internal" << w_P << msg_P << mask_P << atom1_P << atom2_P << handle_P;
    unsigned int pos = 0;
    QByteArray msg = msg_P.toUtf8();
    unsigned int len = strlen( msg.constData() );
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = atom1_P; // leading message
    e.xclient.display = disp;
    e.xclient.window = handle_P;
    e.xclient.format = 8;
    do
    {
        unsigned int i;
        for( i = 0;
             i < 20 && i + pos <= len;
             ++i )
            e.xclient.data.b[ i ] = msg[ i + pos ];
        XSendEvent( disp, w_P, false, mask_P, &e );
        e.xclient.message_type = atom2_P; // following messages
        pos += i;
    } while( pos <= len );
    XFlush( disp );
}

#endif
