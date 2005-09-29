/*
 *   Copyright (C) 2004 Lubos Lunak <l.lunak@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "kdetrayproxy.h"

#include <kapplication.h>
#include <kdebug.h>
#include <netwm.h>
#include <X11/Xlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

KDETrayProxy::KDETrayProxy()
    :   selection( makeSelectionAtom())
    {
    connect( &selection, SIGNAL( newOwner( Window )), SLOT( newOwner( Window )));
    connect( &module, SIGNAL( windowAdded( WId )), SLOT( windowAdded( WId )));
    selection.owner();
    for( QValueList< WId >::ConstIterator it = module.windows().begin();
         it != module.windows().end();
         ++it )
        windowAdded( *it );
    kapp->installX11EventFilter( this ); // XSelectInput( StructureNotifyMask ) on windows is done by KWinModule
//    kdDebug() << "Init done" << endl;
    }

Atom KDETrayProxy::makeSelectionAtom()
    {
    return XInternAtom( qt_xdisplay(), "_NET_SYSTEM_TRAY_S" + QCString().setNum( qt_xscreen()), False );
    }

extern Time qt_x_time;

void KDETrayProxy::windowAdded( WId w )
    {
    NETWinInfo ni( qt_xdisplay(), w, qt_xrootwin(), NET::WMKDESystemTrayWinFor );
    WId trayWinFor = ni.kdeSystemTrayWinFor();
    if ( !trayWinFor ) // not a KDE tray window
        return;
//    kdDebug() << "New tray window:" << w << endl;
    if( !tray_windows.contains( w ))
        tray_windows.append( w );
    withdrawWindow( w );
    // window will be removed from pending_windows when after docked
    if( !pending_windows.contains( w ))
        pending_windows.append( w );
    docked_windows.remove( w );
    Window owner = selection.owner();
    if( owner == None ) // no tray owner, sorry
        {
//        kdDebug() << "No owner, left in pending" << endl;
        return;
        }
    dockWindow( w, owner );
    }
    
void KDETrayProxy::newOwner( Window owner )
    {
//    kdDebug() << "New owner:" << owner << endl;
    for( QValueList< Window >::ConstIterator it = pending_windows.begin();
         it != pending_windows.end();
         ++it )
        dockWindow( *it, owner );
    // remove from pending_windows only in windowRemoved(), after it's really docked
    }

bool KDETrayProxy::x11Event( XEvent* e )
    {
    if( tray_windows.isEmpty())
        return false;
    if( e->type == DestroyNotify && tray_windows.contains( e->xdestroywindow.window ))
        {
        tray_windows.remove( e->xdestroywindow.window );
        pending_windows.remove( e->xdestroywindow.window );
        docked_windows.remove( e->xdestroywindow.window );
        }
    if( e->type == ReparentNotify && tray_windows.contains( e->xreparent.window ))
        {
        if( e->xreparent.parent == qt_xrootwin())
            {
            if( !docked_windows.contains( e->xreparent.window ) || e->xreparent.serial >= docked_windows[ e->xreparent.window ] )
                {
//                kdDebug() << "Window released:" << e->xreparent.window << endl;
                docked_windows.remove( e->xreparent.window );
                if( !pending_windows.contains( e->xreparent.window ))
                    pending_windows.append( e->xreparent.window );
                }
            }
        else
            {
//            kdDebug() << "Window away:" << e->xreparent.window << ":" << e->xreparent.parent << endl;
            pending_windows.remove( e->xreparent.window );
            }
        }
    if( e->type == UnmapNotify && tray_windows.contains( e->xunmap.window ))
        {
        if( docked_windows.contains( e->xunmap.window ) && e->xunmap.serial >= docked_windows[ e->xunmap.window ] )
            {
//            kdDebug() << "Window unmapped:" << e->xunmap.window << endl;
            XReparentWindow( qt_xdisplay(), e->xunmap.window, qt_xrootwin(), 0, 0 );
            // ReparentNotify will take care of the rest
            }
        }
    return false;
    }

void KDETrayProxy::dockWindow( Window w, Window owner )
    {
//    kdDebug() << "Docking " << w << " into " << owner << endl;
    docked_windows[ w ] = XNextRequest( qt_xdisplay());
    static Atom prop = XInternAtom( qt_xdisplay(), "_XEMBED_INFO", False );
    long data[ 2 ] = { 0, 1 };
    XChangeProperty( qt_xdisplay(), w, prop, prop, 32, PropModeReplace, (unsigned char*)data, 2 );
    XSizeHints hints;
    hints.flags = PMinSize | PMaxSize;
    hints.min_width = 24;
    hints.max_width = 24;
    hints.min_height = 24;
    hints.max_height = 24;
    XSetWMNormalHints( qt_xdisplay(), w, &hints );
//    kxerrorhandler ?
    XEvent ev;
    memset(&ev, 0, sizeof( ev ));
    static Atom atom = XInternAtom( qt_xdisplay(), "_NET_SYSTEM_TRAY_OPCODE", False );
    ev.xclient.type = ClientMessage;
    ev.xclient.window = owner;
    ev.xclient.message_type = atom;
    ev.xclient.format = 32;
    ev.xclient.data.l[ 0 ] = qt_x_time;
    ev.xclient.data.l[ 1 ] = 0; // SYSTEM_TRAY_REQUEST_DOCK
    ev.xclient.data.l[ 2 ] = w;
    ev.xclient.data.l[ 3 ] = 0; // unused
    ev.xclient.data.l[ 4 ] = 0; // unused
    XSendEvent( qt_xdisplay(), owner, False, NoEventMask, &ev );
    }

void KDETrayProxy::withdrawWindow( Window w )
    {
    XWithdrawWindow( qt_xdisplay(), w, qt_xscreen());
    static Atom wm_state = XInternAtom( qt_xdisplay(), "WM_STATE", False );
    for(;;)
        {
        Atom type;
        int format;
        unsigned long length, after;
        unsigned char *data;
        int r = XGetWindowProperty( qt_xdisplay(), w, wm_state, 0, 2,
            False, AnyPropertyType, &type, &format,
            &length, &after, &data );
        bool withdrawn = true;
        if ( r == Success && data && format == 32 )
            {
            withdrawn = ( *( long* )data == WithdrawnState );
            XFree( (char *)data );
            }
        if( withdrawn )
            return; // --->
        struct timeval tm;
        tm.tv_sec = 0;
        tm.tv_usec = 10 * 1000; // 10ms
        select(0, NULL, NULL, NULL, &tm);
        }
    }

#include "kdetrayproxy.moc"

#if 0
#include <kcmdlineargs.h>
int main( int argc, char* argv[] )
    {
    KCmdLineArgs::init( argc, argv, "a", "b", "c", "d" );
    KApplication app( false ); // no styles
    app.disableSessionManagement();
    KDETrayProxy proxy;
    return app.exec();
    }
#endif
