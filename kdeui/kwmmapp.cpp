/*  

    $Id:$

    This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Ettrich (ettrich@kde.org)
    

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

    $Log:$


*/

#include "kwmmapp.moc"

int KWMModuleXErrorHandler(Display *, XErrorEvent *){

  return 0; // ignore Xerrors

};

KWMModuleApplication::KWMModuleApplication( int &argc, char *argv[])
                     :KApplication(argc, argv){

    XSetErrorHandler(KWMModuleXErrorHandler);
    module = new QWidget;

}

KWMModuleApplication::KWMModuleApplication( 
					   int &argc, char *argv[], 
					   const QString& rAppName
					   )
                      :KApplication(argc, argv, rAppName){

    XSetErrorHandler(KWMModuleXErrorHandler);
    module = new QWidget;

}


void KWMModuleApplication::connectToKWM(bool dock_module){

  if (!dock_module)
    KWM::setKWMModule(module->winId());
  else
    KWM::setKWMDockModule(module->winId());

}

bool KWMModuleApplication::x11EventFilter( XEvent * ev){


  static bool atoms = FALSE;
  
  static Atom module_init;
  static Atom module_desktop_change;
  static Atom module_win_add;
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

  Atom a;
  Window w;
  Window *wp;


  if (KApplication::x11EventFilter(ev))
    return True;

  if (ev->xany.window == module->winId()
      && ev->type == ClientMessage){

    if (!atoms){

      module_init = XInternAtom(qt_xdisplay(), 
				"KWM_MODULE_INIT", False);
      module_desktop_change = XInternAtom(qt_xdisplay(), 
					  "KWM_MODULE_DESKTOP_CHANGE", False);
      module_desktop_name_change = XInternAtom(qt_xdisplay(), 
				   "KWM_MODULE_DESKTOP_NAME_CHANGE", False);
      module_desktop_number_change = XInternAtom(qt_xdisplay(), 
                                     "KWM_MODULE_DESKTOP_NUMBER_CHANGE", False);
      
      module_win_add = XInternAtom(qt_xdisplay(), 
				   "KWM_MODULE_WIN_ADD", False);
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

      atoms = true; // was missing --Bernd
    }

    a = ev->xclient.message_type;
    w = (Window) (ev->xclient.data.l[0]);

    if (a == module_init){

      windows.clear();
      windows_sorted.clear();
      dock_windows.clear();
      emit init();

    }

    if (a == module_desktop_change){

      int d = (int) w;
      emit desktopChange(d);
    }

    if (a == module_desktop_name_change){

      int d = (int) w;
      emit desktopNameChange(d, KWM::getDesktopName(d));
    }

    if (a == module_desktop_number_change){
      int d = (int) w;
      emit desktopNumberChange(d);
    }
    if (a == module_win_add){

      wp = new Window;
      *wp = w;
      windows.append(wp);
      windows_sorted.append(wp);
      emit windowAdd(w);
    }

    if (a == module_win_remove){

      for (wp=windows.first(); wp; wp=windows.next()){

	if (*wp == w){

	  windows.remove();
	  break;
	}
      }

      for (wp=windows_sorted.first(); wp; wp=windows_sorted.next()){

	if (*wp == w){

	  windows_sorted.remove();
	  delete wp;
	  break;
	}
      }

      emit windowRemove(w);
    }

    if (a == module_win_change){
      emit windowChange(w);
    }

    if (a == module_win_raise){
      for (wp=windows_sorted.first(); wp; wp=windows_sorted.next()){

	if (*wp == w) {

	  windows_sorted.remove();
	  windows_sorted.append(wp);
	  break;
	}
      }
      emit windowRaise(w);
    }

    if (a == module_win_lower){

      for (wp=windows_sorted.first(); wp; wp=windows_sorted.next()){

	if (*wp == w) {
	  windows_sorted.remove();
	  windows_sorted.insert(0, wp);
	  break;
	}
      }

      emit windowLower(w);
    }

    if (a == module_win_activate){
      emit windowActivate(w);
    }

    if (a == module_win_icon_change){
      emit windowIconChanged(w);
    }

    if (a == kwm_command){
      char c[21];
      int i;
      for (i=0;i<20;i++)
	c[i] = ev->xclient.data.b[i];
      c[i] = '\0';
      QString com = c;
      emit commandReceived(com);
    }

    if (a == module_dockwin_add){
      wp = new Window;
      *wp = w;
      dock_windows.append(wp);
      emit dockWindowAdd(w);
    }

    if (a == module_dockwin_remove){
      for (wp=dock_windows.first(); wp; wp=dock_windows.next()){
	if (*wp == w){
	  dock_windows.remove();
	  break;
	}
      }
      emit dockWindowRemove(w);
    }

    if (a == sound){
      char c[21];
      int i;
      for (i=0;i<20;i++)
	c[i] = ev->xclient.data.b[i];
      c[i] = '\0';
      QString com = c;
      emit playSound(com);
    }

    if (a == register_sound){
      char c[21];
      int i;
      for (i=0;i<20;i++)
	c[i] = ev->xclient.data.b[i];
      c[i] = '\0';
      QString com = c;
      emit registerSound(com);
    }

    if (a == unregister_sound){
      char c[21];
      int i;
      for (i=0;i<20;i++)
	c[i] = ev->xclient.data.b[i];
      c[i] = '\0';
      QString com = c;
      emit unregisterSound(com);
    }

    return TRUE;

  }

  return FALSE;
}


bool KWMModuleApplication::hasWindow(Window w){

  Window *wp;
  for (wp=windows.first(); wp && *wp != w; wp=windows.next());

  return wp != 0L;

}
