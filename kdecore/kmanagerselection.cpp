/****************************************************************************

 $Id$

 Copyright (C) 2003 Lubos Lunak        <l.lunak@kde.org>

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

#include <qobject.h>
#ifdef Q_WS_X11 // FIXME(E)

#include "kmanagerselection.h"

#include <kdebug.h>
#include <X11/Xatom.h>

KSelectionOwner::KSelectionOwner( Atom selection_P, int screen_P )
    :   selection( selection_P ),
        screen( screen_P >= 0 ? screen_P : DefaultScreen( qt_xdisplay())),
        window( None ),
        timestamp( CurrentTime ),
        extra1( 0 ), extra2( 0 )
    {
    }
    
KSelectionOwner::KSelectionOwner( const char* selection_P, int screen_P )
    :   selection( XInternAtom( qt_xdisplay(), selection_P, False )),
        screen( screen_P >= 0 ? screen_P : DefaultScreen( qt_xdisplay())),
        window( None ),
        timestamp( CurrentTime ),
        extra1( 0 ), extra2( 0 )
    {
    }
    
bool KSelectionOwner::claim( bool force_P, bool force_kill_P )
    {
    if( manager_atom == None )
        getAtoms();
    if( timestamp != CurrentTime )
        release();
    Display* const dpy = qt_xdisplay();
    Window prev_owner = XGetSelectionOwner( dpy, selection );
    if( prev_owner != None )
        {
        if( !force_P )
            {
//            kdDebug() << "Selection already owned, failing" << endl;
            return false;
            }
        XSelectInput( dpy, prev_owner, StructureNotifyMask );
        }
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    window = XCreateWindow( dpy, RootWindow( dpy, screen ), 0, 0, 1, 1, 
        0, CopyFromParent, InputOnly, CopyFromParent, CWOverrideRedirect, &attrs );
//    kdDebug() << "Using owner window " << window << endl;
    if( window == None )
        return false;
    Atom tmp = XA_ATOM;
    XSelectInput( dpy, window, PropertyChangeMask );
    XChangeProperty( dpy, window, XA_ATOM, XA_ATOM, 32, PropModeReplace,
        reinterpret_cast< unsigned char* >( &tmp ), 1 );
    XEvent ev;
    XSync( dpy, False );
    XCheckTypedWindowEvent( dpy, window, PropertyNotify, &ev ); // get a timestamp
    timestamp = ev.xproperty.time;
    XSelectInput( dpy, window, StructureNotifyMask ); // for DestroyNotify
    XSetSelectionOwner( dpy, selection, window, timestamp );
    Window new_owner = XGetSelectionOwner( dpy, selection );
    if( new_owner != window )
        {
//        kdDebug() << "Failed to claim selection : " << new_owner << endl;
        XDestroyWindow( dpy, window );
        timestamp = CurrentTime;
        return None;
        }
    if( prev_owner != None )
        {
//        kdDebug() << "Waiting for previous owner to disown" << endl;
        for( int cnt = 0;
             ;
             ++cnt )
            {
            if( XCheckTypedWindowEvent( dpy, prev_owner, DestroyNotify, &ev ) == True )
                break;
            struct timeval tm = { 0, 50000 }; // 50 ms
            select( 0, NULL, NULL, NULL, &tm );
            if( cnt == 19 )
                {
                if( force_kill_P )
                    {
//                    kdDebug() << "Killing previous owner" << endl;
                    XKillClient( dpy, prev_owner );
                    }
                break;
                }
            }
        }
    ev.type = ClientMessage;
    ev.xclient.window = RootWindow( dpy, screen );
    ev.xclient.display = dpy;
    ev.xclient.message_type = manager_atom;
    ev.xclient.format = 32;
    ev.xclient.data.l[ 0 ] = timestamp;
    ev.xclient.data.l[ 1 ] = selection;
    ev.xclient.data.l[ 2 ] = window;
    ev.xclient.data.l[ 3 ] = extra1;
    ev.xclient.data.l[ 4 ] = extra2;
    XSendEvent( dpy, RootWindow( dpy, screen ), False, StructureNotifyMask, &ev );
//    kdDebug() << "Claimed selection" << endl;
    return true;
    }

// destroy resource first
void KSelectionOwner::release()
    {
    if( timestamp == CurrentTime )
        return;
    XSetSelectionOwner( qt_xdisplay(), selection, None, timestamp );
//    kdDebug() << "Releasing selection" << endl;
    timestamp = CurrentTime;
    }

void KSelectionOwner::setData( long extra1_P, long extra2_P )
    {
    extra1 = extra1_P;
    extra2 = extra2_P;
    }
    
bool KSelectionOwner::filterSelectionClear( XSelectionClearEvent& ev_P )
    {
    if( timestamp == CurrentTime || ev_P.selection != selection )
        return false;
    timestamp = CurrentTime;
//    kdDebug() << "Lost selection" << endl;
    emit lostOwnership();
    XSelectInput( qt_xdisplay(), window, 0 );
    XDestroyWindow( qt_xdisplay(), window );
    return true;
    }

bool KSelectionOwner::filterDestroyNotify( XDestroyWindowEvent& ev_P )
    {
    if( timestamp == CurrentTime || ev_P.window != window )
        return false;
    timestamp = CurrentTime;
//    kdDebug() << "Lost selection (destroyed)" << endl;
    emit lostOwnership();
    return true;
    }

bool KSelectionOwner::filterSelectionNotify( XSelectionEvent& ev_P )
    {
    if( timestamp == CurrentTime || ev_P.selection != selection )
        return false;
    // ignore?
    return true;
    }

bool KSelectionOwner::filterSelectionRequest( XSelectionRequestEvent& ev_P )
    {
    if( timestamp == CurrentTime || ev_P.selection != selection )
        return false;
    if( ev_P.time != CurrentTime
        && ev_P.time - timestamp > 1U << 31 )
        return false; // too old or too new request
//    kdDebug() << "Got selection request" << endl;
    bool handled = false;
    if( ev_P.target == xa_multiple )
        {
        if( ev_P.property != None )
            {
            const int MAX_ATOMS = 100; // no need to handle more?
            int format;
            Atom type;
            unsigned long items;
            unsigned long after;
            unsigned char* data;
            if( XGetWindowProperty( qt_xdisplay(), ev_P.requestor, ev_P.property, 0,
                MAX_ATOMS, False, AnyPropertyType, &type, &format, &items, &after,
                &data ) == Success && format == 32 && items % 2 == 0 )
                {
                bool handled_array[ MAX_ATOMS ];
                Atom* atoms = reinterpret_cast< Atom* >( data );
                for( unsigned int i = 0;
                     i < items / 2;
                     ++i )
                    handled_array[ i ] = handle_selection(
                        atoms[ i * 2 ], atoms[ i * 2 + 1 ], ev_P.requestor );
                bool all_handled = true;
                for( unsigned int i = 0;
                     i < items / 2;
                     ++i )
                    if( !handled_array[ i ] )
                        {
                        all_handled = false;
                        atoms[ i * 2 + 1 ] = None;
                        }
                if( !all_handled )
                    XChangeProperty( qt_xdisplay(), ev_P.requestor, ev_P.property, XA_ATOM,
                        32, PropModeReplace, reinterpret_cast< unsigned char* >( atoms ), items );
                handled = true;
                XFree( data );
                }
            }
        }
    else
        {
        if( ev_P.property == None ) // obsolete client
            ev_P.property = ev_P.target;
        handled = handle_selection( ev_P.target, ev_P.property, ev_P.requestor );
        }
    XEvent ev;
    ev.xselection.type = SelectionNotify;
    ev.xselection.display = qt_xdisplay();
    ev.xselection.requestor = ev_P.requestor;
    ev.xselection.target = ev_P.target;
    ev.xselection.property = handled ? ev_P.property : None;
    XSendEvent( qt_xdisplay(), ev_P.requestor, False, 0, &ev );
    return true;
    }

bool KSelectionOwner::handle_selection( Atom target_P, Atom property_P, Window requestor_P )
    {
    if( target_P == xa_timestamp )
        {
//        kdDebug() << "Handling timestamp request" << endl;
        XChangeProperty( qt_xdisplay(), requestor_P, property_P, XA_INTEGER, 32,
            PropModeReplace, reinterpret_cast< unsigned char* >( &timestamp ), 1 );
        }
    else if( target_P == xa_targets )
        replyTargets( property_P, requestor_P );
    else if( genericReply( target_P, property_P, requestor_P ))
        ; // handled
    else
        return false; // unknown
    return true;
    }

void KSelectionOwner::replyTargets( Atom property_P, Window requestor_P )
    {
    Atom atoms[ 3 ] = { xa_multiple, xa_timestamp, xa_targets };
//    kdDebug() << "Handling targets request" << endl;
    XChangeProperty( qt_xdisplay(), requestor_P, property_P, XA_ATOM, 32, PropModeReplace,
        reinterpret_cast< unsigned char* >( atoms ), 3 );
    }

bool KSelectionOwner::genericReply( Atom, Atom, Window )
    {
    return false;
    }

void KSelectionOwner::getAtoms()
    {
    if( manager_atom == None )
        {
        Atom atoms[ 4 ];
        const char* const names[] =
            { "MANAGER", "MULTIPLE", "TARGETS", "TIMESTAMP" };
        XInternAtoms( qt_xdisplay(), const_cast< char** >( names ), 4, False, atoms );
        manager_atom = atoms[ 0 ];
        xa_multiple = atoms[ 1];
        xa_targets = atoms[ 2 ];
        xa_timestamp = atoms[ 3 ];
        }
    }

Atom KSelectionOwner::manager_atom = None;
Atom KSelectionOwner::xa_multiple = None;
Atom KSelectionOwner::xa_targets = None;
Atom KSelectionOwner::xa_timestamp = None;

//*******************************************
// KSelectionWatcher
//*******************************************

KSelectionWatcher::KSelectionWatcher( Atom selection_P, int screen_P )
    :   selection( selection_P ),
        screen( screen_P >= 0 ? screen_P : DefaultScreen( qt_xdisplay())),
        selection_owner( None )
    {
    init();
    }
    
KSelectionWatcher::KSelectionWatcher( const char* selection_P, int screen_P )
    :   selection( XInternAtom( qt_xdisplay(), selection_P, False )),
        screen( screen_P >= 0 ? screen_P : DefaultScreen( qt_xdisplay())),
        selection_owner( None )
    {
    init();
    }

void KSelectionWatcher::init()
    {
    if( manager_atom == None )
        {
        Display* const dpy = qt_xdisplay();
        manager_atom = XInternAtom( dpy, "MANAGER", False );
        XWindowAttributes attrs;
        XGetWindowAttributes( dpy, RootWindow( dpy, screen ), &attrs );
        long event_mask = attrs.your_event_mask;
        // SELI co kdyz tohle udela nekdo taky? pak se zmrsi event mask
        // StructureNotifyMask on the root window is needed
        XSelectInput( dpy, RootWindow( dpy, screen ), event_mask | StructureNotifyMask );
        }
    }    

bool KSelectionWatcher::no_error;

int KSelectionWatcher::err_handler( Display*, XErrorEvent* )
    {
    no_error = false;
    return 0;
    }
    
Window KSelectionWatcher::owner()
    {
    Display* const dpy = qt_xdisplay();
    XSync( dpy, False );
    no_error = true;
    int (*old_handler)( Display*, XErrorEvent* ) = XSetErrorHandler( err_handler );
    Window current_owner = XGetSelectionOwner( dpy, selection );
    if( current_owner == None )
        return None;
    if( current_owner == selection_owner )
        return selection_owner;
    XSelectInput( dpy, current_owner, StructureNotifyMask );
    XSync( dpy, False );
    XSetErrorHandler( old_handler );
    if( no_error && current_owner == XGetSelectionOwner( dpy, selection ))
        {
//        kdDebug() << "isOwner: " << current_owner << endl;
        selection_owner = current_owner;
        }
    else
        selection_owner = None;
    return selection_owner;
    }
    
bool KSelectionWatcher::filterEvent( XEvent& ev_P )
    {
    if( ev_P.type == ClientMessage )
        {
//        kdDebug() << "got ClientMessage" << endl;
        if( ev_P.xclient.message_type != manager_atom
            || ev_P.xclient.data.l[ 1 ] != static_cast< long >( selection ))
            return false;
//        kdDebug() << "handling message" << endl;
        if( ev_P.xclient.data.l[ 2 ] == static_cast< long >( selection_owner ))
            {
//            kdDebug() << "already known" << endl;
            return true; // we know this already
            }
        if( static_cast< long >( owner()) == ev_P.xclient.data.l[ 2 ] )
            {
//            kdDebug() << "new owner: " << selection_owner << endl;
            emit newOwner( selection_owner );
            }
        return true;
        }
    if( ev_P.type == DestroyNotify )
        {
        if( selection_owner == None || ev_P.xdestroywindow.window != selection_owner )
            return false;
        selection_owner = None;
        emit lostOwner();
        return true;
        }
    return false;
    }

Atom KSelectionWatcher::manager_atom = None;

#include "kmanagerselection.moc"
#endif
