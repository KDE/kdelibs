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

// #define NETWMDEBUG

#include <qwidget.h>
#ifdef Q_WS_X11 //FIXME

#include "netwm.h"

#include <string.h>
#include <stdio.h>

#include <X11/Xlibint.h>
#include <X11/Xmd.h>

typedef Bool X11Bool;

#include "netwm_p.h"


// UTF-8 string
static Atom UTF8_STRING = 0;

// root window properties
static Atom net_supported            = 0;
static Atom net_client_list          = 0;
static Atom net_client_list_stacking = 0;
static Atom net_desktop_geometry     = 0;
static Atom net_desktop_viewport     = 0;
static Atom net_current_desktop      = 0;
static Atom net_desktop_names        = 0;
static Atom net_number_of_desktops   = 0;
static Atom net_active_window        = 0;
static Atom net_workarea             = 0;
static Atom net_supporting_wm_check  = 0;
static Atom net_virtual_roots        = 0;

// root window messages
static Atom net_close_window         = 0;
static Atom net_wm_moveresize        = 0;

// application window properties
static Atom net_wm_name              = 0;
static Atom net_wm_visible_name      = 0;
static Atom net_wm_icon_name         = 0;
static Atom net_wm_visible_icon_name = 0;
static Atom net_wm_desktop           = 0;
static Atom net_wm_window_type       = 0;
static Atom net_wm_state             = 0;
static Atom net_wm_strut             = 0;
static Atom net_wm_icon_geometry     = 0;
static Atom net_wm_icon              = 0;
static Atom net_wm_pid               = 0;
static Atom net_wm_handled_icons     = 0;

// KDE extensions
static Atom kde_net_system_tray_windows       = 0;
static Atom kde_net_wm_system_tray_window_for = 0;
static Atom kde_net_wm_frame_strut            = 0;
static Atom kde_net_wm_window_type_override   = 0;
static Atom kde_net_wm_window_type_topmenu    = 0;

// application protocols
static Atom net_wm_ping = 0;

// application window types
static Atom net_wm_window_type_normal  = 0;
static Atom net_wm_window_type_desktop = 0;
static Atom net_wm_window_type_dock    = 0;
static Atom net_wm_window_type_toolbar = 0;
static Atom net_wm_window_type_menu    = 0;
static Atom net_wm_window_type_dialog  = 0;

// application window state
static Atom net_wm_state_modal        = 0;
static Atom net_wm_state_sticky       = 0;
static Atom net_wm_state_max_vert     = 0;
static Atom net_wm_state_max_horiz    = 0;
static Atom net_wm_state_shaded       = 0;
static Atom net_wm_state_skip_taskbar = 0;
static Atom net_wm_state_skip_pager   = 0;
static Atom net_wm_state_stays_on_top = 0;

// used to determine whether application window is managed or not
static Atom xa_wm_state = 0;

static Bool netwm_atoms_created      = False;
const unsigned long netwm_sendevent_mask = (SubstructureRedirectMask|
					     SubstructureNotifyMask);


static char *nstrdup(const char *s1) {
    if (! s1) return (char *) 0;

    int l = strlen(s1) + 1;
    char *s2 = new char[l];
    strncpy(s2, s1, l);
    return s2;
}


static char *nstrndup(const char *s1, int l) {
    if (! s1 || l == 0) return (char *) 0;

    char *s2 = new char[l+1];
    strncpy(s2, s1, l);
    s2[l] = '\0';
    return s2;
}


static Window *nwindup(Window *w1, int n) {
    if (! w1 || n == 0) return (Window *) 0;

    Window *w2 = new Window[n];
    while (n--)	w2[n] = w1[n];
    return w2;
}


static void refdec_nri(NETRootInfoPrivate *p) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NET: decrementing NETRootInfoPrivate::ref (%d)\n", p->ref - 1);
#endif

    if (! --p->ref) {

#ifdef    NETWMDEBUG
	fprintf(stderr, "NET: \tno more references, deleting\n");
#endif

	if (p->name) delete [] p->name;
	if (p->stacking) delete [] p->stacking;
	if (p->clients) delete [] p->clients;
	if (p->virtual_roots) delete [] p->virtual_roots;

	int i;
	for (i = 0; i < p->desktop_names.size(); i++)
	    if (p->desktop_names[i]) delete [] p->desktop_names[i];
    }
}


static void refdec_nwi(NETWinInfoPrivate *p) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NET: decrementing NETWinInfoPrivate::ref (%d)\n", p->ref - 1);
#endif

    if (! --p->ref) {

#ifdef    NETWMDEBUG
	fprintf(stderr, "NET: \tno more references, deleting\n");
#endif

	if (p->name) delete [] p->name;
	if (p->visible_name) delete [] p->visible_name;

	int i;
	for (i = 0; i < p->icons.size(); i++)
	    if (p->icons[i].data) delete [] p->icons[i].data;
    }
}


static int wcmp(const void *a, const void *b) {
    return *((Window *) a) - *((Window *) b);
}


static const int netAtomCount = 48;
static void create_atoms(Display *d) {
    static const char * const names[netAtomCount] =
    {
	"UTF8_STRING",
	    "_NET_SUPPORTED",
	    "_NET_SUPPORTING_WM_CHECK",
	    "_NET_CLIENT_LIST",
	    "_NET_CLIENT_LIST_STACKING",
	    "_NET_NUMBER_OF_DESKTOPS",
	    "_NET_DESKTOP_GEOMETRY",
	    "_NET_DESKTOP_VIEWPORT",
	    "_NET_CURRENT_DESKTOP",
	    "_NET_DESKTOP_NAMES",
	    "_NET_ACTIVE_WINDOW",
	    "_NET_WORKAREA",
	    "_NET_VIRTUAL_ROOTS",
	    "_NET_CLOSE_WINDOW",

	    "_NET_WM_MOVERESIZE",
	    "_NET_WM_NAME",
	    "_NET_WM_VISIBLE_NAME",
	    "_NET_WM_ICON_NAME",
	    "_NET_WM_VISIBLE_ICON_NAME",
	    "_NET_WM_DESKTOP",
	    "_NET_WM_WINDOW_TYPE",
	    "_NET_WM_STATE",
	    "_NET_WM_STRUT",
	    "_NET_WM_ICON_GEOMETRY",
	    "_NET_WM_ICON",
	    "_NET_WM_PID",
	    "_NET_WM_HANDLED_ICONS",
	    "_NET_WM_PING",

	    "_NET_WM_WINDOW_TYPE_NORMAL",
	    "_NET_WM_WINDOW_TYPE_DESKTOP",
	    "_NET_WM_WINDOW_TYPE_DOCK",
	    "_NET_WM_WINDOW_TYPE_TOOLBAR",
	    "_NET_WM_WINDOW_TYPE_MENU",
	    "_NET_WM_WINDOW_TYPE_DIALOG",

	    "_NET_WM_STATE_MODAL",
	    "_NET_WM_STATE_STICKY",
	    "_NET_WM_STATE_MAXIMIZED_VERT",
	    "_NET_WM_STATE_MAXIMIZED_HORZ",
	    "_NET_WM_STATE_SHADED",
	    "_NET_WM_STATE_SKIP_TASKBAR",
	    "_NET_WM_STATE_SKIP_PAGER",
	    "_NET_WM_STATE_STAYS_ON_TOP",

	    "_KDE_NET_SYSTEM_TRAY_WINDOWS",
	    "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR",
	    "_KDE_NET_WM_FRAME_STRUT",
	    "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE",
	    "_KDE_NET_WM_WINDOW_TYPE_TOPMENU",

	    "WM_STATE"
	    };

    Atom atoms[netAtomCount], *atomsp[netAtomCount] =
    {
	&UTF8_STRING,
	    &net_supported,
	    &net_supporting_wm_check,
	    &net_client_list,
	    &net_client_list_stacking,
	    &net_number_of_desktops,
	    &net_desktop_geometry,
	    &net_desktop_viewport,
	    &net_current_desktop,
	    &net_desktop_names,
	    &net_active_window,
	    &net_workarea,
	    &net_virtual_roots,
	    &net_close_window,

	    &net_wm_moveresize,
	    &net_wm_name,
	    &net_wm_visible_name,
	    &net_wm_icon_name,
	    &net_wm_visible_icon_name,
	    &net_wm_desktop,
	    &net_wm_window_type,
	    &net_wm_state,
	    &net_wm_strut,
	    &net_wm_icon_geometry,
	    &net_wm_icon,
	    &net_wm_pid,
	    &net_wm_handled_icons,
	    &net_wm_ping,

	    &net_wm_window_type_normal,
	    &net_wm_window_type_desktop,
	    &net_wm_window_type_dock,
	    &net_wm_window_type_toolbar,
	    &net_wm_window_type_menu,
	    &net_wm_window_type_dialog,

	    &net_wm_state_modal,
	    &net_wm_state_sticky,
	    &net_wm_state_max_vert,
	    &net_wm_state_max_horiz,
	    &net_wm_state_shaded,
	    &net_wm_state_skip_taskbar,
	    &net_wm_state_skip_pager,
	    &net_wm_state_stays_on_top,

	    &kde_net_system_tray_windows,
	    &kde_net_wm_system_tray_window_for,
	    &kde_net_wm_frame_strut,
	    &kde_net_wm_window_type_override,
	    &kde_net_wm_window_type_topmenu,

	    &xa_wm_state,
	    };

    int i = netAtomCount;
    while (i--)
	atoms[i] = 0;

    XInternAtoms(d, (char **) names, netAtomCount, False, atoms);

    i = netAtomCount;
    while (i--)
	*atomsp[i] = atoms[i];

    netwm_atoms_created = True;
}


static void readIcon(NETWinInfoPrivate *p) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NET: readIcon\n");
#endif

    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret = 0, after_ret = 0;
    unsigned char *data_ret = 0;

    // allocate buffers
    unsigned char *buffer = 0;
    unsigned long offset = 0;
    unsigned long buffer_offset = 0;
    unsigned long bufsize = 0;

    // read data
    do {
	if (XGetWindowProperty(p->display, p->window, net_wm_icon, offset,
			       BUFSIZE, False, XA_CARDINAL, &type_ret,
			       &format_ret, &nitems_ret, &after_ret, &data_ret)
	    == Success) {
            if (!bufsize)
            {
               if (nitems_ret < 3 || type_ret != XA_CARDINAL ||
                  format_ret != 32) {
                  // either we didn't get the property, or the property has less than
                  // 3 elements in it
                  // NOTE: 3 is the ABSOLUTE minimum:
                  //     width = 1, height = 1, length(data) = 1 (width * height)
                  if ( data_ret )
                     XFree(data_ret);
                  return;
               }

               bufsize = nitems_ret * sizeof(long) + after_ret;
               buffer = (unsigned char *) malloc(bufsize);
            }
            else if (buffer_offset + nitems_ret*sizeof(long) > bufsize)
            {
fprintf(stderr, "NETWM: Warning readIcon() needs buffer adjustment!\n");
               bufsize = buffer_offset + nitems_ret * sizeof(long) + after_ret;
               buffer = (unsigned char *) realloc(buffer, bufsize);
            }
	    memcpy((buffer + buffer_offset), data_ret, nitems_ret * sizeof(long));
	    buffer_offset += nitems_ret * sizeof(long);
	    offset += nitems_ret;

	    if ( data_ret )
		XFree(data_ret);
	} else {
            if (buffer)
               free(buffer);
	    return; // Some error occured cq. property didn't exist.
	}
    }
    while (after_ret > 0);

    CARD32 *data32;
    unsigned long i, j, k, sz, s;
    unsigned long *d = (unsigned long *) buffer;
    for (i = 0, j = 0; i < bufsize; i++) {
	p->icons[j].size.width = *d++;
	i += sizeof(long);
	p->icons[j].size.height = *d++;
	i += sizeof(long);

	sz = p->icons[j].size.width * p->icons[j].size.height;
	s = sz * sizeof(long);

	if ( i + s - 1 > bufsize ) {
	    break;
	}

	if (p->icons[j].data) delete [] p->icons[j].data;
	data32 = new CARD32[sz];
	p->icons[j].data = (unsigned char *) data32;
	for (k = 0; k < sz; k++, i += sizeof(long)) {
	    *data32++ = (CARD32) *d++;
	}
	j++;
    }

#ifdef    NETWMDEBUG
    fprintf(stderr, "NET: readIcon got %d icons\n", p->icons.size());
#endif

    free(buffer);
}


template <class Z>
RArray<Z>::RArray() {
  sz = 0;
  d = 0;
}


template <class Z>
RArray<Z>::~RArray() {
    if (d) delete [] d;
}


template <class Z>
Z &RArray<Z>::operator[](int index) {
    if (!d) {
	d = new Z[index + 1];
	memset( (void*) &d[0], 0, sizeof(Z) );
	sz = 1;
    } else if (index >= sz) {
	// allocate space for the new data
	Z *newdata = new Z[index + 1];

	// move the old data into the new array
	int i;
	for (i = 0; i < sz; i++)
	    newdata[i] = d[i];
	for (; i <= index; i++ )
	    memset( (void*) &newdata[i], 0, sizeof(Z) );

	sz = index + 1;

	// delete old data and reassign
	delete [] d;
	d = newdata;
    }

    return d[index];
}


// Construct a new NETRootInfo object.

NETRootInfo::NETRootInfo(Display *display, Window supportWindow, const char *wmName,
			 unsigned long properties, int screen, bool doActivate)
{

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::NETRootInfo: using window manager constructor\n");
#endif

    p = new NETRootInfoPrivate;
    p->ref = 1;

    p->display = display;
    p->name = nstrdup(wmName);

    if (screen != -1) {
	p->screen = screen;
    } else {
	p->screen = DefaultScreen(p->display);
    }

    p->root = RootWindow(p->display, p->screen);
    p->supportwindow = supportWindow;
    p->protocols = properties;
    p->number_of_desktops = p->current_desktop = 0;
    p->active = None;
    p->clients = p->stacking = p->virtual_roots = (Window *) 0;
    p->clients_count = p->stacking_count = p->virtual_roots_count = 0;
    p->kde_system_tray_windows = 0;
    p->kde_system_tray_windows_count = 0;

    role = WindowManager;

    if (! netwm_atoms_created) create_atoms(p->display);

    if (doActivate) activate();
}


NETRootInfo::NETRootInfo(Display *display, unsigned long properties, int screen,
			 bool doActivate)
{

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::NETRootInfo: using Client constructor\n");
#endif

    p = new NETRootInfoPrivate;
    p->ref = 1;

    p->name = 0;

    p->display = display;

    if (screen != -1) {
	p->screen = screen;
    } else {
	p->screen = DefaultScreen(p->display);
    }

    p->root = RootWindow(p->display, p->screen);
    p->rootSize.width = WidthOfScreen(ScreenOfDisplay(p->display, p->screen));
    p->rootSize.height = HeightOfScreen(ScreenOfDisplay(p->display, p->screen));

    p->supportwindow = None;
    p->protocols = properties;
    p->number_of_desktops = p->current_desktop = 0;
    p->active = None;
    p->clients = p->stacking = p->virtual_roots = (Window *) 0;
    p->clients_count = p->stacking_count = p->virtual_roots_count = 0;
    p->kde_system_tray_windows = 0;
    p->kde_system_tray_windows_count = 0;

    role = Client;

    if (! netwm_atoms_created) create_atoms(p->display);

    if (doActivate) activate();
}


// Copy an existing NETRootInfo object.

NETRootInfo::NETRootInfo(const NETRootInfo &rootinfo) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::NETRootInfo: using copy constructor\n");
#endif

    p = rootinfo.p;
    role = rootinfo.role;

    p->ref++;
}


// Be gone with our NETRootInfo.

NETRootInfo::~NETRootInfo() {
    refdec_nri(p);

    if (! p->ref) delete p;
}


void NETRootInfo::activate() {
    if (role == WindowManager) {

#ifdef    NETWMDEBUG
	fprintf(stderr,
		"NETRootInfo::activate: setting supported properties on root\n");
#endif

	// force support for Supported and SupportingWMCheck for window managers
	setSupported(p->protocols | Supported | SupportingWMCheck);
    } else {

#ifdef    NETWMDEBUG
	fprintf(stderr, "NETRootInfo::activate: updating client information\n");
#endif

	update(p->protocols);
    }
}


void NETRootInfo::setClientList(Window *windows, unsigned int count) {
    if (role != WindowManager) return;

    p->clients_count = count;

    if (p->clients) delete [] p->clients;
    p->clients = nwindup(windows, count);

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setClientList: setting list with %ld windows\n",
	    p->clients_count);
#endif

    XChangeProperty(p->display, p->root, net_client_list, XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *)p->clients,
		    p->clients_count);
}


void NETRootInfo::setClientListStacking(Window *windows, unsigned int count) {
    if (role != WindowManager) return;

    p->stacking_count = count;
    if (p->stacking) delete [] p->stacking;
    p->stacking = nwindup(windows, count);

#ifdef    NETWMDEBUG
    fprintf(stderr,
	    "NETRootInfo::setClientListStacking: setting list with %ld windows\n",
	    p->clients_count);
#endif

    XChangeProperty(p->display, p->root, net_client_list_stacking, XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *) p->stacking,
		    p->stacking_count);
}


void NETRootInfo::setKDESystemTrayWindows(Window *windows, unsigned int count) {
    if (role != WindowManager) return;

    p->kde_system_tray_windows_count = count;
    if (p->kde_system_tray_windows) delete [] p->kde_system_tray_windows;
    p->kde_system_tray_windows = nwindup(windows, count);

#ifdef    NETWMDEBUG
    fprintf(stderr,
	    "NETRootInfo::setKDESystemTrayWindows: setting list with %ld windows\n",
	    p->kde_system_tray_windows_count);
#endif

    XChangeProperty(p->display, p->root, kde_net_system_tray_windows, XA_WINDOW, 32,
		    PropModeReplace,
		    (unsigned char *) p->kde_system_tray_windows,
		    p->kde_system_tray_windows_count);
}


void NETRootInfo::setNumberOfDesktops(int numberOfDesktops) {

#ifdef    NETWMDEBUG
    fprintf(stderr,
	    "NETRootInfo::setNumberOfDesktops: setting desktop count to %d (%s)\n",
	    numberOfDesktops, (role == WindowManager) ? "WM" : "Client");
#endif

    if (role == WindowManager) {
	p->number_of_desktops = numberOfDesktops;
 	long d = numberOfDesktops;
	XChangeProperty(p->display, p->root, net_number_of_desktops, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &d, 1);
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_number_of_desktops;
	e.xclient.display = p->display;
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = numberOfDesktops;
	e.xclient.data.l[1] = 0l;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
    }
}


void NETRootInfo::setCurrentDesktop(int desktop) {

#ifdef    NETWMDEBUG
    fprintf(stderr,
	    "NETRootInfo::setCurrentDesktop: setting current desktop = %d (%s)\n",
	    desktop, (role == WindowManager) ? "WM" : "Client");
#endif

    if (role == WindowManager) {
	p->current_desktop = desktop;
	long d = p->current_desktop - 1;
	XChangeProperty(p->display, p->root, net_current_desktop, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &d, 1);
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_current_desktop;
	e.xclient.display = p->display;
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = desktop - 1;
	e.xclient.data.l[1] = 0l;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
    }
}


void NETRootInfo::setDesktopName(int desktop, const char *desktopName) {
    // return immediately if the requested desk is out of range
    if (desktop < 1 || desktop > p->number_of_desktops) return;

    if (p->desktop_names[desktop - 1]) delete [] p->desktop_names[desktop - 1];
    p->desktop_names[desktop - 1] = nstrdup(desktopName);

    unsigned int i, proplen,
	num = ((p->number_of_desktops < p->desktop_names.size()) ?
	       p->number_of_desktops : p->desktop_names.size());
    for (i = 0, proplen = 0; i < num; i++)
	proplen += (p->desktop_names[i] != 0 ? strlen(p->desktop_names[i]) : 1 ) + 1;

    char *prop = new char[proplen], *propp = prop;

    for (i = 0; i < num; i++)
	if (p->desktop_names[i]) {
	    strcpy(propp, p->desktop_names[i]);
	    propp += strlen(p->desktop_names[i]) + 1;
	} else
	    *propp++ = '\0';

#ifdef    NETWMDEBUG
    fprintf(stderr,
	    "NETRootInfo::setDesktopName(%d, '%s')\n"
	    "NETRootInfo::setDesktopName: total property length = %d",
	    desktop, desktopName, proplen);
#endif

    XChangeProperty(p->display, p->root, net_desktop_names, UTF8_STRING, 8,
		    PropModeReplace, (unsigned char *) prop, proplen);

    delete [] prop;
}


void NETRootInfo::setDesktopGeometry(int , const NETSize &geometry) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setDesktopGeometry( -- , { %d, %d }) (%s)\n",
	    geometry.width, geometry.height, (role == WindowManager) ? "WM" : "Client");
#endif

    if (role == WindowManager) {
	p->geometry = geometry;

	long data[2];
	data[0] = p->geometry.width;
	data[1] = p->geometry.height;

	XChangeProperty(p->display, p->root, net_desktop_geometry, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, 2);
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_desktop_geometry;
	e.xclient.display = p->display;
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = geometry.width;
	e.xclient.data.l[1] = geometry.height;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
    }
}


void NETRootInfo::setDesktopViewport(int desktop, const NETPoint &viewport) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setDesktopViewport(%d, { %d, %d }) (%s)\n",
	    desktop, viewport.x, viewport.y, (role == WindowManager) ? "WM" : "Client");
#endif

    if (desktop < 1) return;

    if (role == WindowManager) {
	p->viewport[desktop - 1] = viewport;

	int d, i, l;
	l = p->viewport.size() * 2;
	long *data = new long[l];
	for (d = 0, i = 0; d < p->viewport.size(); d++) {
	    data[i++] = p->viewport[d].x;
	    data[i++] = p->viewport[d].y;
	}

	XChangeProperty(p->display, p->root, net_desktop_viewport, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, l);

	delete [] data;
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_desktop_viewport;
	e.xclient.display = p->display;
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = viewport.x;
	e.xclient.data.l[1] = viewport.y;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
    }
}


void NETRootInfo::setSupported(unsigned long pr) {
    p->protocols = pr;

    if (role != WindowManager) {
#ifdef    NETWMDEBUG
	fprintf(stderr, "NETRootInfo::setSupported - role != WindowManager\n");
#endif

	return;
    }

    Atom atoms[netAtomCount];
    int pnum = 2;

    // Root window properties/messages
    atoms[0] = net_supported;
    atoms[1] = net_supporting_wm_check;

    if (p->protocols & ClientList)
	atoms[pnum++] = net_client_list;

    if (p->protocols & ClientListStacking)
	atoms[pnum++] = net_client_list_stacking;

    if (p->protocols & NumberOfDesktops)
	atoms[pnum++] = net_number_of_desktops;

    if (p->protocols & DesktopGeometry)
	atoms[pnum++] = net_desktop_geometry;

    if (p->protocols & DesktopViewport)
	atoms[pnum++] = net_desktop_viewport;

    if (p->protocols & CurrentDesktop)
	atoms[pnum++] = net_current_desktop;

    if (p->protocols & DesktopNames)
	atoms[pnum++] = net_desktop_names;

    if (p->protocols & ActiveWindow)
	atoms[pnum++] = net_active_window;

    if (p->protocols & WorkArea)
	atoms[pnum++] = net_workarea;

    if (p->protocols & VirtualRoots)
	atoms[pnum++] = net_virtual_roots;

    if (p->protocols & CloseWindow)
	atoms[pnum++] = net_close_window;


    // Application window properties/messages
    if (p->protocols & WMMoveResize)
	atoms[pnum++] = net_wm_moveresize;

    if (p->protocols & WMName)
	atoms[pnum++] = net_wm_name;

    if (p->protocols & WMVisibleName)
	atoms[pnum++] = net_wm_visible_name;

    if (p->protocols & WMIconName)
	atoms[pnum++] = net_wm_icon_name;

    if (p->protocols & WMVisibleIconName)
	atoms[pnum++] = net_wm_visible_icon_name;

    if (p->protocols & WMDesktop)
	atoms[pnum++] = net_wm_desktop;

    if (p->protocols & WMWindowType) {
	atoms[pnum++] = net_wm_window_type;

	// Application window types
	atoms[pnum++] = net_wm_window_type_normal;
	atoms[pnum++] = net_wm_window_type_desktop;
	atoms[pnum++] = net_wm_window_type_dock;
	atoms[pnum++] = net_wm_window_type_toolbar;
	atoms[pnum++] = net_wm_window_type_menu;
	atoms[pnum++] = net_wm_window_type_dialog;
	atoms[pnum++] = kde_net_wm_window_type_override;
	atoms[pnum++] = kde_net_wm_window_type_topmenu;
    }

    if (p->protocols & WMState) {
	atoms[pnum++] = net_wm_state;

	// Application window states
	atoms[pnum++] = net_wm_state_modal;
	atoms[pnum++] = net_wm_state_sticky;
	atoms[pnum++] = net_wm_state_max_vert;
	atoms[pnum++] = net_wm_state_max_horiz;
	atoms[pnum++] = net_wm_state_shaded;
	atoms[pnum++] = net_wm_state_skip_taskbar;
	atoms[pnum++] = net_wm_state_skip_pager;
	atoms[pnum++] = net_wm_state_stays_on_top;
    }

    if (p->protocols & WMStrut)
	atoms[pnum++] = net_wm_strut;

    if (p->protocols & WMIconGeometry)
	atoms[pnum++] = net_wm_icon_geometry;

    if (p->protocols & WMIcon)
	atoms[pnum++] = net_wm_icon;

    if (p->protocols & WMPid)
	atoms[pnum++] = net_wm_pid;

    if (p->protocols & WMHandledIcons)
	atoms[pnum++] = net_wm_handled_icons;

    if (p->protocols & WMPing)
	atoms[pnum++] = net_wm_ping;

    // KDE specific extensions
    if (p->protocols & KDESystemTrayWindows)
	atoms[pnum++] = kde_net_system_tray_windows;

    if (p->protocols & WMKDESystemTrayWinFor)
	atoms[pnum++] = kde_net_wm_system_tray_window_for;

    if (p->protocols & WMKDEFrameStrut)
	atoms[pnum++] = kde_net_wm_frame_strut;

    XChangeProperty(p->display, p->root, net_supported, XA_ATOM, 32,
		    PropModeReplace, (unsigned char *) atoms, pnum);
    XChangeProperty(p->display, p->root, net_supporting_wm_check, XA_WINDOW, 32,
	 	    PropModeReplace, (unsigned char *) &(p->supportwindow), 1);

#ifdef    NETWMDEBUG
    fprintf(stderr,
	    "NETRootInfo::setSupported: _NET_SUPPORTING_WM_CHECK = 0x%lx on 0x%lx\n"
	    "                         : _NET_WM_NAME = '%s' on 0x%lx\n",
	    p->supportwindow, p->supportwindow, p->name, p->supportwindow);
#endif

    XChangeProperty(p->display, p->supportwindow, net_supporting_wm_check,
		    XA_WINDOW, 32, PropModeReplace,
		    (unsigned char *) &(p->supportwindow), 1);
    XChangeProperty(p->display, p->supportwindow, net_wm_name, UTF8_STRING, 8,
		    PropModeReplace, (unsigned char *) p->name,
		    strlen(p->name));
}


void NETRootInfo::setActiveWindow(Window window) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setActiveWindow(0x%lx) (%s)\n",
            window, (role == WindowManager) ? "WM" : "Client");
#endif

    if (role == WindowManager) {
	p->active = window;
	XChangeProperty(p->display, p->root, net_active_window, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) &(p->active), 1);
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_active_window;
	e.xclient.display = p->display;
	e.xclient.window = window;
	e.xclient.format = 32;
	e.xclient.data.l[0] = 0l;
	e.xclient.data.l[1] = 0l;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
    }
}


void NETRootInfo::setWorkArea(int desktop, const NETRect &workarea) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setWorkArea(%d, { %d, %d, %d, %d }) (%s)\n",
	    desktop, workarea.pos.x, workarea.pos.y, workarea.size.width, workarea.size.height,
	    (role == WindowManager) ? "WM" : "Client");
#endif

    if (role != WindowManager || desktop < 1) return;

    p->workarea[desktop - 1] = workarea;

    long *wa = new long[p->number_of_desktops * 4];
    int i, o;
    for (i = 0, o = 0; i < p->number_of_desktops; i++) {
	wa[o++] = p->workarea[i].pos.x;
	wa[o++] = p->workarea[i].pos.y;
	wa[o++] = p->workarea[i].size.width;
	wa[o++] = p->workarea[i].size.height;
    }

    XChangeProperty(p->display, p->root, net_workarea, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) wa,
		    p->number_of_desktops * 4);

    delete [] wa;
}


void NETRootInfo::setVirtualRoots(Window *windows, unsigned int count) {
    if (role != WindowManager) return;

    p->virtual_roots_count = count;
    p->virtual_roots = windows;

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setVirtualRoots: setting list with %ld windows\n",
	    p->virtual_roots_count);
#endif

    XChangeProperty(p->display, p->root, net_virtual_roots, XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *) p->virtual_roots,
		    p->virtual_roots_count);
}


void NETRootInfo::closeWindowRequest(Window window) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::closeWindowRequest: requesting close for 0x%lx\n",
	    window);
#endif

    XEvent e;

    e.xclient.type = ClientMessage;
    e.xclient.message_type = net_close_window;
    e.xclient.display = p->display;
    e.xclient.window = window;
    e.xclient.format = 32;
    e.xclient.data.l[0] = 0l;
    e.xclient.data.l[1] = 0l;
    e.xclient.data.l[2] = 0l;
    e.xclient.data.l[3] = 0l;
    e.xclient.data.l[4] = 0l;

    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
}


void NETRootInfo::moveResizeRequest(Window window, int x_root, int y_root,
				    Direction direction)
{

#ifdef    NETWMDEBUG
    fprintf(stderr,
	    "NETRootInfo::moveResizeRequest: requesting resize/move for 0x%lx (%d, %d, %d)\n",
	    window, x_root, y_root, direction);
#endif

    XEvent e;

    e.xclient.type = ClientMessage;
    e.xclient.message_type = net_wm_moveresize;
    e.xclient.display = p->display;
    e.xclient.window = window,
    e.xclient.format = 32;
    e.xclient.data.l[0] = x_root;
    e.xclient.data.l[1] = y_root;
    e.xclient.data.l[2] = direction;
    e.xclient.data.l[3] = 0l;
    e.xclient.data.l[4] = 0l;

    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
}


// assignment operator

const NETRootInfo &NETRootInfo::operator=(const NETRootInfo &rootinfo) {

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETRootInfo::operator=()\n");
#endif

    if (p != rootinfo.p) {
	refdec_nri(p);

	if (! p->ref) delete p;
    }

    p = rootinfo.p;
    role = rootinfo.role;
    p->ref++;

    return *this;
}


unsigned long NETRootInfo::event(XEvent *event) {
    unsigned long dirty = 0;

    // the window manager will be interested in client messages... no other
    // client should get these messages
    if (role == WindowManager && event->type == ClientMessage &&
	event->xclient.format == 32) {
#ifdef    NETWMDEBUG
	fprintf(stderr, "NETRootInfo::event: handling ClientMessage event\n");
#endif

	if (event->xclient.message_type == net_number_of_desktops) {
	    dirty = NumberOfDesktops;

#ifdef   NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: changeNumberOfDesktops(%ld)\n",
		    event->xclient.data.l[0]);
#endif

	    changeNumberOfDesktops(event->xclient.data.l[0]);
	} else if (event->xclient.message_type == net_desktop_geometry) {
	    dirty = DesktopGeometry;

	    NETSize sz;
	    sz.width = event->xclient.data.l[0];
	    sz.height = event->xclient.data.l[1];

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: changeDesktopGeometry( -- , { %d, %d })\n",
		    sz.width, sz.height);
#endif

	    changeDesktopGeometry(~0, sz);
	} else if (event->xclient.message_type == net_desktop_viewport) {
	    dirty = DesktopViewport;

	    NETPoint pt;
	    pt.x = event->xclient.data.l[0];
	    pt.y = event->xclient.data.l[1];

#ifdef   NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: changeDesktopViewport(%d, { %d, %d })\n",
		    p->current_desktop, pt.x, pt.y);
#endif

	    changeDesktopViewport(p->current_desktop, pt);
	} else if (event->xclient.message_type == net_current_desktop) {
	    dirty = CurrentDesktop;

#ifdef   NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: changeCurrentDesktop(%ld)\n",
		    event->xclient.data.l[0] + 1);
#endif

	    changeCurrentDesktop(event->xclient.data.l[0] + 1);
	} else if (event->xclient.message_type == net_active_window) {
	    dirty = ActiveWindow;

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: changeActiveWindow(0x%lx)\n",
		    event->xclient.window);
#endif

	    changeActiveWindow(event->xclient.window);
	} else if (event->xclient.message_type == net_wm_moveresize) {

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: moveResize(%ld, %ld, %ld, %ld)\n",
		    event->xclient.window,
		    event->xclient.data.l[0],
		    event->xclient.data.l[1],
		    event->xclient.data.l[2]
		    );
#endif

	    moveResize(event->xclient.window,
		       event->xclient.data.l[0],
		       event->xclient.data.l[1],
		       event->xclient.data.l[2]);
	} else if (event->xclient.message_type == net_close_window) {

#ifdef   NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: closeWindow(0x%lx)\n",
		    event->xclient.window);
#endif

	    closeWindow(event->xclient.window);
	}
    }

    if (event->type == PropertyNotify) {

#ifdef    NETWMDEBUG
	fprintf(stderr, "NETRootInfo::event: handling PropertyNotify event\n");
#endif

	XEvent pe = *event;

	Bool done = False;
	Bool compaction = False;
	while (! done) {

#ifdef   NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: loop fire\n");
#endif

	    if (pe.xproperty.atom == net_client_list)
		dirty |= ClientList;
	    else if (pe.xproperty.atom == net_client_list_stacking)
		dirty |= ClientListStacking;
	    else if (pe.xproperty.atom == kde_net_system_tray_windows)
		dirty |= KDESystemTrayWindows;
	    else if (pe.xproperty.atom == net_desktop_names)
		dirty |= DesktopNames;
	    else if (pe.xproperty.atom == net_workarea)
		dirty |= WorkArea;
	    else if (pe.xproperty.atom == net_number_of_desktops)
		dirty |= NumberOfDesktops;
	    else if (pe.xproperty.atom == net_desktop_geometry)
		dirty |= DesktopGeometry;
	    else if (pe.xproperty.atom == net_desktop_viewport)
		dirty |= DesktopViewport;
	    else if (pe.xproperty.atom == net_current_desktop)
		dirty |= CurrentDesktop;
	    else if (pe.xproperty.atom == net_active_window)
		dirty |= ActiveWindow;
	    else {

#ifdef    NETWMDEBUG
		fprintf(stderr, "NETRootInfo::event: putting back event and breaking\n");
#endif

		if ( compaction )
		    XPutBackEvent(p->display, &pe);
		break;
	    }

	    if (XCheckTypedWindowEvent(p->display, p->root, PropertyNotify, &pe) )
		compaction = True;
	    else
		break;
	}

	update(dirty & p->protocols);
    }

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETRootInfo::event: handled events, returning dirty = 0x%lx\n",
	    dirty & p->protocols);
#endif

    return dirty & p->protocols;
}


// private functions to update the data we keep

void NETRootInfo::update(unsigned long dirty) {
    Atom type_ret;
    int format_ret;
    unsigned char *data_ret;
    unsigned long nitems_ret, unused;

    dirty &= p->protocols;

    if (dirty & ClientList) {
	if (XGetWindowProperty(p->display, p->root, net_client_list,
			       0l, (long) BUFSIZE, False, XA_WINDOW, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_WINDOW && format_ret == 32) {
		Window *wins = (Window *) data_ret;

		qsort(wins, nitems_ret, sizeof(Window), wcmp);

		if (p->clients) {
		    if (role == Client) {
			unsigned long new_index = 0, old_index = 0;
			unsigned long new_count = nitems_ret,
				      old_count = p->clients_count;

			while (old_index < old_count || new_index < new_count) {
			    if (old_index == old_count) {
				addClient(wins[new_index++]);
			    } else if (new_index == new_count) {
				removeClient(p->clients[old_index++]);
			    } else {
				if (p->clients[old_index] <
				    wins[new_index]) {
				    removeClient(p->clients[old_index++]);
				} else if (wins[new_index] <
					   p->clients[old_index]) {
				    addClient(wins[new_index++]);
				} else {
				    new_index++;
				    old_index++;
				}
			    }
			}
		    }

		    delete [] p->clients;
		} else {
#ifdef    NETWMDEBUG
		    fprintf(stderr, "NETRootInfo::update: client list null, creating\n");
#endif

		    unsigned long n;
		    for (n = 0; n < nitems_ret; n++) {
			addClient(wins[n]);
		    }
		}

		p->clients_count = nitems_ret;
		p->clients = nwindup(wins, p->clients_count);
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::update: client list updated (%ld clients)\n",
		    p->clients_count);
#endif
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & KDESystemTrayWindows) {
	if (XGetWindowProperty(p->display, p->root, kde_net_system_tray_windows,
			       0l, (long) BUFSIZE, False, XA_WINDOW, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_WINDOW && format_ret == 32) {
		Window *wins = (Window *) data_ret;

		qsort(wins, nitems_ret, sizeof(Window), wcmp);

		if (p->kde_system_tray_windows) {
		    if (role == Client) {
			unsigned long new_index = 0, new_count = nitems_ret;
			unsigned long old_index = 0,
				      old_count = p->kde_system_tray_windows_count;

			while(old_index < old_count || new_index < new_count) {
			    if (old_index == old_count) {
				addSystemTrayWin(wins[new_index++]);
			    } else if (new_index == new_count) {
				removeSystemTrayWin(p->kde_system_tray_windows[old_index++]);
			    } else {
				if (p->kde_system_tray_windows[old_index] <
				    wins[new_index]) {
				    removeSystemTrayWin(p->kde_system_tray_windows[old_index++]);
				} else if (wins[new_index] <
					   p->kde_system_tray_windows[old_index]) {
				    addSystemTrayWin(wins[new_index++]);
				} else {
				    new_index++;
				    old_index++;
				}
			    }
			}
		    }

		} else {
		    unsigned long n;
		    for (n = 0; n < nitems_ret; n++) {
			addSystemTrayWin(wins[n]);
		    }
		}

		p->kde_system_tray_windows_count = nitems_ret;
		if (p->kde_system_tray_windows)
		    delete [] p->kde_system_tray_windows;
		p->kde_system_tray_windows =
		    nwindup(wins, p->kde_system_tray_windows_count);
	    }

	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & ClientListStacking) {
	if (XGetWindowProperty(p->display, p->root, net_client_list_stacking,
			       0, (long) BUFSIZE, False, XA_WINDOW, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_WINDOW && format_ret == 32) {
		Window *wins = (Window *) data_ret;

		if (p->stacking) {
		    delete [] p->stacking;
		}

		p->stacking_count = nitems_ret;
		p->stacking = nwindup(wins, p->stacking_count);
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr,"NETRootInfo::update: client stacking updated (%ld clients)\n",
		    p->stacking_count);
#endif

	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & NumberOfDesktops) {
	p->number_of_desktops = 0;

	if (XGetWindowProperty(p->display, p->root, net_number_of_desktops,
			       0l, 1l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 1) {
		p->number_of_desktops = *((long *) data_ret);
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::update: number of desktops = %d\n",
		    p->number_of_desktops);
#endif
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & DesktopGeometry) {
	if (XGetWindowProperty(p->display, p->root, net_desktop_geometry,
			       0l, 2l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret == 2) {
		long *data = (long *) data_ret;

		p->geometry.width  = data[0];
		p->geometry.height = data[1];

#ifdef    NETWMDEBUG
		fprintf(stderr, "NETRootInfo::update: desktop geometry updated\n");
#endif
	    }
	    if ( data_ret )
		XFree(data_ret);
	}
    } else {
	// insurance
	p->geometry = p->rootSize;
    }

    if (dirty & DesktopViewport) {
	if (XGetWindowProperty(p->display, p->root, net_desktop_viewport,
			       0l, 2l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret == 2) {
		long *data = (long *) data_ret;

		int d, i, n;
		n = nitems_ret / 2;
		for (d = 0, i = 0; d < n; d++) {
		    p->viewport[d].x = data[i++];
		    p->viewport[d].y = data[i++];
		}

#ifdef    NETWMDEBUG
		fprintf(stderr,
			"NETRootInfo::update: desktop viewport array updated (%d entries)\n",
			p->viewport.size());

		if (nitems_ret % 2 != 0) {
		    fprintf(stderr,
			    "NETRootInfo::update(): desktop viewport array "
			    "size not a multipe of 2\n");
		}
#endif
	    }
	    if ( data_ret )
		XFree(data_ret);
	}
    } else {
	int i;
	for (i = 0; i < p->viewport.size(); i++) {
	    p->viewport[i].x = p->viewport[i].y = 0;
	}
    }

    if (dirty & CurrentDesktop) {
	p->current_desktop = 0;
	if (XGetWindowProperty(p->display, p->root, net_current_desktop,
			       0l, 1l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 1) {
		p->current_desktop = *((long *) data_ret) + 1;
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::update: current desktop = %d\n",
		    p->current_desktop);
#endif
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & DesktopNames) {
	if (XGetWindowProperty(p->display, p->root, net_desktop_names,
			       0l, (long) BUFSIZE, False, UTF8_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == UTF8_STRING && format_ret == 8) {
		const char *d = (const char *) data_ret;
		unsigned int s, n, index;

		for (s = 0, n = 0, index = 0; n < nitems_ret; n++) {
		    if (d[n] == '\0') {
			if (p->desktop_names[index])
			    delete [] p->desktop_names[index];
			p->desktop_names[index++] = nstrndup((d + s), n - s + 1);
			s = n + 1;
		    }
		}
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::update: desktop names array updated (%d entries)\n",
		    p->desktop_names.size());
#endif
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & ActiveWindow) {
	if (XGetWindowProperty(p->display, p->root, net_active_window, 0l, 1l,
			       False, XA_WINDOW, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_WINDOW && format_ret == 32 && nitems_ret == 1) {
		p->active = *((Window *) data_ret);
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::update: active window = 0x%lx\n",
		    p->active);
#endif
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & WorkArea) {
	if (XGetWindowProperty(p->display, p->root, net_workarea, 0l,
			       (p->number_of_desktops * 4), False, XA_CARDINAL,
			       &type_ret, &format_ret, &nitems_ret, &unused,
			       &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret == (unsigned) (p->number_of_desktops * 4)) {
		long *d = (long *) data_ret;
		int i, j;
		for (i = 0, j = 0; i < p->number_of_desktops; i++) {
		    p->workarea[i].pos.x       = d[j++];
		    p->workarea[i].pos.y       = d[j++];
		    p->workarea[i].size.width  = d[j++];
		    p->workarea[i].size.height = d[j++];
		}
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::update: work area array updated (%d entries)\n",
		    p->workarea.size());
#endif
	    if ( data_ret )
		XFree(data_ret);
	}
    }


    if (dirty & SupportingWMCheck) {
	if (XGetWindowProperty(p->display, p->root, net_supporting_wm_check,
			       0l, 1l, False, XA_WINDOW, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_WINDOW && format_ret == 32 && nitems_ret == 1) {
		p->supportwindow = *((Window *) data_ret);

		unsigned char *name_ret;
		if (XGetWindowProperty(p->display, p->supportwindow,
				       net_wm_name, 0l, (long) BUFSIZE, False,
				       UTF8_STRING, &type_ret, &format_ret,
				       &nitems_ret, &unused, &name_ret)
		    == Success) {
		    if (type_ret == UTF8_STRING && format_ret == 8)
			p->name = nstrndup((const char *) name_ret, nitems_ret);

		    if ( name_ret )
			XFree(name_ret);
		}
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr,
		    "NETRootInfo::update: supporting window manager = '%s'\n",
		    p->name);
#endif
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & VirtualRoots) {
	if (XGetWindowProperty(p->display, p->root, net_virtual_roots,
			       0, (long) BUFSIZE, False, XA_WINDOW, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_WINDOW && format_ret == 32) {
		Window *wins = (Window *) data_ret;

		if (p->virtual_roots) {
		    delete [] p->virtual_roots;
		}

		p->virtual_roots_count = nitems_ret;
		p->virtual_roots = nwindup(wins, p->virtual_roots_count);
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::updated: virtual roots updated (%ld windows)\n",
		    p->virtual_roots_count);
#endif
	    if ( data_ret )
		XFree(data_ret);
	}
    }
}


Display *NETRootInfo::x11Display() const {
    return p->display;
}


Window NETRootInfo::rootWindow() const {
    return p->root;
}


Window NETRootInfo::supportWindow() const {
    return p->supportwindow;
}


const char *NETRootInfo::wmName() const {
    return p->name; }


int NETRootInfo::screenNumber() const {
    return p->screen;
}


unsigned long NETRootInfo::supported() const {
    return p->protocols;
}


const Window *NETRootInfo::clientList() const {
    return p->clients;
}


int NETRootInfo::clientListCount() const {
    return p->clients_count;
}


const Window *NETRootInfo::clientListStacking() const {
    return p->stacking;
}


int NETRootInfo::clientListStackingCount() const {
    return p->stacking_count;
}


const Window *NETRootInfo::kdeSystemTrayWindows() const {
    return p->kde_system_tray_windows;
}


int NETRootInfo::kdeSystemTrayWindowsCount() const {
    return p->kde_system_tray_windows_count;
}


NETSize NETRootInfo::desktopGeometry(int) const {
    return p->geometry;
}


NETPoint NETRootInfo::desktopViewport(int desktop) const {
    if (desktop < 1) {
	NETPoint pt;
	return pt;
    }

    return p->viewport[desktop - 1];
}


NETRect NETRootInfo::workArea(int desktop) const {
    if (desktop < 1) {
	NETRect rt;
	return rt;
    }

    return p->workarea[desktop - 1];
}


const char *NETRootInfo::desktopName(int desktop) const {
    if (desktop < 1) {
	return 0;
    }

    return p->desktop_names[desktop - 1];
}


const Window *NETRootInfo::virtualRoots( ) const {
    return p->virtual_roots;
}


int NETRootInfo::virtualRootsCount() const {
    return p->virtual_roots_count;
}


int NETRootInfo::numberOfDesktops() const {
  // XXX rikkus: correct fix ?
    return p->number_of_desktops == 0 ? 1 : p->number_of_desktops;
}


int NETRootInfo::currentDesktop() const {
    return p->current_desktop;
}


Window NETRootInfo::activeWindow() const {
    return p->active;
}


// NETWinInfo stuffs

const int NETWinInfo::OnAllDesktops = (int) -1;

NETWinInfo::NETWinInfo(Display *display, Window window, Window rootWindow,
		       unsigned long properties, Role role)
{

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETWinInfo::NETWinInfo: constructing object with role '%s'\n",
	    (role == WindowManager) ? "WindowManager" : "Client");
#endif

    p = new NETWinInfoPrivate;
    p->ref = 1;

    p->display = display;
    p->window = window;
    p->root = rootWindow;
    p->mapping_state = Withdrawn;
    p->mapping_state_dirty = True;
    p->state = 0;
    p->type = Unknown;
    p->name = (char *) 0;
    p->visible_name = (char *) 0;
    p->icon_name = (char *) 0;
    p->visible_icon_name = (char *) 0;
    p->desktop = p->pid = p->handled_icons = 0;

    // p->strut.left = p->strut.right = p->strut.top = p->strut.bottom = 0;
    // p->frame_strut.left = p->frame_strut.right = p->frame_strut.top =
    // p->frame_strut.bottom = 0;

    p->kde_system_tray_win_for = 0;

    p->properties = properties;
    p->icon_count = 0;

    this->role = role;

    if (! netwm_atoms_created) create_atoms(p->display);

    if (p->properties) update(p->properties);
}


NETWinInfo::NETWinInfo(const NETWinInfo &wininfo) {
    p = wininfo.p;
    p->ref++;
}


NETWinInfo::~NETWinInfo() {
    refdec_nwi(p);

    if (! p->ref) delete p;
}


void NETWinInfo::setIcon(NETIcon icon, Bool replace) {
    if (role != Client) return;

    int proplen, i, sz, j;

    if (replace) {

	for (i = 0; i < p->icons.size(); i++) {
	    if (p->icons[i].data) delete [] p->icons[i].data;
	    p->icons[i].data = 0;
	    p->icons[i].size.width = 0;
	    p->icons[i].size.height = 0;
	}

	p->icon_count = 0;
    }

    // assign icon
    p->icons[p->icon_count] = icon;
    p->icon_count++;

    // do a deep copy, we want to own the data
    NETIcon &ni = p->icons[p->icon_count - 1];
    sz = ni.size.width * ni.size.height;
    CARD32 *d = new CARD32[sz];
    ni.data = (unsigned char *) d;
    memcpy(d, icon.data, sz * sizeof(CARD32));

    // compute property length
    for (i = 0, proplen = 0; i < p->icon_count; i++) {
	proplen += 2 + (p->icons[i].size.width *
			p->icons[i].size.height);
    }

    CARD32 *d32;
    long *prop = new long[proplen], *pprop = prop;
    for (i = 0; i < p->icon_count; i++) {
	// copy size into property
       	*pprop++ = p->icons[i].size.width;
	*pprop++ = p->icons[i].size.height;

	// copy data into property
	sz = (p->icons[i].size.width * p->icons[i].size.height);
	d32 = (CARD32 *) p->icons[i].data;
	for (j = 0; j < sz; j++) *pprop++ = *d32++;
    }

    XChangeProperty(p->display, p->window, net_wm_icon, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) prop, proplen);

    delete [] prop;
}


void NETWinInfo::setIconGeometry(NETRect geometry) {
    if (role != Client) return;

    p->icon_geom = geometry;

    long data[4];
    data[0] = geometry.pos.x;
    data[1] = geometry.pos.y;
    data[2] = geometry.size.width;
    data[3] = geometry.size.height;

    XChangeProperty(p->display, p->window, net_wm_icon_geometry, XA_CARDINAL,
		    32, PropModeReplace, (unsigned char *) data, 4);
}


void NETWinInfo::setStrut(NETStrut strut) {
    if (role != Client) return;

    p->strut = strut;

    long data[4];
    data[0] = strut.left;
    data[1] = strut.right;
    data[2] = strut.top;
    data[3] = strut.bottom;

    XChangeProperty(p->display, p->window, net_wm_strut, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) data, 4);
}


void NETWinInfo::setState(unsigned long state, unsigned long mask) {
    if (p->mapping_state_dirty)
	update(XAWMState);

    if (role == Client && p->mapping_state != Withdrawn) {

#ifdef NETWMDEBUG
        fprintf(stderr, "NETWinInfo::setState (0x%lx, 0x%lx) (Client)\n",
                state, mask);
#endif // NETWMDEBUG

	XEvent e;
	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_wm_state;
	e.xclient.display = p->display;
	e.xclient.window = p->window;
	e.xclient.format = 32;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	if ((mask & Modal) && ((p->state & Modal) != (state & Modal))) {
	    e.xclient.data.l[0] = (state & Modal) ? 1 : 0;
	    e.xclient.data.l[1] = net_wm_state_modal;
	    e.xclient.data.l[2] = 0l;

	    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
	}

	if ((mask & Sticky) && ((p->state & Sticky) != (state & Sticky))) {
	    e.xclient.data.l[0] = (state & Sticky) ? 1 : 0;
	    e.xclient.data.l[1] = net_wm_state_sticky;
	    e.xclient.data.l[2] = 0l;

	    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
	}

	if ((mask & Max) && (( (p->state&mask) & Max) != (state & Max))) {

	    unsigned long wishstate = (p->state & ~mask) | (state & mask);
	    if ( ( (wishstate & MaxHoriz) != (p->state & MaxHoriz) )
		 && ( (wishstate & MaxVert) != (p->state & MaxVert) ) ) {
		if ( (wishstate & Max) == Max ) {
		    e.xclient.data.l[0] = 1;
		    e.xclient.data.l[1] = net_wm_state_max_horiz;
		    e.xclient.data.l[2] = net_wm_state_max_vert;
		    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
		} else if ( (wishstate & Max) == 0 ) {
		    e.xclient.data.l[0] = 0;
		    e.xclient.data.l[1] = net_wm_state_max_horiz;
		    e.xclient.data.l[2] = net_wm_state_max_vert;
		    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
		} else {
		    e.xclient.data.l[0] = ( wishstate & MaxHoriz ) ? 1 : 0;
		    e.xclient.data.l[1] = net_wm_state_max_horiz;
		    e.xclient.data.l[2] = 0;
		    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
		    e.xclient.data.l[0] = ( wishstate & MaxVert ) ? 1 : 0;
		    e.xclient.data.l[1] = net_wm_state_max_vert;
		    e.xclient.data.l[2] = 0;
		    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
		}
	    } else	if ( (wishstate & MaxVert) != (p->state & MaxVert) ) {
		e.xclient.data.l[0] = ( wishstate & MaxVert ) ? 1 : 0;
		e.xclient.data.l[1] = net_wm_state_max_vert;
		e.xclient.data.l[2] = 0;
		XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
	    } else if ( (wishstate & MaxHoriz) != (p->state & MaxHoriz) ) {
		e.xclient.data.l[0] = ( wishstate & MaxHoriz ) ? 1 : 0;
		e.xclient.data.l[1] = net_wm_state_max_horiz;
		e.xclient.data.l[2] = 0;
		XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
	    }
	}

	if ((mask & Shaded) && ((p->state & Shaded) != (state & Shaded))) {
	    e.xclient.data.l[0] = (state & Shaded) ? 1 : 0;
	    e.xclient.data.l[1] = net_wm_state_shaded;
	    e.xclient.data.l[2] = 0l;

	    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
	}

	if ((mask & SkipTaskbar) &&
	    ((p->state & SkipTaskbar) != (state & SkipTaskbar))) {
	    e.xclient.data.l[0] = (state & SkipTaskbar) ? 1 : 0;
	    e.xclient.data.l[1] = net_wm_state_skip_taskbar;
	    e.xclient.data.l[2] = 0l;

	    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
	}

        if ((mask & SkipPager) &&
	    ((p->state & SkipPager) != (state & SkipPager))) {
            e.xclient.data.l[0] = (state & SkipPager) ? 1 : 0;
            e.xclient.data.l[1] = net_wm_state_skip_pager;
            e.xclient.data.l[2] = 0l;

            XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
        }

	if ((mask & StaysOnTop) && ((p->state & StaysOnTop) != (state & StaysOnTop))) {
	    e.xclient.data.l[0] = (state & StaysOnTop) ? 1 : 0;
	    e.xclient.data.l[1] = net_wm_state_stays_on_top;
	    e.xclient.data.l[2] = 0l;

	    XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
	}
    } else {
	p->state &= ~mask;
	p->state |= state;

	long data[8];
	int count = 0;

	// hints
	if (p->state & Modal) data[count++] = net_wm_state_modal;
	if (p->state & MaxVert) data[count++] = net_wm_state_max_vert;
	if (p->state & MaxHoriz) data[count++] = net_wm_state_max_horiz;
	if (p->state & Shaded) data[count++] = net_wm_state_shaded;

	// policy
	if (p->state & StaysOnTop) data[count++] = net_wm_state_stays_on_top;
	if (p->state & Sticky) data[count++] = net_wm_state_sticky;
	if (p->state & SkipTaskbar) data[count++] = net_wm_state_skip_taskbar;
	if (p->state & SkipPager) data[count++] = net_wm_state_skip_pager;

#ifdef NETWMDEBUG
	fprintf(stderr, "NETWinInfo::setState: setting state property (%d)\n", count);
	for (int i = 0; i < count; i++)
	    fprintf(stderr, "NETWinInfo::setState:   state %ld '%s'\n",
		    data[i], XGetAtomName(p->display, (Atom) data[i]));
#endif

	XChangeProperty(p->display, p->window, net_wm_state, XA_ATOM, 32,
			PropModeReplace, (unsigned char *) data, count);
    }
}


void NETWinInfo::setWindowType(WindowType type) {
    if (role != Client) return;

    int len;
    long data[2];

    switch (type) {
    case Override:
	// spec extension: override window type.  we must comply with the spec
	// and provide a fall back (normal seems best)
	data[0] = kde_net_wm_window_type_override;
	data[1] = net_wm_window_type_normal;
	len = 2;
	break;

    case  Dialog:
	data[0] = net_wm_window_type_dialog;
	data[1] = None;
	len = 1;
	break;

    case Menu:
	data[0] = net_wm_window_type_menu;
	data[1] = None;
	len = 1;
	break;

    case TopMenu:
	// spec extension: override window type.  we must comply with the spec
	// and provide a fall back (dock seems best)
	data[0] = kde_net_wm_window_type_topmenu;
	data[1] = net_wm_window_type_dock;
	len = 2;
	break;

    case Tool:
	data[0] = net_wm_window_type_toolbar;
	data[1] = None;
	len = 1;
	break;

    case Dock:
	data[0] = net_wm_window_type_dock;
	data[1] = None;
	len = 1;
	break;

    case Desktop:
	data[0] = net_wm_window_type_desktop;
	data[1] = None;
	len = 1;
	break;

    default:
    case Normal:
	data[0] = net_wm_window_type_normal;
	data[1] = None;
	len = 1;
	break;
    }

    XChangeProperty(p->display, p->window, net_wm_window_type, XA_ATOM, 32,
		    PropModeReplace, (unsigned char *) &data, len);
}


void NETWinInfo::setName(const char *name) {
    if (role != Client) return;

    if (p->name) delete [] p->name;
    p->name = nstrdup(name);
    XChangeProperty(p->display, p->window, net_wm_name, UTF8_STRING, 8,
		    PropModeReplace, (unsigned char *) p->name,
		    strlen(p->name));
}


void NETWinInfo::setVisibleName(const char *visibleName) {
    if (role != WindowManager) return;

    if (p->visible_name) delete [] p->visible_name;
    p->visible_name = nstrdup(visibleName);
    XChangeProperty(p->display, p->window, net_wm_visible_name, UTF8_STRING, 8,
		    PropModeReplace, (unsigned char *) p->visible_name,
		    strlen(p->visible_name));
}


void NETWinInfo::setIconName(const char *iconName) {
    if (role != Client) return;

    if (p->icon_name) delete [] p->icon_name;
    p->icon_name = nstrdup(iconName);
    XChangeProperty(p->display, p->window, net_wm_icon_name, UTF8_STRING, 8,
		    PropModeReplace, (unsigned char *) p->icon_name,
		    strlen(p->icon_name));
}


void NETWinInfo::setVisibleIconName(const char *visibleIconName) {
    if (role != WindowManager) return;

    if (p->visible_icon_name) delete [] p->visible_icon_name;
    p->visible_icon_name = nstrdup(visibleIconName);
    XChangeProperty(p->display, p->window, net_wm_visible_icon_name, UTF8_STRING, 8,
		    PropModeReplace, (unsigned char *) p->visible_icon_name,
		    strlen(p->visible_icon_name));
}


void NETWinInfo::setDesktop(int desktop) {
    if (p->mapping_state_dirty)
	update(XAWMState);

    if (role == Client && p->mapping_state != Withdrawn) {
	// we only send a ClientMessage if we are 1) a client and 2) managed

	if ( desktop == 0 )
	    return; // we can't do that while being managed

	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_wm_desktop;
	e.xclient.display = p->display;
	e.xclient.window = p->window;
	e.xclient.format = 32;
	e.xclient.data.l[0] = desktop - 1;
	e.xclient.data.l[1] = 0l;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, netwm_sendevent_mask, &e);
    } else {
	// otherwise we just set or remove the property directly
	p->desktop = desktop;
	long d = desktop;

	if ( d != OnAllDesktops ) {
	    if ( d == 0 ) {
		XDeleteProperty( p->display, p->window, net_wm_desktop );
		return;
	    }

	    d -= 1;
	}

	XChangeProperty(p->display, p->window, net_wm_desktop, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &d, 1);
    }
}


void NETWinInfo::setPid(int pid) {
    if (role != Client) return;

    p->pid = pid;
    long d = pid;
    XChangeProperty(p->display, p->window, net_wm_pid, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &d, 1);
}


void NETWinInfo::setHandledIcons(Bool handled) {
    if (role != Client) return;

    p->handled_icons = handled;
    long d = handled;
    XChangeProperty(p->display, p->window, net_wm_handled_icons, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &d, 1);
}


void NETWinInfo::setKDESystemTrayWinFor(Window window) {
    if (role != Client) return;

    p->kde_system_tray_win_for = window;
    XChangeProperty(p->display, p->window, kde_net_wm_system_tray_window_for,
		    XA_WINDOW, 32, PropModeReplace,
		    (unsigned char *) &(p->kde_system_tray_win_for), 1);
}


void NETWinInfo::setKDEFrameStrut(NETStrut strut) {
    if (role != WindowManager) return;

    p->frame_strut = strut;

    long d[4];
    d[0] = strut.left;
    d[1] = strut.right;
    d[2] = strut.top;
    d[3] = strut.bottom;

    XChangeProperty(p->display, p->window, kde_net_wm_frame_strut, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) d, 4);
}


void NETWinInfo::kdeGeometry(NETRect& frame, NETRect& window) {
    if (p->win_geom.size.width == 0 || p->win_geom.size.height == 0) {
	Window unused;
	int x, y;
	unsigned int w, h, junk;
	XGetGeometry(p->display, p->window, &unused, &x, &y, &w, &h, &junk, &junk);
	XTranslateCoordinates(p->display, p->window, p->root, 0, 0, &x, &y, &unused
			      );

	p->win_geom.pos.x = x;
	p->win_geom.pos.y = y;

	p->win_geom.size.width = w;
	p->win_geom.size.height = h;
    }

    window = p->win_geom;

    frame.pos.x = window.pos.x - p->frame_strut.left;
    frame.pos.y = window.pos.y - p->frame_strut.top;
    frame.size.width = window.size.width + p->frame_strut.left + p->frame_strut.right;
    frame.size.height = window.size.height + p->frame_strut.top + p->frame_strut.bottom;
}


NETIcon NETWinInfo::icon(int width, int height) const {
    NETIcon result;

    if ( !p->icons.size() ) {
	result.size.width = 0;
	result.size.height = 0;
	result.data = 0;
	return result;
    }

    result = p->icons[0];

    // find the icon that's closest in size to w x h...
    // return the first icon if w and h are -1
    if (width == height && height == -1) return result;

    int i;
    for (i = 0; i < p->icons.size(); i++) {
	if ((p->icons[i].size.width >= width &&
	     p->icons[i].size.width < result.size.width) &&
	    (p->icons[i].size.height >= height &&
	     p->icons[i].size.height < result.size.height))
	    result = p->icons[i];
    }

    return result;
}


unsigned long NETWinInfo::event(XEvent *event) {
    unsigned long dirty = 0;

    if (role == WindowManager && event->type == ClientMessage &&
	event->xclient.format == 32) {

#ifdef NETWMDEBUG
        fprintf(stderr, "NETWinInfo::event: handling ClientMessage event\n");
#endif // NETWMDEBUG

	if (event->xclient.message_type == net_wm_state) {
	    dirty = WMState;

	    // we need to generate a change mask

#ifdef NETWMDEBUG
	    fprintf(stderr,
		    "NETWinInfo::event: state client message, getting new state/mask\n");
#endif

	    int i;
	    long state = 0, mask = 0;

	    for (i = 1; i < 3; i++) {
#ifdef NETWMDEBUG
		fprintf(stderr, "NETWinInfo::event:  message %ld '%s'\n",
			event->xclient.data.l[i],
			XGetAtomName(p->display, (Atom) event->xclient.data.l[i]));
#endif

		if ((Atom) event->xclient.data.l[i] == net_wm_state_modal)
		    mask |= Modal;
		else if ((Atom) event->xclient.data.l[i] == net_wm_state_sticky)
		    mask |= Sticky;
		else if ((Atom) event->xclient.data.l[i] == net_wm_state_max_vert)
		    mask |= MaxVert;
		else if ((Atom) event->xclient.data.l[i] == net_wm_state_max_horiz)
		    mask |= MaxHoriz;
		else if ((Atom) event->xclient.data.l[i] == net_wm_state_shaded)
		    mask |= Shaded;
		else if ((Atom) event->xclient.data.l[i] == net_wm_state_skip_taskbar)
		    mask |= SkipTaskbar;
                else if ((Atom) event->xclient.data.l[i] == net_wm_state_skip_pager)
		    mask |= SkipPager;
		else if ((Atom) event->xclient.data.l[i] == net_wm_state_stays_on_top)
		    mask |= StaysOnTop;
	    }

	    // when removing, we just leave newstate == 0
	    switch (event->xclient.data.l[0]) {
	    case 1: // set
		// to set... the change state should be the same as the mask
		state = mask;
		break;

	    case 2: // toggle
		// to toggle, we need to xor the current state with the new state
		state = (p->state & mask) ^ mask;
		break;

	    default:
		// to clear state, the new state should stay zero
		;
	    }

#ifdef NETWMDEBUG
	    fprintf(stderr, "NETWinInfo::event: calling changeState(%lx, %lx)\n",
		    state, mask);
#endif

	    changeState(state, mask);
	} else if (event->xclient.message_type == net_wm_desktop) {
	    dirty = WMDesktop;

	    if( event->xclient.data.l[0] == OnAllDesktops )
		changeDesktop( OnAllDesktops );
	    else
    		changeDesktop(event->xclient.data.l[0] + 1);
	}
    }

    if (event->type == PropertyNotify) {

#ifdef    NETWMDEBUG
	fprintf(stderr, "NETWinInfo::event: handling PropertyNotify event\n");
#endif

	XEvent pe = *event;

	Bool done = False;
	Bool compaction = False;
	while (! done) {

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETWinInfo::event: loop fire\n");
#endif

	    if (pe.xproperty.atom == net_wm_name)
		dirty |= WMName;
	    else if (pe.xproperty.atom == net_wm_visible_name)
		dirty |= WMVisibleName;
	    else if (pe.xproperty.atom == net_wm_window_type)
		dirty |=WMWindowType;
	    else if (pe.xproperty.atom == net_wm_strut)
		dirty |= WMStrut;
	    else if (pe.xproperty.atom == net_wm_icon_geometry)
		dirty |= WMIconGeometry;
	    else if (pe.xproperty.atom == net_wm_icon)
		dirty |= WMIcon;
	    else if (pe.xproperty.atom == xa_wm_state)
		dirty |= XAWMState;
	    else if (pe.xproperty.atom == net_wm_state)
		dirty |= WMState;
	    else if (pe.xproperty.atom == net_wm_desktop)
		dirty |= WMDesktop;
	    else if (pe.xproperty.atom == kde_net_wm_frame_strut)
		dirty |= WMKDEFrameStrut;
	    else if (pe.xproperty.atom == kde_net_wm_system_tray_window_for)
		dirty |= WMKDESystemTrayWinFor;
	    else {

#ifdef    NETWMDEBUG
		fprintf(stderr, "NETWinInfo::event: putting back event and breaking\n");
#endif

		if ( compaction )
		    XPutBackEvent(p->display, &pe);
		break;
	    }

	    if (XCheckTypedWindowEvent(p->display, p->window, PropertyNotify, &pe) )
		compaction = True;
	    else
		break;
	}

	update(dirty);
    } else if (event->type == ConfigureNotify) {

#ifdef NETWMDEBUG
	fprintf(stderr, "NETWinInfo::event: handling ConfigureNotify event\n");
#endif

	dirty |= WMGeometry;

	// update window geometry
	p->win_geom.pos.x = event->xconfigure.x;
	p->win_geom.pos.y = event->xconfigure.y;
	p->win_geom.size.width = event->xconfigure.width;
	p->win_geom.size.height = event->xconfigure.height;
    }

    return dirty;
}


void NETWinInfo::update(unsigned long dirty) {
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret, unused;
    unsigned char *data_ret;

    if (dirty & XAWMState) {
	if (XGetWindowProperty(p->display, p->window, xa_wm_state, 0l, 1l,
			       False, xa_wm_state, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == xa_wm_state && format_ret == 32 &&
		nitems_ret == 1) {
		long *state = (long *) data_ret;

		switch(*state) {
		    case IconicState:
			p->mapping_state = Iconic;
			break;
		    case WithdrawnState:
			p->mapping_state = Withdrawn;
			break;
		    case NormalState:
		    default:
			p->mapping_state = Visible;

		}

		p->mapping_state_dirty = False;
	    }
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    // we do this here because we *always* want to update WM_STATE
    dirty &= p->properties;

    if (dirty & WMState) {
	p->state = 0;
	if (XGetWindowProperty(p->display, p->window, net_wm_state, 0l, 2048l,
			       False, XA_ATOM, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_ATOM && format_ret == 32 && nitems_ret > 0) {
		// determine window state
#ifdef NETWMDEBUG
		fprintf(stderr, "NETWinInfo::update: updating window state (%ld)\n",
			nitems_ret);
#endif

		long *states = (long *) data_ret;
		unsigned long count;

		for (count = 0; count < nitems_ret; count++) {
#ifdef NETWMDEBUG
		    fprintf(stderr,
			    "NETWinInfo::update:   adding window state %ld '%s'\n",
			    states[count],
			    XGetAtomName(p->display, (Atom) states[count]));
#endif

		    if ((Atom) states[count] == net_wm_state_modal)
			p->state |= Modal;
		    else if ((Atom) states[count] == net_wm_state_sticky)
			p->state |= Sticky;
		    else if ((Atom) states[count] == net_wm_state_max_vert)
			p->state |= MaxVert;
		    else if ((Atom) states[count] == net_wm_state_max_horiz)
			p->state |= MaxHoriz;
		    else if ((Atom) states[count] == net_wm_state_shaded)
			p->state |= Shaded;
		    else if ((Atom) states[count] == net_wm_state_skip_taskbar)
			p->state |= SkipTaskbar;
		    else if ((Atom) states[count] == net_wm_state_skip_pager)
			p->state |= SkipPager;
		    else if ((Atom) states[count] == net_wm_state_stays_on_top)
			p->state |= StaysOnTop;
		}
	    }
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & WMDesktop) {
	p->desktop = 0;
	if (XGetWindowProperty(p->display, p->window, net_wm_desktop, 0l, 1l,
			       False, XA_CARDINAL, &type_ret,
			       &format_ret, &nitems_ret,
			       &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret == 1) {
		p->desktop = *((long *) data_ret);
		if ((signed) p->desktop != OnAllDesktops)
		    p->desktop++;

		if ( p->desktop == 0 )
		    p->desktop = OnAllDesktops;
	    }
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & WMName) {
	if (XGetWindowProperty(p->display, p->window, net_wm_name, 0l,
			       (long) BUFSIZE, False, UTF8_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == UTF8_STRING && format_ret == 8 && nitems_ret > 0) {
		if (p->name) delete [] p->name;
		p->name = nstrndup((const char *) data_ret, nitems_ret);
	    }

	    if( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & WMVisibleName) {
	if (XGetWindowProperty(p->display, p->window, net_wm_visible_name, 0l,
			       (long) BUFSIZE, False, UTF8_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == UTF8_STRING && format_ret == 8 && nitems_ret > 0) {
		if (p->visible_name) delete [] p->visible_name;
		p->visible_name = nstrndup((const char *) data_ret, nitems_ret);
	    }

	    if( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & WMIconName) {

	char* text_ret = 0;
	if (XGetWindowProperty(p->display, p->window, net_wm_icon_name, 0l,
			       (long) BUFSIZE, False, UTF8_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == UTF8_STRING && format_ret == 8 && nitems_ret > 0) {
		if (p->icon_name) delete [] p->icon_name;
		p->icon_name = nstrndup((const char *) data_ret, nitems_ret);
	    }

	    if( data_ret )
		XFree(data_ret);
	}

	if ( !p->visible_icon_name &&  XGetIconName(p->display, p->window, &text_ret) ) {
	    if (p->icon_name) delete [] p->icon_name;
	    p->icon_name = strdup((const char *) text_ret);

	    if( text_ret )
		XFree(text_ret);
	}
    }

    if (dirty & WMVisibleIconName)
    {
	char* text_ret = 0;
	if (XGetWindowProperty(p->display, p->window, net_wm_visible_icon_name, 0l,
			       (long) BUFSIZE, False, UTF8_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == UTF8_STRING && format_ret == 8 && nitems_ret > 0) {
		if (p->visible_icon_name) delete [] p->visible_icon_name;
		p->visible_icon_name = nstrndup((const char *) data_ret, nitems_ret);
	    }

	    if( data_ret )
		XFree(data_ret);
	}


	if ( !p->visible_icon_name && XGetIconName(p->display, p->window, &text_ret) ) {
	    if (p->visible_icon_name) delete [] p->visible_icon_name;
	    p->visible_icon_name = strdup((const char *) text_ret);

	    if( text_ret )
		XFree(text_ret);
	}
    }

    if (dirty & WMWindowType) {
	p->type = Unknown;
	if (XGetWindowProperty(p->display, p->window, net_wm_window_type, 0l, 2048l,
			       False, XA_ATOM, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_ATOM && format_ret == 32 && nitems_ret > 0) {
		// determine the window type
#ifdef NETWMDEBUG
		fprintf(stderr, "NETWinInfo::update: getting window type (%ld)\n",
			nitems_ret);
#endif

		unsigned long count = 0;
		long *types = (long *) data_ret;

		while (p->type == Unknown && count < nitems_ret) {
		    // check the types for the types we know about... types[count] is
		    // not known, p->type is unchanged (Unknown)

#ifdef NETWMDEBUG
		    fprintf(stderr,
			    "NETWinInfo::update:   examining window type %ld %s\n",
			    types[count],
			    XGetAtomName(p->display, (Atom) types[count]));
#endif

		    if ((Atom) types[count] == net_wm_window_type_normal)
			p->type = Normal;
		    else if ((Atom) types[count] == net_wm_window_type_desktop)
			p->type = Desktop;
		    else if ((Atom) types[count] == net_wm_window_type_dock)
			p->type = Dock;
		    else if ((Atom) types[count] == net_wm_window_type_toolbar)
			p->type = Tool;
		    else if ((Atom) types[count] == net_wm_window_type_menu)
			p->type = Menu;
		    else if ((Atom) types[count] == net_wm_window_type_dialog)
			p->type = Dialog;
		    else if ((Atom) types[count] == kde_net_wm_window_type_override)
			p->type = Override;
		    else if ((Atom) types[count] == kde_net_wm_window_type_topmenu)
			p->type = TopMenu;

		    count++;
		}
	    }

	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & WMStrut) {
	if (XGetWindowProperty(p->display, p->window, net_wm_strut, 0l, 4l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret == 4) {
		long *d = (long *) data_ret;
		p->strut.left   = d[0];
		p->strut.right  = d[1];
		p->strut.top    = d[2];
		p->strut.bottom = d[3];
	    }
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & WMIconGeometry) {
	if (XGetWindowProperty(p->display, p->window, net_wm_icon_geometry, 0l, 4l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret == 4) {
		long *d = (long *) data_ret;
		p->icon_geom.pos.x       = d[0];
		p->icon_geom.pos.y       = d[1];
		p->icon_geom.size.width  = d[2];
		p->icon_geom.size.height = d[3];
	    }
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & WMIcon) {
	readIcon(p);
    }

    if (dirty & WMKDESystemTrayWinFor) {
	p->kde_system_tray_win_for = 0;
	if (XGetWindowProperty(p->display, p->window, kde_net_wm_system_tray_window_for,
			       0l, 1l, False, XA_WINDOW, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_WINDOW && format_ret == 32 &&
		nitems_ret == 1) {
		p->kde_system_tray_win_for = *((Window *) data_ret);
		if ( p->kde_system_tray_win_for == 0 )
		    p->kde_system_tray_win_for = p->root;
	    }
	    if ( data_ret )
		XFree(data_ret);
        }
    }

    if (dirty & WMKDEFrameStrut) {
	if (XGetWindowProperty(p->display, p->window, kde_net_wm_frame_strut,
			       0l, 4l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret) == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 4) {
		long *d = (long *) data_ret;

		p->frame_strut.left   = d[0];
		p->frame_strut.right  = d[1];
		p->frame_strut.top    = d[2];
		p->frame_strut.bottom = d[3];
	    }
	    if ( data_ret )
		XFree(data_ret);
	}
    }

    if (dirty & WMPid) {
	p->pid = 0;
	if (XGetWindowProperty(p->display, p->window, net_wm_pid, 0l, 1l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret) == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 1) {
		p->pid = *((long *) data_ret);
	    }
	    if ( data_ret )
		XFree(data_ret);
	}
    }
}


NETRect NETWinInfo::iconGeometry() const {
    return p->icon_geom;
}


unsigned long NETWinInfo::state() const {
    return p->state;
}


NETStrut NETWinInfo::strut() const {
    return p->strut;
}


NET::WindowType NETWinInfo::windowType() const {
    return p->type;
}


const char *NETWinInfo::name() const {
    return p->name;
}


const char *NETWinInfo::visibleName() const {
    return p->visible_name;
}


const char *NETWinInfo::iconName() const {
    return p->icon_name;
}


const char *NETWinInfo::visibleIconName() const {
    return p->visible_icon_name;
}


int NETWinInfo::desktop() const {
    return p->desktop;
}

int NETWinInfo::pid() const {
    return p->pid;
}


Bool NETWinInfo::handledIcons() const {
    return p->handled_icons;
}


Window NETWinInfo::kdeSystemTrayWinFor() const {
    return p->kde_system_tray_win_for;
}


unsigned long NETWinInfo::properties() const {
    return p->properties;
}


NET::MappingState NETWinInfo::mappingState() const {
    return p->mapping_state;
}

void NETRootInfo::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void NETWinInfo::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#endif
