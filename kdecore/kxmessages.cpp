/****************************************************************************

 $Id$

 Copyright (C) 2001 Lubos Lunak        <l.lunak@kde.org>

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

#include <kapp.h>
#include <X11/Xlib.h>
#include <kdebug.h>

#ifndef None  // CHECKME
#define None 0
#endif

// for broadcasting
const long BROADCAST_MASK = PropertyChangeMask;
// CHECKME

KXMessages::KXMessages( const char* accept_broadcast_P, QWidget* parent_P )
    : QWidget( parent_P )
    {
    if( accept_broadcast_P != NULL )
        {
        ( void ) kapp->desktop(); //trigger desktop widget creation to select root window events
        kapp->installX11EventFilter( this ); // i.e. PropertyChangeMask
        accept_atom = XInternAtom( qt_xdisplay(), accept_broadcast_P, false );
        cached_atom_name = accept_broadcast_P;
        cached_atom = accept_atom;
        }
    else
        {
        accept_atom = None;
        cached_atom_name = "";
        }
    handle = new QWidget( this );
    }

KXMessages::~KXMessages()    
    {
//    delete d; no private data yet
    }

void KXMessages::broadcastMessage( const char* msg_type_P, const QString& message_P )
    {
    if( cached_atom_name != msg_type_P )
        {
        cached_atom = XInternAtom( qt_xdisplay(), msg_type_P, false );
        cached_atom_name = msg_type_P;
        }
    send_message_internal( qt_xrootwin(), message_P, BROADCAST_MASK, qt_xdisplay(),
        cached_atom, handle->winId());
    }

void KXMessages::sendMessage( WId w_P, const char* msg_type_P, const QString& message_P )
    {
    if( cached_atom_name != msg_type_P )
        {
        cached_atom = XInternAtom( qt_xdisplay(), msg_type_P, false );
        cached_atom_name = msg_type_P;
        }
    send_message_internal( w_P, message_P, 0, qt_xdisplay(), cached_atom, handle->winId());
    }
    
bool KXMessages::broadcastMessageX( Display* disp, const char* msg_type_P,
    const QString& message_P )
    {
    if( disp == NULL )
        return false;
    Atom atom = XInternAtom( disp, msg_type_P, false );
    Window win = XCreateSimpleWindow( disp, DefaultRootWindow( disp ), 0, 0, 1, 1,
        0, BlackPixelOfScreen( DefaultScreenOfDisplay( disp )),
        BlackPixelOfScreen( DefaultScreenOfDisplay( disp )));
    send_message_internal( DefaultRootWindow( disp ), message_P, BROADCAST_MASK, disp,
        atom, win );
    XDestroyWindow( disp, win );
    return true;
    }

bool KXMessages::sendMessageX( Display* disp, WId w_P, const char* msg_type_P,
    const QString& message_P )
    {
    if( disp == NULL )
        return false;
    Atom atom = XInternAtom( disp, msg_type_P, false );
    Window win = XCreateSimpleWindow( disp, DefaultRootWindow( disp ), 0, 0, 1, 1,
        0, BlackPixelOfScreen( DefaultScreenOfDisplay( disp )),
        BlackPixelOfScreen( DefaultScreenOfDisplay( disp )));
    send_message_internal( w_P, message_P, 0, disp, atom, win );
    XDestroyWindow( disp, win );
    return true;
    }
    
void KXMessages::send_message_internal( WId w_P, const QString& msg_P, long mask_P,
    Display* disp, Atom atom_P, Window handle_P )
    {
    unsigned int pos = 0;
    QCString msg = msg_P.utf8();
    unsigned int len = strlen( msg );
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = atom_P;
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
        pos += i;
        } while( pos <= len );
    XFlush( disp );
    }

bool KXMessages::x11Event( XEvent* ev_P )
    {
    if( ev_P->type != ClientMessage || ev_P->xclient.message_type != accept_atom
        || ev_P->xclient.format != 8 )
        return QWidget::x11Event( ev_P );
    char buf[ 21 ]; // can't be longer
    int i;
    for( i = 0;
         i < 20 && ev_P->xclient.data.b[ i ] != '\0';
         ++i )
        buf[ i ] = ev_P->xclient.data.b[ i ];
    buf[ i ] = '\0';
    if( incoming_messages.contains( ev_P->xclient.window ))
        incoming_messages[ ev_P->xclient.window ] += buf;
    else
        incoming_messages[ ev_P->xclient.window ] = buf;
    if( i < 20 ) // last message fragment
        {
        emit gotMessage( QString::fromUtf8( incoming_messages[ ev_P->xclient.window ] ));
        incoming_messages.remove( ev_P->xclient.window );
        }
    return false;
    }

#undef None

#include "kxmessages.moc"
