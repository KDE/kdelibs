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

#include "kuniqueapplication.h"
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


class TestApp : public KUniqueApplication
{
public:
   TestApp()
	: KUniqueApplication() { }
   int newInstance( QValueList<QCString> params );
   KExtendedSocket sock;
};


int
TestApp::newInstance( QValueList<QCString> )
{
   printf("Use netstat -t -n in a shell to check actual connection status.\n");
   printf("Connecting to tink:110...\n");
   KSocket *sock1 = new KSocket( "tink", 110);
   printf("OK Fd = %d\n", sock1->socket());
   printf("Connecting to www.kde.org:80...\n");
   KSocket *sock2 = new KSocket( "www.kde.org", 80);
   printf("OK Fd = %d\n", sock2->socket());
   printf("Connecting to www.kde.org:70...\n");
   KSocket *sock3 = new KSocket( "www.kde.org", 70);
   printf("OK Fd = %d\n", sock3->socket());
   exit(0);
   printf("Exiting\n");
   return 1;
}

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
   KUniqueApplication::addCmdLineOptions();

   QString host, port;
   
   KInetSocketAddress host_address("213.203.58.36", 80);

   check("KInetSocketAddress(\"213.203.58.36\", 80)", host_address.pretty(), "213.203.58.36 port 80");

   int result = KExtendedSocket::resolve(&host_address, host, port, NI_NAMEREQD);
   check("KExtendedSocket::resolve() host=", host, "www.kde.org");
   check("KExtendedSocket::resolve() port=", port, "http");
   QPtrList<KAddressInfo> list;
   list = KExtendedSocket::lookup("www.kde.org", "http", KExtendedSocket::inetSocket);
   for(KAddressInfo *info = list.first(); info; info = list.next())
   {
      qWarning("Lookup: %s", info->address()->pretty().latin1());
   }
   check("KExtendedSocket::lookup()", list.first()->address()->pretty(), "213.203.58.36 port 80");

#if 0
   TestApp a(&host_address);
   a.exec();   
#endif
   return 0;
}
