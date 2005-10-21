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

#include <qwidget.h>
#ifdef Q_WS_X11 //FIXME

#include "kxerrorhandler.h"

KXErrorHandler** KXErrorHandler::handlers = NULL;
int KXErrorHandler::pos = 0;
int KXErrorHandler::size = 0;

KXErrorHandler::KXErrorHandler( Display* dpy )
    :   user_handler1( NULL ),
        user_handler2( NULL ),
        old_handler( XSetErrorHandler( handler_wrapper )),
        first_request( XNextRequest( dpy )),
        display( dpy ),
        was_error( false )
    {
    addHandler();
    }

KXErrorHandler::KXErrorHandler( bool (*handler)( int request, int error_code, unsigned long resource_id ), Display* dpy )
    :   user_handler1( handler ),
        user_handler2( NULL ),
        old_handler( XSetErrorHandler( handler_wrapper )),
        first_request( XNextRequest( dpy )),
        display( dpy ),
        was_error( false )
    {
    addHandler();
    }

KXErrorHandler::KXErrorHandler( int (*handler)( Display*, XErrorEvent* ), Display* dpy )
    :   user_handler1( NULL ),
        user_handler2( handler ),
        old_handler( XSetErrorHandler( handler_wrapper )),
        first_request( XNextRequest( dpy )),
        display( dpy ),
        was_error( false )
    {
    addHandler();
    }

KXErrorHandler::~KXErrorHandler()
    {
    XSetErrorHandler( old_handler );
    Q_ASSERT_X( this == handlers[ pos-1 ], __FUNCTION__, "out of order" );
    --pos;
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
        XSync( display, False );
    return was_error;
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
    if( dpy == display
        && e->serial - first_request < 1000000000 ) // e->serial > first_request, with wrapping
        { // it's for us
        //qDebug( "Handling: %p", static_cast< void* >( this ));
        if( user_handler1 != NULL )
            was_error |= user_handler1( e->request_code, e->error_code, e->resourceid );
        else if( user_handler2 != NULL )
            was_error |= ( user_handler2( dpy, e ) != 0 );
        else // no handler set, simply set that there was an error
            was_error = true;
        return 0;
        }
    //qDebug( "Going deeper: %p", static_cast< void* >( this ));
    return old_handler( dpy, e );
    }

#endif
