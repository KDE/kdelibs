/*
    $Id$

    This file is part of the KDE libraries
    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)


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
*/

#include "kwinmodule.h"
#include "kwin.h"
#include "kwm.h"
#include <X11/Xatom.h>
#include "kapp.h"
#include "qtl.h"

static bool atoms = FALSE;

static Atom module_init;
static Atom module_initialized;
static Atom module_desktop_change;
static Atom module_win_add;
static Atom module_dialog_win_add;
static Atom module_win_remove;
static Atom module_win_change;
static Atom module_win_raise;
static Atom module_win_lower;
static Atom module_win_activate;
static Atom module_win_icon_change;
static Atom module_desktop_name_change;
static Atom module_desktop_number_change;
static Atom kwm_command;
static Atom module_dockwin_add;
static Atom module_dockwin_remove;
static Atom sound;
static Atom register_sound;
static Atom unregister_sound;
static Atom kwm_window_region_changed;
static Atom kwm_win_desktop;

//new stuff
static Atom net_client_list;
static Atom net_client_list_stacking;
#ifndef KDE_USE_FINAL
static Atom net_active_window;
static Atom net_current_desktop;
#endif
static Atom net_kde_docking_windows;


extern Atom qt_wm_state;

static void createModuleAtoms() {
    if (!atoms){
	// #### TODO make this an X11 big call (less roundtrips)

	module_init = XInternAtom(qt_xdisplay(),
				  "KWM_MODULE_INIT", False);
	module_initialized = XInternAtom(qt_xdisplay(),
					 "KWM_MODULE_INITIALIZED", False);
	module_desktop_change = XInternAtom(qt_xdisplay(),
					    "KWM_MODULE_DESKTOP_CHANGE", False);
	module_desktop_name_change = XInternAtom(qt_xdisplay(),
						 "KWM_MODULE_DESKTOP_NAME_CHANGE", False);
	module_desktop_number_change = XInternAtom(qt_xdisplay(),
						   "KWM_MODULE_DESKTOP_NUMBER_CHANGE", False);

	module_win_add = XInternAtom(qt_xdisplay(),
				     "KWM_MODULE_WIN_ADD", False);
	module_dialog_win_add = XInternAtom(qt_xdisplay(),
					    "KWM_MODULE_DIALOG_WIN_ADD", False);
	module_win_remove = XInternAtom(qt_xdisplay(),
					"KWM_MODULE_WIN_REMOVE", False);
	module_win_change = XInternAtom(qt_xdisplay(),
					"KWM_MODULE_WIN_CHANGE", False);
	module_win_raise = XInternAtom(qt_xdisplay(),
				       "KWM_MODULE_WIN_RAISE", False);
	module_win_lower = XInternAtom(qt_xdisplay(), "KWM_MODULE_WIN_LOWER", False);
	module_win_activate = XInternAtom(qt_xdisplay(),
					  "KWM_MODULE_WIN_ACTIVATE", False);
	module_win_icon_change = XInternAtom(qt_xdisplay(),
					     "KWM_MODULE_WIN_ICON_CHANGE", False);
	kwm_command = XInternAtom(qt_xdisplay(),
				  "KWM_COMMAND", False);

	module_dockwin_add = XInternAtom(qt_xdisplay(),
					 "KWM_MODULE_DOCKWIN_ADD", False);
	module_dockwin_remove = XInternAtom(qt_xdisplay(),
					    "KWM_MODULE_DOCKWIN_REMOVE", False);
	sound = XInternAtom(qt_xdisplay(),
			    "KDE_SOUND_EVENT", False);
	register_sound = XInternAtom(qt_xdisplay(),
				     "KDE_REGISTER_SOUND_EVENT", False);
	unregister_sound = XInternAtom(qt_xdisplay(),
				       "KDE_UNREGISTER_SOUND_EVENT", False);

	kwm_window_region_changed = XInternAtom(qt_xdisplay(), "KWM_WINDOW_REGION_CHANGED", False);

	
	kwm_win_desktop = XInternAtom(qt_xdisplay(), "KWM_WIN_DESKTOP", False);
	
	// new stuff
	net_client_list = XInternAtom(qt_xdisplay(), "_NET_CLIENT_LIST", False);
	net_client_list_stacking = XInternAtom(qt_xdisplay(), "_NET_CLIENT_LIST_STACKING", False);
	net_active_window = XInternAtom(qt_xdisplay(), "_NET_ACTIVE_WINDOW", False);
	net_current_desktop = XInternAtom(qt_xdisplay(), "_NET_CURRENT_DESKTOP", False);
	net_kde_docking_windows = XInternAtom(qt_xdisplay(), "_NET_KDE_DOCKING_WINDOWS", False);
	atoms = true;
    }
}

class KWinModulePrivate : public QWidget
{
public:
    KWinModulePrivate( KWinModule* m  )
	: QWidget(0,0)
    {
	createModuleAtoms();
	module = m;
	desktop = KWin::currentDesktop();
	kapp->installX11EventFilter( this );
	(void ) kapp->desktop(); //trigger desktop widget creation to select root window events
	startTimer( 0 );
    }
    ~KWinModulePrivate()
    {
    }

    KWinModule* module;

    QValueList<WId> windows;
    QValueList<WId> windowsSorted;
    QValueList<WId> dockWindows;

    bool x11Event( XEvent * ev );
    void timerEvent( QTimerEvent * )
    {
	killTimers();
	updateWindows();
	updateWindowsSorted();
	updateDockWindows();
    }
    int desktop;


    QValueList<WId> readWindowList( Atom a );

    void updateWindows();
    void updateWindowsSorted();
    void updateDockWindows();

};

QValueList<WId> KWinModulePrivate::readWindowList( Atom a )
{
    Atom type;
    int format;
    ulong  nitems, after;
    long offset = 0;
    WId  *data;

    QValueList<WId> result;

    int e = XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), a, 0, 1,
			     FALSE, XA_WINDOW, &type, &format, &nitems,
			     &after,  (unsigned char**)&data );
    if ( data )
	XFree(  (unsigned char*)data );
    if ( e != Success || !nitems )
	return result;

    after = 1;
    while (after > 0) {
	XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), a,
			    offset, 1024, FALSE, XA_WINDOW,
			    &type, &format, &nitems, &after, (unsigned char**) &data );
	for ( unsigned int i = 0; i < nitems; ++i ) {
	    result.append( data[i] );
	}
	offset += 1024;
	XFree(  (unsigned char*)data );
    }
    return result;
}

void KWinModulePrivate::updateWindows()
{
    QValueList<WId> old = windows;

    windows = readWindowList( net_client_list );
    qHeapSort( windows );

    QValueList<WId>::Iterator it1, it2;

    it1 = old.begin();
    it2 = windows.begin();
    while ( it1 != old.end() || it2 != windows.end() ) {
	if ( it1 == old.end() ) {
	    if ( !QWidget::find( *it2 ) )
		XSelectInput(qt_xdisplay(), *it2, PropertyChangeMask );
	    emit module->windowAdd( *it2 );
	    ++it2;
	} else if ( it2 == windows.end() ) {
	    emit module->windowRemove( *it1 );
	    ++it1;
	} else {
	    if (*it1 < *it2 ) {
		emit module->windowRemove( *it1 );
		++it1;
	    } else if ( *it2 < *it1 ) {
		if ( !QWidget::find( *it2 ) )
		    XSelectInput(qt_xdisplay(), *it2, PropertyChangeMask );
		emit module->windowAdd( *it2 );
		++it2;
	    } else {
		++it1;
		++it2;
	    }
	}
    }
}

void KWinModulePrivate::updateWindowsSorted()
{
    windowsSorted = readWindowList( net_client_list_stacking );
    emit module->stackingOrderChanged();
}

void KWinModulePrivate::updateDockWindows()
{
    QValueList<WId> old = dockWindows;

    dockWindows = readWindowList( net_kde_docking_windows );
    qHeapSort( dockWindows );

    QValueList<WId>::Iterator it1, it2;

    it1 = old.begin();
    it2 = dockWindows.begin();
    while ( it1 != old.end() || it2 != dockWindows.end() ) {
	if ( it1 == old.end() ) {
	    if ( !QWidget::find( *it2 ) )
		XSelectInput(qt_xdisplay(), *it2, PropertyChangeMask );
	    emit module->dockWindowAdd( *it2 );
	    ++it2;
	} else if ( it2 == dockWindows.end() ) {
	    emit module->dockWindowRemove( *it1 );
	    ++it1;
	} else {
	    if (*it1 < *it2 ) {
		emit module->windowRemove( *it1 );
		++it1;
	    } else if ( *it2 < *it1 ) {
		if ( !QWidget::find( *it2 ) )
		    XSelectInput(qt_xdisplay(), *it2, PropertyChangeMask );
		emit module->dockWindowAdd( *it2 );
		++it2;
	    } else {
		++it1;
		++it2;
	    }
	}
    }
}


KWinModule::KWinModule( QObject* parent, bool dockModule )
    : QObject( parent, "kwin_module" )
{
    d = new KWinModulePrivate( this );
    if (!dockModule)
	KWM::setKWMModule( d->winId() );
    else
	KWM::setKWMDockModule( d->winId() );
}

KWinModule::~KWinModule()
{
    delete d;
}


const QValueList<WId>& KWinModule::windows() const
{
    return d->windows;
}

const QValueList<WId>& KWinModule::windowsSorted() const
{
    return d->windowsSorted;
}


bool KWinModule::hasWId(WId w) const
{
    return d->windows.contains( w );
}

const QValueList<WId>& KWinModule::dockWindows() const
{
    return d->dockWindows;
}

bool KWinModulePrivate::x11Event( XEvent * ev )
{


    // the new net stuff
    if ( ev->type == PropertyNotify && ev->xproperty.window == qt_xrootwin()  ) {
	if ( ev->xproperty.atom == net_client_list ) {
	    updateWindows();
	}
	else if ( ev->xproperty.atom == net_client_list_stacking ) {
	    updateWindowsSorted();
	}
	else if ( ev->xproperty.atom == net_kde_docking_windows ) {
	    updateDockWindows();
	}
	else if (  ev->xproperty.atom == net_active_window ) {
	    emit module->windowActivate( KWin::activeWindow() );
	}
	else if (  ev->xproperty.atom == net_current_desktop ) {
	    int nd = KWin::currentDesktop();
	    if ( nd != desktop ) {
		desktop = nd;
		emit module->desktopChange( desktop );
	    }
	}
	return FALSE;
    }

    if ( ev->type == PropertyNotify &&  module->hasWId( ev->xproperty.window ) ) {
	Atom a = ev->xproperty.atom;
	bool  doit = FALSE;
	switch  ( a ) {
	case XA_WM_ICON_NAME:
	case XA_WM_NAME:
	case XA_WM_NORMAL_HINTS:
	case XA_WM_HINTS:
	    doit = TRUE;
	default:
	    if ( doit || a == qt_wm_state || a == kwm_win_desktop ) {
		XEvent dummy;
		while (XCheckTypedWindowEvent (qt_xdisplay(), ev->xproperty.window,
					       PropertyNotify, &dummy) )
		    ;
		emit module->windowChange( ev->xproperty.window );
		return FALSE;
	    }
	    break;
	}
    }


    // old stuff below

    if ( ev->type != ClientMessage )
	return FALSE; //not interested

    Atom a = ev->xclient.message_type;

    WId w = (WId) (ev->xclient.data.l[0]);
    if ( a ==  module_init) {
	windows.clear();
	windowsSorted.clear();
	dockWindows.clear();
// 	emit module->init();
    }
    else if ( a == module_initialized) {
// 	emit module->initialized();
    }
    else if ( a == module_desktop_change) {
	desktop = (int) w;
	emit module->desktopChange( (int) w);
    }
    else if (a == module_desktop_name_change){
	emit module->desktopNameChange( (int) w, KWM::desktopName( (int) w ) );
    }
    else if (a == module_desktop_number_change){
	emit module->desktopNumberChange( (int) w);
    }
    else if (a == module_win_add){

	windows.append(w);
	windowsSorted.append(w);
	emit module->windowAdd(w);
    }
    else if (a == module_dialog_win_add){
// 	emit module->dialogWindowAdd(w);
    }

    else if (a == module_win_remove){
	windows.remove( w );
	windowsSorted.remove( w );
	emit module->windowRemove(w);
    }

    else if (a == module_win_change || a == module_win_icon_change ){
	emit module->windowChange(w);
    }

    else if (a == module_win_raise){
	windowsSorted.remove(w);
	windowsSorted.append(w);
	emit module->stackingOrderChanged();
    }

    else if (a == module_win_lower){
	windowsSorted.remove(w);
	windowsSorted.prepend(w);
	emit module->stackingOrderChanged();
    }

    else if (a == module_win_activate){
	emit module->windowActivate(w);
    }


    else if (a == kwm_command){
	char c[21];
	int i;
	for (i=0;i<20;i++)
	    c[i] = ev->xclient.data.b[i];
	c[i] = '\0';
	QString com = c;
// 	emit module->commandReceived(com);
    }

    else if (a == module_dockwin_add){
	dockWindows.append(w);
	emit module->dockWindowAdd(w);
    }

    else if (a == module_dockwin_remove){
	dockWindows.remove(w);
	emit module->dockWindowRemove(w);
    }

    else if (a == sound){
	char c[21];
	int i;
	for (i=0;i<20;i++)
	    c[i] = ev->xclient.data.b[i];
	c[i] = '\0';
	QString com = c;
// 	emit module->playSound(com);
    }

    else if (a == register_sound){
	char c[21];
	int i;
	for (i=0;i<20;i++)
	    c[i] = ev->xclient.data.b[i];
	c[i] = '\0';
	QString com = c;
// 	emit module->registerSound(com);
    }

    else if (a == unregister_sound){
	char c[21];
	int i;
	for (i=0;i<20;i++)
	    c[i] = ev->xclient.data.b[i];
	c[i] = '\0';
	QString com = c;
// 	emit module->unregisterSound(com);
    }

    else if (a == kwm_window_region_changed){
 	emit module->workspaceAreaChanged();
    }

    return FALSE; // no not hide clientmessages from other filters (for example Qt ;-)
}


int KWinModule::currentDesktop() const
{
    return d->desktop;
}

#include "kwinmodule.moc"
