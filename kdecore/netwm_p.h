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

template <class Z> class RArray {
public:
    RArray();
    ~RArray();

    Z &operator[](int);
    int size() const { return sz; }

private:
    int sz;
    Z *d;
};

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
    NETPoint viewport;
    RArray<NETRect> workarea;
    NETSize geometry;
    Window active;
    Window *clients, *stacking, *virtual_roots, *kde_docking_windows;
    RArray<const char *> desktop_names;
    CARD32 number_of_desktops;
    CARD32 current_desktop;

    unsigned long clients_count, stacking_count, virtual_roots_count,
	kde_docking_windows_count;

    // the list of supported protocols shouldn't change, but NET clients will
    // need to read this property to know what is supported...
    unsigned long protocols;

    int ref;
};


struct NETWinInfoPrivate {
    Display *display;
    Window window, root;
    NET::MappingState mapping_state;
    Bool mapping_state_dirty;

    RArray<NETIcon> icons;
    int icon_count;

    NETRect icon_geom, win_geom, frame_geom;
    unsigned long state;
    NETStrut strut, frame_strut;
    NET::WindowType type;
    char *name, *visible_name;
    CARD32 desktop;
    CARD32 pid;
    CARD32 handled_icons;
    Window kde_dockwin_for;

    unsigned long properties;

    int ref;
};


#endif // __net_wm_p_h
