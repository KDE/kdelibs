#include <qobject.h>
#include <qcstring.h>
#include <qstring.h>
#include <qsocketnotifier.h>
#include <qlist.h>
#include <qvaluelist.h>
#include <qcstring.h>
#include <qdict.h>
#include <qptrdict.h>
#include <qapplication.h>

#define INT32 QINT32
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/ICE/ICElib.h>
extern "C" {
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
}


class DCOPConnection;
class DCOPListener;

typedef QValueList<QCString> QCStringList;

class DCOPServer : public QObject
{
Q_OBJECT
 public:
  DCOPServer();
 ~DCOPServer();

 void* watchConnection( IceConn ice_conn );
 void removeConnection( void* data );
 void processMessage( IceConn iceConn, int opcode, unsigned long length, Bool swap);

 virtual bool receive(const QCString &app, const QCString &obj,
		      const QCString &fun, const QByteArray& data,
		      QByteArray &replyData);

private slots:
  void newClient( int socket );
  void processData( int socket );

 private:
  int majorOpcode;
  QList<DCOPListener> listener;
  QDict<DCOPConnection> appIds;
  QPtrDict<DCOPConnection> clients;
};
