#include <unistd.h>

#include <qapplication.h>
#include <qstring.h>
#include <qcstring.h>

#include <qsocketbase.h>
#include <qstreamsocket.h>
#include <qresolver.h>
#include <qsocketaddress.h>
#include <qserversocket.h>

#include <iostream>
using namespace std;

int timeout = 0;
class Test : public QObject
{
  Q_OBJECT

public:
  QServerSocket socket;

  Test(QString host, QString service, bool blocking)
    : socket(host, service)
  {
    QObject::connect(&socket, SIGNAL(gotError(int)), this, SLOT(gotErrorSlot(int)));
    QObject::connect(&socket, SIGNAL(hostFound()), this, SLOT(hostFoundSlot()));
    QObject::connect(&socket, SIGNAL(bound(const QResolverEntry&)), 
		     this, SLOT(boundSlot(const QResolverEntry&)));
    QObject::connect(&socket, SIGNAL(closed()), this, SLOT(closedSlot()));
    QObject::connect(&socket, SIGNAL(readyAccept()), this, SLOT(readyAcceptSlot()));
    socket.setBlocking(blocking);
    if (!socket.listen())
      exit(1);
    if (blocking)
      readyAcceptSlot();
  }

public slots:
  void gotErrorSlot(int errorcode)
  {
    cerr << "Socket got error " << errorcode << endl;
    QApplication::exit();
  }

  void hostFoundSlot()
  {
    cout << "Socket name lookup finished; got "
	 << socket.resolverResults().count() << " results" << endl;
  }

  void boundSlot(const QResolverEntry& target)
  {
    cout << "Socket has bound to " << target.address().toString().latin1() 
	 << " (really " << socket.localAddress().toString().latin1() << ")" << endl;
  }

  void closedSlot()
  {
    cout << "Socket has closed" << endl;
    QApplication::exit();
  }
    
  void readyAcceptSlot()
  {
    cout << "Socket is ready to accept" << endl;
    QActiveSocketBase* newsocket = socket.accept();

    if (!newsocket)
      {
	cout << "Couldn't accept anything" << endl;
	return;
      }

    cout << "Accepted connection from "
	 << newsocket->peerAddress().toString().latin1() << endl;

    QCString data("HTTP/1.0 200 Ok\r\n\r\n<html><p>Hello</p></html>");

    cout << endl << "Socket is ready for writing; will write: " << endl;
    cout << data.data() << endl;

    newsocket->writeBlock(data.data(), data.length());
    delete newsocket;
  }

};

int main(int argc, char **argv)
{
  QApplication a(argc, argv, false);

  bool blocking = false;
  int c;
  while ((c = getopt(argc, argv, "bt:")) != -1)
    switch (c)
      {
      case 'b':
	blocking = true;
	break;

      case 't':
	timeout = QString::fromLatin1(optarg).toInt();
	break;
      }

  if (argc - optind < 2)
    return 1;

  Test test(QString::fromLocal8Bit(argv[optind]), QString::fromLocal8Bit(argv[optind + 1]), 
	    blocking);

  return a.exec();
}

#include "serversockettest.moc"
