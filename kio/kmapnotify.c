/* vi: set ts=2 sw=2 tw=78:
 *
 * Override X11 functions and set X window properties, to provide app-starting
 * notification.
 *
 * The original functions XMapWindow() and XMapRaised() are replaced with new
 * versions. These will call the function KDE_InterceptXMapRequest() if it
 * hasn't been called before, then call the original function.
 *
 * KDE_InterceptXMapRequest() first dlopen()s libX11 and sets
 * KDE_RealXMapRaised and KDE_RealXMapWindow to the original functions, so
 * that they may be called later.
 *
 * Next we set properties on the initial window, which will be picked up by
 * the wm when it sees the window map.
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 * (C) 2000 Simon Hausmann <hausmann@kde.org>
 * (C) 2000 Bill Soudan <soudan@kde.org>
 */

#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <ltdl.h>
#include <config.h>

int XMapWindow(Display *, Window);
int XMapRaised(Display *, Window);

/* this lib, when LD_PRELOADed, overrides XMapWindow and XMapRaised.
 * we need somewhere to store pointers to the real functions */
typedef Window (*KDE_XMapRequestSignature)(Display *, Window);

KDE_XMapRequestSignature KDE_RealXMapWindow = NULL;
KDE_XMapRequestSignature KDE_RealXMapRaised = NULL;

/* other functions we need from X11 are found using lt_dlsym, instead
 * of assuming the application we're preloaded in is linked to X11 */
typedef Atom (*KDE_XInternAtomSignature)(Display *, char *, Bool);
typedef Atom *(*KDE_XListPropertiesSignature)(Display *, Window, int *);
typedef int (*KDE_XChangePropertySignature)(Display *, Window, Atom,
  Atom, int, int, unsigned char *, int);

KDE_XInternAtomSignature KDE_XInternAtom = NULL;
KDE_XListPropertiesSignature KDE_XListProperties = NULL;
KDE_XChangePropertySignature KDE_XChangeProperty = NULL;

static long KDE_initialDesktop = 0;
static long KDE_mapNotifyEnabled = 1;

void KDE_InterceptXMapRequest(Display *, Window);
void KDE_SetInitialDesktop(Display *, Window);
void KDE_SetNetWmPid(Display *, Window);

  int
XMapWindow(Display * d, Window w)
{
  //fprintf(stderr, "XMapWindow\n");
  if (NULL == KDE_RealXMapWindow) {
    //fprintf(stderr, "intercepting XMapWindow\n");
    KDE_InterceptXMapRequest(d, w);
  }

  /* kdeinit needs to have libkmapnotify preloaded, so all new
   * applications spawned are in effect also preloaded.  however,
   * we need to disable kmapnotify for some apps (e.g. kwin) */
  if (KDE_mapNotifyEnabled) {

    /* The first Window that's mapped doesn't seem to actually be the
     * first Window that KWin manages.  The first window that KWin manages
     * seems to be the first one that has more than 0 properties.
     * Is this a valid assumption? */

    /* only set the desktop property on the first window that has
     * properties */
    if (KDE_initialDesktop) {
      int num=0;
      (void)KDE_XListProperties(d, w, &num);
      if (num) {
        KDE_SetInitialDesktop(d, w);
        KDE_initialDesktop=0;
      }
    }

    /* all application windows should have _NET_WM_PID set */
    KDE_SetNetWmPid(d, w);

  }

  return KDE_RealXMapWindow(d, w);
}

  int
XMapRaised(Display * d, Window w)
{
  if (NULL == KDE_RealXMapRaised)
    KDE_InterceptXMapRequest(d, w);

  /* kdeinit needs to have libkmapnotify preloaded, so all new
   * applications spawned are in effect also preloaded.  however,
   * we need to disable kmapnotify for some apps (e.g. kwin) */
  if (KDE_mapNotifyEnabled) {

    /* The first Window that's mapped doesn't seem to actually be the
     * first Window that KWin manages.  The first window that KWin manages
     * seems to be the first one that has more than 0 properties.
     * Is this a valid assumption? */

    /* only set the desktop property on the first window that has
     * properties */
    if (KDE_initialDesktop) {
      int num=0;
      (void)KDE_XListProperties(d, w, &num);
      if (num) {
        KDE_SetInitialDesktop(d, w);
        KDE_initialDesktop=0;
      }
    }

    /* all application windows should have _NET_WM_PID set */
    KDE_SetNetWmPid(d, w);
  }

  return KDE_RealXMapRaised(d, w);
}

  void
KDE_InterceptXMapRequest(Display * d, Window w)
{
  /* Vars *****************************************************************/

  Atom netMapNotify;
  XTextProperty prop;
  Status status;
  char * pidString = 0L;
  char * envStr;

  lt_dlhandle libX11Handle;

  /* Init *****************************************************************/

  envStr = getenv("KDE_INITIAL_DESKTOP");
  if (envStr)
    KDE_initialDesktop = atoi(envStr);

  envStr = getenv("KDE_DISABLE_KMAPNOTIFY");
  if (envStr)
    KDE_mapNotifyEnabled = !atoi(envStr);
  
  unsetenv("LD_PRELOAD");
  unsetenv("KDE_INITIAL_DESKTOP");
  unsetenv("KDE_DISABLE_KMAPNOTIFY");

  /* Find symbols *********************************************************/

  libX11Handle = lt_dlopen("libX11.so");

  if (NULL == libX11Handle)
    libX11Handle = lt_dlopen("libX11.so.6");

  if (NULL == libX11Handle) {
    fprintf(stderr, "KDE: Could not dlopen libX11\n");
    exit(1);
  }

  KDE_RealXMapWindow =
    (KDE_XMapRequestSignature)lt_dlsym(libX11Handle, "XMapWindow");

  if (NULL == KDE_RealXMapWindow) {
    fprintf(stderr, "KDE: Could not find symbol XMapWindow in libX11\n");
    exit(1);
  }

  KDE_RealXMapRaised =
    (KDE_XMapRequestSignature)lt_dlsym(libX11Handle, "XMapRaised");

  if (NULL == KDE_RealXMapRaised) {
    fprintf(stderr, "KDE: Could not find symbol XMapRaised in libX11\n");
    exit(1);
  }

  KDE_XInternAtom =
    (KDE_XInternAtomSignature)lt_dlsym(libX11Handle, "XInternAtom");

  if (NULL == KDE_XInternAtom) {
    fprintf(stderr, "KDE: Could not find symbol XInternAtom in libX11\n");
    exit(1);
  }

  KDE_XListProperties =
    (KDE_XListPropertiesSignature)lt_dlsym(libX11Handle,  "XListProperties");

  if (NULL == KDE_XListProperties) {
    fprintf(stderr, "KDE: Could not find symbol XListProperties in libX11\n");
    exit(1);
  }

  KDE_XChangeProperty =
    (KDE_XChangePropertySignature)lt_dlsym(libX11Handle, "XChangeProperty");

  if (NULL == KDE_XChangeProperty) {
    fprintf(stderr, "KDE: Could not find symbol XChangeProperty in libX11\n");
    exit(1);
  }

}

  void
KDE_SetInitialDesktop(Display *d, Window w) {

  Atom a;
  long net_desktop = KDE_initialDesktop-1;

  a = KDE_XInternAtom(d, "_NET_WM_DESKTOP", False);

  KDE_XChangeProperty(d, w, a, XA_CARDINAL, 32, PropModeReplace, 
    (unsigned char *)&net_desktop, 1);

}

  void
KDE_SetNetWmPid(Display *d, Window w) {

  Atom a = KDE_XInternAtom(d, "_NET_WM_PID", False);
  int pid = getpid();

  KDE_XChangeProperty(d, w, a, XA_CARDINAL, 32, PropModeReplace,
    (unsigned char *)&pid, 1);

}
