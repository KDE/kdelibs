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

#undef NETWMDEBUG

#include "netwm.h"

#include <string.h>
#include <stdio.h>

#include <X11/Xlibint.h>
#include <X11/Xmd.h>

#include "netwm_p.h"


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
static Atom net_kde_system_tray_windows  = 0;

// root window messages
static Atom net_close_window         = 0;
static Atom net_wm_moveresize        = 0;

// application window properties
static Atom net_wm_name              = 0;
static Atom net_wm_visible_name      = 0;
static Atom net_wm_desktop           = 0;
static Atom net_wm_window_type       = 0;
static Atom net_wm_state             = 0;
static Atom net_wm_strut             = 0;
static Atom net_wm_icon_geometry     = 0;
static Atom net_wm_icon              = 0;
static Atom net_wm_pid               = 0;
static Atom net_wm_handled_icons     = 0;
static Atom net_wm_kde_system_tray_window_for = 0;
static Atom net_wm_kde_frame_strut   = 0;

// application protocols
static Atom net_wm_ping              = 0;

// used to determine whether application window is managed or not
static Atom xa_wm_state              = 0;

static Bool netwm_atoms_created      = False;


static char *nstrdup(const char *s1) {
    if (! s1) return (char *) 0;

    int l = strlen(s1) + 1;
    char *s2 = new char[l];
    strncpy(s2, s1, l);
    return s2;
}


static char *nstrndup(const char *s1, int l) {
    if (! s1 || l == 0) return (char *) 0;

    char *s2 = new char[l];
    strncpy(s2, s1, l);
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

	int i;
	for (i = 0; i < p->icons.size(); i++)
	    if (p->icons[i].data) delete [] p->icons[i].data;
    }
}


static int wcmp(const void *a, const void *b) {
    return *((Window *) a) - *((Window *) b);
}


static void create_atoms(Display *d) {
    static const char *names[29] =
    {
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
	    "_NET_WM_DESKTOP",
	    "_NET_WM_WINDOW_TYPE",
	    "_NET_WM_STATE",
	    "_NET_WM_STRUT",
	    "_NET_WM_ICON_GEOMETRY",
	    "_NET_WM_ICON",
	    "_NET_WM_PID",
	    "_NET_WM_HANDLED_ICONS",
	    "_NET_WM_PING",
	    "_NET_KDE_SYSTEM_TRAY_WINDOWS",
	    "_NET_KDE_SYSTEM_TRAY_WINDOW_FOR",
	    "WM_STATE",
	    "_NET_WM_KDE_FRAME_STRUT"
	    };

    Atom atoms[29], *atomsp[29] =
    {
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
	    &net_wm_desktop,
	    &net_wm_window_type,
	    &net_wm_state,
	    &net_wm_strut,
	    &net_wm_icon_geometry,
	    &net_wm_icon,
	    &net_wm_pid,
	    &net_wm_handled_icons,
	    &net_wm_ping,
	    &net_kde_system_tray_windows,
	    &net_wm_kde_system_tray_window_for,
	    &xa_wm_state,
	    &net_wm_kde_frame_strut
	    };

    int i = 29;
    while (i--)
	atoms[i] = 0;

    XInternAtoms(d, (char **) names, 29, False, atoms);

    i = 29;
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

    int ret =
	XGetWindowProperty(p->display, p->window, net_wm_icon, 0l, 3l, False,
			   XA_CARDINAL, &type_ret, &format_ret, &nitems_ret,
			   &after_ret, &data_ret);

    if (data_ret) XFree(data_ret);

    if (ret != Success || nitems_ret < 3 || type_ret != XA_CARDINAL ||
	format_ret != 32) {
	// either we didn't get the property, or the property has less than
	// 3 elements in it
	// NOTE: 3 is the ABSOLUTE minimum:
	//     width = 1, height = 1, length(data) = 1 (width * height)
	return;
    }

    // allocate space after_ret (bytes remaining in property) + 4
    // (the single 32bit quantity we just read)
    unsigned long proplen = after_ret + 12;
    unsigned char *buffer = new unsigned char[proplen];
    unsigned long offset = 0, buffer_offset = 0;

    while (after_ret >0) {
	XGetWindowProperty(p->display, p->window, net_wm_icon, offset,
			   (long) BUFSIZE, False, XA_CARDINAL, &type_ret,
			   &format_ret, &nitems_ret, &after_ret, &data_ret);
	memcpy((buffer + buffer_offset), data_ret, nitems_ret * 4);
	buffer_offset += nitems_ret * 4;
 	offset += nitems_ret;
	XFree(data_ret);
    }

    unsigned long i, j;
    CARD32 *d = (CARD32 *) buffer;
    for (i = 0, j = 0; i < proplen - 3; i++) {
	p->icons[j].size.width = *d++;
	i += 4;
	p->icons[j].size.height = *d++;
	i += 4;

	unsigned long s = (p->icons[j].size.width *
			   p->icons[j].size.height * 4);

	if ( i + s - 1 > proplen ) {
	    break;
	}

	if (p->icons[j].data) delete [] p->icons[j].data;
	CARD32 *data = new CARD32[s/4];
	p->icons[j].data = (unsigned char *) data; // new CARD32[ s/4 ];
	memcpy(p->icons[j].data, d, s);
	i += s;
	d += s/4;
	j++;
    }

#ifdef    NETWMDEBUG
    fprintf(stderr, "NET: readIcon got %d icons\n", p->icons.size());
#endif

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
		    PropModeReplace, *((unsigned char **) &p->clients),
		    p->clients_count);
}


void NETRootInfo::setClientListStacking(Window *windows, unsigned int count) {
    if (role != WindowManager) return;

    p->stacking_count = count;
    if (p->stacking) delete [] p->stacking;
    p->stacking = nwindup(windows, count);

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::SetClientListStacking: setting list with %ld windows\n",
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
    fprintf(stderr, "NETRootInfo::setKDESystemTrayWindows: setting list with %ld windows\n",
	    p->kde_system_tray_windows_count);
#endif

    XChangeProperty(p->display, p->root, net_kde_system_tray_windows, XA_WINDOW, 32,
		    PropModeReplace,
		    (unsigned char *) p->kde_system_tray_windows,
		    p->kde_system_tray_windows_count);
}


void NETRootInfo::setNumberOfDesktops(int numberOfDesktops) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setNumberOfDesktops: setting desktop count to %d (%s)\n",
	    numberOfDesktops, (role == WindowManager) ? "WM" : "Client");
#endif

    if (role == WindowManager) {
	p->number_of_desktops = numberOfDesktops;

	XChangeProperty(p->display, p->root, net_number_of_desktops, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &p->number_of_desktops, 1);
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

	XSendEvent(p->display, p->root, False, SubstructureRedirectMask, &e);
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
	CARD32 d = p->current_desktop - 1;
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

	XSendEvent(p->display, p->root, False, SubstructureRedirectMask, &e);
    }
}


void NETRootInfo::setDesktopName(int desktop, const char *desktopName) {
    // return immediately if the requested desk is out of range
    if (desktop < 1 || desktop > p->number_of_desktops) return;

    if (p->desktop_names[desktop - 1]) delete p->desktop_names[desktop - 1];
    p->desktop_names[desktop - 1] = nstrdup(desktopName);

    unsigned int i, proplen,
	num = ((p->number_of_desktops < p->desktop_names.size()) ?
	       p->number_of_desktops : p->desktop_names.size());
    for (i = 0, proplen = 0; i < num; i++)
	proplen += strlen(p->desktop_names[i]) + 1;

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

    XChangeProperty(p->display, p->root, net_desktop_names, XA_STRING, 8,
		    PropModeReplace, (unsigned char *) prop, proplen);

    delete [] prop;
}


void NETRootInfo::setDesktopGeometry(int desktop, const NETSize &geometry) {

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setDesktopGeometry(%d, { %d, %d }) (%s)\n",
	    desktop, geometry.width, geometry.height, (role == WindowManager) ? "WM" : "Client");
#endif

    if (desktop <  1) return;

    if (role == WindowManager) {
	p->geometry[desktop - 1] = geometry;

	int d, i, l;
	l = p->geometry.size() * 2;
	CARD32 *data = new CARD32[l];
	for (d = 0, i = 0; d < p->geometry.size(); d++) {
	    data[i++] = p->geometry[d].width;
	    data[i++] = p->geometry[d].height;
	}

	XChangeProperty(p->display, p->root, net_desktop_geometry, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, l);

	delete [] data;
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_desktop_geometry;
	e.xclient.display = p->display;
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = desktop - 1;
	e.xclient.data.l[1] = geometry.width;
	e.xclient.data.l[2] = geometry.height;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, SubstructureRedirectMask, &e);
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
	CARD32 *data = new CARD32[l];
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
	e.xclient.data.l[0] = desktop - 1;
	e.xclient.data.l[1] = viewport.x;
	e.xclient.data.l[2] = viewport.y;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, SubstructureRedirectMask, &e);
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

    Atom atoms[27];
    int pnum = 2;

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

    if (p->protocols & KDESystemTrayWindows)
	atoms[pnum++] = net_kde_system_tray_windows;

    if (p->protocols & CloseWindow)
	atoms[pnum++] = net_close_window;

    if (p->protocols & WMMoveResize)
	atoms[pnum++] = net_wm_moveresize;

    if (p->protocols & WMName)
	atoms[pnum++] = net_wm_name;

    if (p->protocols & WMDesktop)
	atoms[pnum++] = net_wm_desktop;

    if (p->protocols & WMWindowType)
	atoms[pnum++] = net_wm_window_type;

    if (p->protocols & WMState)
	atoms[pnum++] = net_wm_state;

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

    if (p->protocols & WMKDESystemTrayWinFor)
	atoms[pnum++] = net_wm_kde_system_tray_window_for;

    if (p->protocols & WMKDEFrameStrut)
	atoms[pnum++] = net_wm_kde_frame_strut;

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
    XChangeProperty(p->display, p->supportwindow, net_wm_name, XA_STRING, 8,
		    PropModeReplace, (unsigned char *) p->name,
		    strlen(p->name) + 1);
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
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = window;
	e.xclient.data.l[1] = 0l;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, SubstructureRedirectMask, &e);
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

    CARD32 *wa = new CARD32[p->number_of_desktops * 4];
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
    e.xclient.window = p->root;
    e.xclient.format = 32;
    e.xclient.data.l[0] = window;
    e.xclient.data.l[1] = 0l;
    e.xclient.data.l[2] = 0l;
    e.xclient.data.l[3] = 0l;
    e.xclient.data.l[4] = 0l;

    XSendEvent(p->display, p->root, False, SubstructureRedirectMask, &e);
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
    e.xclient.window = p->root;
    e.xclient.format = 32;
    e.xclient.data.l[0] = window;
    e.xclient.data.l[1] = x_root;
    e.xclient.data.l[2] = y_root;
    e.xclient.data.l[3] = direction;
    e.xclient.data.l[4] = 0l;

    XSendEvent(p->display, p->root, False, SubstructureRedirectMask, &e);
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

	    int desktop = event->xclient.data.l[0];

	    NETSize sz;
	    sz.width = event->xclient.data.l[1];
	    sz.height = event->xclient.data.l[2];

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: changeDesktopGeometry(%d, { %d, %d })\n",
		    desktop, sz.width, sz.height);
#endif

	    changeDesktopGeometry(desktop, sz);
	} else if (event->xclient.message_type == net_desktop_viewport) {
	    dirty = DesktopViewport;

	    int desktop = event->xclient.data.l[0];

	    NETPoint pt;
	    pt.x = event->xclient.data.l[0];
	    pt.y = event->xclient.data.l[1];

#ifdef   NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: changeDesktopViewport(%d, { %d, %d })\n",
		    desktop, pt.x, pt.y);
#endif

	    changeDesktopViewport(desktop, pt);
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
		    event->xclient.data.l[0]);
#endif

	    changeActiveWindow(event->xclient.data.l[0]);
	} else if (event->xclient.message_type == net_wm_moveresize) {

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: moveResize(%ld, %ld, %ld, %ld)\n",
		    event->xclient.data.l[0],
		    event->xclient.data.l[1],
		    event->xclient.data.l[2],
		    event->xclient.data.l[3]
		    );
#endif

	    moveResize(event->xclient.data.l[0],
		       event->xclient.data.l[1],
		       event->xclient.data.l[2],
		       event->xclient.data.l[3]);
	} else if (event->xclient.message_type == net_close_window) {

#ifdef   NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: closeWindow(0x%lx)\n",
		    event->xclient.data.l[0]);
#endif

	    closeWindow(event->xclient.data.l[0]);
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
	    else if (pe.xproperty.atom == net_kde_system_tray_windows)
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

	    XFree(data_ret);
	}
    }

    if (dirty & KDESystemTrayWindows) {
	if (XGetWindowProperty(p->display, p->root, net_kde_system_tray_windows,
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

		    delete [] p->kde_system_tray_windows;
		} else {
		    unsigned long n;
		    for (n = 0; n < nitems_ret; n++) {
			addSystemTrayWin(wins[n]);
		    }
		}

		p->kde_system_tray_windows_count = nitems_ret;
		p->kde_system_tray_windows =
		    nwindup(wins, p->kde_system_tray_windows_count);
	    }

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
		p->number_of_desktops = *((CARD32 *) data_ret);
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::update: number of desktops = %d\n",
		    p->number_of_desktops);
#endif

	    XFree(data_ret);
	}
    }

    if (dirty & DesktopGeometry) {
	if (XGetWindowProperty(p->display, p->root, net_desktop_geometry,
			       0l, 2l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret >= 2) {
		CARD32 *data = (CARD32 *) data_ret;

		int d, i, n;
		n = nitems_ret / 2;
		for (d = 0, i = 0; d < n; d++) {
		    p->geometry[d].width  = data[i++];
		    p->geometry[d].height = data[i++];
		}

#ifdef    NETWMDEBUG
		fprintf(stderr,
			"NETRootInfo::update: desktop geometry array updated (%d entries)\n",
			p->geometry.size());

		if (nitems_ret % 2 != 0) {
		    fprintf(stderr,
			    "NETRootInfo::update: desktop geometry array size "
			    "not a multipe of 2\n");
		}
#endif
	    }

	    XFree(data_ret);
	}
    } else {
	int i;
	for (i = 0; i < p->geometry.size(); i++) {
	    p->geometry[i] = p->rootSize;
	}
    }

    if (dirty & DesktopViewport) {
	if (XGetWindowProperty(p->display, p->root, net_desktop_viewport,
			       0l, 2l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret == 2) {
		CARD32 *data = (CARD32 *) data_ret;

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
		p->current_desktop = *((CARD32 *) data_ret) + 1;
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::update: current desktop = %d\n",
		    p->current_desktop);
#endif

	    XFree(data_ret);
	}
    }

    if (dirty & DesktopNames) {
	if (XGetWindowProperty(p->display, p->root, net_desktop_names,
			       0l, (long) BUFSIZE, False, XA_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_STRING && format_ret == 8) {
		// force the last element in the data array to be NUL
		data_ret[nitems_ret - 1] = '\0';

		const char *d = (const char *) data_ret;
		unsigned int s, n, index;

		for (s = 0, n = 0, index = 0; n < nitems_ret; n++) {
		    if (d[n] == '\0') {
			p->desktop_names[index++] = nstrndup((d + s), n - s + 1);
			s = n + 1;
		    }
		}
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::update: desktop names array updated (%d entries)\n",
		    p->desktop_names.size());
#endif

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
		CARD32 *d = (CARD32 *) data_ret;
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
				       XA_STRING, &type_ret, &format_ret,
				       &nitems_ret, &unused, &name_ret)
		    == Success) {
		    if (type_ret == XA_STRING && format_ret == 8)
			p->name = nstrdup((const char *) name_ret);

		    XFree(name_ret);
		}
	    }

#ifdef    NETWMDEBUG
	    fprintf(stderr,
		    "NETRootInfo::update: supporting window manager = '%s'\n",
		    p->name);
#endif

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


NETSize NETRootInfo::desktopGeometry(int desktop) const {
    if (desktop < 1) {
	NETSize sz;
	return sz;
    }

    return p->geometry[desktop - 1];
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
    p->state = Unknown;
    p->type = Unknown;
    p->name = (char *) 0;
    p->visible_name = (char *) 0;
    p->desktop = p->pid = p->handled_icons = 0;
    p->strut.left = p->strut.right = p->strut.top = p->strut.bottom = 0;
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

    if (replace) {
	int i;
	for (i = 0; i < p->icons.size(); i++) {
	    if (p->icons[i].data) delete [] p->icons[i].data;
	    p->icons[i].data = 0;
	    p->icons[i].size.width = 0;
	    p->icons[i].size.height = 0;
	}

	p->icon_count = 0;
    }

    p->icons[p->icon_count] = icon;
    p->icon_count++;

    // do a deep copy, we want to own the data
    NETIcon& ni = p->icons[ p->icon_count - 1 ];
    CARD32 *d = new CARD32[ ni.size.width * ni.size.height ];
    ni.data = (unsigned char *) d;
    (void) memcpy( ni.data, icon.data, ni.size.width * ni.size.height * 4 );

    int proplen, i;
    for (i = 0, proplen = 0; i < p->icon_count; i++) {
	proplen += 2 + (p->icons[i].size.width *
			p->icons[i].size.height);
    }

    CARD32 *prop = new CARD32[proplen], *pprop = prop;
    int sz;
    for (i = 0; i < p->icon_count; i++) {
       	*pprop++ = p->icons[i].size.width;
	*pprop++ = p->icons[i].size.height;
	sz = (p->icons[i].size.width *
	      p->icons[i].size.height * 4);
	(void) memcpy(pprop, p->icons[i].data, sz);
	pprop += sz/4;
    }

    XChangeProperty(p->display, p->window, net_wm_icon, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) prop, proplen);

    delete [] prop;
}


void NETWinInfo::setIconGeometry(NETRect geometry) {
    if (role != Client) return;

    p->icon_geom = geometry;

    CARD32 data[4];
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

    CARD32 data[4];
    data[0] = strut.left;
    data[1] = strut.right;
    data[2] = strut.top;
    data[3] = strut.bottom;

    XChangeProperty(p->display, p->window, net_wm_strut, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) data, 4);
}


void NETWinInfo::setState(unsigned long state, unsigned long mask) {
    if (p->mapping_state_dirty) update(XAWMState);

    if (role == Client && p->mapping_state != Withdrawn) {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_wm_state;
	e.xclient.display = p->display;
	e.xclient.window = p->window;
	e.xclient.format = 32;
	e.xclient.data.l[0] = state;
	e.xclient.data.l[1] = mask;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, SubstructureRedirectMask, &e);
    } else {
	p->state &= ~mask;
	p->state |= state;

	XChangeProperty(p->display, p->window, net_wm_state, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &(p->state), 1);
    }
}


void NETWinInfo::setWindowType(WindowType type) {
    if (role != Client) return;

    CARD32 data = type;
    XChangeProperty(p->display, p->window, net_wm_window_type, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &data, 1);
}


void NETWinInfo::setName(const char *name) {
    if (role != Client) return;

    if (p->name) delete [] p->name;
    p->name = nstrdup(name);
    XChangeProperty(p->display, p->window, net_wm_name, XA_STRING, 8,
		    PropModeReplace, (unsigned char *) p->name,
		    strlen(p->name) + 1);
}


void NETWinInfo::setVisibleName(const char *visibleName) {
    if (role != WindowManager) return;

    if (p->visible_name) delete [] p->visible_name;
    p->visible_name = nstrdup(visibleName);
    XChangeProperty(p->display, p->window, net_wm_visible_name, XA_STRING, 8,
		    PropModeReplace, (unsigned char *) p->visible_name,
		    strlen(p->visible_name) + 1);
}


void NETWinInfo::setDesktop(int desktop) {
    if (p->mapping_state_dirty) update(XAWMState);

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

	XSendEvent(p->display, p->root, False, SubstructureRedirectMask, &e);
    } else {
	// otherwise we just set or remove the property directly
	p->desktop = desktop;
	int d = desktop;

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
    XChangeProperty(p->display, p->window, net_wm_pid, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &(p->pid), 1);
}


void NETWinInfo::setHandledIcons(Bool handled) {
    if (role != Client) return;

    p->handled_icons = handled;
    CARD32 d = handled;
    XChangeProperty(p->display, p->window, net_wm_handled_icons, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &d, 1);
}


void NETWinInfo::setKDESystemTrayWinFor(Window window) {
    if (role != Client) return;

    p->kde_system_tray_win_for = window;
    XChangeProperty(p->display, p->window, net_wm_kde_system_tray_window_for,
		    XA_WINDOW, 32, PropModeReplace,
		    (unsigned char *) &(p->kde_system_tray_win_for), 1);
}


void NETWinInfo::setKDEFrameStrut(NETStrut strut) {
    if (role != WindowManager) return;

    p->frame_strut = strut;

    CARD32 d[4];
    d[0] = strut.left;
    d[1] = strut.right;
    d[2] = strut.top;
    d[3] = strut.bottom;

    XChangeProperty(p->display, p->window, net_wm_kde_frame_strut, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) d, 4);
}


void NETWinInfo::kdeGeometry(NETRect& frame, NETRect& window) {
    Window unused;
    int x, y;
    unsigned int w, h, junk;
    XGetGeometry(p->display, p->window, &unused, &x, &y, &w, &h, &junk, &junk);
    XTranslateCoordinates(p->display, p->window, p->root, 0, 0, &x, &y, &unused);

    p->win_geom.pos.x = x;
    p->win_geom.pos.y = y;

    p->win_geom.size.width = w;
    p->win_geom.size.height = h;

    p->frame_geom.pos.x = x - p->frame_strut.left;
    p->frame_geom.pos.y = y - p->frame_strut.top;

    p->frame_geom.size.width = w + p->frame_strut.left + p->frame_strut.right;
    p->frame_geom.size.height = h + p->frame_strut.top + p->frame_strut.bottom;

    frame = p->frame_geom;
    window = p->win_geom;
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
	if (event->xclient.message_type == net_wm_state) {
	    dirty = WMState;

	    changeState(event->xclient.data.l[0], event->xclient.data.l[1]);
	} else if (event->xclient.message_type == net_wm_desktop) {
	    dirty = WMDesktop;

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
	    else if (pe.xproperty.atom == net_wm_kde_frame_strut)
		dirty |= WMKDEFrameStrut;
	    else if (pe.xproperty.atom == net_wm_kde_system_tray_window_for)
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
		CARD32 *state = (CARD32 *) data_ret;

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

	    XFree(data_ret);
	}
    }

    // we do this here because we *always* want to update WM_STATE
    dirty &= p->properties;

    if (dirty & WMState) {
	if (XGetWindowProperty(p->display, p->window, net_wm_state, 0l, 1l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 1) {
		p->state = *((CARD32 *) data_ret);
	    }

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
		p->desktop = *((CARD32 *) data_ret);
		if ((signed) p->desktop != OnAllDesktops)
		    p->desktop++;

		if ( p->desktop == 0 )
		    p->desktop = OnAllDesktops;
	    }

	    XFree(data_ret);
	}
    }

    if (dirty & WMName) {
	if (XGetWindowProperty(p->display, p->window, net_wm_name, 0l,
			       (long) BUFSIZE, False, XA_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_STRING && format_ret == 8 && nitems_ret > 0) {
		if (p->name) delete [] p->name;
		p->name = nstrndup((const char *) data_ret, nitems_ret);
	    }

	    XFree(data_ret);
	}
    }

    if (dirty & WMVisibleName) {
	if (XGetWindowProperty(p->display, p->window, net_wm_visible_name, 0l,
			       (long) BUFSIZE, False, XA_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_STRING && format_ret == 8 &&
		nitems_ret > 0) {
		if (p->visible_name) delete [] p->visible_name;
		p->visible_name = nstrndup((const char *) data_ret, nitems_ret);
	    }

	    XFree(data_ret);
	}
    }

    if (dirty & WMWindowType) {
	p->type = Unknown;
	if (XGetWindowProperty(p->display, p->window, net_wm_window_type, 0l, 1l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret == 1)
		p->type = (WindowType) *((CARD32 *) data_ret);

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
		CARD32 *d = (CARD32 *) data_ret;
		p->strut.left   = d[0];
		p->strut.right  = d[1];
		p->strut.top    = d[2];
		p->strut.bottom = d[3];
	    }

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
		CARD32 *d = (CARD32 *) data_ret;
		p->icon_geom.pos.x       = d[0];
		p->icon_geom.pos.y       = d[1];
		p->icon_geom.size.width  = d[2];
		p->icon_geom.size.height = d[3];
	    }

	    XFree(data_ret);
	}
    }

    if (dirty & WMIcon) {
	readIcon(p);
    }

    if (dirty & WMKDESystemTrayWinFor) {
	p->kde_system_tray_win_for = 0;
	if (XGetWindowProperty(p->display, p->window, net_wm_kde_system_tray_window_for,
			       0l, 1l, False, XA_WINDOW, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret)
	    == Success) {
	    if (type_ret == XA_WINDOW && format_ret == 32 &&
		nitems_ret == 1) {
		p->kde_system_tray_win_for = *((Window *) data_ret);
		if ( p->kde_system_tray_win_for == 0 )
		    p->kde_system_tray_win_for = p->root;
	    }

	    XFree(data_ret);
        }
    }

    if (dirty & WMKDEFrameStrut) {
	if (XGetWindowProperty(p->display, p->window, net_wm_kde_frame_strut,
			       0l, 4l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret) == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 4) {
		CARD32 *d = (CARD32 *) data_ret;

		p->frame_strut.left   = d[0];
		p->frame_strut.right  = d[1];
		p->frame_strut.top    = d[2];
		p->frame_strut.bottom = d[3];
	    }

	    XFree(data_ret);
	}
    }

    if (dirty & WMPid) {
	p->pid = 0;
	if (XGetWindowProperty(p->display, p->window, net_wm_pid, 0l, 1l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret) == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 1) {
		p->pid = *((CARD32 *) data_ret);
	    }

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
