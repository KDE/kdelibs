#include <kapp.h>
#include <qbitarray.h>
#include <dcopclient.h>
#include <dcopobject.h>

/**
 g++ -o dcoptest dcoptest.cpp -I$KDEDIR/include -I$QTDIR/include -L$KDEDIR/lib -L$QTDIR/lib -lkdecore

**/


class MyDCOPObject : public DCOPObject
{
public:
  MyDCOPObject(const QString &name) : DCOPObject(name) {}
  bool process(const QString &fun, const QByteArray &data,
	       QByteArray &replyData);
  void function(const QString &arg) { qDebug("function got arg: %s",arg.latin1()); }
};

bool MyDCOPObject::process(const QString &fun, const QByteArray &data,
			   QByteArray &replyData)
{
  qDebug("in MyDCOPObject::process");
  if (fun == "aFunction") {
    QDataStream args(data, IO_ReadOnly);
    QString arg;
    args >> arg;
    function(arg);
    return true;
  }

  return false;
}

int main(int argc, char **argv)
{
  KApplication app(argc, argv);

  QByteArray data, reply;
  DCOPClient *client; client = app.dcopClient();
  if (!client->call(app.name(), "unknownObj", "unknownFunction", data, reply))
    qDebug("I couldn't call myself.");

  DCOPObject *obj1 = new MyDCOPObject("object1");

  QDataStream ds(data, IO_WriteOnly);
  ds << QString("This is the argument string");

  if (!client->call(app.name(), "object1", "aFunction", data, reply))
    qDebug("I couldn't call myself");

  return app.exec();
}
