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


#ifndef   __net_wm_p_h
#define   __net_wm_p_h

/**
   Resizable array class.

   This resizable array is used to simplify the implementation.  The existance of
   this class is to keep the implementation from depending on a separate
   framework/library.
   @internal  
**/

template <class Z> class RArray {
public:
    /**
       Constructs an empty (size == 0) array.
    **/

    RArray();

    /**
       Resizable array destructor.
    **/

    ~RArray();

    /**
       The [] operator does the work.  If the index is larger than the current
       size of the array, it is resized.
     **/

    Z &operator[](int);

    /**
       Returns the size of the array.
     **/

    int size() const { return sz; }
    
    /**
       Resets the array (size == 0).
     **/
    void reset();

private:
    int sz;
    Z *d;
};


/**
   Private data for the NETRootInfo class.
   @internal  
**/

struct NETRootInfoPrivate {
    // information about the X server
    Display *display;
    NETSize rootSize;
    Window root;
    Window supportwindow;
    const char *name;
    int screen;

    // data that changes (either by the window manager or by a client)
    // and requires updates
    RArray<NETPoint> viewport;
    RArray<NETRect> workarea;
    NETSize geometry;
    Window active;
    Window *clients, *stacking, *virtual_roots, *kde_system_tray_windows;
    RArray<const char *> desktop_names;
    int number_of_desktops;
    int current_desktop;

    unsigned long clients_count, stacking_count, virtual_roots_count,
	kde_system_tray_windows_count;

    // the list of supported protocols shouldn't change, but NET clients will
    // need to read this property to know what is supported...
    unsigned long protocols;

    int ref;
};


/**
   Private data for the NETWinInfo class.
   @internal  
**/

struct NETWinInfoPrivate {
    Display *display;
    Window window, root;
    NET::MappingState mapping_state;
    Bool mapping_state_dirty;

    RArray<NETIcon> icons;
    int icon_count;

    NETRect icon_geom, win_geom;
    unsigned long state;
    NETStrut strut, frame_strut;
    NET::WindowType type;
    char *name, *visible_name, *icon_name, *visible_icon_name;
    int desktop;
    int pid;
    int handled_icons;
    Window kde_system_tray_win_for;

    unsigned long properties;

    int ref;
};


#endif // __net_wm_p_h
