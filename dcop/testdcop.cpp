/*****************************************************************

Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
******************************************************************
*/

#include <kapp.h>
#include <qbitarray.h>
#include <dcopclient.h>
#include <dcopobject.h>

/**
 g++ -o testdcop testdcop.cpp -I$KDEDIR/include -I$QTDIR/include -L$KDEDIR/lib -L$QTDIR/lib -lkdecore

**/


class MyDCOPObject : public DCOPObject
{
public:
  MyDCOPObject(const QCString &name) : DCOPObject(name) {}
  bool process(const QCString &fun, const QByteArray &data,
	       QCString& replyType, QByteArray &replyData);
  void function(const QString &arg1, int arg2) { qDebug("function got arg: %s and %d",arg1.data(), arg2); }
};

bool MyDCOPObject::process(const QCString &fun, const QByteArray &data,
			   QCString& replyType, QByteArray &replyData)
{
  qDebug("in MyDCOPObject::process");
  
  // note "fun" is normlized here (i.e. whitespace clean)
  if (fun == "aFunction(QString,int)") {
    QDataStream args(data, IO_ReadOnly);
    QString arg1;
    int arg2;
    args >> arg1 >> arg2;
    function(arg1, arg2);
    replyType = "void";
    return true;
  }

  return false;
}

int main(int argc, char **argv)
{
  KApplication app(argc, argv, "testdcop");

  QCString replyType;
  QByteArray data, reply;
  DCOPClient *client; client = app.dcopClient();

  client->attach(); // attach to the server, now we can use DCOP service


  client->registerAs( app.name() ); // register at the server, now others can call us.
  qDebug("I registered as '%s'", client->appId().data() );

  if ( client->isApplicationRegistered( app.name() ) )
      qDebug("indeed, we are registered!");


  DCOPObject *obj1 = new MyDCOPObject("object1");

  QDataStream ds(data, IO_WriteOnly);
  ds << QString("fourty-two") << 42;
  if (!client->call(app.name(), "object1", "  aFunction(   QString , int  )", data, replyType, reply))
    qDebug("I couldn't call myself");
  else
      qDebug("return type was '%s'", replyType.data() ); 

  client->send(app.name(), "object1", "  aFunction(   QString , int  )", data );

  int n = client->registeredApplications().count();
  qDebug("number of attached applications = %d", n );

  return app.exec();

  client->detach();
}
