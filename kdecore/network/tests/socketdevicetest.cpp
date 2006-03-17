#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>

#include "ksocketdevice.h"
#include "ksocketaddress.h"
#include "kresolver.h"
#include "ksocketaddress.h"

#include <iostream>
using namespace std;
using namespace KNetwork;

int socktype;

KSocketDevice* testserving()
{
  cout << "Testing creation of a server socket" << endl;
  KInetSocketAddress addrV6(KIpAddress::anyhostV6, 0),
    addrV4(KIpAddress::anyhostV4, 0);

  KSocketDevice* socket = new KSocketDevice;
  cout << "Trying to bind to " << addrV6.toString().toLatin1().constData() << endl;
  if (!socket->bind(KResolverEntry(addrV6, socktype, 0)))
    {
      cout << "Failed to bind to " << addrV6.toString().toLatin1().constData() << endl;

      cout << "Trying " << addrV4.toString().toLatin1().constData() << endl;
      if (!socket->bind(KResolverEntry(addrV4, socktype, 0)))
	{
	  cout << "Failed as well" << endl;
	  delete socket;
	  return 0L;
	}
    }

  cout << "Bound to: " << socket->localAddress().toString().toLatin1().constData() << endl;

  if (!socket->listen())
    cout << "Failed to place socket in listen mode" << endl;
  return socket;
}

KSocketDevice* testconnecting(KSocketAddress addr)
{
  KSocketDevice* socket = new KSocketDevice;
  cout << "Testing connection of client socket" << endl;

  if (!socket->connect(KResolverEntry(addr, socktype, 0)))
    {
      cout << "Connection failed to " << addr.toString().toLatin1().constData() << endl;
      delete socket;
      return 0L;
    }

  cout << "Connection succeeded" << endl
       << "Local address is: " << socket->localAddress().toString().toLatin1().constData() << endl
       << "Peer address is: " << socket->peerAddress().toString().toLatin1().constData() << endl;
  return socket;
}

KSocketDevice* testaccepting(KSocketDevice* server)
{
  cout << "Testing accepting of clients" << endl;
  KSocketDevice* accepted = server->accept();

  if (accepted)
    cout << "Accepted one client, peer address: " << accepted->peerAddress().toString().toLatin1().constData() << endl;
  else
    cout << "Failed to accept" << endl;

  return accepted;
}

bool testio(KSocketDevice* c1, KSocketDevice* c2)
{
  static char sendbuf[] = "Test message";
  char recvbuf[sizeof sendbuf];
  qint64 recvd, sent;
  KSocketAddress sa;

  cout << "Testing I/O through sockets: from "
       << c1->localAddress().toString().toLatin1().constData() << " to "
       << c2->localAddress().toString().toLatin1().constData() << endl;

  sa = c2->localAddress();
  if ((sent = c1->writeData(sendbuf, sizeof sendbuf - 1, &sa)) == -1)
    {
      cout << "Failed to write" << endl;
      return false;
    }
  cout << "Wrote " << sent << " bytes" << endl;

  if ((recvd = c2->readData(recvbuf, sizeof recvbuf, &sa)) == -1)
    {
      cout << "Failed to read" << endl;
      return false;
    }
  cout << "Received " << recvd << " bytes";
  if (sa)
    cout << " from " << sa.toString().toLatin1().constData();
  cout << endl;

  if (sent != recvd || memcmp(sendbuf, recvbuf, recvd) != 0)
    {
      cout << "Received and sent data differ" << endl;
      return false;
    }

  cout << "I/O is ok" << endl;
  return true;
}

bool testdgram()
{
  cout << "Testing datagram behaviour" << endl;

  KInetSocketAddress addrV6(KIpAddress::localhostV6, 0),
    addrV4(KIpAddress::localhostV4, 0);

  KSocketDevice* c1 = new KSocketDevice;
  cout << "Trying to bind to " << addrV6.toString().toLatin1().constData() << endl;
  if (!c1->bind(KResolverEntry(addrV6, socktype, 0)))
    {
      cout << "Failed to bind to " << addrV6.toString().toLatin1().constData() << endl;

      cout << "Trying " << addrV4.toString().toLatin1().constData() << endl;
      if (!c1->bind(KResolverEntry(addrV4, socktype, 0)))
	{
	  cout << "Failed as well" << endl;
	  delete c1;
	  return false;
	}
    }

  cout << "Bound to: " << c1->localAddress().toString().toLatin1().constData() << endl;

  KSocketDevice *c2 = new KSocketDevice;
  if (!c2->create(c1->localAddress().family(), socktype, 0))
    {
      cout << "Failed to create second socket" << endl;
      delete c1;
      return false;
    }

  testio(c2, c1);

  delete c1;
  delete c2;
  return true;
}


int runtest()
{
  KSocketDevice *server, *client, *accepted;

  if ((server = testserving()) == 0L)
    return 1;
  cout << endl;

  if ((client = testconnecting(server->localAddress())) == 0L)
    return 1;
  cout << endl;

  if (socktype == SOCK_STREAM)
    {
      if ((accepted = testaccepting(server)) == 0L)
	return 1;
      cout << endl;
      
      if (!testio(client, accepted))
	return 1;
      cout << endl;
      delete accepted;
    }
  else
    if (!testdgram())
      return 1;

  cout << "Test done. Closing sockets." << endl;
  delete server;
  delete client;

  return 0;
}

int main()
{
  signal(SIGPIPE, SIG_IGN);

  cout << "Using stream sockets" << endl;
  socktype = SOCK_STREAM;
  if (runtest())
    exit(1);

  cout << endl << "Using datagram sockets" << endl;
  socktype = SOCK_DGRAM;
  if (runtest())
    exit(1);

  return 0;
}
