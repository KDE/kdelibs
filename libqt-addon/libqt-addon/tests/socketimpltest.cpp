#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>

#include <qsocketimpl.h>
#include <qsocketaddress.h>
#include <qresolver.h>
#include <qsocketaddress.h>

#include <iostream>
using namespace std;

int socktype;

QSocketImpl* testserving()
{
  cout << "Testing creation of a server socket" << endl;
  QInetSocketAddress addrV6(QIpAddress::anyhostV6, 0),
    addrV4(QIpAddress::anyhostV4, 0);

  QSocketImpl* socket = new QSocketImpl;
  cout << "Trying to bind to " << addrV6.toString().latin1() << endl;
  if (!socket->bind(QResolverEntry(addrV6, socktype, 0)))
    {
      cout << "Failed to bind to " << addrV6.toString().latin1() << endl;

      cout << "Trying " << addrV4.toString().latin1() << endl;
      if (!socket->bind(QResolverEntry(addrV4, socktype, 0)))
	{
	  cout << "Failed as well" << endl;
	  delete socket;
	  return 0L;
	}
    }

  cout << "Bound to: " << socket->localAddress().toString().latin1() << endl;

  if (!socket->listen())
    cout << "Failed to place socket in listen mode" << endl;
  return socket;
}

QSocketImpl* testconnecting(QSocketAddress addr)
{
  QSocketImpl* socket = new QSocketImpl;
  cout << "Testing connection of client socket" << endl;

  if (!socket->connect(QResolverEntry(addr, socktype, 0)))
    {
      cout << "Connection failed to " << addr.toString().latin1() << endl;
      delete socket;
      return 0L;
    }

  cout << "Connection succeeded" << endl
       << "Local address is: " << socket->localAddress().toString().latin1() << endl
       << "Peer address is: " << socket->peerAddress().toString().latin1() << endl;
  return socket;
}

QSocketImpl* testaccepting(QSocketImpl* server)
{
  cout << "Testing accepting of clients" << endl;
  QSocketImpl* accepted = server->accept();

  if (accepted)
    cout << "Accepted one client, peer address: " << accepted->peerAddress().toString().latin1() << endl;
  else
    cout << "Failed to accept" << endl;

  return accepted;
}

bool testio(QSocketImpl* c1, QSocketImpl* c2)
{
  static char sendbuf[] = "Test message";
  char recvbuf[sizeof sendbuf];
  Q_LONG recvd, sent;
  QSocketAddress sa;

  cout << "Testing I/O through sockets: from "
       << c1->localAddress().toString().latin1() << " to "
       << c2->localAddress().toString().latin1() << endl;

  if ((sent = c1->writeBlock(sendbuf, sizeof sendbuf - 1, c2->localAddress())) == -1)
    {
      cout << "Failed to write" << endl;
      return false;
    }
  cout << "Wrote " << sent << " bytes" << endl;

  if ((recvd = c2->readBlock(recvbuf, sizeof recvbuf, sa)) == -1)
    {
      cout << "Failed to read" << endl;
      return false;
    }
  cout << "Received " << recvd << " bytes";
  if (sa)
    cout << " from " << sa.toString().latin1();
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

  QInetSocketAddress addrV6(QIpAddress::localhostV6, 0),
    addrV4(QIpAddress::localhostV4, 0);

  QSocketImpl* c1 = new QSocketImpl;
  cout << "Trying to bind to " << addrV6.toString().latin1() << endl;
  if (!c1->bind(QResolverEntry(addrV6, socktype, 0)))
    {
      cout << "Failed to bind to " << addrV6.toString().latin1() << endl;

      cout << "Trying " << addrV4.toString().latin1() << endl;
      if (!c1->bind(QResolverEntry(addrV4, socktype, 0)))
	{
	  cout << "Failed as well" << endl;
	  delete c1;
	  return false;
	}
    }

  cout << "Bound to: " << c1->localAddress().toString().latin1() << endl;

  QSocketImpl *c2 = new QSocketImpl;
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
  QSocketImpl *server, *client, *accepted;

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

  socktype = SOCK_STREAM;
  runtest();

  socktype = SOCK_DGRAM;
  runtest();

  return 0;
}
