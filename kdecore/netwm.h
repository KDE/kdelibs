/*

  Copyright (c) 2000 Troll Tech AS

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


#ifndef   __net_wm_h
#define   __net_wm_h

#include <qwidget.h>
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "netwm_def.h"

// forward declaration
struct NETRootInfoPrivate;
struct NETWinInfoPrivate;


/**
   Common API for root window properties/protocols.

   The NETRootInfo class provides a common API for clients and window managers
   to set/read/change properties on the root window as defined by the NET Window
   Manager Specification..

   @author Bradley T. Hughes <bhughes@trolltech.com>
   @see NET
 **/

class NETRootInfo : public NET {
public:
    /**
       Window Managers should use this constructor to create a NETRootInfo object,
       which will be used to set/update information stored on the rootWindow.
       The application role is automatically set to WindowManager
       when using this constructor.

       @param display An X11 Display struct.

       @param supportWindow The Window id of the supportWindow.  The supportWindow
       must be created by the window manager as a child of the rootWindow.  The
       supportWindow must not be destroyed until the Window Manager exits.

       @param wmName A string which should be the window manager's name (ie. "KWin"
       or "Blackbox").

       @param properties An OR'ed list of all properties and protocols the window
       manager supports (see the NET base class documentation for a description
       of all properties and protocols).

       @param screen For Window Managers that support multiple screen (ie.
       "multiheaded") displays, the screen number may be explicitly defined.  If
       this argument is omitted, the default screen will be used.
    **/
    NETRootInfo(Display *display, Window supportWindow, const char *wmName,
		unsigned long properties, int screen = -1, bool doActivate = true);

    /**
       Clients should use this constructor to create a NETRootInfo object, which
       will be used to query information set on the root window. The application
       role is automatically set to Client when using this constructor.

       @param display An X11 Display struct.

       @param properties An OR'ed list of all properties and protocols the client
       supports (see the NET base class documentation for a description of all
       properties and protocols).

       @param screen For Clients that support multiple screen (ie. "multiheaded")
       displays, the screen number may be explicitly defined. If this argument is
       omitted, the default screen will be used.
    **/
    NETRootInfo(Display *display, unsigned long properties, int screen = -1,
		bool doActivate = true);

    /**
       Creates a shared copy of the specified NETRootInfo object.
    **/
    NETRootInfo(const NETRootInfo &rootinfo);

    /**
       Destroys the NETRootInfo object.
    **/
    virtual ~NETRootInfo();

    /**
       Returns the X11 Display struct used.
    **/
    Display *x11Display() const;

    /**
       Returns the Window id of the rootWindow.
    **/
    Window rootWindow() const;

    /**
       Returns the Window id of the supportWindow.
    **/
    Window supportWindow() const;

    /**
       Returns the name of the Window Manager.
    **/
    const char *wmName() const;

    /**
       Returns the screenNumber.
    **/
    int screenNumber() const;

    /**
       Returns an OR'ed list of supported protocols and properties.
    **/
    unsigned long supported() const;

    /**
       Returns an array of Window id's, which contain all managed windows.

       @see clientListCount
    **/
    const Window *clientList() const;

    /**
       Returns the number of managed windows in clientList array.
    **/
    int clientListCount() const;

    /**
       Returns an array of Window id's, which contain all managed windows in
       stacking order.
    **/
    const Window *clientListStacking() const;

    /**
       Returns the number of managed windows in the clientListStacking array.
    **/
    int clientListStackingCount() const;

    /**
       Returns an array of Window id's, which contain all KDE system tray windows.
    **/
    const Window *kdeSystemTrayWindows() const;

    /**
       Returns the number of windows in the kdeSystemTrayWindows array.
    **/
    int kdeSystemTrayWindowsCount() const;

    /**
       Returns the desktop geometry size.

       The desktop argument is ignored. Early drafts of the NET WM
       Specification were unclear about the semantics of this property.
    **/
    NETSize desktopGeometry(int desktop) const;

    /**
       Returns the viewport of the specified desktop.
    **/
    NETPoint desktopViewport(int desktop) const;

    /**
       Returns the workArea for the specified desktop.
    **/
    NETRect workArea(int desktop) const;

    /**
       Returns the name for the specified desktop.
    **/
    const char *desktopName(int desktop) const;

    /**
       Returns an array of Window id's, which contain the virtual root windows.
    **/
    const Window *virtualRoots( ) const;

    /**
       Returns the number of window in the virtualRoots array.
    **/
    int virtualRootsCount() const;

    /**
       Returns the number of desktops.
    **/
    int numberOfDesktops() const;

    /**
       Returns the current desktop.
    **/
    int currentDesktop() const;

    /**
       Returns the active (focused) window.
    **/
    Window activeWindow() const;

    /**
       Window Managers must call this after creating the NETRootInfo object, and
       before using any other method in the class.  This method sets initial data
       on the root window and does other post-construction duties.

       Clients must also call this after creating the object to do an initial
       data read/update.
    **/
    void activate();

    /**
       Sets the list of managed windows on the Root/Desktop window.

       @param windows The array of Window id's

       @param count The number of windows in the array
    **/
    void setClientList(Window *windows, unsigned int count);

    /**
       Sets the list of managed windows in stacking order on the Root/Desktop
       window.

       @param windows The array of Window id's

       @param count The number of windows in the array.
    **/
    void setClientListStacking(Window *windows, unsigned int count);

    /**
       Sets the list of KDE system tray windows on the root window.

       @param window The array of window id's

       @param count The number of windows in the array.
    **/
    void setKDESystemTrayWindows(Window *windows, unsigned int count);

    /**
       Sets the current desktop to the specified desktop.
    **/
    void setCurrentDesktop(int desktop);

    /**
       Sets the desktop geometry to the specified geometry.

       The desktop argument is ignored. Early drafts of the NET WM
       Specification were unclear about the semantics of this property.
    **/
    void setDesktopGeometry(int desktop, const NETSize &geometry);

    /**
       Sets the viewport for the current desktop to the specified point.
    **/
    void setDesktopViewport(int desktop, const NETPoint &viewport);

    /**
       Sets the number of desktops the the specified number.
    **/
    void setNumberOfDesktops(int numberOfDesktops);

    /**
       Sets the name of the specified desktop.
    **/
    void setDesktopName(int desktop, const char *desktopName);

    /**
       Sets the active (focused) window the specified window.
    **/
    void setActiveWindow(Window window);

    /**
       Sets the workarea for the specified desktop
    **/
    void setWorkArea(int desktop, const NETRect &workArea);

    /**
       Sets the list of virtual root windows on the root window.

       @param windows The array of Window id's

       @param count The number of windows in the array.
    **/
    void setVirtualRoots(Window *windows, unsigned int count);

    /**
       Assignment operator.  Ensures that the shared data reference counts are
       correct.
    **/
    const NETRootInfo &operator=(const NETRootInfo &rootinfo);

    /**
       Clients (such as pagers/taskbars) that wish to close a window should call
       this function.  This will send a request to the Window Manager, which
       usually can usually decide how to react to such requests.
    **/
    void closeWindowRequest(Window window);

    /**
       Clients (such as pagers/taskbars) that wish to start a WMMoveResize
       (where the window manager controls the resize/movement) should call
       this function.  This will send a request to the Window Manager.

       @param window The client window that would be resized/moved.

       @param x_root X position of the cursor relative to the root window.

       @param y_root Y position of the cursor relative to the root window.

       @param direction One of NET::Direction (see base class documentation for
       a description of the different directions).
    **/
    void moveResizeRequest(Window window, int x_root, int y_root,
			   Direction direction);

    /**
       This function takes the passed XEvent and returns an OR'ed list of
       NETRootInfo properties that have changed.  The new information will be
       read immediately by the class.
    **/
    unsigned long event(XEvent *event);


protected:
    /**
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a window has been added.
    **/
    // virtual void addClient(Window window) { }
    virtual void addClient(Window) { }

    /**
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a window has been removed.
    **/
    // virtual void removeClient(Window window) { }
    virtual void removeClient(Window) { }

    /**
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a system tray window has been added.  This is a KDE 2.0
       extension.
    **/
    // virtual void addSystemTrayWin(Window window) { }
    virtual void addSystemTrayWin(Window) { }

    /**
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a system tray window has been removed.  This is a KDE 2.0
       extension.
    **/
    // virtual void removeSystemTrayWin(Window window) { }
    virtual void removeSystemTrayWin(Window) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the number
       of desktops.
    **/
    // virtual void changeNumberOfDesktops(int numberOfDesktops) { }
    virtual void changeNumberOfDesktops(int) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the specified
       desktop geometry.
    **/
    // virtual void changeDesktopGeometry(int desktop, const NETSize &geom) { }
    virtual void changeDesktopGeometry(int, const NETSize &) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the specified
       desktop viewport.
    **/
    // virtual void changeDesktopViewport(int desktop, const NETPoint &viewport) { }
    virtual void changeDesktopViewport(int, const NETPoint &) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the current
       desktop.
    **/
    // virtual void changeCurrentDesktop(int desktop) { }
    virtual void changeCurrentDesktop(int) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the active
       (focused) window.
    **/
    // virtual void changeActiveWindow(Window window) { }
    virtual void changeActiveWindow(Window) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to close a window.
    **/
    // virtual void closeWindow(Window window) { }
    virtual void closeWindow(Window) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to start a move/resize.

       @param window The window that wants to move/resize

       @param x_root X position of the cursor relative to the root window.

       @param y_root Y position of the cursor relative to the root window.

       @param direction One of NET::Direction (see base class documentation for
       a description of the different directions).
    **/
    // virtual void moveResize(Window window, int x_root, int y_root,
    // 			    unsigned long direction) { }
    virtual void moveResize(Window, int, int, unsigned long) { }


private:
    void update(unsigned long);
    void setSupported(unsigned long);
    Role role;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    NETRootInfoPrivate *p;
};


/**
   Common API for application window properties/protocols.

   The NETWinInfo class provides a common API for clients and window managers to
   set/read/change properties on an application window as defined by the NET
   Window Manager Specification.

   @author Bradley T. Hughes <bhughes@trolltech.com>
   @see http://www.freedesktop.org/standards/wm-spec/
 **/

class NETWinInfo : public NET {
public:
    /**
       Create a NETWinInfo object, which will be used to set/read/change
       information stored on an application window.

       @param display An X11 Display struct.

       @param window The Window id of the application window.

       @param rootWindow The Window id of the root window.

       @param properties An OR'ed list of all properties and protocols the
       client/window manager supports (see the NET base class documentation
       for a description of all properties and protocols).

       @param role Select the application role.  If this argument is omitted,
       the role will default to Client.
    **/
    NETWinInfo(Display *display, Window window,
	       Window rootWindow, unsigned long properties,
	       Role role = Client);

    /**
       Creates a shared copy of the specified NETWinInfo object.
    **/
    NETWinInfo(const NETWinInfo & wininfo);

    /**
       Destroys the NETWinInfo object.
    **/
    virtual ~NETWinInfo();

    /**
       Returns an OR'ed list of supported protocols and properties.
    **/
    unsigned long properties() const;

    /**
       Returns the icon geometry.
    **/
    NETRect iconGeometry() const;

    /**
       Returns the state of the window (see the NET base class documentation for a
       description of the various states).
    **/
    unsigned long state() const;

    /**
       Returns the strut specified by this client.
    **/
    NETStrut strut() const;

    /**
       Returns the window type for this client (see the NET base class
       documentation for a description of the various window types).
    **/
    WindowType windowType() const;

    /**
       Returns the name of the window in UTF-8 format.
    **/
    const char *name() const;

    /**
       Returns the visible name as set by the window manager in UTF-8 format.
    **/
    const char *visibleName() const;

    /**
       Returns the icon name of the window in UTF-8 format.
    **/
    const char *iconName() const;

    /**
       Returns the visible icon name as set by the window manager in UTF-8 format.
    **/
    const char *visibleIconName() const;

    /**
       Returns the desktop where the window is residing.
    **/
    int desktop() const;

    /**
       Returns the process id for the client window.
    **/
    int pid() const;

    /**
       Returns whether or not this client handles icons.
    **/
    Bool handledIcons() const;

    /**
       Returns a Window id, telling the window manager which window we are
       representing.
    **/
    Window kdeSystemTrayWinFor() const;

    /**
       Returns the mapping state for the window (see the NET base class
       documentation for a description of mapping state).
    **/
    MappingState mappingState() const;

    /**
       Set icons for the application window.  If replace is True, then
       the specified icon is defined to be the only icon.  If replace is False,
       then the specified icon is added to a list of icons.
    **/
    void setIcon(NETIcon icon, Bool replace = True);

    /**
       Set the icon geometry for the application window.
    **/
    void setIconGeometry(NETRect geometry);

    /**
       Set the strut for the application window.
    **/
    void setStrut(NETStrut strut);

    /**
       Set the state for the application window (see the NET base class documentation
       for a description of window state).
    **/
    void setState(unsigned long state, unsigned long mask);

    /**
       Sets the window type for this client (see the NET base class
       documentation for a description of the various window types).
    **/
    void setWindowType(WindowType type);

    /**
       Sets the name for the application window.
    **/
    void setName(const char *name);

    /**
       For Window Managers only:  set the visible name ( i.e. xterm, xterm <2>,
       xterm <3>, ... )
    **/
    void setVisibleName(const char *visibleName);

    /**
       Sets the icon name for the application window.
    **/
    void setIconName(const char *name);

    /**
       For Window Managers only: set the visible icon name ( i.e. xterm, xterm <2>,
       xterm <3>, ... )
    **/
    void setVisibleIconName(const char *name);

    /**
       Set which window the desktop is (should be) on.
    **/
    void setDesktop(int desktop);

    /**
       Set the application window's process id.
    **/
    void setPid(int pid);

    /**
       Set whether this application window handles icons.
    **/
    void setHandledIcons(Bool handled);

    /**
       Set which window we are representing as a system tray window.
    **/
    void setKDESystemTrayWinFor(Window window);

    /**
       Set the frame decoration strut.  This is a KDE 2.0 extension to aid in
       writing pager applications.
    **/
    void setKDEFrameStrut(NETStrut strut);

    /**
       Returns an icon.  If width and height are passed, the icon returned will be
       the closest it can find (the next biggest).  If width and height are omitted,
       then the first icon in the list is returned.
    **/
    NETIcon icon(int width = -1, int height = -1) const;

    /**
       Places the window frame geometry in frame, and the application window
       geometry in window.  Both geometries are relative to the root window.
    **/
    void kdeGeometry(NETRect &frame, NETRect &window);

    /**
       This function takes the pass XEvent and returns an OR'ed list of NETWinInfo
       properties that have changed.  The new information will be read
       immediately by the class.
    **/
    unsigned long event(XEvent *event);

    /**
       Sentinel value to indicate that the client wishes to be visible on
       all desktops.
    **/
    static const int OnAllDesktops;


protected:
    /**
       A Window Manager should subclass NETWinInfo and reimplement this function when
       it wants to know when a Client made a request to change desktops (ie. move to
       another desktop).
    **/
    // virtual void changeDesktop(int desktop) { }
    virtual void changeDesktop(int) { }

    /**
       A Window Manager should subclass NETWinInfo and reimplement this function when
       it wants to know when a Client made a request to change state (ie. to
       Shade / Unshade).
    **/
    virtual void changeState(unsigned long /*state*/, unsigned long /*mask*/) { }

private:
    void update(unsigned long);
    Role role;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    NETWinInfoPrivate *p;
};


#endif
#endif // __net_wm_h
