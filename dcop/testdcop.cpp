#include <dcopclient.h>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  DCOPClient *client = new MyClient(argv[1]);
  if (!client->attach())
    qDebug("could not attach to DCOP server.");
  else {
    qDebug("successful attach to DCOP server.");
  }

  QByteArray callData, replyData;
  QDataStream ds(callData, IO_WriteOnly);
  ds << QString("ALALALALALALALALALALALALALALALALALALALALALALALA");

  DCOPObject *object1 = new DCOPObject("object1");

  if (argc > 2) {
    if (!client->call(argv[2], "", "fun1", callData, replyData))
      qDebug("failed RPC");
    else {
      QDataStream gotit(replyData, IO_ReadOnly);
      if (!gotit.atEnd()) {
	QString s;
	gotit >> s;
	qDebug("got back an string of %s",s.latin1());
      }
    }

    if (!client->call(argv[2], "object1", "fun1", callData, replyData))
      qDebug("failed RPC");
    else {
      QDataStream gotit(replyData, IO_ReadOnly);
      if (!gotit.atEnd()) {
	QString s;
	gotit >> s;
	qDebug("got back an string of %s",s.latin1());
      }
    }
    
    if (!client->call("DCOPServer", "", "clientList",
		      replyData, replyData))
      qDebug("failed to get client Data");
    else {
      QDataStream reply(replyData, IO_ReadOnly);
      QStringList slist;
      reply >> slist;
      qDebug("list of clients: ");
      for (QStringList::ConstIterator it(slist.begin());
	   it != slist.end(); ++it)
	qDebug("   %s",(*it).latin1());
    }
  }

  return app.exec();
}
