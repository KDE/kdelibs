/*
 * kwm.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include "kwm.h"
#include <unistd.h>
#include <qwmatrix.h>
#include <qbitmap.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <kmisc.h>

bool kwm_error;

static int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p){
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;
  
  status = XGetWindowProperty(qt_xdisplay(), w, a, 0L, len, False, type, &real_type, &format, &n, &extra, p);
  if (status != Success || *p == 0)
    return -1;
  if (n == 0)
    XFree((void*) *p);
  /* could check real_type, format, extra here... */
  return n;
}

static bool getSimpleProperty(Window w, Atom a, long &result){
  long *p = 0;
  
  if (_getprop(w, a, a, 1L, (unsigned char**)&p) <= 0){
    kwm_error = TRUE;
    return FALSE;
  }
  
  result = (long) *p;
  XFree((char *) p);
  kwm_error = FALSE;
  return TRUE;
}

static void setSimpleProperty(Window w, Atom a, long data){
  XChangeProperty(qt_xdisplay(), w, a, a, 32,
		  PropModeReplace, (unsigned char *)&data, 1);
}

static bool getDoubleProperty(Window w, Atom a, long &result1, long &result2){
  long *p = 0;
  
  if (_getprop(w, a, a, 2L, (unsigned char**)&p) <= 0){
    kwm_error = TRUE;
    return FALSE;
  }
  
  result1 = (long) *p;
  result2 = (long) *(p + sizeof(long));
  XFree((char *) p);
  kwm_error = FALSE;
  return TRUE;
}

static void setDoubleProperty(Window w, Atom a, long data1, long data2){
  long data[2];
  data[0] = data1;
  data[1] = data2;
  XChangeProperty(qt_xdisplay(), w, a, a, 32,
		  PropModeReplace, (unsigned char *)data, 2);
}

static bool getQRectProperty(Window w, Atom a, QRect &rect){
  long *p = 0;
  
  if (_getprop(w, a, a, 4L, (unsigned char**)&p) <= 0){
    kwm_error = TRUE;
    return FALSE;
  }
  rect.setRect(p[0], p[1], p[2], p[3]);
  XFree((char *) p);
  kwm_error = FALSE;
  return TRUE;
}
static void setQRectProperty(Window w, Atom a, const QRect &rect){
  long data[4];
  data[0] = rect.x();
  data[1] = rect.y();
  data[2] = rect.width();
  data[3] = rect.height();
  XChangeProperty(qt_xdisplay(), w, a, a, 32,
		  PropModeReplace, (unsigned char *)&data, 4);
}

static bool getQStringProperty(Window w, Atom a, QString &str){
  unsigned char *p = 0;
  
  if (_getprop(w, a, XA_STRING, 100L, (unsigned char**)&p) <= 0){
    kwm_error = TRUE;
    return FALSE;
  }
  str = (char*) p;
  XFree((char *) p);
  kwm_error = FALSE;
  return TRUE;
}

static void sendClientMessage(Window w, Atom a, long x){
  XEvent ev;
  long mask;
  
  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = x;
  ev.xclient.data.l[1] = CurrentTime;
  mask = 0L;
  if (w == qt_xrootwin())
    mask = SubstructureRedirectMask;
  XSendEvent(qt_xdisplay(), w, False, mask, &ev);
}



static void setQStringProperty(Window w, Atom a, const QString &str){
  XChangeProperty(qt_xdisplay(), w, a, XA_STRING, 8,
		  PropModeReplace, (unsigned char *)(str.data()), 
		  str.length()+1);
}

QString KWM::getProperties(Window w){
  QString result;
  QRect rect;
  int data[13];
  int i = 0;
  int n = 0;
  data[n++]=desktop(w);
  rect = geometry(w);
  data[n++]=rect.x();
  data[n++]=rect.y();
  data[n++]=rect.width();
  data[n++]=rect.height();
  rect = geometryRestore(w);
  data[n++]=rect.x();
  data[n++]=rect.y();
  data[n++]=rect.width();
  data[n++]=rect.height();
  data[n++]=isIconified(w)?1:0;
  data[n++]=isMaximized(w)?1:0;
  data[n++]=isSticky(w)?1:0;
  data[n++]=isDecorated(w)?1:0;
  
  QString s;
  for (i=0;i<n;i++){
    s.setNum(data[i]);
    result.append(s);
    if (i<n-1)
      result.append("+");
  }
  return result;
} 

void KWM::setProperties(Window w, const QString &props){
  int a;
  int data[13];
  int d1,d2,d3,d4;
  int n = 0;
  QString arg = props.data();
  while ((a = arg.find('+', 0)) != -1){
    if (n<13)
      data[n++]=arg.left(a).toInt();
    arg.remove(0,a+1);
  }
  if (n<13)
    data[n++] = arg.toInt();
  if (n!=13){
//     fprintf(stderr, "KWM::setProperties: <bad properties error>\n");
    return;
  }
  n = 0;
  moveToDesktop(w, data[n++]);
  d1 = data[n++];
  d2 = data[n++];
  d3 = data[n++];
  d4 = data[n++];
  setGeometry(w, QRect(d1,d2,d3,d4));
  d1 = data[n++];
  d2 = data[n++];
  d3 = data[n++];
  d4 = data[n++];
  setGeometryRestore(w, QRect(d1,d2,d3,d4));
  setIconify(w, (data[n++] != 0) );
  setMaximize(w, (data[n++] != 0) );
  setSticky(w, (data[n++] != 0) );
  setDecoration(w, (data[n++] != 0) );
}


void KWM::enableSessionManagement(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "WM_SAVE_YOURSELF", False);
  Atom *p;
  int i,n;
  if (XGetWMProtocols(qt_xdisplay(), w, &p, &n)){
    for (i = 0; i < n; i++){
      if (p[i] == a)
	return;
    }
    Atom *pn = new Atom[n+1];
    for (i=0; i<n; i++)
      pn[i] = p[i];
    p[i] = a;
    XSetWMProtocols(qt_xdisplay(), w, pn, n+1);
    if (n>0)
      XFree(p);
    delete[] pn;	
  }
  else
    XSetWMProtocols(qt_xdisplay(), w, &a, 1);
}


void KWM::setWmCommand(Window w, const QString &command){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "WM_CLIENT_MACHINE", False);
  setQStringProperty(w, XA_WM_COMMAND, command);
  QString machine;
  QString domain;
  QString all="";
  char buf[200];
  if (!gethostname(buf, 200))
    machine = buf;
  if (!getdomainname(buf, 200))
    domain = buf;
  if (!machine.isEmpty())
      all = machine + "." + domain;
  setQStringProperty(w, a, all);
}

void KWM::setUnsavedDataHint(Window w, bool value){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_UNSAVED_DATA", False);
  setSimpleProperty(w, a, value?1:0);
}

void KWM::setMiniIcon(Window w, const QPixmap &pm){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_ICON", False);
  QPixmap *p = new QPixmap;
  *p = pm;
  setDoubleProperty(w, a, (long) p->handle(), 
		    (long) (p->mask()?p->mask()->handle():None));
}

void KWM::setIcon(Window w, const QPixmap &pm){
  XWMHints *hints = XGetWMHints(qt_xdisplay(), w);
  QPixmap *p = new QPixmap;
  *p = pm;
  hints->icon_pixmap=p->handle();
  hints->flags |= IconPixmapHint;
  if(p->mask()){
    hints->icon_mask = p->mask()->handle();
    hints->flags |= IconMaskHint;
  }
  else 
    hints->flags &= ~IconMaskHint;
  XSetWMHints( qt_xdisplay(), w, hints );
  XFree(hints);
}


void KWM::setDockWindow(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_DOCKWINDOW", False);
  setSimpleProperty(w, a, 1);
}

void KWM::setDecoration(Window w, bool value){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_DECORATED", False);
  setSimpleProperty(w, a, value?1:0);
}


void KWM::logout(){
  sendKWMCommand("logout");
}

void KWM::refreshScreen(){
  sendKWMCommand("refreshScreen");
}

void KWM::darkenScreen(){
  sendKWMCommand("darkenScreen");
}
  
void KWM::configureWm(){
  sendKWMCommand("configure");
}

int KWM::currentDesktop(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_CURRENT_DESKTOP", False);
  long result = 1;
  if (!getSimpleProperty(qt_xrootwin(), a, result))
    switchToDesktop((int)result);
  return (int) result;
}

void KWM::setKWMModule(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_MODULE", False);
  setSimpleProperty(w, a, 1);
}

void KWM::setKWMDockModule(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_MODULE", False);
  setSimpleProperty(w, a, 2);
}

bool KWM::isKWMModule(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_MODULE", False);
  long result = 0;
  getSimpleProperty(w, a, result);
  return result != 0;
}

bool KWM::isKWMDockModule(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_MODULE", False);
  long result = 0;
  getSimpleProperty(w, a, result);
  return result == 2;
}

bool KWM::isKWMInitialized(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_RUNNING", False);
  long result = 0;
  getSimpleProperty(qt_xrootwin(), a, result);
  return result != 0;
}


Window KWM::activeWindow(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_ACTIVE_WINDOW", False);
  long result = 0;
  getSimpleProperty(qt_xrootwin(), a, result);
  return (Window) result;
}
void KWM::switchToDesktop(int desk){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_CURRENT_DESKTOP", False);
  setSimpleProperty(qt_xrootwin(), a, (long) desk);
}

void KWM::setWindowRegion(int desk, const QRect &region){
  static Atom a[8] = {0,0,0,0,0,0,0,0};
  if (desk < 1 || desk > 8){
    kwm_error = TRUE;
    return;
  }
  if (!a[desk-1]){
    QString n;
    n.setNum(desk);
    n.prepend("KWM_WINDOW_REGION_");
    a[desk-1] = XInternAtom(qt_xdisplay(), n.data(), False);
  }
  setQRectProperty(qt_xrootwin(), a[desk-1], region);
}

QRect KWM::getWindowRegion(int desk){
  static Atom a[8] = {0,0,0,0,0,0,0,0};
  if (desk < 1 || desk > 8){
    kwm_error = TRUE;
    return QApplication::desktop()->geometry();;
  }
  if (!a[desk-1]){
    QString n;
    n.setNum(desk);
    n.prepend("KWM_WINDOW_REGION_");
    a[desk-1] = XInternAtom(qt_xdisplay(), n.data(), False);
  }
  QRect result = QApplication::desktop()->geometry();
  getQRectProperty(qt_xrootwin(), a[desk-1], result);
  return result;
}

int KWM::numberOfDesktops(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_NUMBER_OF_DESKTOPS", False);
  long result = 1;
  getSimpleProperty(qt_xrootwin(), a, result);
  return (int) result;
}

void KWM::setNumberOfDesktops(int num){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_NUMBER_OF_DESKTOPS", False);
  setSimpleProperty(qt_xrootwin(), a, (long) num);
}

void KWM::setDesktopName(int desk, const QString &name){
  static Atom a[8] = {0,0,0,0,0,0,0,0};
  if (desk < 1 || desk > 8){
    kwm_error = TRUE;
    return;
  }
  if (!a[desk-1]){
    QString n;
    n.setNum(desk);
    n.prepend("KWM_DESKTOP_NAME_");
    a[desk-1] = XInternAtom(qt_xdisplay(), n.data(), False);
  }
  setQStringProperty(qt_xrootwin(), a[desk-1], name);
}

QString KWM::getDesktopName(int desk){
  static Atom a[8] = {0,0,0,0,0,0,0,0};
  QString result;
  if (desk < 1 || desk > 8){
    kwm_error = TRUE;
    return result;
  }
  if (!a[desk-1]){
    QString n;
    n.setNum(desk);
    n.prepend("KWM_DESKTOP_NAME_");
    a[desk-1] = XInternAtom(qt_xdisplay(), n.data(), False);
  }
  getQStringProperty(qt_xrootwin(), a[desk-1], result);
  return result;
}

void KWM::sendKWMCommand(const QString &command){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_COMMAND", False);

  XEvent ev;
  int status;
  long mask;
  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = qt_xrootwin();
  ev.xclient.message_type = a;
  ev.xclient.format = 8;

  int i;
  const char* s = command.data();
  for (i=0;i<19 && s[i];i++)
    ev.xclient.data.b[i]=s[i];
  
  mask = SubstructureRedirectMask; 

  status = XSendEvent(qt_xdisplay(),
		      qt_xrootwin(),
		      False, mask, &ev);
}

QString KWM::title(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_TITLE", False);
  QString result;
  if (!getQStringProperty(w, a, result)){
    getQStringProperty(w, XA_WM_NAME, result);
  }
  return result;
}

QString KWM::titleWithState(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_TITLE", False);
  QString result;
  if (!getQStringProperty(w, a, result)){
    getQStringProperty(w, XA_WM_NAME, result);
  }
  if (isIconified(w)){
    result.prepend("(");
    result.append(")");
  }
  return result;
}

QPixmap KWM::miniIcon(Window w, int width, int height){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_ICON", False);
  QPixmap result;
  Pixmap p = None;
  Pixmap p_mask = None;

  long tmp[2] = {None, None};
  if (!getDoubleProperty(w, a, tmp[0], tmp[1])){
    XWMHints *hints = XGetWMHints(qt_xdisplay(), w);
    if (hints && (hints->flags & IconPixmapHint)){
      p = hints->icon_pixmap;
    }
    if (hints && (hints->flags & IconMaskHint)){
      p_mask = hints->icon_mask;
    }
  }
  else {
    p = (Pixmap) tmp[0];
    p_mask = (Pixmap) tmp[1];
  }

  if (p != None){
    Window root;
    int x, y;
    unsigned int w = 0;
    unsigned int h = 0; 
    unsigned int border_w, depth;
    XGetGeometry(qt_xdisplay(), p,
		 &root, 
		 &x, &y, &w, &h, &border_w, &depth);
    if (w > 0 && h > 0){
      QPixmap pm(w, h, depth);
      XCopyArea(qt_xdisplay(), p, pm.handle(),
		qt_xget_temp_gc(depth==1),
		0, 0, w, h, 0, 0);
      if (p_mask != None){
	QBitmap bm(w, h);
	XCopyArea(qt_xdisplay(), p_mask, bm.handle(),
		  qt_xget_temp_gc(TRUE),
		  0, 0, w, h, 0, 0);
	pm.setMask(bm);
      }
      if (width > 0 && height > 0 && (w > (unsigned int)width 
				      || h > (unsigned int) height)){
	// scale
	QWMatrix m;
	m.scale(width/(float)w, height/(float)h);
	result = pm.xForm(m);
      }
      else
	result = pm;
    }  
  }
  return result;
}

QPixmap KWM::icon(Window w, int width, int height){
  QPixmap result;
  Pixmap p = None;
  Pixmap p_mask = None;

  XWMHints *hints = XGetWMHints(qt_xdisplay(), w);
  if (hints && (hints->flags & IconPixmapHint)){
    p = hints->icon_pixmap;
  }
  if (hints && (hints->flags & IconMaskHint)){
    p_mask = hints->icon_mask;
  }
  if (hints)
    XFree(hints);

  if (p != None){
    Window root;
    int x, y;
    unsigned int w = 0;
    unsigned int h = 0; 
    unsigned int border_w, depth;
    XGetGeometry(qt_xdisplay(), p,
		 &root, 
		 &x, &y, &w, &h, &border_w, &depth);
    if (w > 0 && h > 0){
      QPixmap pm(w, h, depth);
      XCopyArea(qt_xdisplay(), p, pm.handle(),
		qt_xget_temp_gc(depth==1),
		0, 0, w, h, 0, 0);
      if (p_mask != None){
	QBitmap bm(w, h);
	XCopyArea(qt_xdisplay(), p_mask, bm.handle(),
		  qt_xget_temp_gc(TRUE),
		  0, 0, w, h, 0, 0);
	pm.setMask(bm);
      }
      if (width > 0 && height > 0 && (w > (unsigned int)width 
				      || h > (unsigned int) height)){
	// scale
	QWMatrix m;
	m.scale(width/(float)w, height/(float)h);
	result = pm.xForm(m);
      }
      else
	result = pm;
    }  
  }
  return result;
}

int KWM::desktop(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_DESKTOP", False);
  if (isSticky(w))
      return currentDesktop();
  long result = 1;
  if (!getSimpleProperty(w, a, result) || result <= 0){
    result = currentDesktop();
    moveToDesktop(w, (int) result);
    kwm_error = TRUE; // restore error
  }
  return (int) result;
}
QRect KWM::geometry(Window w){
  XWindowAttributes attr;
  QRect result;
  if (XGetWindowAttributes(qt_xdisplay(), w, &attr)){
    int x, y;
    Window child;
    XTranslateCoordinates(qt_xdisplay(),
			  w, qt_xrootwin(),
			  0, 0, &x, &y, &child);
    result.setRect(x-attr.x, y-attr.y, attr.width, attr.height);
  }
  return result;
}
QRect KWM::geometryRestore(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_GEOMETRY_RESTORE", False);
  QRect result;
  if (!getQRectProperty(w, a, result)){
    result = geometry(w);
    setGeometryRestore(w, result);
  }
  return result;
}
bool KWM::isIconified(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_ICONIFIED", False);
  long result = 0;
  if (!getSimpleProperty(w, a, result)){
    setIconify(w, result != 0);
  }
  return result != 0;
}
bool KWM::isMaximized(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_MAXIMIZED", False);
  long result = 0;
  if (!getSimpleProperty(w, a, result)){
    setMaximize(w, result != 0);
  }
  return result != 0;
}
bool KWM::isSticky(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_STICKY", False);
  long result = 0;
  if (!getSimpleProperty(w, a, result)){
    setSticky(w, result != 0);
  }
  return result != 0;
}
bool KWM::isDecorated(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_DECORATED", False);
  long result = 1;
  if (!getSimpleProperty(w, a, result)){
    setDecoration(w, result != 0);
  }
  return result != 0;
}
bool KWM::fixedSize(Window w){
  XSizeHints size;
  long msize;
  if (XGetWMNormalHints(qt_xdisplay(), w, &size, &msize))
    return (size.flags & PMaxSize ) && (size.flags & PMinSize)
      && (size.max_width <= size.min_width)
      && (size.max_height <= size.min_height);
  return FALSE;
}

bool KWM::containsUnsavedData(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_UNSAVED_DATA", False);
  long result = 0;
  getSimpleProperty(w, a, result);
  return result != 0;
}

bool KWM::unsavedDataHintDefined(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_UNSAVED_DATA", False);
  long tmp = 0;
  return getSimpleProperty(w, a, tmp);
}

bool KWM::isActive(Window w){
  return (w == activeWindow());
}


void KWM::moveToDesktop(Window w, int desk){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_DESKTOP", False);
  setSimpleProperty(w, a, (long) desk);
}
void KWM::setGeometry(Window w, const QRect &geom){
  XMoveResizeWindow(qt_xdisplay(), w, geom.x(), geom.y(),
		    geom.width(), geom.height());
}
void KWM::setGeometryRestore(Window w, const QRect &geom){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_GEOMETRY_RESTORE", False);
  setQRectProperty(w, a, geom);
}
void KWM::move(Window w, const QPoint &pos){
  XMoveWindow(qt_xdisplay(), w, pos.x(), pos.y());
}
void KWM::setMaximize(Window w, bool value){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_MAXIMIZED", False);
  setSimpleProperty(w, a, value?1:0);
}
void KWM::setIconify(Window w, bool value){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_ICONIFIED", False);
  setSimpleProperty(w, a, value?1:0);
}
void KWM::setSticky(Window w, bool value){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_WIN_STICKY", False);
  setSimpleProperty(w, a, value?1:0);
}
void KWM::close(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "WM_DELETE_WINDOW", False);
  static Atom ap = 0;
  if (!ap)
    ap = XInternAtom(qt_xdisplay(), "WM_PROTOCOLS", False);

  // clients with WM_DELETE_WINDOW protocol set are
  // closed via wm_delete_window ClientMessage.
  // Others are destroyed.
  Atom *p;
  int i,n;
  if (XGetWMProtocols(qt_xdisplay(), w, &p, &n)){
    for (i = 0; i < n; i++){
      if (p[i] == a){
	sendClientMessage(w, ap, a);
	XFree(p);
	return;
      }
    }
    if (n>0)
      XFree(p);
  }
  // client will not react on wm_delete_window. We have no choice
  // but destroy his connection to the XServer.
  XDestroyWindow(qt_xdisplay(), w);
}

void KWM::activate(Window w){
  if (desktop(w) != currentDesktop())
    switchToDesktop(desktop(w));
  if (isIconified(w))
    setIconify(w, FALSE);
  raise(w);
  activateInternal(w);
}

void KWM::activateInternal(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_ACTIVATE_WINDOW", False);
  sendClientMessage(qt_xrootwin(), a, (long) w);
}

void KWM::raise(Window w){
  XRaiseWindow(qt_xdisplay(), w);
}

void KWM::lower(Window w){
  XLowerWindow(qt_xdisplay(), w);
}

void KWM::prepareForSwallowing(Window w){
  XWithdrawWindow(qt_xdisplay(), w, qt_xscreen());
  while (getWindowState(w) != WithdrawnState);
}


QString KWM::getMaximizeString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_MAXIMIZE", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}
QString KWM::getUnMaximizeString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_UNMAXIMIZE", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}
QString KWM::getIconifyString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_ICONIFY", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}
QString KWM::getUnIconifyString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_UNICONIFY", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}
QString KWM::getStickyString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_STICKY", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}
QString KWM::getUnStickyString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_UNSTICKY", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}
QString KWM::getMoveString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_MOVE", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}
QString KWM::getResizeString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_RESIZE", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}
QString KWM::getCloseString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_CLOSE", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}

QString KWM::getToDesktopString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_TODESKTOP", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}
QString KWM::getOntoCurrentDesktopString(){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_STRING_ONTOCURRENTDESKTOP", False);
  QString result;
  getQStringProperty(qt_xrootwin(), a, result);
  return result;
}

bool KWM::isDockWindow(Window w){
  static Atom a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "KWM_DOCKWINDOW", False);
  long result = 0;
  getSimpleProperty(w, a, result);
  return result != 0;
}

int KWM::getWindowState(Window w){
  static a = 0;
  if (!a)
    a = XInternAtom(qt_xdisplay(), "WM_STATE", False);
  long result = WithdrawnState;
  getSimpleProperty(w, a, result);
  return (int) result;
}


