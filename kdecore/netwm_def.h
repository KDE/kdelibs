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


struct NETPoint {
    CARD32 x, y;
};


struct NETSize {
    CARD32 width, height;
};


struct NETRect {
    NETPoint pos;
    NETSize size;
};


struct NETIcon {
    NETSize size;
    CARD32 *data;
};


struct NETStrut {
    CARD32 left, right, top, bottom;
};


class NET {
public:
    enum Role {
	Client,
	WindowManager
    };

    enum WindowType {
	Unknown = -1,
	Normal  = 0,
	Desktop = 1,
	Dock    = 2,
	Toolbar = 3,
	Menu    = 4,
	Dialog  = 5
    };

    enum State {
	Modal        = 1<<0,
	Sticky       = 1<<1,
	MaxVert      = 1<<2,
	MaxHoriz     = 1<<3,
	Max = MaxVert | MaxHoriz,
	Shaded       = 1<<4,
	SkipTaskbar  = 1<<5,
	StaysOnTop   = 1<<6
    };

    enum Direction {
	TopLeft      =0,
	Top          =1,
	TopRight     =2,
	Right        =3,
	BottomRight  =4,
	Bottom       =5,
	BottomLeft   =6,
	Left         =7,
	Move         =8 // movement only
    };

    enum MappingState {
	Visible, // ie. NormalState
	Withdrawn,
	Iconic
    };

    enum Property {
	// root
	Supported           = 1<<0,
	ClientList          = 1<<1,
	ClientListStacking  = 1<<2,
	NumberOfDesktops    = 1<<3,
	DesktopGeometry     = 1<<4,
	DesktopViewport     = 1<<5,
	CurrentDesktop      = 1<<6,
	DesktopNames        = 1<<7,
	ActiveWindow        = 1<<8,
	WorkArea            = 1<<9,
	SupportingWMCheck   = 1<<10,
	VirtualRoots        = 1<<11,
	KDEDockingWindows   = 1<<12, // NOT STANDARD
	CloseWindow         = 1<<13,
	WMMoveResize        = 1<<14,

	// window
	WMName              = 1<<15,
	WMVisibleName       = 1<<16,
	WMDesktop           = 1<<17,
	WMWindowType        = 1<<18,
	WMState             = 1<<19,
	WMStrut             = 1<<20,
	WMIconGeometry      = 1<<21,
	WMIcon              = 1<<22,
	WMPid               = 1<<23,
	WMHandledIcons      = 1<<24,
	WMPing              = 1<<25,
	WMKDEDockWinFor     = 1<<26, // NOT STANDARD
	XAWMState           = 1<<27,  // NOT STANDARD
	WMKDEFrameStrut     = 1<<28 // NOT STANDARD
	
    };
};


#endif // __netwm_def_h
