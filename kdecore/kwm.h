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
 * Kwm.h. Part of the KDE project.
 */

#ifndef KWM_H
#define KWM_H

#include <qstring.h>
#include <qapp.h>
#include <qpixmap.h>
#include <X11/Xlib.h>


/** 
 * The class KWM allows usual X11 clients to get or modify window
 * properties and to interact with the windowmanager. It also offers
 * some high level functions to support the easy implementation of
 * session managment.  
 * @short class for interaction with the window- and sessionmanager
 * @author Matthias Ettrich (ettrich@kde.org)
 */
class KWM {

public:
  
  /**
   * Return all window properties needed to restore a window in 
   * a string. The string can be used as a command line argument
   * for the session management (See setWmCommand below).  */
  static QString getProperties(Window w);

  /** 
   * Apply properties from a property string to a window. This will
   * have an immediate effect if the window is already managed by the
   * window manager (that means it is either in Normal- or in
   * IconicState). If the window is still WithDrawn, the windowmanager
   * will apply the properties when the window is mapped (This happens
   * for example with a QWidget::show() ). If setProperties is used
   * for session management, you should call it first and map the
   * window later, since this avoids unnecessary flickering.
   *
   * setProperties returns the geometry of the property string.
   *
   * Note: Some window managers may not understand all kwm properties,
   * but all common properties such as geometry or IconicState should
   * nevertheless work anyway.
   */
  static QRect setProperties(Window w, const QString &props);


  /**
   * This will tell the windowmanager that your client is able to do
   * session management. If you do this, your client MUST react on
   * WM_SAVE_YOURSELF client messages by setting the XA_WM_COMMAND
   * property. This can be easily done with a call to setWmCommand
   * (see below). If your application has several top level windows
   * which are not transient, you can enable session management only
   * for the main toplevel window, but call setWmCommand for the
   * others with an empty (not null) string as argument. You
   * should/can do that already when you create these windows. It will
   * prevent the session manager from believing that this window does
   * not support session management at all. Whenever you react on
   * WM_SAVE_YOURSELF, you will have to call setWmCommand. Passing an
   * empty (not null) string is legal: The session manager will
   * understand this as an indicator that everything is ok and that
   * your application window may be restored by somebody else ;-)
   *
   * Note: If you enable session management although your client does
   * not react correctly on WM_SAVE_YOURSELF client messages, the
   * window manager cannot perform the logout process, that means
   * the GUI will hang!
   *
   * Note 2: Session management needs a session manager. In KDE this is
   * also done by the windowmanager kwm. Other windowmangers might not
   * support this, so you may have to run a standalone session manager
   * instead. Anyway, enabling session management does not harm in the
   * case a user does not run a session manager. He/she simply will
   * not have session management, but all other functionality.  
   *
   * Note 3: PSEUDO SESSION MANAGEMENT
   * There is also another way to do pseudo session management:
   *  1) do NOT enable session management
   *  2) call setWmCommand (see below) anyway and set the command
   *     which was used to start your application.
   *
   * This is exactly what old X11 applications like xterm, xedit or xv
   * do. BTW setWmCommand will also set the WM_CLIENT_MACHINE
   * property, which allows the session manager to restart the
   * application on the correct machine.
   *
   * kwm will act similar to smproxy: the window properties will be
   * stored and applied again when the window will be mapped after the
   * restart. To avoid unnecessary flickering or jumping of windows it
   * is important to call setWmCommand BEFORE your window is mapped!
   *
   * Anyway your application will appear in a warning box in kwm,
   * which indicates that the user might have some data loss. If you
   * do not want that (because your application does not contain any
   * data to be lost, for example a clock), you may simply call
   * setUnsavedDataHint with FALSE as value. Note that this hint is
   * read only when the user logs out, so the last value will be
   * used. You can also set it to TRUE to indicate that the user will
   * certainly have some data loss. All these things may of course
   * have no effect with other window- or session managers.  
   *
   * Note 4: kwm also supports session management for toplevel windows
   * that are not mapped (and never have been mapped). This is very
   * usefull for programms with n windows, where n can be 0 (like kfm,
   * for example), which want to recieve a WM_SAVE_YOURSELF message
   * nevertheless. The same as usual: If you do not want the session
   * manager to restart your application, since it is already started
   * by the usual startup script (True for kfm), set the command to an
   * empty string (setWmCommand(<your window>, "");)
   */
  static void enableSessionManagement(Window w);


  /** 
   * Store a shell command that can be used to restore the client.
   * This should be done as reaction on a WM_SAVE_YOURSELF client
   * message. setWmCommand also sets the WM_CLIENT_MACHINE property to
   * allow a session management to restart your application even on
   * the correct machine.
   */
  static void setWmCommand(Window w, const QString &command);

  /* Clients who react on WM_SAVE_YOURSELF should be able to save
   * their entire state. If not (version under developent, lazy
   * author, etc...)  they should at least save their geometry
   * informations and call setUnsavedDataHint. This will indicate to
   * the session manager that the user might lose some data if he/she
   * continues with the logout process. 
   *
   * The hint can be set or unset either in the response to
   * WM_SAVE_YOURSELF or anytime during runtime. Please make sure that
   * the hint is also reset after the user saved the data!
   *
   * Note: The hint has also effect for clients that are not session management
   * enabled or do not even do pseudo session management
   *
   * Note 2: May have no effekt with other window- or session managers 
   */
  static void setUnsavedDataHint(Window w, bool value);

  /** 
   * Set a mini icon for your application window. This icon may appear
   * in the window decoration as well as on taskbar buttons. Therefore
   * it should be very small (kwm for example will scale it to 14x14,
   * so 14x14 may be the size of choice).
   *
   * setMiniIcon will _not_ change the icon_pixmap property of the
   * standard XWMHints, but define a special KWM_WIN_ICON
   * property. This has the advantage, that your application can
   * support standard X11 icons (which are usually _very_ large) if it
   * runs with mwm for example, as well as modern KDE-like miniicons.
   *
   * Note: kwm will even detect changes of this property when your
   * window is mapped.
   *
   * Note 2: May have no effekt with other windowmanagers */
  static void setMiniIcon(Window w, const QPixmap &pm);

  /**
   * Set a standard (large) icon for the application window. If you
   * are using Qt this should be almost aequivalent to a call to
   * QWidget::setIcon(..)  
   */
  static void setIcon(Window w, const QPixmap &pm);

  /**
   * This declares a toplevel window to be a docking window. If a KWMDockModule
   * is running (for example kpanel), it will swallow the window on
   * its docking area when it is mapped.
   *
   * For docking call setDockWindow() on a new _unmapped_ toplevel
   * window. If this window will be mapped (show), it will appear on
   * the docking area, if you unmap it again (hide) it will disappear
   * from the docking area again. Docking windows MUST be toplevel
   * windows. If you create them with Qt (new QWidget()->winId()) be
   * sure not to pass any parent.
   *
   * Note: May have no effect with other window mangers 
   */
  static void setDockWindow(Window w);

  /** 
   *Some windows do not want to be decorated at all but should not be
   * override_redirect (for example toolbars which are dragged out of
   * their main window). This can be achieved with a simple call to
   * setDecoration() even if the window is already mapped.
   *
   * KWM understands the following values at present:
   *   noDecoration :      Not decorated at all
   *   normalDecoration :  Normal decoration (transient windows with 
   *                       dialog decoration, shaped windows not decorated 
   *                       at all)
   *                       
   *   tinyDecoration :    Tiny decoration (just a little frame)
   *
   * If your window does not want the focus, you can OR the decoration
   * property with noFocus, for example kpager:
   *    KWM::setDecoration(winId(), KWM::tinyDecoration | KWM::noFocus);
   *
   * Note: X11R6 does not offer a standard property or protocoll for
   * this purpose. So kwm uses a KDE specific property which may have
   * no effect with other window managers.  
   */
  static void setDecoration(Window w, long value);
  
  enum {noDecoration = 0, normalDecoration = 1, tinyDecoration = 2,
	noFocus = 256};

  /** 
   * Invokes the logout process (session management, logout dialog, ...)
   *
   * Note: May have no effect with other window mangers
   */
  static void logout();

  /** 
   * Clients who draw over other windows should call refreshScreen()
   * afterwards to force an ExposureEvent on all visible windows.
   * Also necessary after darkenScreen();
   *
   * Note: May have no effect with other window mangers
   */
  static void refreshScreen();

  /**
   * Draw a black matrix over the entire screen to make it look darker.
   * Clients who use this should grab the X server afterwards and also
   * call refreshScreen() after releasing the X server.
   *
   * Note: May have no effect with other window mangers
   */
  static void darkenScreen();
  
  /**
   * Clients who manipulated the config file of the windowmanager
   * should call this afterwards
   *
   * Note: May have no effect with other window mangers
   */
  static void configureWm();


  /**
   * The current virtual desktop. This is usefull if your program behaves
   * different on different desktops (for example a background drawing
   * program) but does not itself need to be a window manager module
   * (which recieve a message when the current desktop changes)
   */
  static int currentDesktop();



  /**
    * Raise/register/unregister sound events. The number of character in 
    * the event name is limited to 20.
    *
    Note: May have no effect with other window mangers
    */
  static void raiseSoundEvent(const QString &event);
  static void registerSoundEvent(const QString &event);
  static void unregisterSoundEvent(const QString &event);




  /**
   *************************************************************************
   *
   * Functions below are probably only important for window manager modules 
   *
   * DO NOT USE THEM WITH REGULAR CLIENTS
   *
   ************************************************************************* 
   */

  /**
   * Declare a toplevel window to be used for module
   * communication. This window does not need to be mapped, although
   * it can be mapped of course.  Module windows MUST be toplevel
   * windows. If you create them with Qt (new QWidget()->winId()) be
   * sure not to pass any parent. Modules get the following client
   * messages from kwm. Argument is either a window ((Window)
   * XClientMessageEvent.data.l[0]) or nothing.
   *
   * KWM_MODULE_INIT             - initinalization
   * KWM_MODULE_DESKTOP_CHANGE   - new current virtual desktop
   * KWM_MODULE_WIN_ADD          - new window
   * KWM_MODULE_WIN_REMOVE       - remove window
   * KWM_MODULE_WIN_CHANGE       - size, properties, map state etc.
   * KWM_MODULE_WIN_RAISE        - raise and lower allow a module (for example
   * KWM_MODULE_WIN_LOWER          a pager) to keep the stacking order
   * KWM_MODULE_WIN_ACTIVATE     - new active (focus) window
   *
   * KDE_SOUND_EVENT             - a sound event
   * KDE_REGISTER_SOUND_EVENT    - registration of a new sound event
   * KDE_UNREGISTER_SOUND_EVENT  - remove a registration 
   *
   * Please check out the KWMModuleApplication class which gives you easy
   * access to all these messages via Qt signals and slots. It also keeps
   * automatically a list of windows in both stacking and creation order.  
   */
  static void setKWMModule(Window w);
  static bool isKWMModule(Window w);

  /**
   * Note that there can only be one single KWMDockModule at the same
   * time.  The first one to come wins. The later can check the result
   * with isKWMDockModule.  
   *
   * The KWMDockModule recieves two additional client messages:
   * KWM_MODULE_DOCKWIN_ADD
   * KWM_MODULE_DOCKWIN_REMOVE
   */
  static void setKWMDockModule(Window w);
  static bool isKWMDockModule(Window w);

  /**
    * This is usefull for modules which depend on a running
    * windowmanager for the correct settings of the number of desktops
    * or the desktop names (for example kpanel).
    * Just insert
    *         while (!KWM::isKWMInitialized()) sleep(1);
    * in the main function of your application to wait for kwm being
    * ready. As an alternative you can also wait for the init-signal
    * of KWMModuleApplication.
    */
  static bool isKWMInitialized();

  static Window activeWindow();
  static void switchToDesktop(int desk);

  /** 
   *Window region define the rectangle where windows can appear. A
   * window which can be fully maximized will exactly fill this
   * region. The regions are settable withing kwm to allow desktop
   * panels like kpanel to stay visible even if a window becomes maximized.
   * kwm will store the regions in the kwmrc when exiting.
   */
  static void setWindowRegion(int desk, const QRect &region);
  static QRect getWindowRegion(int desk);

  /** 
   * At present the maximium number of desktops is limited to 32
   *
   * As with the window regions, kwm will store these properties in
   * the kwmrc when exiting
   */
  static int numberOfDesktops();
  static void setNumberOfDesktops(int num);
  static void setDesktopName(int desk, const QString &name);
  static QString getDesktopName(int desk);

  /**
   * low level kwm communication. This can also be used for a one-way
   * communication with modules, since kwm sends messages, which it
   * does not understand, to all modules. You module should declare a
   * short unque praefix for this task.
   *
   * Note: the length of a command is limited to 20 character. This
   * stems from the limitations of an ordinary X11 client messages.
   */
  static void sendKWMCommand(const QString &command);

  /**
   * The standard window title. If kwm runs, title() will return the
   * title kwm uses. These titles are always unique.  
   */
  static QString title(Window w);

  /** 
   *titleWithState is aequivalent to title(), but inside round brackets
   * for iconified window 
   */
  static QString titleWithState(Window w);

  /** 
   * if no miniicon is set, miniIcon() will return the standard
   * icon. The result will be scaled to (width, height) if it is
   * larger. Otherwise it will not be modified. If you do not specify
   * width or height, the result will be returned in its native
   * size. Both functions will return a null pixmap in the case no
   * icon is defined.
   */
  static QPixmap miniIcon(Window w, int width=0, int height=0);
  static QPixmap icon(Window w, int width=0, int height=0);


  /**
   ***** GET WINDOW PROPERTIES *****
   */

  static int desktop(Window w);
  static QRect geometry(Window w, bool including_frame = FALSE);

  /**
   * geometry restore is only defined for maximized window. It is the
   * geometry a window will get when it is unmaximized.  
   */
  static QRect geometryRestore(Window w);
  static bool isIconified(Window w);
  static bool isMaximized(Window w);
  static bool isSticky(Window w);
  static long getDecoration(Window w);
  static bool fixedSize(Window w);
  static bool containsUnsavedData(Window w);
  static bool unsavedDataHintDefined(Window w);
  static bool isActive(Window w);
  

  /**
   * ***** SET WINDOW PROPERTIES *****
   *
   * Note: The functions for some general properties (decoration,
   * icons) are in the general section at the beginning of this
   * document.
   */

  static void moveToDesktop(Window w, int desk);
  static void setGeometry(Window w, const QRect &geom);
  static void setGeometryRestore(Window w, const QRect &geom);
  static void move(Window w, const QPoint &pos);
  static void setMaximize(Window w, bool value);
  static void setIconify(Window w, bool value);
  static void setSticky(Window w, bool value);

  /**
   * close will send a WM_DELETE_WINDOW message to the window, if this window
   * requested for this protocol. Otherwise it will simply be destroyed.
   */
  static void close(Window w);

  /** 
   *activate will deiconify the window, if is is iconified, or switch to
   * another desktop, if the window is there. Then the window is raised
   * and activated with activateInteral (see below)
   */
  static void activate(Window w);

  /** 
   * activateInternal will simply activate the window. Be carefull: you might 
   * run into trouble if the window is iconified or on another desktop!
   * You probably prefer activate (see above).
   */
  static void activateInternal(Window w);

  /**
   * raise or lower a window. Should work with any windowmanager
   */
  static void raise(Window w);
  static void lower(Window w);

  /**
   * if you like to swallow a window ( == XReparentWindow) you MUST call 
   * prepareForSwallowing before. This will set the window to WithDrawnState
   * (and also wait until the window achieved this state), what usually let
   * the windowmanager remove all its decorations and re-reparent the window
   * to the root window.
   *
   * Note: It may happen that you are using a windowmanager which behaves 
   * different. Maybe this one is able to handle the XReparentWindow anyway,
   * maybe XReparentWindow will fail. There is nothing I can do about it.
   */
  static void prepareForSwallowing(Window w);

  /**
    * doNotManage(...) allows to define the first 20 character of a window
    * title that should not be managed by the windowmanager. This is useful
    * to avoid flickering when a panel swallows legacy applications.
    * doNotManage(...) applies only for the first window which meets the
    * criteria. 
    *
    * Note: May have no effect with other windowmanagers.
    */
  static void doNotManage(const QString&);


  /**
   * ***** CURRENT NAMES OF WINDOW OPERATION COMMANDS *****
   *
   * For 100% consitancy of the desktop, clients which do window operations 
   * should use these strings.
   */
  static QString getMaximizeString();
  static QString getUnMaximizeString();
  static QString getIconifyString();
  static QString getUnIconifyString();
  static QString getStickyString();
  static QString getUnStickyString();
  static QString getMoveString();
  static QString getResizeString();
  static QString getCloseString();
  static QString getToDesktopString();
  static QString getOntoCurrentDesktopString();


  /**
   * ***** MORE OR LESS ONLY USEFULL FOR INTERNAL PURPOSES OR THE
   *       WINDOWMANAGER ITSELF *****
   */

  static bool isDockWindow(Window w);
  static int getWindowState(Window w);

};

#endif // KWM_H



