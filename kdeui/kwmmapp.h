/* This file is part of the KDE libraries
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
*/
/*
 * kwmmapp.h. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#ifndef KWM_MODULE_APPLICATION_H
#define KWM_MODULE_APPLICATION_H

#include <qapplication.h>
#include <qlist.h>
#include <qwidget.h>
#include <kapp.h>

#include "kwm.h"


/**
 * The class KWMModuleApplication is the base class for KDE
 * windowmanager modules. It mainly informs a module about all
 * currently managed windows and changes to them (via Qt
 * signals). There are no methods to manipulate windows. These are
 * defined in the class KWM (see kwm.h).
 * @short Base class for KDE Window Manager modules.
 * @author Matthias Ettrich (ettrich@kde.org)
 * @version $Id$
 */
class KWMModuleApplication:public KApplication {
  Q_OBJECT

public:

  KWMModuleApplication( int &argc, char *argv[]);
  KWMModuleApplication( int &argc, char *argv[], const QString& rAppName);
  virtual ~KWMModuleApplication(){};

  /**
   * Connect to KWM. This cannot be done in the constructor, since your
   * application probably is not ready to recieve messages at this state.
   */
  void connectToKWM(bool dock_module = false);

  /**
	if you inherit KWMModuleApplication and overload x11EventFilter,
     be sure to call its x11EventFilter in your x11EventFilter:
           if (KWMModuleApplication::x11EventFilter(XEvent *))
               return True;
  */
  virtual bool x11EventFilter( XEvent * );

  /**
   * A list of all toplevel windows currently managed by the
   * windowmanger in the order of creation. Please do not rely on
   * indexes of this list: Whenever you enter Qt's eventloop in your
   * application it may happen, that entries are removed or added! So
   * your module should perhaps work on a copy of this list and verify a
   * window with hasWindow() before any operations.
   */
  QList <Window> windows;

  /**
   * A list of all toplevel windows currently managed by the
   * windowmanger in the current stacking order (from lower to
   * higher). May be useful for pagers.
   */
  QList <Window> windows_sorted;

  /**
   * Is <Window> still managed at present?
   */
  bool hasWindow(Window);


  /**
    * The dock windows. Only valid if you are succesfully connected as
    * docking module
    */
  QList <Window> dock_windows;

signals:

  /**
   * Note that an init() may also be emitted if the window manager is
   * restarted. So your module MUST react on it by clearing all internal
   * data structures.
   */
  void init();


  /**
   *    This signal is emitted when a connect is complete, i.e. when
   *    all existing windows or soundevents have been transfered to
   *    the module
   */
  void initialized();

  /**
   * Switch to another virtual desktop
   */
  void desktopChange(int);

  /**
   * Add a window
   */
  void windowAdd(Window);

  /**
   * Remove a window
   */
  void windowRemove(Window);

  /**
   * A window has been changed (size, title, etc.)
   */
  void windowChange(Window);

  /**
   * Raise a window
   */
  void windowRaise(Window);

  /**
   * Lower a window
   */
  void windowLower(Window);

  /**
   * Hint that <Window> is active (= has focus) now.
   */
  void windowActivate(Window);

  /**
    * A command kwm did not understand. Maybe it is for
    * your module.
    */
  void commandReceived(QString);


  /**
    * This is not integrated into windowChange since reading
    * icons is somewhat expensive via the KWM class.
    */
  void windowIconChanged(Window);

  /**
    * The specified desktop got a new name
    */
  void desktopNameChange(int, QString);

  /**
    * The number of desktops changed
    */
  void desktopNumberChange(int);

  /**
   * Add a dock window
   */
  void dockWindowAdd(Window);

  /**
   * Remove a dock window
   */
  void dockWindowRemove(Window);

  /**
    * Play/register/unregister a sound
    */
  void playSound(QString);
  void registerSound(QString);
  void unregisterSound(QString);


private:
  QWidget* module;

};

#endif // KWM_MODULE_APPLICATION_H



