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
 * Next we send a DCOP signal... I'll explain what happens there once I figure
 * it out myself.
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 * (C) 2000 Simon Hausmann <hausmann@kde.org>
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <string.h>

#include <dlfcn.h>

/* sometimes __STDC__ is defined, but to 0. The hateful X headers
   ask for '#if __STDC__', so they become confused. */
#ifdef __STDC__
#if !__STDC__
#undef __STDC__
#define __STDC__ 1
#endif
#endif
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/ICE/ICE.h>
#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEconn.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>

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

/* Helper functions ****************************************************/

  char *
writeInt(char * buf, int i)
{
  char * p = (char *)(&i);
  buf[3] = *p++;
  buf[2] = *p++;
  buf[1] = *p++;
  buf[0] = *p;

  return buf + 4;
}

  char *
writeText(char * buf, const char * text)
{
  char * pos = buf;
  int l = strlen( text ) + 1; /* we need the \0! (Simon) */
  pos = writeInt(buf, l);
  memcpy(pos, text, l);
  return pos + l;
}

  void
KDE_InterceptXMapRequest()
{
  /* Vars *****************************************************************/

    struct
  DCOPMsg {
    CARD8 majorOpcode;
    CARD8 minorOpcode;
    CARD8 data[2];
    CARD32 length B32;
    CARD32 time;
  };

    extern
    IcePoAuthStatus
  _IcePoMagicCookie1Proc(
    IceConn,
    void **,
    int,
    int,
    int,
    void *,
    int *,
    void **,
    char **
  );

  const char * receiver     = "kicker";
  const char * object       = "AppStarting";
  const char * function     = "clientMapped(int)";

  char  * pos               = 0L;
  char  * vendor            = 0L;
  char  * release           = 0L;
  char  * newline           = 0L;
  void  * libX11Handle      = 0L;
  char  * homeDir           = 0L;
  char  * dcopServer        = 0L;
  char  * header            = 0L;
  unsigned int headerLen    = 0;
  char  * data              = 0L;
  unsigned int dataLen      = 0;

  int     setupstat         = 0;
  int     majorOpcode       = 0;
  int     fd                = 0;
  int     majorVersion      = 0;
  int     minorVersion      = 0;
  ssize_t bytesRead         = 0;

  char    errBuf[1024];

  IceConn     iceConn;
  IcePointer  context = 0;
  IcePointer  somethingToPassToProtoSetup = 0;

  struct DCOPMsg * pMsgPtr = 0;

/*  extern int _IceLastMajorOpcode; */

  static IcePoVersionRec DCOPVersions[] = {
    { 1, 1, 0L } /* last arg is callback - none ok as 0L ? */
  };

  const char * DCOPAuthNames[] = { "MIT-MAGIC-COOKIE-1" };

  IcePoAuthProc DCOPClientAuthProcs[] = { _IcePoMagicCookie1Proc };

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

  /* Find home dir ********************************************************/

  homeDir = getenv("HOME");

  if (NULL == homeDir) {
    fprintf(stderr, "KDE: Could not find home directory !\n");
    return;
  }

  /* ICE setup ************************************************************/

  majorOpcode =
    IceRegisterForProtocolSetup(
      (char *)("DCOP"),
      (char *)("KDE"),
      (char *)("1.1"),
      1,
      DCOPVersions,
      1,
      (char **)DCOPAuthNames,
      DCOPClientAuthProcs,
      0L
    );

  if (majorOpcode < 0) {
    fprintf(stderr, "IceRegisterForProtocolSetup failed\n");
    return;
  }

  /* Find DCOP server and open connection ********************************/

  dcopServer = getenv("DCOPSERVER");

  if (NULL == dcopServer) {

    if (-1 == chdir(homeDir)) {
      fprintf(stderr, "Could not change directory to %s\n", homeDir);
      return;
    }

    fd = open(".DCOPserver", O_RDONLY);

    if (-1 == fd) {
      fprintf(stderr, "Cannot open ~/.DCOPserver\n");
      return;
    }

    dcopServer = (char *)malloc(1024);

    bytesRead = read(fd, dcopServer, 1024);

    if (-1 == bytesRead) {
      fprintf(stderr, "Cannot read from ~/.DCOPserver\n");
      return;
    }

    newline = strchr(dcopServer, '\n');

    if (NULL == newline) {
      fprintf(stderr, "Invalid data in ~/.DCOPserver\n");
      return;
    }

    *newline = '\0';

    iceConn =
      IceOpenConnection(
        dcopServer,
        context,
        False,
        majorOpcode,
        sizeof(errBuf),
        errBuf
      );

    free(dcopServer);

  } else {

    iceConn =
      IceOpenConnection(
        dcopServer,
        context,
        False,
        majorOpcode,
        sizeof(errBuf),
        errBuf
      );
  }

  if (iceConn == 0L) {
    fprintf(stderr, "IceOpenConnection failed\n");
    return;
  }

  IceSetShutdownNegotiation(iceConn, False);

  /* Setup protocol ******************************************************/

  setupstat =
    IceProtocolSetup(
      iceConn,
      majorOpcode,
      somethingToPassToProtoSetup,
      True, /* must authenticate */
      &(majorVersion), &(minorVersion), &(vendor), &(release),
      1024, errBuf
    );

  if (
    setupstat == IceProtocolSetupFailure ||
    setupstat == IceProtocolSetupIOError ||
    setupstat == IceProtocolAlreadyActive)
  {
    fprintf(stderr, "IceProtocolSetup failed\n");
    return;
  }

  if (IceConnectionStatus(iceConn) != IceConnectAccepted) {
    fprintf(stderr, "DCOP server did not accept the connection\n");
    return;
  }

  /* Send data to DCOP ***************************************************/

  /*
   * First let ICE initialize the ICE Message Header and give us a pointer
   * to it (ICE manages that buffer internally)
   */
  IceGetHeader(
    iceConn,
    majorOpcode,
    2 /* DCOPSend */,
    sizeof(struct DCOPMsg),
    struct DCOPMsg,
    pMsgPtr
  );

  data = (char *)malloc(128);
  header = (char *)malloc(1024);

  /* marshall the arguments for the function call. here's it's just the pid */
  pos = writeInt(data, (int)getpid());

  /* calculate the data length */
  dataLen = pos - data;

  /* marshall the arguments for the DCOP message header (callerApp, destApp,
   * destObj, destFunc.
   * The last argument is actually part of the data part of the call, but
   * we add it to the header. It's the size of the marshalled argument data.
   * In Qt it would look like
   * QDataStream str( ... )
   * str << callerApp << destApp << destObj << destFun << argumentQByteArrayDataStuff;
   * (where as str is the complete data stream sent do the dcopserver, excluding
   *  the ICE header)
   * As the QByteArray is marshalled as [size][data] and as we (below) send the
   * data in two chunks, first the dcop msg header and the the data, we just
   * put the [size] field as last field into the dcop msg header ;-)
   */
  pos = header;

  pos = writeText(pos, "kmapnotify");
  pos = writeText(pos, receiver);
  pos = writeText(pos, object);
  pos = writeText(pos, function);
  pos = writeInt(pos, dataLen );

  headerLen = pos - header;

  pMsgPtr->time = 0;
  /* the length field tells the dcopserver how much bytes the dcop message
   * takes up. We add that size to the already by IceGetHeader initialized
   * length value, as it seems that under some circumstances (depending on the
   * DCOPMsg structure size) the length field is aligned/padded. 
   */
  pMsgPtr->length += headerLen + dataLen;

  /* first let's send the dcop message header 
   * IceSendData automatically takes care of first sending the Ice Message
   * Header (outbufptr > outbuf -> flush the connection buffer)
   */
  IceSendData(iceConn, headerLen, header);
  /* and now the function argument data */
  IceSendData(iceConn, dataLen, data);

  /* send it all ;-) */
  IceFlush( iceConn );

  free(data);
  free(header);

  if (IceConnectionStatus(iceConn) != IceConnectAccepted) {
    fprintf(stderr, "IceConnectionStatus != IceConnectAccepted\n");
    return;
  }

  IceCloseConnection( iceConn );

  /* Done ***************************************************************/
}

/* vim: set ts=2:sw=2:tw=78: */
