/*
 * Override X11 functions and communicate with kxmlrpcd, to provide
 * app-starting indication.
 *
 * The original functions XMapWindow() and XMapRaised() are replaced with new
 * versions. These will call the function KDE_InterceptXMapRequest() if it
 * hasn't been called before, then call the original function.
 *
 * KDE_InterceptXMapRequest() first dlopen()s libX11 and sets
 * KDE_RealXMapRaised and KDE_RealXMapWindow to the original functions, so
 * that they may be called later.
 *
 * Next, we reads the file ~/.kxmlrpcd and extracts the port and key. We then
 * attempt to connect to localhost on the given port, authenticate ourselves
 * using the key, and send a signal out, informing whichever app wants to know
 * (currently kicker) that the application we are being preloaded before is
 * executing.
 *
 * If any part of the process of dlopen()ing libX11 and finding the functions
 * XMapRaised() and XMapWindow() fails, this is a fatal error. The dynamic
 * linker would normally do this automatically, so if we can't do it, we
 * presume it wouldn't be able to either. Hopefully this is a valid
 * presumption.
 *
 * If any part of the process of talking to kxmlrpcd fails, this is non-fatal.
 * It is not vital that we send a signal out, so we just give up.
 *
 * Note: All symbols should be prefixed with KDE_, to avoid polluting the
 * global namespace.
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */

#include <sys/types.h>

/* Stolen from ksock.h */
/* we define STRICT_ANSI to get rid of some warnings in glibc */
#ifndef __STRICT_ANSI__
#define __STRICT_ANSI__
#define _WE_DEFINED_IT_
#endif
#include <sys/socket.h>
#ifdef _WE_DEFINED_IT_
#undef __STRICT_ANSI__
#undef _WE_DEFINED_IT_
#endif

#include <netinet/in.h>
#include <netdb.h>

#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include <X11/Xlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define ksize_t size_t
#endif

#ifdef INET6
#define KDE_SOCKADDR_IN sockaddr_in6
#define KDE_PF_INET PF_INET6
#define KDE_SET_SIN_FAMILY(a, b) (a).sin6_family = b
#define KDE_SET_SIN_PORT(a, b) (a).sin6_port = b
#define KDE_SET_SIN_ADDR(a, b) memcpy(&((a).sin6_addr), b->h_addr_list[0], b->h_length)
#else
#define KDE_SOCKADDR_IN sockaddr_in
#define KDE_PF_INET PF_INET
#define KDE_SET_SIN_FAMILY(a, b) (a).sin_family = b
#define KDE_SET_SIN_PORT(a, b) (a).sin_port = b
#define KDE_SET_SIN_ADDR(a, b) memcpy(&((a).sin_addr), b->h_addr_list[0], b->h_length)
#endif

// We're overriding these two functions.
int XMapWindow(Display *, Window);
int XMapRaised(Display *, Window);

typedef Window (*KDE_XMapRequestSignature)(Display *, Window);

KDE_XMapRequestSignature KDE_RealXMapWindow = NULL;
KDE_XMapRequestSignature KDE_RealXMapRaised = NULL;

void KDE_InterceptXMapRequest();

int KDE_WriteToSocket(int sock, const char * data)
{
  int bytesWritten;
  int retval = 0;
  int stringLength = strlen(data);

  bytesWritten = write(sock, data, stringLength); 

  if (-1 == bytesWritten) {
    fprintf(stderr, "kmapnotify: ");
    perror("write");
    close(sock);
    retval = -1;
  }

  if (bytesWritten != stringLength) {
    fprintf(stderr, "kmapnotify: Short write to socket !\n");
    close(sock);
    retval = -1;
  }

  return retval;
}

  extern
  int
XMapWindow(Display * d, Window w)
{
  if (NULL == KDE_RealXMapWindow)
    KDE_InterceptXMapRequest();

  return KDE_RealXMapWindow(d, w);
}

  extern
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
  struct KDE_SOCKADDR_IN serverAddress; 

  struct hostent * hostinfo = 0L;

  int     fd                = -1;
  void  * libX11Handle      = 0L;
  char  * homeDir           = 0L;
  char  * xmlrpcFile        = 0L;
  char  * xmlrpcBuf         = 0L;
  char  * commaPosition     = 0L;
  char  * key               = 0L;
  int     port              = -1;
  ssize_t bytesRead         = 0;
  int     sock              = -1;
  int     connectOK         = 0;
  int     contentLength     = 0;
  char  * contentLengthStr  = 0L;
  char  * authParamStr      = 0L;
  char  * pidParamStr       = 0L;

  const char * xmlrpcFilename = "/.kxmlrpcd";
 
  const char * commandStart = "<?xml version=\"1.0\"?><methodCall><methodName>AppStarting.clientMapped</methodName><params>";

  const char * textParamPattern = "<param><value>%s</value></param>";
  const char * longParamPattern = "<param><value>%ld</value></param>";

  const char * commandEnd = "</params></methodCall>";

  const char * header =
    "POST /kicker HTTP/1.0\nContent-Type: text/xml\nContent-length: ";

  putenv("LD_PRELOAD=");

  libX11Handle = dlopen("libX11.so", RTLD_GLOBAL | RTLD_NOW);

  if (NULL == libX11Handle)
    libX11Handle = dlopen("libX11.so.6", RTLD_GLOBAL | RTLD_NOW);

  if (NULL == libX11Handle) {
    fprintf(stderr, "KDE: Could not dlopen libX11\n");
    exit(1);
  }

  KDE_RealXMapWindow = dlsym(libX11Handle, "XMapWindow");

  if (NULL == KDE_RealXMapWindow) {
    fprintf(stderr, "KDE: Could not find symbol XMapWindow in libX11\n");
    exit(1);
  }
 
  KDE_RealXMapRaised = dlsym(libX11Handle, "XMapRaised");

  if (NULL == KDE_RealXMapRaised) {
    fprintf(stderr, "KDE: Could not find symbol XMapRaised in libX11\n");
    exit(1);
  }
  
  homeDir = getenv("HOME");

  if (NULL == homeDir) {
    fprintf(stderr, "KDE: Could not find home directory !\n");
    return;
  }

  xmlrpcFile = (char *)malloc(strlen(homeDir) + strlen(xmlrpcFilename));

  if (NULL == xmlrpcFile) {
    fprintf(stderr, "kmapnotify: ");
    perror("malloc");
    return;
  }

  memcpy(xmlrpcFile, homeDir, strlen(homeDir));
  memcpy(xmlrpcFile + strlen(homeDir), xmlrpcFilename, strlen(xmlrpcFilename) + 1);

  fd = open(xmlrpcFile, O_RDONLY);

  free(xmlrpcFile);

  if (-1 == fd) {
    fprintf(stderr, "kmapnotify: ");
    perror("open");
    return;
  }

  xmlrpcBuf = (char *)malloc(64);

  if (NULL == xmlrpcBuf) {
    fprintf(stderr, "kmapnotify: ");
    perror("malloc");
    return;
  }

  bytesRead = read(fd, (void *)xmlrpcBuf, 64);

  if (-1 == bytesRead) {
    fprintf(stderr, "kmapnotify: ");
    perror("read");
    return;
  }

  close(fd);

  commaPosition = (char *)memchr(xmlrpcBuf, ',', bytesRead);

  if (NULL == commaPosition) {
    fprintf(stderr, "kmapnotify: .kxmlrpcd file is in wrong format !\n");
    return;
  }

  *commaPosition = '\0';
  port = atoi(xmlrpcBuf);

  key = ++commaPosition;
  key[16] = '\0';

  sock = socket(KDE_PF_INET, SOCK_STREAM, 0);

  if (-1 == sock) {
    fprintf(stderr, "kmapnotify: ");
    perror("socket");
    return;
  }

  serverAddress.sin_family = KDE_PF_INET;

  memset(&serverAddress, 0, sizeof(serverAddress));

  hostinfo = gethostbyname("localhost");
  
  if (NULL == hostinfo) {
    fprintf(stderr, "kmapnotify: ");
    perror("gethostbyname");
    return;
  }

  KDE_SET_SIN_FAMILY(serverAddress, hostinfo->h_addrtype);
  KDE_SET_SIN_PORT(serverAddress, htons(port));
  KDE_SET_SIN_ADDR(serverAddress, hostinfo);

  connectOK = connect(sock, &serverAddress, sizeof(serverAddress));

  if (-1 == connectOK) {
    fprintf(stderr, "kmapnotify: ");
    perror("connect");
    return;
  }

  authParamStr = (char *)malloc(strlen(textParamPattern) + 14);

  if (NULL == authParamStr) {
    fprintf(stderr, "kmapnotify: ");
    perror("malloc");
    return;
  }

  pidParamStr = (char *)malloc(strlen(longParamPattern) + sizeof(long) * 8 - 2);

  if (NULL == pidParamStr) {
    fprintf(stderr, "kmapnotify: ");
    perror("malloc");
    return;
  }

  sprintf(authParamStr, textParamPattern, key);
  sprintf(pidParamStr, longParamPattern, (long)getpid());

  contentLength =
    strlen(commandStart) +
    strlen(authParamStr) +
    strlen(pidParamStr) +
    strlen(commandEnd);

  contentLengthStr = (char *)malloc(sizeof(int) * 8);

  if (NULL == contentLengthStr) {
    fprintf(stderr, "kmapnotify: ");
    perror("malloc");
    return;
  }

  sprintf(contentLengthStr, "%d", contentLength);
  
  if (0 != KDE_WriteToSocket(sock, header))           return;
  if (0 != KDE_WriteToSocket(sock, contentLengthStr)) return;
  if (0 != KDE_WriteToSocket(sock, commandStart))     return;
  if (0 != KDE_WriteToSocket(sock, authParamStr))     return;
  if (0 != KDE_WriteToSocket(sock, pidParamStr))      return;
  if (0 != KDE_WriteToSocket(sock, commandEnd))       return;

  fprintf(stderr, "kmapnotify finished OK\n");
  close(sock);
}

// vim:ts=2:sw=2:tw=78
