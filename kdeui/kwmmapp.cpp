/*
 * kwmmapp.cpp. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include "kwmmapp.moc"

KWMModuleApplication::KWMModuleApplication( int &argc, char *argv[])
  :KApplication(argc, argv){
    module = new QWidget;
}


void KWMModuleApplication::connectToKWM(){
  KWM::setKWMModule(module->winId());
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
  static Atom kwm_command;

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
      kwm_command = XInternAtom(qt_xdisplay(), 
				"KWM_COMMAND", False);
    }
    a = ev->xclient.message_type;
    w = (Window) (ev->xclient.data.l[0]);

    if (a == module_init){
      windows.clear();
      windows_sorted.clear();
      emit init();
    }
    if (a == module_desktop_change){
      int d = (int) w;
      emit desktopChange(d);
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
	if (*wp == w)
	  windows.remove();
      }
      for (wp=windows_sorted.first(); wp; wp=windows_sorted.next()){
	if (*wp == w){
	  windows_sorted.remove();
	  delete wp;
	}
      }
      emit windowRemove(w);
    }
    if (a == module_win_change){
      emit windowChange(w);
    }
    if (a == module_win_raise){
      for (wp=windows_sorted.first(); wp; wp=windows_sorted.next()){
	if (*wp == w)
	  windows_sorted.remove();
      }
      windows_sorted.append(wp);
      emit windowRaise(w);
    }
    if (a == module_win_lower){
      for (wp=windows_sorted.first(); wp; wp=windows_sorted.next()){
	if (*wp == w)
	  windows_sorted.remove();
      }
      windows_sorted.insert(0, wp);
      emit windowLower(w);
    }
    if (a == module_win_activate){
      emit windowActivate(w);
    }
    if (a == kwm_command){
      QString com = ev->xclient.data.b;
      emit commandRecieved(com);
    }

    return TRUE;
  }
  return FALSE;
}

bool KWMModuleApplication::hasWindow(Window w){
  Window *wp;
  for (wp=windows.first(); wp && *wp != w; wp=windows.next());
  return wp != NULL;
}
