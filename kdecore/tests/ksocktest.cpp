/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kuniqueapp.h"
#include "kglobal.h"
#include "kdebug.h"
#include "ksock.h"
#include "ksockaddr.h"
#include "kextsock.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

bool check(QString txt, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}


int
main(int argc, char *argv[])
{
   KAboutData about("socktest", "SockTest", "version");
   KCmdLineArgs::init(argc, argv, &about);
   KApplication::addCmdLineOptions();

   KApplication app;

   QString host, port;

   KInetSocketAddress host_address("213.203.58.36", 80);

   check("KInetSocketAddress(\"213.203.58.36\", 80)", host_address.pretty(), "213.203.58.36 port 80");

   int result = KExtendedSocket::resolve(&host_address, host, port, NI_NAMEREQD);
   printf( "resolve result: %d\n", result );
   check("KExtendedSocket::resolve() host=", host, "www.kde.org");
//   check("KExtendedSocket::resolve() port=", port, "http");
   QList<KAddressInfo> list;
   list = KExtendedSocket::lookup("www.kde.org", "http", KExtendedSocket::inetSocket);
   for(KAddressInfo *info = list.first(); info; info = list.next())
   {
      qWarning("Lookup: %s %s %s", info->address()->pretty().latin1(),
		                   info->address()->isEqual(KInetSocketAddress("213.203.58.36", 80)) ?
				   "is equal to" : "is NOT equal to",
				   "213.203.58.36 port 80");
   }
   check("KExtendedSocket::lookup()", list.first()->address()->pretty(), "213.203.58.36 port 80");



   int err;

   QList<KAddressInfo> cns = KExtendedSocket::lookup("www.kde.org", 0, KExtendedSocket::canonName, &err);
   for (KAddressInfo *x = cns.first(); x; x = cns.next()) {
        const char *canon = x->canonname();
        qWarning( "Lookup: %s", canon ? canon : "<Null>");
   }
   check("KExtendedSocket::lookup() canonical", cns.first()->canonname(), "www.kde.org");

   KExtendedSocket * sock2 = new KExtendedSocket( "www.kde.org", 80 );
   check( "KExtendedSocket ctor / connect", QString::number( sock2->connect() ), "0" );

   printf("FD %d\n", sock2->fd());

   KSocketAddress* addr = KExtendedSocket::peerAddress( sock2->fd() );
   check( "peerAddress:", addr->nodeName().latin1(), "213.203.58.36" );

   check( "isEqual:", addr->isEqual(KInetSocketAddress("213.203.58.36", 80)) ? "TRUE" : "FALSE", "TRUE");
   check( "isEqual:", addr->isEqual(KInetSocketAddress("213.203.58.36", 8080)) ? "TRUE" : "FALSE", "FALSE");
   check( "isEqual:", addr->isCoreEqual(KInetSocketAddress("213.203.58.36", 8080)) ? "TRUE" : "FALSE", "TRUE");
}
