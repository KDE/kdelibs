/*
 * Overriding X11 functions to provide app-starting indication for KDE.
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <dcopclient.h>
#include <X11/Xlib.h>

// We're overriding these two functions.
int     XMapWindow(Display *, Window);
int     XMapRaised(Display *, Window);

typedef Window (*KDE_XMapRequestSignature)(Display *, Window);

// These two will be set to the real X calls after the app-starting signal is
// sent.
void *  KDE_RealXMapWindow = NULL;
void *  KDE_RealXMapRaised = NULL;

// Utilities.
void    KDE_InterceptXMapRequest();
bool    KDE_SendXMapRequestNotification();
bool    KDE_DlopenXLibs();

  void
KDE_InterceptXMapRequest()
{
  putenv("LD_PRELOAD=");

  if (!KDE_DlopenXLibs()) {
    fprintf(stderr, "KDE: Could not resolve symbols in libX11\n");
    abort();
  }

  if (!KDE_SendXMapRequestNotification())
    fprintf(stderr, "KDE: Could not send app-starting signal\n");
  else
    fprintf(stderr, "kmapnotify: Sent app-starting signal for pid %d OK\n", getpid());
}

  extern
  int
XMapWindow(Display * d, Window w)
{
  if (NULL == KDE_RealXMapWindow)
    KDE_InterceptXMapRequest();

  return ((KDE_XMapRequestSignature)KDE_RealXMapWindow)(d, w);
}

  extern
  int
XMapRaised(Display * d, Window w)
{
  if (NULL == KDE_RealXMapRaised)
    KDE_InterceptXMapRequest();

  return ((KDE_XMapRequestSignature)KDE_RealXMapRaised)(d, w);
}

  bool
KDE_SendXMapRequestNotification()
{
  DCOPClient c;

  bool attached = c.attach();

  if (!attached) {

    fprintf(stderr, "could not attach to DCOP\n");
    return false;
  }

  QByteArray param;
  QDataStream str(param, IO_WriteOnly);

  str << getpid();

  if (c.send("kicker", "TaskbarApplet", "clientMapped(int)", param)) {

    return true;

  } else {

    fprintf(stderr, "could not send to kicker\n");
    return false;
  }
}

  bool
KDE_DlopenXLibs()
{
  void * KDE_LibX11Handle = dlopen("libX11.so", RTLD_GLOBAL | RTLD_NOW);

  if (NULL == KDE_LibX11Handle)
    KDE_LibX11Handle = dlopen("libX11.so.6", RTLD_GLOBAL | RTLD_NOW);

  if (NULL == KDE_LibX11Handle) {
    fprintf(stderr, "KDE: Could not dlopen libX11\n");
    return false;
  }

  KDE_RealXMapWindow = dlsym(KDE_LibX11Handle, "XMapWindow");

  if (NULL == KDE_RealXMapWindow) {
    fprintf(stderr, "KDE: Could not find symbol XMapWindow in libX11\n");
    return false;
  }
 
  KDE_RealXMapRaised = dlsym(KDE_LibX11Handle, "XMapRaised");

  if (NULL == KDE_RealXMapRaised) {
    fprintf(stderr, "KDE: Could not find symbol XMapRaised in libX11\n");
    return false;
  }

  return true;
}

