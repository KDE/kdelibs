/*
 * kwmmapp.h. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#ifndef KWM_MODULE_APPLICATION_H
#define KWM_MODULE_APPLICATION_H

#include <qapp.h>
#include <qlist.h>
#include <qwidget.h>
#include <kapp.h>

#include "kwm.h"


/* The class KWMModuleApplication is the base class for KDE
 * windowmanager modules. It mainly informs a module about all
 * currently managed windows and changes to them (via Qt
 * signals). There are no methods to manipulate windows. These are
 * defined in the class KWM (see kwm.h).  
 */
class KWMModuleApplication:public KApplication {
  Q_OBJECT

public:

  KWMModuleApplication( int &argc, char *argv[]);
  virtual ~KWMModuleApplication(){};

  /* Connect to KWM. This cannot be done in the constructor, since your
   * application probably is not ready to recieve messages at this state.
   */
  void connectToKWM();
  
  /* if you inherit KWMModuleApplication and overload x11EventFilter,
     be sure to call its x11EventFilter in your x11EventFilter:
           if (KWMModuleApplication::x11EventFilter(XEvent *))
               return True;
  */
  virtual bool x11EventFilter( XEvent * );

  /* A list of all toplevel windows currently managed by the
   * windowmanger in the order of creation. Please do not rely on
   * indexes of this list: Whenever you enter Qt's eventloop in your
   * application it may happen, that entries are removed or added! So
   * your module should perhaps work on a copy of this list and verify a 
   * window with hasWindow() before any operations. 
   */
  QList <Window> windows;

  /* A list of all toplevel windows currently managed by the
   * windowmanger in the current stacking order (from lower to
   * higher). May be usefull for pagers.  
   */
  QList <Window> windows_sorted;

  /* Is <Window> still managed at present?
   */
  bool hasWindow(Window);

signals:

  /* Note that an init() may also be emitted if the window manager is
   * restarted. So your module MUST react on it by clearing all internal
   * data structures.
   */
  void init();

  /* Switch to another virtual desktop
   */
  void desktopChange(int);

  /* add/remove windows
   */
  void windowAdd(Window);
  void windowRemove(Window);

  /* a window has been changed (size, title, etc.)
   */
  void windowChange(Window);

  /* raise/lower windows 
   */ 
  void windowRaise(Window);
  void windowLower(Window);
  
  /* hint that <Window> is active (= has focus) now.
   */
  void windowActivate(Window);

private:
  QWidget* module;

};  

#endif // KWM_MODULE_APPLICATION_H



