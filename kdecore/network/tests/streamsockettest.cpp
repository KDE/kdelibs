#include <unistd.h>

#include <QCoreApplication>
#include <QString>
#include <QByteArray>
#include "ksocketaddress.h"

#include "streamsockettest.h"

#include <iostream>
using namespace std;
using namespace KNetwork;

int timeout = 0;
Test::Test(QString host, QString service, QString lhost, QString lservice, bool blocking)
    : socket(host, service)
{
  QObject::connect(&socket, SIGNAL(stateChanged(int)), this, SLOT(stateChangedSlot(int)));
  QObject::connect(&socket, SIGNAL(gotError(int)), this, SLOT(gotErrorSlot(int)));
  QObject::connect(&socket, SIGNAL(hostFound()), this, SLOT(hostFoundSlot()));
  QObject::connect(&socket, SIGNAL(bound(const KResolverEntry&)), 
		   this, SLOT(boundSlot(const KResolverEntry&)));
  QObject::connect(&socket, SIGNAL(aboutToConnect(const KResolverEntry&, bool&)),
		   this, SLOT(aboutToConnectSlot(const KResolverEntry&)));
  QObject::connect(&socket, SIGNAL(connected(const KResolverEntry&)), 
		   this, SLOT(connectedSlot(const KResolverEntry&)));
  QObject::connect(&socket, SIGNAL(timedOut()),  this, SLOT(timedOutSlot()));
  QObject::connect(&socket, SIGNAL(closed()), this, SLOT(closedSlot()));
  QObject::connect(&socket, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
  QObject::connect(&socket, SIGNAL(readyWrite()), this, SLOT(readyWriteSlot()));
  socket.enableRead(true);
  socket.enableWrite(true);
  socket.setBlocking(blocking);
  socket.bind(lhost, lservice);
  socket.setTimeout(timeout);
  if (!socket.connect())
    exit(1);
}

void Test::stateChangedSlot(int newstate)
{
  cout << "Socket state changed to " << newstate << endl;
}

void Test::gotErrorSlot(int errorcode)
{
  cerr << "Socket got error " << errorcode << endl
       << "Current socket state: " << socket.state() << endl
       << "Device status: " << socket.status() << endl;
  if (socket.isFatalError(errorcode))
    QCoreApplication::exit();
}

void Test::hostFoundSlot()
{
  cout << "Socket name lookup finished; got "
       << socket.localResolver().results().count() << " local results and "
       << socket.peerResolver().results().count() << " peer results" << endl;
}

void Test::boundSlot(const KResolverEntry& target)
{
  cout << "Socket has bound to " << target.address().toString().latin1() 
       << " (really " << socket.localAddress().toString().latin1() << ")" << endl;
}

void Test::aboutToConnectSlot(const KResolverEntry& target)
{
  cout << "Socket is about to connect to " << target.address().toString().latin1() << endl;
}

void Test::connectedSlot(const KResolverEntry& target)
{
  cout << "Socket has connected to " << target.address().toString().latin1() << endl;
}

void Test::timedOutSlot()
{
  cout << "Socket timed out connecting" << endl;
  QCoreApplication::exit();
}

void Test::closedSlot()
{
  cout << "Socket has closed" << endl;
  QCoreApplication::exit();
}
    
void Test::readyReadSlot()
{
  char buf[512];
  socket.readLine(buf, sizeof buf);
  if (strlen(buf) == 0)
    socket.close();		// EOF?
  else
    cout << buf;
}

void Test::readyWriteSlot()
{
  QByteArray data("GET / HTTP/1.0\r\nHost: ");
  data += socket.peerResolver().results().at(0).encodedName();
  data += "\r\n\r\n";
  
  cout << endl << "Socket is ready for writing; will write: " << endl;
  cout << data.data() << endl;
  
  socket.writeBlock(data.data(), data.length());
  socket.enableWrite(false);
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
	timeout = QByteArray(optarg).toInt();
	break;
      }

  if (argc - optind < 2)
    return 1;

  QString lhost, lservice;
  if (argc - optind >= 3)
    lhost = QString::fromLocal8Bit(argv[optind + 2]);
  if (argc - optind >= 4)
    lservice = QString::fromLocal8Bit(argv[optind + 3]);
  Test test(QString::fromLocal8Bit(argv[optind]), QString::fromLocal8Bit(argv[optind + 1]), 
	    lhost, lservice, blocking);

  return a.exec();
}

#include "streamsockettest.moc"
