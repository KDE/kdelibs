/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Thiago Macieira <thiago.macieira@kdemail.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <qptrlist.h>
#include <qstring.h>

#include <kuniqueapplication.h>
#include <ksocks.h>
#include <ksockaddr.h>
#include <kextsock.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "netsupp.h"

/*
 * These constants tell the flags in KDE::resolverFlags
 * This is copied from ../netsupp.cpp
 */
#define KRF_KNOWS_AF_INET6		0x01	/* if present, the code knows about AF_INET6 */
#define KRF_USING_OWN_GETADDRINFO	0x02	/* if present, we are using our own getaddrinfo */
#define KRF_USING_OWN_INET_NTOP		0x04	/* if present, we are using our own inet_ntop */
#define KRF_USING_OWN_INET_PTON		0x08	/* if present, we are using our own inet_pton */
#define KRF_CAN_RESOLVE_UNIX		0x100	/* if present, the resolver can resolve Unix sockets */
#define KRF_CAN_RESOLVE_IPV4		0x200	/* if present, the resolver can resolve to IPv4 */
#define KRF_CAN_RESOLVE_IPV6		0x400	/* if present, the resolver can resolve to IPv6 */

namespace KDE
{
  extern const int resolverFlags;
}

class TestApp : public KUniqueApplication
{
public:
  TestApp() :
    KUniqueApplication()
  { }

  int newInstance(QValueList<QCString> params);
};

bool tryLookup(const char* node, const char *serv)
{
  int error;
  QString _node = QString::fromLatin1(node);
  QString _serv = QString::fromLatin1(serv);

  printf("\tTrying to lookup %s|%s... ", node, serv);
  QPtrList<KAddressInfo> list = KExtendedSocket::lookup(_node, _serv, 0, &error);
  list.setAutoDelete(true);
  if (!list.isEmpty())
    {
      printf("worked\n");
      return true;
    }

  printf("failed\n\tReason was: %s\n",
	 (const char*)KExtendedSocket::strError(IO_LookupError, error).local8Bit());
  return false;
}

#ifdef AF_INET6
bool try_ntop()
{
  char buf[50];			// 46 is enough
  kde_in6_addr in;

  memset(&in, 0, sizeof(in));
  ((unsigned char*)&in)[15] = 1;	// set this to be ::1

  printf("\tTrying to convert ::1 into string...");
  if (inet_ntop(AF_INET6, &in, buf, sizeof(buf)) == NULL)
    {
      printf("failed\n");
      return false;
    }

  printf("suceeded\n\treturned '%s'\n", buf);
  return strcmp(buf, "::1") == 0;
}

bool try_pton()
{
  const char *buf = "::1";
  kde_in6_addr in;

  printf("\tTrying to convert '::1' into binary form...");
  if (inet_pton(AF_INET6, buf, &in) == 0)
    {
      printf("failed\n");
      return false;
    }

  if (KDE_IN6_IS_ADDR_LOOPBACK(&in))
    {
      printf("succeeded\n");
      return true;
    }

  printf("claims to have suceeded, but returned invalid value\n");
  return false;
}
#endif

bool tryLookup6(const char *node, const char *serv)
{
  int error;
  QString _node = QString::fromLatin1(node);
  QString _serv = QString::fromLatin1(serv);

  printf("\tTrying to lookup IPv6 of %s|%s... ", node, serv);
  QPtrList<KAddressInfo> list = KExtendedSocket::lookup(_node, _serv, KExtendedSocket::ipv6Socket, &error);
  list.setAutoDelete(true);
  if (!list.isEmpty())
    {
      printf("worked\n");
      return true;
    }

  printf("failed\n\tReason was: %s\n",
	 (const char*)KExtendedSocket::strError(IO_LookupError, error).local8Bit());
  return false;
}

bool testKernel()
{
#ifndef AF_INET6
  printf("\tAF_INET6 unknown. Can't test anything\n");
  return false;

#else
  int sock;
  kde_sockaddr_in6 sin6;
  socklen_t len = sizeof(sin6);

  printf("\tAF_INET6 == %d\n", AF_INET6);
  printf("\tTrying to create an IPv6 socket...");
  sock = socket(AF_INET6, SOCK_STREAM, 0);
  if (sock == -1)
    printf("failed\n\tReason was: %s", strerror(errno));
  else
    {
      printf("succeeded\n");

      if (getsockname(sock, (struct sockaddr*)&sin6, &len) == 0)
	printf("\tSize of kernel's sockaddr_in6 is %d bytes\n", len);
      else
	printf("\tCould not get socket name\n");
    }

  printf("\tSize of KDE's internal sockaddr_in6 is %d bytes\n",
	 sizeof(kde_sockaddr_in6));

# ifdef HAVE_SOCKADDR_IN6
  printf("\tSize of system libraries' sockaddr_in6 is %d bytes\n",
	 sizeof(sockaddr_in6));
# else
  printf("\tSystem libraries don't define sockaddr_in6\n");
# endif

  if (sock == -1)
    return false;

  printf("\tTrying to bind the socket to an address...");
  sin6.sin6_family = AF_INET6;
# ifdef HAVE_SOCKADDR_SA_LEN
  sin6.sin6_len = sizeof(sin6);
# endif
  sin6.sin6_flowinfo = 0;
  sin6.sin6_scope_id = 0;
  sin6.sin6_port = 0;		// bind to any port
  memset(&sin6.sin6_addr, 0, sizeof(sin6.sin6_addr)); // any address

  if (bind(sock, (sockaddr*)&sin6, sizeof(sin6)) == -1)
    {
      printf("failed\n\tReason was: %s\n", strerror(errno));
      close(sock);
      return false;
    }

  printf("succeeded\n");

  KSocketAddress *ksin = KExtendedSocket::localAddress(sock);
  if (ksin != NULL)
    {
      printf("\tWe got socket %s\n", (const char*)ksin->pretty().latin1());
      delete ksin;
    }

  close(sock);
  return true;
#endif // AF_INET6
}

bool tryConnectLocal()
{
  KExtendedSocket ks1("::", "0", KExtendedSocket::ipv6Socket | KExtendedSocket::passiveSocket),
    ks2;
  const KInetSocketAddress *ksin1, *ksin2;

  printf("Attempting a loop-back connection\n\tTrying to listen on socket...");
  if (ks1.listen() != 0)
    {
      printf("failed\n\tReason was: %s\n",
	     (const char*)KExtendedSocket::strError(ks1.status(), ks1.systemError()).local8Bit());
      return false;
    }

  ks1.setBlockingMode(false);
  ksin1 = (KInetSocketAddress*)ks1.localAddress();

  printf("succeeded\n\tWe have socket %s listening\n",
	 (const char*)ksin1->pretty().local8Bit());

  ks2.setAddress("::1", ksin1->port());
  ks2.setSocketFlags(KExtendedSocket::ipv6Socket | KExtendedSocket::noResolve);

  printf("\tTrying to connect to that socket...");
  if (ks2.connect() != 0)
    {
      printf("failed\n\tReason was: %s\n",
	     (const char*)KExtendedSocket::strError(ks2.status(), ks2.systemError()).local8Bit());
      return false;
    }

  printf("suceeded\n");

  ksin2 = dynamic_cast<const KInetSocketAddress *>(ks2.localAddress());
  printf("\tIf you may flip to another terminal/xterm and run netstat to see\n"
	 "\tthis connection. It should be a connection from %s to %s.\n"
	 "\tPress any key to continue\n", 
	 (const char*)ksin2->pretty().local8Bit(), (const char*)ksin1->pretty().local8Bit());
  getchar();
  return true;
}

bool tryConnectRemote()
{
  KExtendedSocket ks("www.6bone.net", "80", KExtendedSocket::ipv6Socket);

  printf("\nAttempting a remote connection to www.6bone.net|80\n");

  if (ks.connect() != 0)
    {
      printf("\tConnection failed with error: %s\n",
	     (const char*)KExtendedSocket::strError(ks.status(), ks.systemError()).local8Bit());
      return false;
    }

  printf("\tConnection succeeded\n");
  return true;
}

void go()
{
  int rf = KDE::resolverFlags;
  printf("The resolver claims to:\n");
  if (rf & KRF_USING_OWN_GETADDRINFO)
    {
      printf(" - Be using its own version of getaddrinfo()\n");
      if (rf & KRF_CAN_RESOLVE_UNIX)
	printf(" - Be able to resolve Unix-domain sockets\n");
      else
	printf(" - Be unable to resolve Unix-domain sockets -- This shouldn't happen\n");
      if (rf & KRF_CAN_RESOLVE_IPV4)
	printf(" - Be able to resolve IPv4 Internet sockets\n");
      else
	printf(" - Be unable to resolve IPv4 Internet sockets -- This shouldn't happen\n");
      if (rf & KRF_CAN_RESOLVE_IPV6)
	printf(" - Be able to resolve IPv6 Internet sockets\n");
      else
	printf(" - Be unable to resolve IPv6 Internet sockets\n");
    }
  else
    printf(" - Be using the system getaddrinfo()\n");

  if (rf & KRF_USING_OWN_INET_NTOP)
    printf(" - Be using its own inet_ntop()\n");
  else
    printf(" - Be using the system inet_ntop()\n");

  if (rf & KRF_USING_OWN_INET_PTON)
    printf(" - Be using its own inet_pton()\n");
  else
    printf(" - Be using the system inet_pton()\n");

  if (rf & KRF_KNOWS_AF_INET6)
    printf(" - To know the value of AF_INET6\n");
  else
    printf(" - Not to know the value of AF_INET6\n");

  printf("\n\nGeneral conclusion is:\n");
  if ((rf & KRF_USING_OWN_GETADDRINFO) == 0 &&
      rf & KRF_KNOWS_AF_INET6)
    printf(" Your system probably supports full IPv6 implementation.\n"
	   " This depends on whether your system's getaddrinfo() supports IPv6.\n"
	   " However, KDE Libraries were compiled to use the support whenever available.\n");
  else if ((rf & (KRF_USING_OWN_GETADDRINFO|KRF_KNOWS_AF_INET6)) == 0)
    printf(" Your system supports partial IPv6 implementation.\n"
	   " That is, your system has a getaddrinfo() implementation, but KDE Libraries\n"
	   " don't know how to detect an IPv6 socket. That means that only request to"
	   " any kind of socket will use IPv6, if your getaddrinfo() returns them.");
  else if (rf & KRF_USING_OWN_GETADDRINFO)
    {
      if (rf & KRF_KNOWS_AF_INET6)
	{
	  printf(" Your system supports partial IPv6 implementation.\n");
	  if (rf & KRF_CAN_RESOLVE_IPV6)
	    printf(" The KDE implementation of getaddrinfo() claims to be able to resolve\n"
		   " IPv6 lookups and the value of AF_INET6 is known.\n");
	  else
	    printf(" The KDE implementation of getaddrinfo() cannot resolve IPv6 lookups.\n"
		   " That means that IPv6 support is limited to two addresses (:: and ::1)\n");
	}
      else
	printf(" Your system doesn't support IPv6\n");
    }

  /* Make sure KSocks doesn't interfere in testing */
  KSocks::disable();

  /* Begin testing */
  printf("\nReady to start testing\nPress any key to continue...");
  getchar();
  printf("\n");

  /* Start with simple lookups */
  printf("Trying simple lookups\n"
	 "All of the following look ups should work\n\n");
  tryLookup(NULL, "/tmp/something");
  tryLookup("127.0.0.1", "80");
  tryLookup("localhost", "http");

#ifdef AF_INET6
  printf("\nPress any key for next test");
  getchar();

  printf("\nThis test determines if the inet_ntop and inet_pton functions work\n");
  try_ntop();
  try_pton();
#endif

  printf("\nPress any key for next test");
  getchar();

  printf("\nThis test determines how far the IPv6 resolution can go\n");
  if (!tryLookup6("::1", "80"))
    printf("Your system can't resolve a numeric IPv6 address\n");
  else if (!tryLookup6("localhost", "80"))
    printf("Your system can resolve a numeric IPv6 address, but not localhost\n");
  else if (!tryLookup6("www.6bone.net", "80"))
    printf("Your system can resolve numeric IPv6 addresses and localhost, \n"
	   "but cannot resolve an external address to IPv6 (www.6bone.net)\n");
  else
    printf("Your system can resolve any kind of IPv6 addresses\n");

  printf("\nPress any key for next test");
  getchar();

  printf("\nThis test determines how supported IPv6 is in your kernel\n");
  testKernel();

  printf("\nPress any key for next test");
  getchar();

  printf("\nThis test determines if you can connect to IPv6 addresses via TCP\n");
  tryConnectLocal();
  tryConnectRemote();

  printf("\n\nTest finished\n");
}

int TestApp::newInstance(QValueList<QCString> /*params*/)
{
  go();
}

int main(int argc, char **argv)
{
  KAboutData about("socktest2", "SockTest", "1.0");
  KCmdLineArgs::init(argc, argv, &about);
  KUniqueApplication::addCmdLineOptions();

  /*  TestApp a;
      a.exec();*/
  go();
}
