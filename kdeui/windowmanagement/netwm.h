/*

  Copyright (c) 2000 Troll Tech AS
  Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>

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


#ifndef   netwm_h
#define   netwm_h

#include <kdeui_export.h>
#include <QtGui/QWidget>
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <fixx11h.h>

#include "netwm_def.h"

// forward declaration
struct NETRootInfoPrivate;
struct NETWinInfoPrivate;
template <class Z> class NETRArray;


/**
   Common API for root window properties/protocols.

   The NETRootInfo class provides a common API for clients and window managers
   to set/read/change properties on the root window as defined by the NET Window
   Manager Specification..

   @author Bradley T. Hughes <bhughes@trolltech.com>
   @see NET
   @see NETWinInfo
 **/

class KDEUI_EXPORT NETRootInfo : public NET {
public:
    /**
        Indexes for the properties array.
    **/
    // update also NETRootInfoPrivate::properties[] size when extending this
    enum { PROTOCOLS, WINDOW_TYPES, STATES, PROTOCOLS2, ACTIONS,
        PROPERTIES_SIZE };

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

       @param properties An array of elements listing all properties and protocols
       the window manager supports. The elements contain OR'ed values of constants
       from the NET base class, in the following order: [0]= NET::Property,
       [1]= NET::WindowTypeMask (not NET::WindowType!), [2]= NET::State,
       [3]= NET::Property2, [4]= NET::Action.
       In future versions, the list may be extended. In case you pass less elements,
       the missing ones will be replaced with default values.

       @param properties_size The number of elements in the properties array.

       @param screen For Window Managers that support multiple screen (ie.
       "multiheaded") displays, the screen number may be explicitly defined.  If
       this argument is omitted, the default screen will be used.

       @param doActivate true to activate the window
    **/
    NETRootInfo(Display *display, Window supportWindow, const char *wmName,
		const unsigned long properties[], int properties_size,
                int screen = -1, bool doActivate = true);


    /**
       Clients should use this constructor to create a NETRootInfo object, which
       will be used to query information set on the root window. The application
       role is automatically set to Client when using this constructor.

       @param display An X11 Display struct.

       @param properties An array of elements listing all protocols the client
       is interested in. The elements contain OR'ed values of constants
       from the NET base class, in the following order: [0]= NET::Property,
       [1]= NET::Property2.

       @param properties_size The number of elements in the properties array.

       @param screen For Clients that support multiple screen (ie. "multiheaded")
       displays, the screen number may be explicitly defined. If this argument is
       omitted, the default screen will be used.

       @param doActivate true to call activate() to do an initial data read/update
       of the query information.
    **/
    NETRootInfo(Display *display, const unsigned long properties[], int properties_size,
                int screen = -1, bool doActivate = true);

    /**
        This constructor differs from the above one only in the way it accepts
        the list of supported properties. The properties argument is equivalent
        to the first element of the properties array in the above constructor,
        and therefore you cannot read all root window properties using it.
    **/
    NETRootInfo(Display *display, unsigned long properties, int screen = -1,
		bool doActivate = true);

    /**
       Creates a shared copy of the specified NETRootInfo object.

       @param rootinfo the NETRootInfo object to copy
    **/
    NETRootInfo(const NETRootInfo &rootinfo);

    /**
       Destroys the NETRootInfo object.
    **/
    virtual ~NETRootInfo();

    /**
       Returns the X11 Display struct used.

       @return the X11 Display
    **/
    Display *x11Display() const;

    /**
       Returns the Window id of the rootWindow.

       @return the id of the root window
    **/
    Window rootWindow() const;

    /**
       Returns the Window id of the supportWindow.

       @return the id of the support window
    **/
    Window supportWindow() const;

    /**
       Returns the name of the Window Manager.

       @return the name of the window manager
    **/
    const char *wmName() const;

    /**
       Returns the screenNumber.

       @return the screen number
    **/
    int screenNumber() const;

    /**
      Sets the given property if on is true, and clears the property otherwise.
      In WindowManager mode this function updates _NET_SUPPORTED.
      In Client mode this function does nothing.

      @since 4.4
     **/
    void setSupported( NET::Property property, bool on = true );

    /**
      @overload
      @since 4.4
     **/
    void setSupported( NET::Property2 property, bool on = true );

    /**
      @overload
      @since 4.4
     **/
    void setSupported( NET::WindowType property, bool on = true );

    /**
      @overload
      @since 4.4
     **/
    void setSupported( NET::State property, bool on = true );

    /**
      @overload
      @since 4.4
     **/
    void setSupported( NET::Action property, bool on = true );

    /**
       Returns true if the given property is supported by the window
       manager. Note that for Client mode, NET::Supported needs
       to be passed in the properties argument for this to work.
    **/
    bool isSupported( NET::Property property ) const;
    /**
       @overload
    **/
    bool isSupported( NET::Property2 property ) const;
    /**
       @overload
    **/
    bool isSupported( NET::WindowType type ) const;
    /**
       @overload
    **/
    bool isSupported( NET::State state ) const;

    /**
       @overload
    **/
    bool isSupported( NET::Action action ) const;

    /**
       In the Window Manager mode, this is equivalent to the properties
       argument passed to the constructor. In the Client mode, if
       NET::Supported was passed in the properties argument, the returned
       value is array of all protocols and properties supported
       by the Window Manager. The elements of the array are the same
       as they would be passed to the Window Manager mode constructor,
       the size is the maximum array size the constructor accepts.
    **/
    const unsigned long* supportedProperties() const;

    /**
       Returns the properties argument passed to the constructor.
       The size is the maximum array size the constructor accepts.
    **/
    const unsigned long* passedProperties() const;

    /**
       Returns an array of Window id's, which contain all managed windows.

       @return the array of Window id's

       @see clientListCount()
    **/
    const Window *clientList() const;

    /**
       Returns the number of managed windows in clientList array.

       @return the number of managed windows in the clientList array

       @see clientList()
    **/
    int clientListCount() const;

    /**
       Returns an array of Window id's, which contain all managed windows in
       stacking order.

       @return the array of Window id's in stacking order

       @see clientListStackingCount()
    **/
    const Window *clientListStacking() const;

    /**
       Returns the number of managed windows in the clientListStacking array.

       @return the number of Window id's in the client list

       @see clientListStacking()
    **/
    int clientListStackingCount() const;

    /**
       Returns the desktop geometry size.

       The desktop argument is ignored. Early drafts of the NET WM
       Specification were unclear about the semantics of this property.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. You should use calls for virtual desktops,
       viewport is mapped to them if needed.

       @param desktop the number of the desktop

       @return the size of the desktop
    **/
    NETSize desktopGeometry(int desktop) const;

    /**
       Returns the viewport of the specified desktop.
       
       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. You should use calls for virtual desktops,
       viewport is mapped to them if needed.

       @param desktop the number of the desktop

       @return the position of the desktop's viewport
    **/
    NETPoint desktopViewport(int desktop) const;

    /**
       Returns the workArea for the specified desktop.

       @param desktop the number of the desktop

       @return the size of the work area
    **/
    NETRect workArea(int desktop) const;

    /**
       Returns the name for the specified desktop.

       @param desktop the number of the desktop

       @return the name of the desktop
    **/
    const char *desktopName(int desktop) const;

    /**
       Returns an array of Window id's, which contain the virtual root windows.

       @return the array of Window id's

       @see virtualRootsCount()
    **/
    const Window *virtualRoots( ) const;

    /**
       Returns the number of window in the virtualRoots array.

       @return the number of Window id's in the virtual root array

       @see virtualRoots()
    **/
    int virtualRootsCount() const;

    /**
       Returns the desktop layout orientation.
    **/
    NET::Orientation desktopLayoutOrientation() const;

    /**
       Returns the desktop layout number of columns and rows. Note that
       either may be 0 (see _NET_DESKTOP_LAYOUT).
    **/
    QSize desktopLayoutColumnsRows() const;

    /**
       Returns the desktop layout starting corner.
    **/
    NET::DesktopLayoutCorner desktopLayoutCorner() const;

    /**
       Returns the number of desktops.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. They are however mapped to virtual desktops
       if needed.
       
       @param ignore_viewport if false, viewport is mapped to virtual desktops

       @return the number of desktops
    **/
    int numberOfDesktops( bool ignore_viewport = false ) const;

    /**
       Returns the current desktop.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. They are however mapped to virtual desktops
       if needed.
       
       @param ignore_viewport if false, viewport is mapped to virtual desktops

       @return the number of the current desktop
    **/
    int currentDesktop( bool ignore_viewport = false ) const;

    /**
       Returns the active (focused) window.

       @return the id of the active window
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
    void setClientList(const Window *windows, unsigned int count);

    /**
       Sets the list of managed windows in stacking order on the Root/Desktop
       window.

       @param windows The array of Window id's

       @param count The number of windows in the array.
    **/
    void setClientListStacking(const Window *windows, unsigned int count);

    /**
       Sets the current desktop to the specified desktop.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. It is however mapped to virtual desktops
       if needed.
       
       @param desktop the number of the desktop
       @param ignore_viewport if false, viewport is mapped to virtual desktops
    **/
    void setCurrentDesktop(int desktop, bool ignore_viewport = false);

    /**
       Sets the desktop geometry to the specified geometry.

       The desktop argument is ignored. Early drafts of the NET WM
       Specification were unclear about the semantics of this property.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. You should use calls for virtual desktops,
       viewport is mapped to them if needed.

       @param desktop the number of the desktop

       @param geometry the new size of the desktop
    **/
    void setDesktopGeometry(int desktop, const NETSize &geometry);

    /**
       Sets the viewport for the current desktop to the specified point.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. You should use calls for virtual desktops,
       viewport is mapped to them if needed.

       @param desktop the number of the desktop

       @param viewport the new position of the desktop's viewport
    **/
    void setDesktopViewport(int desktop, const NETPoint &viewport);

    /**
       Sets the number of desktops to the specified number.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. Viewport is mapped to virtual desktops
       if needed, but not for this call.

       @param numberOfDesktops the number of desktops
    **/
    void setNumberOfDesktops(int numberOfDesktops);

    /**
       Sets the name of the specified desktop.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. Viewport is mapped to virtual desktops
       if needed, but not for this call.

       @param desktop the number of the desktop

       @param desktopName the new name of the desktop
    **/
    void setDesktopName(int desktop, const char *desktopName);

    /**
       Requests that the specified window becomes the active (focused) one.

       @param window the id of the new active window
       @param src whether the request comes from normal application
          or from a pager or similar tool
       @param timestamp X server timestamp of the user action that
          caused the request
       @param active_window active window of the requesting application, if any
    **/
    void setActiveWindow(Window window, NET::RequestSource src,
        Time timestamp, Window active_window);

    /**
       Sets the active (focused) window the specified window. This should
       be used only in the window manager mode.

       @param window the if of the new active window
    **/
    void setActiveWindow(Window window);

    /**
       Sets the workarea for the specified desktop

       @param desktop the number of the desktop

       @param workArea the new work area of the desktop
    **/
    void setWorkArea(int desktop, const NETRect &workArea);

    /**
       Sets the list of virtual root windows on the root window.

       @param windows The array of Window id's

       @param count The number of windows in the array.
    **/
    void setVirtualRoots(const Window *windows, unsigned int count);

    /**
       Sets the desktop layout. This is set by the pager. When setting, the pager must
       own the _NET_DESKTOP_LAYOUT_Sn manager selection. See _NET_DESKTOP_LAYOUT for details.
    **/
    void setDesktopLayout(NET::Orientation orientation, int columns, int rows,
        NET::DesktopLayoutCorner corner);

    /**
     * Sets the _NET_SHOWING_DESKTOP status (whether desktop is being shown).
     */
    void setShowingDesktop( bool showing );
    /**
     * Returns the status of _NET_SHOWING_DESKTOP.
     */
    bool showingDesktop() const;

    /**
       Assignment operator.  Ensures that the shared data reference counts are
       correct.
    **/
    const NETRootInfo &operator=(const NETRootInfo &rootinfo);

    /**
       Clients (such as pagers/taskbars) that wish to close a window should call
       this function.  This will send a request to the Window Manager, which
       usually can usually decide how to react to such requests.

       @param window the id of the window to close
    **/
    void closeWindowRequest(Window window);

    /**
       Clients (such as pagers/taskbars) that wish to start a WMMoveResize
       (where the window manager controls the resize/movement,
       i.e. _NET_WM_MOVERESIZE) should call this function.
       This will send a request to the Window Manager.

       @param window The client window that would be resized/moved.

       @param x_root X position of the cursor relative to the root window.

       @param y_root Y position of the cursor relative to the root window.

       @param direction One of NET::Direction (see base class documentation for
       a description of the different directions).
    **/
    void moveResizeRequest(Window window, int x_root, int y_root,
			   Direction direction);

    /**
       Clients (such as pagers/taskbars) that wish to move/resize a window
       using WM2MoveResizeWindow (_NET_MOVERESIZE_WINDOW) should call this function.
       This will send a request to the Window Manager. See _NET_MOVERESIZE_WINDOW
       description for details.

       @param window The client window that would be resized/moved.
       @param flags Flags specifying the operation (see _NET_MOVERESIZE_WINDOW description)
       @param x Requested X position for the window
       @param y Requested Y position for the window
       @param width Requested width for the window
       @param height Requested height for the window
    **/
    void moveResizeWindowRequest(Window window, int flags, int x, int y, int width, int height );

    /**
       Sends the _NET_RESTACK_WINDOW request.
    **/
    void restackRequest(Window window, RequestSource source, Window above, int detail, Time timestamp);

    /**
      Sends a ping with the given timestamp to the window, using
      the _NET_WM_PING protocol.
    */
    void sendPing( Window window, Time timestamp );

    /**
       Sends a take activity message with the given timestamp to the window, using
       the _NET_WM_TAKE_ACTIVITY protocol (see the WM spec for details).
       @param window the window to which the message should be sent
       @param timestamp timestamp of the message
       @param flags arbitrary flags
    */
    void takeActivity( Window window, Time timestamp, long flags );

    /**
       This function takes the passed XEvent and returns an OR'ed list of
       NETRootInfo properties that have changed in the properties argument.
       The new information will be read immediately by the class.
       The elements of the properties argument are as they would be passed
       to the constructor, if the array is not large enough,
       changed properties that don't fit in it won't be listed there
       (they'll be updated in the class though).

       @param event the event
       @param properties properties that changed
       @param properties_size size of the passed properties array

    **/
    void event( XEvent* event, unsigned long* properties, int properties_size );

    /**
       This function takes the passed XEvent and returns an OR'ed list of
       NETRootInfo properties that have changed.  The new information will be
       read immediately by the class. This overloaded version returns
       only a single mask, and therefore cannot check state of all properties
       like the other variant.

       @param event the event

       @return the properties
    **/
    unsigned long event(XEvent *event);


protected:
    /**
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a window has been added.

       @param window the id of the window to add
    **/
    virtual void addClient(Window window) { Q_UNUSED(window); }

    /**
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a window has been removed.

       @param window the id of the window to remove
    **/
    virtual void removeClient(Window window) { Q_UNUSED(window); }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the number
       of desktops.

       @param numberOfDesktops the new number of desktops
    **/
    virtual void changeNumberOfDesktops(int numberOfDesktops) { Q_UNUSED(numberOfDesktops); }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the specified
       desktop geometry.

       @param desktop the number of the desktop

       @param geom the new size
    **/
    virtual void changeDesktopGeometry(int desktop, const NETSize &geom) { Q_UNUSED(desktop); Q_UNUSED(geom); }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the specified
       desktop viewport.

       @param desktop the number of the desktop

       @param viewport the new position of the viewport
    **/
    virtual void changeDesktopViewport(int desktop, const NETPoint &viewport) { Q_UNUSED(desktop); Q_UNUSED(viewport); }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the current
       desktop.

       @param desktop the number of the desktop
    **/
    virtual void changeCurrentDesktop(int desktop) { Q_UNUSED(desktop); }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to close a window.

       @param window the id of the window to close
    **/
    virtual void closeWindow(Window window) { Q_UNUSED(window); }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to start a move/resize.

       @param window The window that wants to move/resize

       @param x_root X position of the cursor relative to the root window.

       @param y_root Y position of the cursor relative to the root window.

       @param direction One of NET::Direction (see base class documentation for
       a description of the different directions).
    **/
    virtual void moveResize(Window window, int x_root, int y_root,
    			    unsigned long direction) { Q_UNUSED(window); Q_UNUSED(x_root); Q_UNUSED(y_root); Q_UNUSED(direction); }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to receive replies to the _NET_WM_PING protocol.
       @param window the window from which the reply came
       @param timestamp timestamp of the ping
     */
    virtual void gotPing( Window window, Time timestamp ) { Q_UNUSED(window); Q_UNUSED(timestamp); }
    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the active
       (focused) window.

       @param window the id of the window to activate
       @param src the source from which the request came
       @param timestamp the timestamp of the user action causing this request
       @param active_window active window of the requesting application, if any
    **/
    virtual void changeActiveWindow(Window window,NET::RequestSource src,
        Time timestamp, Window active_window ) { Q_UNUSED(window); Q_UNUSED(src); Q_UNUSED(timestamp); Q_UNUSED(active_window);}

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a pager made a request to move/resize a window.
       See _NET_MOVERESIZE_WINDOW for details.

       @param window the id of the window to more/resize
       @param flags Flags specifying the operation (see _NET_MOVERESIZE_WINDOW description)
       @param x Requested X position for the window
       @param y Requested Y position for the window
       @param width Requested width for the window
       @param height Requested height for the window
    **/
    virtual void moveResizeWindow(Window window, int flags, int x, int y, int width, int height) { Q_UNUSED(window); Q_UNUSED(flags); Q_UNUSED(x); Q_UNUSED(y); Q_UNUSED(width); Q_UNUSED(height); }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to restack a window.
       See _NET_RESTACK_WINDOW for details.

       @param window the id of the window to restack
       @param source the source of the request
       @param above other window in the restack request
       @param detail restack detail
       @param timestamp the timestamp of the request
    **/
    virtual void restackWindow(Window window, RequestSource source,
           Window above, int detail, Time timestamp) { Q_UNUSED(window); Q_UNUSED(source); Q_UNUSED(above); Q_UNUSED(detail); Q_UNUSED(timestamp); }
    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to receive replies to the _NET_WM_TAKE_ACTIVITY protocol.
       @param window the window from which the reply came
       @param timestamp timestamp of the ping
       @param flags flags passed in the original message
     */
    virtual void gotTakeActivity(Window window, Time timestamp, long flags ) { Q_UNUSED(window); Q_UNUSED(timestamp); Q_UNUSED(flags); }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a pager made a request to change showing the desktop.
       See _NET_SHOWING_DESKTOP for details.

       @param showing whether to activate the showing desktop mode
    **/
    virtual void changeShowingDesktop(bool showing) { Q_UNUSED(showing); }

private:
    void update( const unsigned long[] );
    void setSupported();
    void setDefaultProperties();
    void updateSupportedProperties( Atom atom );

protected:
    /** Virtual hook, used to add new "virtual" functions while maintaining
	binary compatibility. Unused in this class.
    */
    virtual void virtual_hook( int id, void* data );
private:
    NETRootInfoPrivate *p; // krazy:exclude=dpointer (implicitly shared)
};

/**
   Common API for application window properties/protocols.

   The NETWinInfo class provides a common API for clients and window managers to
   set/read/change properties on an application window as defined by the NET
   Window Manager Specification.

   @author Bradley T. Hughes <bhughes@trolltech.com>
   @see NET
   @see NETRootInfo
   @see http://www.freedesktop.org/standards/wm-spec/
 **/

class KDEUI_EXPORT NETWinInfo : public NET {
public:
    /**
        Indexes for the properties array.
    **/
    // update also NETWinInfoPrivate::properties[] size when extending this
    enum { PROTOCOLS, PROTOCOLS2,
        PROPERTIES_SIZE };
    /**
       Create a NETWinInfo object, which will be used to set/read/change
       information stored on an application window.

       @param display An X11 Display struct.

       @param window The Window id of the application window.

       @param rootWindow The Window id of the root window.

       @param properties An array of elements listing all properties the client
       is interested in.The elements contain OR'ed values of constants
       from the NET base class, in the following order: [0]= NET::Property,
       [1]= NET::Property2.
       In future versions, the list may be extended. In case you pass less elements,
       the missing ones will be replaced with default values.

       @param properties_size The number of elements in the properties array.

       @param role Select the application role.  If this argument is omitted,
       the role will default to Client.
    **/
    NETWinInfo(Display *display, Window window, Window rootWindow,
               const unsigned long properties[], int properties_size,
	       Role role = Client);

    /**
        This constructor differs from the above one only in the way it accepts
        the list of properties the client is interested in. The properties argument
        is equivalent to the first element of the properties array
        in the above constructor.
    **/
    NETWinInfo(Display *display, Window window,
	       Window rootWindow, unsigned long properties,
	       Role role = Client);

    /**
       Creates a shared copy of the specified NETWinInfo object.

       @param wininfo the NETWinInfo to copy
    **/
    NETWinInfo(const NETWinInfo & wininfo);

    /**
       Destroys the NETWinInfo object.
    **/
    virtual ~NETWinInfo();

    /**
       Assignment operator.  Ensures that the shared data reference counts are
       correct.
    **/
    const NETWinInfo &operator=(const NETWinInfo &wintinfo);

    /**
       Returns true if the window has any window type set, even if the type
       itself is not known to this implementation. Presence of a window type
       as specified by the NETWM spec is considered as the window supporting
       this specification.
       @return true if the window has support for the NETWM spec
    **/
    bool hasNETSupport() const;

    /**
       Returns the properties argument passed to the constructor.
       The size is the maximum array size the constructor accepts.
    **/
    const unsigned long* passedProperties() const;

    /**
       Returns the icon geometry.

       @return the geometry of the icon
    **/
    NETRect iconGeometry() const;

    /**
       Returns the state of the window (see the NET base class documentation for a
       description of the various states).

       @return the state of the window
    **/
    unsigned long state() const;

    /**
       Returns the extended (partial) strut specified by this client.
       See _NET_WM_STRUT_PARTIAL in the spec.
    **/
    NETExtendedStrut extendedStrut() const;

    /**
       @deprecated use strutPartial()
       Returns the strut specified by this client.

       @return the strut of the window
    **/
    NETStrut strut() const;

    /**
       Returns the window type for this client (see the NET base class
       documentation for a description of the various window types).
       Since clients may specify several windows types for a window
       in order to support backwards compatibility and extensions
       not available in the NETWM spec, you should specify all
       window types you application supports (see the NET::WindowTypeMask
       mask values for various window types). This method will
       return the first window type that is listed in the supported types,
       or NET::Unknown if none of the window types is supported.

       @return the type of the window
    **/
    WindowType windowType( unsigned long supported_types ) const;

    /**
      This function returns false if the window has not window type
      specified at all. Used by KWindowInfo::windowType() to return either
      NET::Normal or NET::Dialog as appropriate as a fallback.
    **/
    bool hasWindowType() const;

    /**
       Returns the name of the window in UTF-8 format.

       @return the name of the window
    **/
    const char *name() const;

    /**
       Returns the visible name as set by the window manager in UTF-8 format.

       @return the visible name of the window
    **/
    const char *visibleName() const;

    /**
       Returns the iconic name of the window in UTF-8 format. Note that this has
       nothing to do with icons, but it's for "iconic"
       representations of the window (taskbars etc.), that should be shown
       when the window is in iconic state. See description of _NET_WM_ICON_NAME
       for details.

       @return the iconic name
    **/
    const char *iconName() const;

    /**
       Returns the visible iconic name as set by the window manager in UTF-8 format.
       Note that this has nothing to do with icons, but it's for "iconic"
       representations of the window (taskbars etc.), that should be shown
       when the window is in iconic state. See description of _NET_WM_VISIBLE_ICON_NAME
       for details.

       @return the visible iconic name
    **/
    const char *visibleIconName() const;

    /**
       Returns the desktop where the window is residing.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. It is however mapped to virtual desktops
       if needed.
       
       @param ignore_viewport if false, viewport is mapped to virtual desktops

       @return the number of the window's desktop

       @see OnAllDesktops()
    **/
    int desktop( bool ignore_viewport = false ) const;

    /**
       Returns the process id for the client window.

       @return the process id of the window
    **/
    int pid() const;

    /**
       Returns whether or not this client handles icons.

       @return true if this client handles icons, false otherwise
    **/
    Bool handledIcons() const;

    /**
       Returns the mapping state for the window (see the NET base class
       documentation for a description of mapping state).

       @return the mapping state
    **/
    MappingState mappingState() const;

    /**
       Set icons for the application window.  If replace is True, then
       the specified icon is defined to be the only icon.  If replace is False,
       then the specified icon is added to a list of icons.

       @param icon the new icon

       @param replace true to replace, false to append to the list of icons
    **/
    void setIcon(NETIcon icon, Bool replace = True);

    /**
       Set the icon geometry for the application window.

       @param geometry the new icon geometry
    **/
    void setIconGeometry(NETRect geometry);

    /**
       Set the extended (partial) strut for the application window.

       @param extended_strut the new strut
    **/
    void setExtendedStrut(const NETExtendedStrut& extended_strut );

    /**
       @deprecated use setExtendedStrut()
       Set the strut for the application window.

       @param strut the new strut
    **/
    void setStrut(NETStrut strut);

    /**
       Set the state for the application window (see the NET base class documentation
       for a description of window state).

       @param state the name state

       @param mask the mask for the state
    **/
    void setState(unsigned long state, unsigned long mask);

    /**
       Sets the window type for this client (see the NET base class
       documentation for a description of the various window types).

       @param type the window type
    **/
    void setWindowType(WindowType type);

    /**
       Sets the name for the application window.

       @param name the new name of the window
    **/
    void setName(const char *name);

    /**
       For Window Managers only:  set the visible name ( i.e. xterm, xterm <2>,
       xterm <3>, ... )

       @param visibleName the new visible name
    **/
    void setVisibleName(const char *visibleName);

    /**
       Sets the iconic name for the application window.

       @param name the new iconic name
    **/
    void setIconName(const char *name);

    /**
       For Window Managers only: set the visible iconic name ( i.e. xterm, xterm <2>,
       xterm <3>, ... )

       @param name the new visible iconic name
    **/
    void setVisibleIconName(const char *name);

    /**
       Set which window the desktop is (should be) on.

       NOTE: KDE uses virtual desktops and does not directly support
       viewport in any way. It is however mapped to virtual desktops
       if needed.
       
       @param desktop the number of the new desktop
       @param ignore_viewport if false, viewport is mapped to virtual desktops

       @see OnAllDesktops()
    **/
    void setDesktop(int desktop, bool ignore_viewport = false);

    /**
       Set the application window's process id.

       @param pid the window's process id
    **/
    void setPid(int pid);

    /**
       Set whether this application window handles icons.

       @param handled true if the window handles icons, false otherwise
    **/
    void setHandledIcons(Bool handled);

    /**
       Set the frame decoration strut, i.e. the width of the decoration borders.

       @param strut the new strut
    **/
    void setFrameExtents(NETStrut strut);

    /**
       Returns the frame decoration strut, i.e. the width of the decoration borders.

       @since 4.3
    **/
    NETStrut frameExtents() const;

    /**
       Sets the window frame overlap strut, i.e. how far the window frame extends
       behind the client area on each side.

       Set the strut values to -1 if you want the window frame to cover the whole
       client area.

       The default values are 0.

       @since 4.4
    **/
    void setFrameOverlap(NETStrut strut);

    /**
       Returns the frame overlap strut, i.e. how far the window frame extends
       behind the client area on each side.

       @since 4.4
    **/ 
    NETStrut frameOverlap() const;

    /**
       Returns an icon.  If width and height are passed, the icon returned will be
       the closest it can find (the next biggest).  If width and height are omitted,
       then the largest icon in the list is returned.

       @param width the preferred width for the icon, -1 to ignore

       @param height the preferred height for the icon, -1 to ignore

       @return the icon
    **/
    NETIcon icon(int width = -1, int height = -1) const;
    
    /**
      Returns a list of provided icon sizes. Each size is pair width,height, terminated
      with pair 0,0.
      @since 4.3
    **/
    const int* iconSizes() const;

    /**
     * Sets user timestamp @p time on the window (property _NET_WM_USER_TIME).
     * The timestamp is expressed as XServer time. If a window
     * is shown with user timestamp older than the time of the last
     * user action, it won't be activated after being shown, with the special
     * value 0 meaning not to activate the window after being shown.
     */
    void setUserTime( Time time );

    /**
     * Returns the time of last user action on the window, or -1 if not set.
     */
    Time userTime() const;

    /**
     * Sets the startup notification id @p id on the window.
     */
    void setStartupId( const char* startup_id );

    /**
     * Returns the startup notification id of the window.
     */
    const char* startupId() const;

    /**
     * Sets opacity (0 = transparent, 0xffffffff = opaque ) on the window.
     */
    void setOpacity( unsigned long opacity );

    /**
     * Returns the opacity of the window.
     */
    unsigned long opacity() const;

    /**
     * Sets actions that the window manager allows for the window.
     */
    void setAllowedActions( unsigned long actions );

    /**
     * Returns actions that the window manager allows for the window.
     */
    unsigned long allowedActions() const;

    /**
     * Returns the WM_TRANSIENT_FOR property for the window, i.e. the mainwindow
     * for this window.
     */
    Window transientFor() const;

    /**
     * Returns the leader window for the group the window is in, if any.
     */
    Window groupLeader() const;

    /**
     * Returns the class component of the window class for the window
     * (i.e. WM_CLASS property).
     */
    const char* windowClassClass() const;

    /**
     * Returns the name component of the window class for the window
     * (i.e. WM_CLASS property).
     */
    const char* windowClassName() const;

    /**
     * Returns the window role for the window (i.e. WM_WINDOW_ROLE property).
     */
    const char* windowRole() const;

    /**
     * Returns the client machine for the window (i.e. WM_CLIENT_MACHINE property).
     */
    const char* clientMachine() const;

    /**
       Places the window frame geometry in frame, and the application window
       geometry in window.  Both geometries are relative to the root window.

       @param frame the geometry for the frame

       @param window the geometry for the window
    **/
    void kdeGeometry(NETRect &frame, NETRect &window);

    /**
       This function takes the passed XEvent and returns an OR'ed list of
       NETWinInfo properties that have changed in the properties argument.
       The new information will be read immediately by the class.
       The elements of the properties argument are as they would be passed
       to the constructor, if the array is not large enough,
       changed properties that don't fit in it won't be listed there
       (they'll be updated in the class though).

       @param event the event
       @param properties properties that changed
       @param properties_size size of the passed properties array
    **/
    void event( XEvent* event, unsigned long* properties, int properties_size );

    /**
       This function takes the pass XEvent and returns an OR'ed list of NETWinInfo
       properties that have changed.  The new information will be read
       immediately by the class. This overloaded version returns
       only a single mask, and therefore cannot check state of all properties
       like the other variant.

       @param event the event

       @return the properties
    **/
    unsigned long event(XEvent *event);

    /**
       Sentinel value to indicate that the client wishes to be visible on
       all desktops.

       @return the value to be on all desktops
    **/
    static const int OnAllDesktops;

protected:
    /**
       A Window Manager should subclass NETWinInfo and reimplement this function when
       it wants to know when a Client made a request to change desktops (ie. move to
       another desktop).

       @param desktop the number of the desktop
    **/
    virtual void changeDesktop(int desktop) { Q_UNUSED(desktop); }

    /**
       A Window Manager should subclass NETWinInfo and reimplement this function when
       it wants to know when a Client made a request to change state (ie. to
       Shade / Unshade).

       @param state the new state

       @param mask the mask for the state
    **/
    virtual void changeState(unsigned long state, unsigned long mask) { Q_UNUSED(state); Q_UNUSED(mask); }

private:
    void update( const unsigned long[] );
    void updateWMState();
    void setIconInternal(NETRArray<NETIcon>& icons, int& icon_count, Atom property, NETIcon icon, Bool replace);
    NETIcon iconInternal(NETRArray<NETIcon>& icons, int icon_count, int width, int height) const;

protected:
    /** Virtual hook, used to add new "virtual" functions while maintaining
	binary compatibility. Unused in this class.
    */
    virtual void virtual_hook( int id, void* data );
private:
    NETWinInfoPrivate *p; // krazy:exclude=dpointer (implicitly shared)
    friend class NETWinInfo2;
};


/**
 This class is an extension of the NETWinInfo class, and exists solely
 for binary compatibility reasons (adds new virtual methods) until KDE5. Simply
 use it instead of NETWinInfo and override also the added virtual methods.
 @since 4.2
*/
class KDEUI_EXPORT NETWinInfo2 : public NETWinInfo {
public:
    NETWinInfo2(Display *display, Window window, Window rootWindow,
                const unsigned long properties[], int properties_size,
                Role role = Client);

    NETWinInfo2(Display *display, Window window,
                Window rootWindow, unsigned long properties,
                Role role = Client);

    /**
       Sets the desired multiple-monitor topology (4 monitor indices indicating
       the top, bottom, left, and right edges of the window) when the fullscreen
       state is enabled. The indices are from the set returned by the Xinerama
       extension.
       See _NET_WM_FULLSCREEN_MONITORS for details.

       @param topology A struct that models the desired monitor topology, namely:
       top is the monitor whose top edge defines the top edge of the
       fullscreen window, bottom is the monitor whose bottom edge defines
       the bottom edge of the fullscreen window, left is the monitor whose
       left edge defines the left edge of the fullscreen window, and right
       is the monitor whose right edge defines the right edge of the fullscreen
       window.

    **/
    void setFullscreenMonitors(NETFullscreenMonitors topology);

    /**
       Returns the desired fullscreen monitor topology for this client, should
       it be in fullscreen state.
       See _NET_WM_FULLSCREEN_MONITORS in the spec.
    **/
    NETFullscreenMonitors fullscreenMonitors() const;

protected:
    friend class NETWinInfo;
    /**
       A Window Manager should subclass NETWinInfo2 and reimplement this function
       when it wants to know when a Client made a request to change the
       fullscreen monitor topology for its fullscreen state.

       @param topology A structure (top, bottom, left, right) representing the
       fullscreen monitor topology.
    **/
    virtual void changeFullscreenMonitors(NETFullscreenMonitors topology) { Q_UNUSED(topology); }
};

//#define KWIN_FOCUS

#endif
#endif // netwm_h
