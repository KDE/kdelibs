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
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <string.h>

#include <dlfcn.h>

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

  void
KDE_InterceptXMapRequest()
{
  /* Helper functions ****************************************************/

    char *
  writeLong(char * buf, long i)
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
    pos = writeLong(buf, strlen(text));
    memcpy(pos, text, strlen(text));
    return pos + strlen(text);
  }

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
  const char * function     = "clientMapped(long int)";

  char  * pos               = 0L;
  char  * vendor            = 0L;
  char  * release           = 0L;
  char  * appId             = 0L;
  char  * newline           = 0L;
  void  * libX11Handle      = 0L;
  char  * homeDir           = 0L;
  char  * dcopServer        = 0L;
  char  * streamed          = 0L;
  char  * data              = 0L;

  int     setupstat         = 0; 
  int     streamedSize      = 0;
  int     majorOpcode       = 0;
  int     fd                = 0;
  int     majorVersion      = 0;
  int     minorVersion      = 0;
  ssize_t bytesRead         = 0;

  char    errBuf[1024];

  IceConn     iceConn;
  IcePointer  context;
  IcePointer  somethingToPassToProtoSetup;

  struct DCOPMsg   pMsg;
  struct DCOPMsg * pMsgPtr = &pMsg;

  CARD32  time;

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
  
  /* DISABLED */
  return;
  
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

  /* Register with DCOP **************************************************/

/*  fprintf(stderr, "Registering with DCOP\n"); */

  appId = (char *)malloc(64);
  sprintf(appId, "%s-%d", "anonymous", getpid());

  /* now do equivalent of:
   * call("DCOPServer", "", "registerAs(QCString)", data, replyType,replyData)
   * and check result.
   */

/*  fprintf(stderr, "Registered with DCOP\n"); */

  /* Send data to DCOP ***************************************************/

  IceGetHeader(
    iceConn,
    majorOpcode,
    1 /* DCOPSend */,
    sizeof(struct DCOPMsg),
    struct DCOPMsg,
    pMsgPtr
  );

  data = (char *)malloc(128);
  streamed = (char *)malloc(1024);

  writeLong(data, (long)getpid());

  pos = streamed;

  pos = writeText(pos, appId);
  pos = writeText(pos, receiver);
  pos = writeText(pos, object);
  pos = writeText(pos, function);
  pos = writeLong(pos, strlen(data));

  streamedSize = pos - streamed;

  pMsg.time = time = 0;
  pMsg.length += streamedSize + strlen(data);

  IceSendData(iceConn, streamedSize, streamed);
  IceSendData(iceConn, strlen(data), data);
  
  free(appId);
  free(data);
  free(streamed);

  if (IceConnectionStatus(iceConn) != IceConnectAccepted) {
    fprintf(stderr, "IceConnectionStatus != IceConnectAccepted\n");
    return;
  }
  
  /* Done ***************************************************************/
}

/* vim: set ts=2:sw=2:tw=78: */
