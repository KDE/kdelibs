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

    $Log$
    Revision 1.3  1999/08/26 02:44:34  tibirna
    CT: kill a segfault which affected any kwm module app when a window lowering was operated

    Revision 1.2  1999/08/18 22:40:29  ettrich
    shortcut to get the current desktop faster

    Revision 1.1  1999/08/16 20:31:07  ettrich
    Easier access to the window manager with KWinModule.
    This makes the former KWMModuleApplication obsolete.


*/


#include "kwinmodule.h"
#include "kwm.h"

class KWinModulePrivate : public QWidget
{
public:
    KWinModulePrivate( KWinModule* m  )
	: QWidget(0,0)
    {
	module = m;
	desktop = KWM::currentDesktop();
    }
    ~KWinModulePrivate()
    {
    }

    KWinModule* module;

    QValueList<WId> windows;
    QValueList<WId> windowsSorted;
    QValueList<WId> dockWindows;

    bool x11Event( XEvent * ev );
    int desktop;

};


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


    if ( ev->type != ClientMessage )
	return FALSE; //not interested

    if (!atoms){

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

	atoms = true;
    }


    Atom a = ev->xclient.message_type;
    WId w = (WId) (ev->xclient.data.l[0]);
    if ( a ==  module_init) {
	windows.clear();
	windowsSorted.clear();
	dockWindows.clear();
	emit module->init();
    }
    else if ( a == module_initialized) {
	emit module->initialized();
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
	emit module->dialogWindowAdd(w);
    }

    else if (a == module_win_remove){
	windows.remove( w );
	windowsSorted.remove( w );
	emit module->windowRemove(w);
    }

    else if (a == module_win_change){
	emit module->windowChange(w);
    }

    else if (a == module_win_raise){
	windowsSorted.remove(w);
	windowsSorted.append(w);
	emit module->windowRaise(w);
    }

    else if (a == module_win_lower){
	windowsSorted.remove(w);
	windowsSorted.prepend(w);
	emit module->windowLower(w);
    }

    else if (a == module_win_activate){
	emit module->windowActivate(w);
    }

    else if (a == module_win_icon_change){
	emit module->windowIconChanged(w);
    }

    else if (a == kwm_command){
	char c[21];
	int i;
	for (i=0;i<20;i++)
	    c[i] = ev->xclient.data.b[i];
	c[i] = '\0';
	QString com = c;
	emit module->commandReceived(com);
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
	emit module->playSound(com);
    }

    else if (a == register_sound){
	char c[21];
	int i;
	for (i=0;i<20;i++)
	    c[i] = ev->xclient.data.b[i];
	c[i] = '\0';
	QString com = c;
	emit module->registerSound(com);
    }

    else if (a == unregister_sound){
	char c[21];
	int i;
	for (i=0;i<20;i++)
	    c[i] = ev->xclient.data.b[i];
	c[i] = '\0';
	QString com = c;
	emit module->unregisterSound(com);
    }

    else if (a == kwm_window_region_changed){
	emit module->windowRegionChange();
    }

    return TRUE;
}


int KWinModule::currentDesktop() const
{
    return d->desktop;
}

#include "kwinmodule.moc"
