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

#ifndef   __netwm_def_h
#define   __netwm_def_h


/**
  Simple point class.

  This class is a convenience class defining a point x, y.  The existence of
  this class is to keep the implementation from being dependant on a
  separate framework/library.

  @author Bradley T. Hughes <bhughes@trolltech.com>
**/

struct NETPoint {
    /**
       Constructor to initialize this point to 0,0
    **/
    NETPoint() : x(0), y(0) { }

    /**
       Public data member.
    **/
    int x, y;
};


/**
  Simple size class.

  This class is a convenience class defining a size width by height.  The
  existence of this class is to keep the implementation from being dependant
  on a separate framework/library.

  @author Bradley T. Hughes <bhughes@trolltech.com>
**/

struct NETSize {
    /**
       Constructor to initialize this size to 0x0
    **/
    NETSize() : width(0), height(0) { }

    /**
       Public data member.
    **/
    int width, height;
};

/**
   Simple rectangle class.

   This class is a convenience class defining a rectangle as a point x,y with a
   size width by height.  The existence of this class is to keep the implementation
   from being dependant on a separate framework/library;
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
   Simple icon class.

   This class is a convenience class defining an icon of size width by height.
   The existence of this class is to keep the implementation from being
   dependant on a separate framework/library.
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
   Simple strut class.

   This class is a convenience class defining a strut with left, right, top and
   bottom border values.  The existence of this class is to keep the implementation
   from being dependant on a separate framework/library.
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
  Base namespace class.

  This class is the base class for the NETRootInfo and NETWinInfo classes.  To keep
  the namespace relatively clean, all enums are defined here.
 **/

class NET {
public:
    /**
       Application role.  This is used internally to determine how several action
       should be performed (if at all).

       @li Client indicates that the application is a client application.

       @li WindowManager indicates that the application is a window manager
       application.
    **/

    enum Role {
	Client,
	WindowManager
    };

    /**
       Window type.

       @li Unknown indicates that the window did not define a window type.

       @li Normal indicates that this is a normal, top-level window. Windows with
       Unknown window type or WM_TRANSIENT_FOR unset must be taken as this type.

       @li Desktop indicates a desktop feature. This can include a single window
       containing desktop icons with the same dimensions as the screen, allowing
       the desktop environment to have full control of the desktop, without the
       need for proxying root window clicks.

       @li Dock indicates a dock or panel feature. Typically a window manager would
       keep such windows on top of all other windows.

       @li Toolbar and Menu indicate toolbar and pinnable menu windows, respectively.

       @li Dialog indicates that this is a dialog window. If _NET_WM_WINDOW_TYPE is
       not set, then windows with WM_TRANSIENT_FOR set must be taken as this type.

       @li Override indicates that this window wants to have no Window Manager
       decorations. This is for windows that would normally use either override_redirect
       or Motif hints to give no decorations.  This is a KDE extension to the
       _NET_WM_WINDOW_TYPE mechanism.
    **/

    enum WindowType {
	Unknown  = -1,
	Normal   = 0,
	Desktop  = 1,
	Dock     = 2,
	Tool     = 3,
       	Menu     = 4,
	Dialog   = 5,
	Override = 6, // NON STANDARD
	Toolbar = Tool // This will go away soon, COMPAT
    };

    /**
       Window state.

       @li Modal ndicates that this is a modal dialog box. The WM_TRANSIENT_FOR hint
       MUST be set to indicate which window the dialog is a modal for, or set to
       the root window if the dialog is a modal for its window group.

       @li Sticky indicates that the Window Manager SHOULD keep the window's position
       fixed on the screen, even when the virtual desktop scrolls.

       @li Max{Vert,Horiz} indicates that the window is {vertically,horizontally}
       maximized.

       @li Max is more convenient than MaxVert | MaxHoriz.

       @li Shaded indicates that the window is shaded (rolled-up).

       @li SkipTaskbar indicates that a window should not be included on a taskbar.

       @li StaysOnTop indicate that a window should be kept on top of other windows.

       @li SkipPager indicates that a window should not be included on a pager.
    **/

    enum State {
	Modal        = 1<<0,
	Sticky       = 1<<1,
	MaxVert      = 1<<2,
	MaxHoriz     = 1<<3,
	Max = MaxVert | MaxHoriz,
	Shaded       = 1<<4,
	SkipTaskbar  = 1<<5,
	StaysOnTop   = 1<<6,
	SkipPager    = 1<<7
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
	Move         = 8  // movement only
    };

    /**
       Client window mapping state.  The class automatically watches the mapping
       state of the client windows, and uses the mapping state to determine how
       to set/change different properties.

       @li Visible indicates the client window is visible to the user.

       @li Withdrawn indicates that neither the client window nor its icon is visible.

       @li Iconic indicates that the client window is not visible, but its icon is.
    **/

    enum MappingState {
	Visible, // ie. NormalState
	Withdrawn,
	Iconic
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
       @li WMStrut
       @li WMIconGeometry
       @li WMIcon
       @li WMPid

       ICCCM properties (provided for convenience):

       @li XAWMState

       Extended KDE protocols and properties (NOT STANDARD):

       @li KDESystemTrayWindows
       @li WMKDESystemTrayWinFor
       @li WMKDEFrameStrut
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
	KDESystemTrayWindows  = 1<<12, // NOT STANDARD
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
	WMKDESystemTrayWinFor = 1<<26, // NOT STANDARD
	XAWMState             = 1<<27, // NOT STANDARD
	WMKDEFrameStrut       = 1<<28, // NOT STANDARD

	// Need to be reordered
	WMIconName            = 1<<29,
	WMVisibleIconName     = 1<<30,
	WMGeometry	      = 1<<31
    };
};


#endif // __netwm_def_h
