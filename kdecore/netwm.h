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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <qglobal.h>

#include "netwm_def.h"
#include "netwm_p.h"


class NETRootInfo : public NET {
public:
    // Use this constructor for window managers.
    NETRootInfo(Display *, Window, const char *, unsigned long, int = -1, bool doActivate = true );
    // Use this constructor for clients. 
    NETRootInfo(Display *, unsigned long, int = -1, bool doActivate = true );
    NETRootInfo(const NETRootInfo &);
    virtual ~NETRootInfo();

    // late initialization
    void activate();

    inline Display *x11Display() const;
    inline Window rootWindow() const;
    inline Window supportWindow() const;
    inline const char *wmName() const;
    inline int screenNumber() const;
    inline unsigned long supported() const;

    void setClientList(Window *, unsigned int);
    void setClientListStacking(Window *, unsigned int);
    void setKDEDockingWindows(Window *, unsigned int);
    void setCurrentDesktop(Q_UINT32);
    void setDesktopGeometry(const NETSize &);
    void setDesktopViewport(const NETPoint &);
    void setNumberOfDesktops(Q_UINT32);
    void setDesktopName(Q_UINT32, const char *);
    void setActiveWindow(Window);
    void setWorkArea(Q_UINT32, const NETRect &);
    void setVirtualRoots(Window *, int);

    const NETRootInfo &operator=(const NETRootInfo &);

    inline const Window *clientList() const;
    inline int clientListCount() const;
    inline const Window *clientListStacking() const;
    inline int clientListStackingCount() const;
    inline const Window *kdeDockingWindows() const;
    inline int kdeDockingWindowsCount() const;
    inline NETSize desktopGeometry() const;
    inline NETPoint desktopViewport() const;
    inline NETRect workArea(Q_UINT32) const;
    inline const char *desktopName(Q_UINT32) const;
    inline const Window *virtualRoots( ) const;
    inline int virtualRootsCount() const;
    inline Q_UINT32 numberOfDesktops() const;
    inline Q_UINT32 currentDesktop() const;
    inline Window activeWindow() const;

    // This function takes the pass XEvent and returns an ORed list of NETRootInfo
    // properties that have changed.  The new information will be read
    // immediately by the class.
    unsigned long event(XEvent *);

    void closeWindowRequest(Window);


protected:
    // Notify a NETClient that a window has been added/removed.
    virtual void addClient(Window) { }
    virtual void removeClient(Window) { }

    // NOT STANDARD - KDE 2.0 EXTENSIONS
    virtual void addDockWin(Window) { }
    virtual void removeDockWin(Window) { }

    // Notify a NETWindowManager that a request has been made.
    virtual void changeNumberOfDesktops(Q_UINT32) { }
    virtual void changeDesktopGeometry(const NETSize &) { }
    virtual void changeDesktopViewport(const NETPoint &) { }
    virtual void changeCurrentDesktop(Q_UINT32) { }
    virtual void changeActiveWindow(Window) { }
    virtual void closeWindow(Window) { }
    virtual void moveResize(Window, int, int, unsigned long) { }


private:
    void update(unsigned long);
    void setSupported(unsigned long);

    NETRootInfoPrivate *p;
    Role role;
};


class NETWinInfo : public NET {
public:
    NETWinInfo(Display * /* display */, Window /*  window */,
	       Window /* root window */, unsigned long /* support flags */,
	       Role /* role */ = Client);
    NETWinInfo(const NETWinInfo & /* wininfo */);
    virtual ~NETWinInfo();

    inline unsigned long properties() const;

    void setIcon(NETIcon /* icon */, Bool /* replace */ = true);
    void setIconGeometry(NETRect /* rect */);
    void setStrut(NETStrut /* strut */);
    void setState(unsigned long /* state */, unsigned long /* mask */);
    void setWindowType(WindowType /* type */);
    void setName(const char * /* name */);
    void setVisibleName(const char * /* visible name */);

    static const Q_UINT32 OnAllDesktops = (Q_UINT32) -1;
    void setDesktop(Q_UINT32 /* desktop */);
    void setPid(Q_UINT32 /* pid */);
    void setHandledIcons(Bool /* handled */);
    void setKDEDockWinFor(Window /* window */);
    void setKDEFrameStrut(NETStrut);

    NETIcon icon(int = -1, int = -1) const;

    inline NETRect iconGeometry() const;
    inline unsigned long state() const;
    inline NETStrut strut() const;
    inline WindowType windowType() const;
    inline const char *name() const;
    inline const char *visibleName() const;
    inline Q_UINT32 desktop() const;
    inline Q_UINT32 pid() const;
    inline Bool handledIcons() const;
    inline Window kdeDockWinFor() const;
    inline MappingState mappingState() const;
    void kdeGeometry(NETRect &frame, NETRect &window);

    // This function takes the pass XEvent and returns an ORed list of NETWinInfo
    // properties that have changed.  The new information will be read
    // immediately by the class.
    unsigned long event(XEvent *);


protected:
    virtual void changeDesktop(Q_UINT32 /* desktop */) { }
    virtual void changeState(Q_UINT32 /* state */, Q_UINT32 /* mask */) { }


private:
    void update(unsigned long /* dirty */);

    NETWinInfoPrivate *p;
    Role role;
};


// NETRootInfo inlines

inline Display *NETRootInfo::x11Display() const { return p->display; }

inline Window NETRootInfo::rootWindow() const { return p->root; }

inline Window NETRootInfo::supportWindow() const { return p->supportwindow; }

inline const char *NETRootInfo::wmName() const { return p->name; }

inline int NETRootInfo::screenNumber() const { return p->screen; }

inline unsigned long NETRootInfo::supported() const  { return p->protocols; }

inline const Window *NETRootInfo::clientList() const { return p->clients; }

inline int NETRootInfo::clientListCount() const { return p->clients_count; }

inline const Window *NETRootInfo::clientListStacking() const {
    return p->stacking;
}

inline int NETRootInfo::clientListStackingCount() const {
    return p->stacking_count;
}

inline const Window *NETRootInfo::kdeDockingWindows() const {
    return p->kde_docking_windows;
}

inline int NETRootInfo::kdeDockingWindowsCount() const {
    return p->kde_docking_windows_count;
}

inline NETSize NETRootInfo::desktopGeometry() const { return p->geometry; }

inline NETPoint NETRootInfo::desktopViewport() const { return p->viewport; }

inline NETRect NETRootInfo::workArea(Q_UINT32 d) const { return p->workarea[d]; }

inline const char *NETRootInfo::desktopName(Q_UINT32 d) const {
    return p->desktop_names[d];
}

inline const Window *NETRootInfo::virtualRoots( ) const {
    return p->virtual_roots;
}

inline int NETRootInfo::virtualRootsCount() const {
    return p->virtual_roots_count;
}

inline Q_UINT32 NETRootInfo::numberOfDesktops() const {
    return p->number_of_desktops;
}

inline Q_UINT32 NETRootInfo::currentDesktop() const { return p->current_desktop; }

inline Window NETRootInfo::activeWindow() const { return p->active; }


// NETWinInfo inlines

inline NETRect NETWinInfo::iconGeometry() const { return p->icon_geom; }

inline unsigned long NETWinInfo::state() const { return p->state; }

inline NETStrut NETWinInfo::strut() const { return p->strut; }

inline NET::WindowType NETWinInfo::windowType() const { return p->type; }

inline const char *NETWinInfo::name() const { return p->name; }

inline const char *NETWinInfo::visibleName() const { return p->visible_name; }

inline Q_UINT32 NETWinInfo::desktop() const { return p->desktop; }

inline Q_UINT32 NETWinInfo::pid() const { return p->pid; }

inline Bool NETWinInfo::handledIcons() const { return p->handled_icons; }

inline Window NETWinInfo::kdeDockWinFor() const { return p->kde_dockwin_for; }

inline unsigned long NETWinInfo::properties() const { return p->properties; }

inline NET::MappingState NETWinInfo::mappingState() const { return p->mapping_state; }

#endif // __net_wm_h
