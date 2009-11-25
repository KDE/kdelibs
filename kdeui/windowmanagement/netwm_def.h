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

#ifndef   netwm_def_h
#define   netwm_def_h
#include <kdeui_export.h>

/**
  Simple point class for NET classes.

  This class is a convenience class defining a point x, y.  The existence of
  this class is to keep the implementation from being dependent on a
  separate framework/library.

  NETPoint is only used by the NET API. Usually QPoint is the 
  appropriate class for representing a point.

  @author Bradley T. Hughes <bhughes@trolltech.com>
**/

struct NETPoint {
    /**
       Constructor to initialize this point to 0,0.
    **/
    NETPoint() : x(0), y(0) { }

    /*
       Public data member.
    **/
    int x, ///< x coordinate.
      y;   ///< y coordinate
};


/**
  Simple size class for NET classes.

  This class is a convenience class defining a size width by height.  The
  existence of this class is to keep the implementation from being dependent
  on a separate framework/library.

  NETSize is only used by the NET API. Usually QSize is the 
  appropriate class for representing a size.

  @author Bradley T. Hughes <bhughes@trolltech.com>
**/

struct NETSize {
    /**
       Constructor to initialize this size to 0x0
    **/
    NETSize() : width(0), height(0) { }

    /*
       Public data member.
    **/
    int width,  ///< Width.
      height;   ///< Height.
};

/**
   Simple rectangle class for NET classes.

   This class is a convenience class defining a rectangle as a point x,y with a
   size width by height.  The existence of this class is to keep the implementation
   from being dependent on a separate framework/library;

   NETRect is only used by the NET API. Usually QRect is the 
   appropriate class for representing a rectangle.
**/
struct NETRect {
    /**
       Position of the rectangle.

       @see NETPoint
    **/
    NETPoint pos;

    /**
       Size of the rectangle.

       @see NETSize
    **/
    NETSize size;
};


/**
   Simple icon class for NET classes.

   This class is a convenience class defining an icon of size width by height.
   The existence of this class is to keep the implementation from being
   dependent on a separate framework/library.

   NETIcon is only used by the NET API. Usually QIcon is the 
   appropriate class for representing an icon.
**/

struct NETIcon {
    /**
       Constructor to initialize this icon to 0x0 with data=0
    **/
    NETIcon() : data(0) { }

    /**
       Size of the icon.

       @see NETSize
    **/
    NETSize size;

    /**
       Image data for the icon.  This is an array of 32bit packed CARDINAL ARGB
       with high byte being A, low byte being B. First two bytes are width, height.
       Data is in rows, left to right and top to bottom.
    **/
    unsigned char *data;
};


/**
   Partial strut class for NET classes.

   This class is a convenience class defining a strut with left, right, top and
   bottom border values, and ranges for them.  The existence of this class is to
   keep the implementation from being dependent on a separate framework/library.
   See the _NET_WM_STRUT_PARTIAL property in the NETWM spec.
**/

struct NETExtendedStrut {
    /**
       Constructor to initialize this struct to 0,0,0,0
    **/
    NETExtendedStrut() : left_width(0), left_start(0), left_end(0),
        right_width(0), right_start(0), right_end(0), top_width(0), top_start(0), top_end(0),
        bottom_width(0), bottom_start(0), bottom_end(0) {}

    /**
       Left border of the strut, width and range.
           **/
    int left_width, left_start, left_end;

    /**
       Right border of the strut, width and range.
    **/
    int right_width, right_start, right_end;

    /**
       Top border of the strut, width and range.
           **/
    int top_width, top_start, top_end;

    /**
       Bottom border of the strut, width and range.
           **/
    int bottom_width, bottom_start, bottom_end;
    
};


/**
   @deprecated use NETExtendedStrut

   Simple strut class for NET classes.

   This class is a convenience class defining a strut with left, right, top and
   bottom border values.  The existence of this class is to keep the implementation
   from being dependent on a separate framework/library. See the _NET_WM_STRUT
   property in the NETWM spec.
**/

struct NETStrut {
    /**
       Constructor to initialize this struct to 0,0,0,0
    **/
    NETStrut() : left(0), right(0), top(0), bottom(0) { }

    /**
       Left border of the strut.
           **/
    int left;

    /**
       Right border of the strut.
    **/
    int right;

    /**
       Top border of the strut.
           **/
    int top;

    /**
       Bottom border of the strut.
           **/
    int bottom;
};


/**
   Simple multiple monitor topology class for NET classes.

   This class is a convenience class, defining a multiple monitor topology
   for fullscreen applications that wish to be present on more than one
   monitor/head. As per the _NET_WM_FULLSCREEN_MONITORS hint in the EWMH spec,
   this topology consists of 4 monitor indices such that the bounding rectangle
   is defined by the top edge of the top monitor, the bottom edge of the bottom
   monitor, the left edge of the left monitor, and the right edge of the right
   monitor. See the _NET_WM_FULLSCREEN_MONITORS hint in the EWMH spec.
**/

struct NETFullscreenMonitors {
    /**
       Constructor to initialize this struct to -1,0,0,0 (an initialized,
       albeit invalid, topology).
    **/
    NETFullscreenMonitors() : top(-1), bottom(0), left(0), right(0) { }

    /**
       Monitor index whose top border defines the top edge of the topology.
    **/
    int top;

    /**
       Monitor index whose bottom border defines the bottom edge of the topology.
    **/
    int bottom;

    /**
       Monitor index whose left border defines the left edge of the topology.
    **/
    int left;

    /**
       Monitor index whose right border defines the right edge of the topology.
    **/
    int right;

    /**
       Convenience check to make sure that we are not holding the initial (invalid)
       values. Note that we don't want to call this isValid() because we're not
       actually validating the monitor topology here, but merely that our initial
       values were overwritten at some point by real (non-negative) monitor indices.
    **/
    bool isSet() const { return (top != -1); };
};


/**
  Base namespace class.

  The NET API is an implementation of the NET Window Manager Specification.

  This class is the base class for the NETRootInfo and NETWinInfo classes, which
  are used to retrieve and modify the properties of windows. To keep
  the namespace relatively clean, all enums are defined here.
  
  @see http://www.freedesktop.org/standards/wm-spec/
 **/

class KDEUI_EXPORT NET {
public:
    /**
       Application role.  This is used internally to determine how several action
       should be performed (if at all).
    **/

    enum Role {
	/**
	   indicates that the application is a client application.
	**/
	Client,
	/**
	   indicates that the application is a window manager application.
	**/
	WindowManager
    };

    /**
       Window type.
    **/

    enum WindowType {
	/**
	   indicates that the window did not define a window type.
	**/
	Unknown  = -1,
	/**
	   indicates that this is a normal, top-level window
	**/
	Normal   = 0,
	/**
	   indicates a desktop feature. This can include a single window
	   containing desktop icons with the same dimensions as the screen, allowing
	   the desktop environment to have full control of the desktop, without the
	   need for proxying root window clicks.
	**/
	Desktop  = 1,
	/**
	   indicates a dock or panel feature
	**/
	Dock     = 2,
	/**
	   indicates a toolbar window
	**/
	Toolbar  = 3,
	/**
	   indicates a pinnable (torn-off) menu window
	**/
	Menu     = 4,
	/**
	   indicates that this is a dialog window
	**/
	Dialog   = 5,
	/**
           @deprecated has unclear meaning and is KDE-only
	**/
	Override = 6, // NON STANDARD
	/**
	   indicates a toplevel menu (AKA macmenu). This is a KDE extension to the
	   _NET_WM_WINDOW_TYPE mechanism.
	**/
	TopMenu  = 7, // NON STANDARD
	/**
	   indicates a utility window
	**/
	Utility  = 8,
	/**
	   indicates that this window is a splash screen window.
	**/
	Splash   = 9,
        /**
           indicates a dropdown menu (from a menubar typically)
        **/
        DropdownMenu = 10,
        /**
           indicates a popup menu (a context menu typically)
        **/
        PopupMenu = 11,
        /**
           indicates a tooltip window
        **/
        Tooltip = 12,
        /**
           indicates a notification window
        **/
        Notification = 13,
        /**
           indicates that the window is a list for a combobox
        **/
        ComboBox = 14,
        /**
           indicates a window that represents the dragged object during DND operation
        **/
        DNDIcon = 15
    };
    
    /**
        Values for WindowType when they should be OR'ed together, e.g.
        for the properties argument of the NETRootInfo constructor.
    **/
    enum WindowTypeMask {
	NormalMask   = 1<<0,   ///< @see Normal
	DesktopMask  = 1<<1,   ///< @see Desktop
	DockMask     = 1<<2,   ///< @see Dock
	ToolbarMask  = 1<<3,   ///< @see Toolbar
	MenuMask     = 1<<4,   ///< @see Menu
	DialogMask   = 1<<5,   ///< @see Dialog
	OverrideMask = 1<<6,   ///< @see Override
	TopMenuMask  = 1<<7,   ///< @see TopMenu
	UtilityMask  = 1<<8,   ///< @see Utility
	SplashMask   = 1<<9,   ///< @see Splash
        DropdownMenuMask = 1<<10, ///< @see DropdownMenu
        PopupMenuMask    = 1<<11, ///< @see PopupMenu
        TooltipMask      = 1<<12, ///< @see Tooltip
        NotificationMask = 1<<13, ///< @see Notification
        ComboBoxMask     = 1<<14, ///< @see ComboBox
        DNDIconMask      = 1<<15, ///< @see DNDIcon
        AllTypesMask     = 0LU-1  ///< All window types.
    };

    /**
     * Returns true if the given window type matches the mask given
     * using WindowTypeMask flags.
     */    
    static bool typeMatchesMask( WindowType type, unsigned long mask );

    /**
       Window state.

       To set the state of a window, you'll typically do something like:
       \code
         KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager );
       \endcode

       for example to not show the window on the taskbar and desktop pager.
       winId() is a function of QWidget()

       Note that KeepAbove (StaysOnTop) and KeepBelow are meant as user preference and
       applications should avoid setting these states themselves.
    **/

    enum State {
	/**
	   indicates that this is a modal dialog box. The WM_TRANSIENT_FOR hint
	   MUST be set to indicate which window the dialog is a modal for, or set to
	   the root window if the dialog is a modal for its window group.
	**/
	Modal        = 1<<0,
	/**
	   indicates that the Window Manager SHOULD keep the window's position
	   fixed on the screen, even when the virtual desktop scrolls. Note that this is
	   different from being kept on all desktops.
	**/
	Sticky       = 1<<1,
	/**
	   indicates that the window is vertically maximized.
	**/
	MaxVert      = 1<<2,
	/**
	   indicates that the window is horizontally maximized.
	**/
	MaxHoriz     = 1<<3,
	/**
	   convenience value. Equal to MaxVert | MaxHoriz.
	**/
	Max = MaxVert | MaxHoriz,
	/**
	   indicates that the window is shaded (rolled-up).
	**/
	Shaded       = 1<<4,
	/**
	   indicates that a window should not be included on a taskbar.
	**/
	SkipTaskbar  = 1<<5,
	/**
	   indicates that a window should on top of most windows (but below fullscreen
	   windows).
	**/
	KeepAbove    = 1<<6,
	/**
	   @deprecated This is an obsolete name for KeepAbove.
	**/
	StaysOnTop   = KeepAbove,	// NOT STANDARD
	/**
	   indicates that a window should not be included on a pager.
	**/
	SkipPager    = 1<<7,
	/**
	   indicates that a window should not be visible on the screen (e.g. when minimised).
	   Only the window manager is allowed to change it.
	**/
	Hidden       = 1<<8,
	/**
	   indicates that a window should fill the entire screen and have no window
	   decorations.
	**/
	FullScreen   = 1<<9,
	/**
	   indicates that a window should be below most windows (but above any desktop windows).
	**/
	KeepBelow    = 1<<10,
	/**
	   there was an attempt to activate this window, but the window manager prevented
	   this. E.g. taskbar should mark such window specially to bring user's attention to
	   this window. Only the window manager is allowed to change it.
	**/
        DemandsAttention = 1<<11
    };

    /**
       Direction for WMMoveResize.

       When a client wants the Window Manager to start a WMMoveResize, it should
       specify one of:

       @li TopLeft
       @li Top
       @li TopRight
       @li Right
       @li BottomRight
       @li Bottom
       @li BottomLeft
       @li Left
       @li Move (for movement only)
       @li KeyboardSize (resizing via keyboard)
       @li KeyboardMove (movement via keyboard)
    **/

    enum Direction {
	TopLeft      = 0,
	Top          = 1,
	TopRight     = 2,
	Right        = 3,
	BottomRight  = 4,
	Bottom       = 5,
	BottomLeft   = 6,
	Left         = 7,
	Move         = 8,  // movement only
	KeyboardSize = 9,  // size via keyboard
	KeyboardMove = 10, // move via keyboard
	MoveResizeCancel = 11 // to ask the WM to stop moving a window
    };

    /**
       Client window mapping state.  The class automatically watches the mapping
       state of the client windows, and uses the mapping state to determine how
       to set/change different properties. Note that this is very lowlevel
       and you most probably don't want to use this state.
    **/
    enum MappingState {
	/**
	   indicates the client window is visible to the user.
	**/
	Visible = 1, //NormalState,
	/**
	   indicates that neither the client window nor its icon is visible.
	**/
	Withdrawn = 0, //WithdrawnState,
	/**
	   indicates that the client window is not visible, but its icon is.
	   This can be when the window is minimized or when it's on a
	   different virtual desktop. See also NET::Hidden.
	**/
	Iconic = 3 // IconicState
    };

    /**
      Actions that can be done with a window (_NET_WM_ALLOWED_ACTIONS).
    **/
    enum Action {
        ActionMove           = 1<<0,
        ActionResize         = 1<<1,
        ActionMinimize       = 1<<2,
        ActionShade          = 1<<3,
        ActionStick          = 1<<4,
        ActionMaxVert        = 1<<5,
        ActionMaxHoriz       = 1<<6,
        ActionMax            = ActionMaxVert | ActionMaxHoriz,
        ActionFullScreen     = 1<<7,
        ActionChangeDesktop  = 1<<8,
        ActionClose          = 1<<9
    };
    
    /**
       Supported properties.  Clients and Window Managers must define which
       properties/protocols it wants to support.

       Root/Desktop window properties and protocols:

       @li Supported
       @li ClientList
       @li ClientListStacking
       @li NumberOfDesktops
       @li DesktopGeometry
       @li DesktopViewport
       @li CurrentDesktop
       @li DesktopNames
       @li ActiveWindow
       @li WorkArea
       @li SupportingWMCheck
       @li VirtualRoots
       @li CloseWindow
       @li WMMoveResize

       Client window properties and protocols:

       @li WMName
       @li WMVisibleName
       @li WMDesktop
       @li WMWindowType
       @li WMState
       @li WMStrut  (obsoleted by WM2ExtendedStrut)
       @li WMGeometry
       @li WMFrameExtents
       @li WMIconGeometry
       @li WMIcon
       @li WMIconName
       @li WMVisibleIconName
       @li WMHandledIcons
       @li WMPid
       @li WMPing

       ICCCM properties (provided for convenience):

       @li XAWMState

    **/

    enum Property {
	// root
	Supported             = 1<<0,
	ClientList            = 1<<1,
	ClientListStacking    = 1<<2,
	NumberOfDesktops      = 1<<3,
	DesktopGeometry       = 1<<4,
	DesktopViewport       = 1<<5,
	CurrentDesktop        = 1<<6,
	DesktopNames          = 1<<7,
	ActiveWindow          = 1<<8,
	WorkArea              = 1<<9,
	SupportingWMCheck     = 1<<10,
	VirtualRoots          = 1<<11,
        //
	CloseWindow           = 1<<13,
	WMMoveResize          = 1<<14,

	// window
	WMName                = 1<<15,
	WMVisibleName         = 1<<16,
	WMDesktop             = 1<<17,
	WMWindowType          = 1<<18,
	WMState               = 1<<19,
	WMStrut               = 1<<20,
	WMIconGeometry        = 1<<21,
	WMIcon                = 1<<22,
	WMPid                 = 1<<23,
	WMHandledIcons        = 1<<24,
	WMPing                = 1<<25,
	XAWMState             = 1<<27,
        WMFrameExtents        = 1<<28,

	// Need to be reordered
	WMIconName            = 1<<29,
	WMVisibleIconName     = 1<<30,
	WMGeometry	      = 1<<31
    };
    
    /**
        Supported properties. This enum is an extension to NET::Property,
        because them enum is limited only to 32 bits.

        Client window properties and protocols:

        @li WM2UserTime
        @li WM2StartupId
        @li WM2TransientFor mainwindow for the window (WM_TRANSIENT_FOR)
        @li WM2GroupLeader  group leader (window_group in WM_HINTS)
        @li WM2AllowedActions
        @li WM2RestackWindow
        @li WM2MoveResizeWindow
        @li WM2ExtendedStrut
        @li WM2TemporaryRules internal, for kstart
        @li WM2WindowClass  WM_CLASS
        @li WM2WindowRole   WM_WINDOW_ROLE
        @li WM2ClientMachine WM_CLIENT_MACHINE
        @li WM2ShowingDesktop
        @li WM2Opacity _NET_WM_WINDOW_OPACITY
        @li WM2DesktopLayout _NET_DESKTOP_LAYOUT
        @li WM2FullPlacement _NET_WM_FULL_PLACEMENT
        @li WM2FullscreenMonitors _NET_WM_FULLSCREEN_MONITORS
    **/
    enum Property2 {
        WM2UserTime            = 1<<0,
        WM2StartupId           = 1<<1,
        WM2TransientFor        = 1<<2,
        WM2GroupLeader         = 1<<3,
        WM2AllowedActions      = 1<<4,
        WM2RestackWindow       = 1<<5,
        WM2MoveResizeWindow    = 1<<6,
        WM2ExtendedStrut       = 1<<7,
        WM2TakeActivity        = 1<<8,
        WM2KDETemporaryRules   = 1<<9,  // NOT STANDARD
        WM2WindowClass         = 1<<10,
        WM2WindowRole          = 1<<11,
        WM2ClientMachine       = 1<<12,
        WM2ShowingDesktop      = 1<<13,
        WM2Opacity             = 1<<14,
        WM2DesktopLayout       = 1<<15,        
        WM2FullPlacement       = 1<<16,
        WM2FullscreenMonitors  = 1<<17,
        WM2FrameOverlap        = 1<<18  // NOT STANDARD
    };

    /**
       Sentinel value to indicate that the client wishes to be visible on
       all desktops.
     **/ 
    enum { OnAllDesktops = -1 };
    
    /**
       Source of the request.
    **/
    // must match the values for data.l[0] field in _NET_ACTIVE_WINDOW message
    enum RequestSource {
        /**
          @internal indicates that the source of the request is unknown
        **/
        FromUnknown = 0, // internal
        /**
           indicates that the request comes from a normal application
        **/
        FromApplication = 1,
        /**
           indicated that the request comes from pager or similar tool
        **/
        FromTool = 2
    };
    
    /**
      Orientation.
    **/
    enum Orientation {
        OrientationHorizontal = 0,
        OrientationVertical = 1
    };
    
    /**
     Starting corner for desktop layout.
    **/
    enum DesktopLayoutCorner {
        DesktopLayoutCornerTopLeft = 0,
        DesktopLayoutCornerTopRight = 1,
        DesktopLayoutCornerBottomLeft = 2,
        DesktopLayoutCornerBottomRight = 3
    };
    
    /**
     Compares two X timestamps, taking into account wrapping and 64bit architectures.
     Return value is like with strcmp(), 0 for equal, -1 for time1 < time2, 1 for time1 > time2.
     @deprecated Moved to KXUtils namespace.
    */
    static int timestampCompare( unsigned long time1, unsigned long time2 );
    /**
     Returns a difference of two X timestamps, time2 - time1, where time2 must be later than time1,
     as returned by timestampCompare().
     @deprecated Moved to KXUtils namespace.
    */
    static int timestampDiff( unsigned long time1, unsigned long time2 );

};

#endif // netwm_def_h
