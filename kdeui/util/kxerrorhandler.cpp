/*

  Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>

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

*/

#include "kxerrorhandler.h"

#ifdef Q_WS_X11 //FIXME

#include "netwm_def.h"

class KXErrorHandlerPrivate
{
public:
    KXErrorHandlerPrivate( Display* dpy ) :
        first_request( XNextRequest( dpy )),
        display( dpy ),
        was_error( false )
    {
    }
    unsigned long first_request;
    Display* display;
    bool was_error;
    XErrorEvent error_event;
};

KXErrorHandler** KXErrorHandler::handlers = NULL;
int KXErrorHandler::pos = 0;
int KXErrorHandler::size = 0;

KXErrorHandler::KXErrorHandler( Display* dpy )
    :   user_handler1( NULL ),
        user_handler2( NULL ),
        old_handler( XSetErrorHandler( handler_wrapper )),
        d( new KXErrorHandlerPrivate(dpy) )
    {
    addHandler();
    }

KXErrorHandler::KXErrorHandler( bool (*handler)( int request, int error_code, unsigned long resource_id ), Display* dpy )
    :   user_handler1( handler ),
        user_handler2( NULL ),
        old_handler( XSetErrorHandler( handler_wrapper )),
        d( new KXErrorHandlerPrivate(dpy) )
    {
    addHandler();
    }

KXErrorHandler::KXErrorHandler( int (*handler)( Display*, XErrorEvent* ), Display* dpy )
    :   user_handler1( NULL ),
        user_handler2( handler ),
        old_handler( XSetErrorHandler( handler_wrapper )),
        d( new KXErrorHandlerPrivate(dpy) )
    {
    addHandler();
    }

KXErrorHandler::~KXErrorHandler()
    {
    XSetErrorHandler( old_handler );
    Q_ASSERT_X( this == handlers[ pos-1 ], "KXErrorHandler", "out of order" );
    --pos;
    delete d;
    }

void KXErrorHandler::addHandler()
    {
    if( size == pos )
        {
        size += 16;
        handlers = static_cast< KXErrorHandler** >( qRealloc( handlers, size * sizeof( KXErrorHandler* )));
        }
    handlers[ pos++ ] = this;
    }

bool KXErrorHandler::error( bool sync ) const
    {
    if( sync )
        XSync( d->display, False );
    return d->was_error;
    }

XErrorEvent KXErrorHandler::errorEvent() const
    {
    return d->error_event;
    }

int KXErrorHandler::handler_wrapper( Display* dpy, XErrorEvent* e )
    {
    --pos;
    int ret = handlers[ pos ]->handle( dpy, e );
    ++pos;
    return ret;
    }

int KXErrorHandler::handle( Display* dpy, XErrorEvent* e )
    {
    if( dpy == d->display
        // e->serial >= d->first_request , compare like X timestamps to handle wrapping
        && NET::timestampCompare( e->serial, d->first_request ) >= 0 )
        { // it's for us
        //qDebug( "Handling: %p", static_cast< void* >( this ));
        bool error = false;
        if( user_handler1 != NULL )
            {
            if( user_handler1( e->request_code, e->error_code, e->resourceid ))
                error = true;
            }
        else if( user_handler2 != NULL )
            {
            if( user_handler2( dpy, e ) != 0 )
                error = true;
            }
        else // no handler set, simply set that there was an error
            error = true;
        if( error && !d->was_error )
            { // only remember the first
            d->was_error = true;
            d->error_event = *e;
            }
        return 0;
        }
    //qDebug( "Going deeper: %p", static_cast< void* >( this ));
    return old_handler( dpy, e );
    }

#endif
