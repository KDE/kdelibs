#include <unistd.h>

#include <QCoreApplication>
#include <QString>
#include <QByteArray>

#include <iostream>
using namespace std;

#include "serversockettest.h"

int timeout = 0;
Test::Test(const QString &host, const QString &service, bool blocking)
  : socket(host, service)
{
  QObject::connect(&socket, SIGNAL(gotError(int)), this, SLOT(gotErrorSlot(int)));
  QObject::connect(&socket, SIGNAL(hostFound()), this, SLOT(hostFoundSlot()));
  QObject::connect(&socket, SIGNAL(bound(const KResolverEntry&)), 
		   this, SLOT(boundSlot(const KResolverEntry&)));
  QObject::connect(&socket, SIGNAL(closed()), this, SLOT(closedSlot()));
  QObject::connect(&socket, SIGNAL(readyAccept()), this, SLOT(readyAcceptSlot()));
  socket.setBlocking(blocking);
  socket.setAcceptBuffered(false);
  if (!socket.listen())
    exit(1);
  if (blocking)
    readyAcceptSlot();
}

void Test::gotErrorSlot(int errorcode)
{
  cerr << "Socket got error " << errorcode << endl;
  if (socket.isFatalError(errorcode))
    QCoreApplication::exit(1);
}

void Test::hostFoundSlot()
{
  cout << "Socket name lookup finished; got "
       << socket.resolverResults().count() << " results" << endl;
}

void Test::boundSlot(const KResolverEntry& target)
{
  cout << "Socket has bound to " << target.address().toString().latin1() 
       << " (really " << socket.localAddress().toString().latin1() << ")" << endl;
}

void Test::closedSlot()
{
  cout << "Socket has closed" << endl;
  QCoreApplication::exit(0);
}
    
void Test::readyAcceptSlot()
{
  cout << "Socket is ready to accept" << endl;
  KStreamSocket* newsocket = socket.accept();

  if (!newsocket)
    {
      cout << "Couldn't accept anything" << endl;
      return;
    }

  cout << "Accepted connection from "
       << newsocket->peerAddress().toString().latin1() << endl;
  
  QByteArray data("HTTP/1.0 200 Ok\r\n\r\n<html><p>Hello</p></html>");

  cout << endl << "Socket is ready for writing; will write: " << endl;
  cout << data.data() << endl;

  newsocket->write(data.data(), data.length());
  newsocket->deleteLater();
}

int main(int argc, char **argv)
{
  QCoreApplication a(argc, argv);

  bool blocking = false;
  int c;
  while ((c = getopt(argc, argv, "bt:")) != -1)
    switch (c)
      {
      case 'b':
	blocking = true;
	break;

      case 't':
	timeout = QLatin1String(optarg).toInt();
	break;
      }

  if (argc - optind < 2)
    return 1;

  Test test(QString::fromLocal8Bit(argv[optind]), QString::fromLocal8Bit(argv[optind + 1]), 
	    blocking);

  return a.exec();
}

#include "serversockettest.moc"
