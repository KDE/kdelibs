/*
 * Override X11 functions and communicate via DCOP, to provide app-starting
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
 * Next we send a DCOP signal via the C interface.
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 * (C) 2000 Simon Hausmann <hausmann@kde.org>
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include <X11/Xlib.h>
#include <X11/X.h>

#include "dcopc.h"

int XMapWindow(Display *, Window);
int XMapRaised(Display *, Window);

typedef Window (*KDE_XMapRequestSignature)(Display *, Window);

KDE_XMapRequestSignature KDE_RealXMapWindow = NULL;
KDE_XMapRequestSignature KDE_RealXMapRaised = NULL;

void KDE_InterceptXMapRequest();

  int
XMapWindow(Display * d, Window w)
{
  if (NULL == KDE_RealXMapWindow)
    KDE_InterceptXMapRequest();

  return KDE_RealXMapWindow(d, w);
}

  int
XMapRaised(Display * d, Window w)
{
  if (NULL == KDE_RealXMapRaised)
    KDE_InterceptXMapRequest();

  return KDE_RealXMapRaised(d, w);
}

  void
KDE_InterceptXMapRequest()
{
  /* Vars *****************************************************************/

  void  * libX11Handle    = 0L;
  char  * dcopData        = 0L;
  int     dcopDataLength  = 0;

  /* Init *****************************************************************/

  putenv("LD_PRELOAD=");

  /* Find symbols *********************************************************/

  libX11Handle = dlopen("libX11.so", RTLD_GLOBAL | RTLD_NOW);

  if (NULL == libX11Handle)
    libX11Handle = dlopen("libX11.so.6", RTLD_GLOBAL | RTLD_NOW);

  if (NULL == libX11Handle) {
    fprintf(stderr, "KDE: Could not dlopen libX11\n");
    exit(1);
  }

  KDE_RealXMapWindow =
    (KDE_XMapRequestSignature)dlsym(libX11Handle, "XMapWindow");

  if (NULL == KDE_RealXMapWindow) {
    fprintf(stderr, "KDE: Could not find symbol XMapWindow in libX11\n");
    exit(1);
  }

  KDE_RealXMapRaised =
    (KDE_XMapRequestSignature)dlsym(libX11Handle, "XMapRaised");

  if (NULL == KDE_RealXMapRaised) {
    fprintf(stderr, "KDE: Could not find symbol XMapRaised in libX11\n");
    exit(1);
  }

  /* Do some DCOP *********************************************************/

  dcopData = (char *)malloc(128);

  /* marshall the arguments for the function call. here's it's just the pid */
  dcopDataLength = dcop_write_int(dcopData, (int)getpid()) - dcopData;
  
  dcop_send_signal(
    "kicker",
    "TaskbarApplet",
    "clientMapped(int)",
    dcopData,
    dcopDataLength
  );

  /* Done ***************************************************************/
}

/* vim: set ts=2:sw=2:tw=78: */
