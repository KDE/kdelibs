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
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
******************************************************************
*/

#include <testdcop.h>
#include <qtimer.h>

DCOPClientTransaction *countDownAction = 0;
int countDownCount = 0;

DCOPClientTransaction *countDownAction2 = 0;
int countDownCount2 = 0;

bool MyDCOPObject::process(const Q3CString &fun, const QByteArray &data,
			   Q3CString& replyType, QByteArray &replyData)
{
  qDebug("in MyDCOPObject::process, fun = %s", fun.data());
  
  // note "fun" is normlized here (i.e. whitespace clean)
  if (fun == "aFunction(QString,int)") {
    QDataStream args(data, QIODevice::ReadOnly);
    QString arg1;
    int arg2;
    args >> arg1 >> arg2;
    function(arg1, arg2);
    replyType = "void";
    return true;
  }
  if (fun == "canLaunchRockets(QRect)") {
    QDataStream args(data, QIODevice::ReadOnly);
    QRect arg1;
    args >> arg1;

    printf("Rect x = %d, y = %d, w = %d, h = %d\n", arg1.x(), arg1.y(), arg1.width(), arg1.height());

    replyType = "QRect";
    QDataStream reply( replyData, QIODevice::WriteOnly );
    QRect r(10,20,100,200);
    reply << r;
    return true;
  }
  if (fun == "isAliveSlot(int)") {
    
    qDebug("isAliveSlot(int)");
    bool connectResult = kapp->dcopClient()->disconnectDCOPSignal("", objId(), "", objId(), "" );
    qDebug("disconnectDCOPSignal returns %s", connectResult ? "true" : "false");
    return true;
  }
  if (fun == "countDown()") {
qDebug("countDown() countDownAction = %p", countDownAction);
    if (countDownAction2)
    {
       replyType = "QString";
       QDataStream reply( replyData, QIODevice::WriteOnly );
       reply << QString("Hey");
       return true;
    }

    if (countDownAction == 0)
    {
       countDownCount = 10;       
       countDownAction = kapp->dcopClient()->beginTransaction();
       QTimer::singleShot(1000, this, SLOT(slotTimeout()));
    }
    else
    {
       countDownCount2 = 10;       
       countDownAction2 = kapp->dcopClient()->beginTransaction();
       QTimer::singleShot(1000, this, SLOT(slotTimeout2()));
    }
    return true;
  }

  return DCOPObject::process(fun, data, replyType, replyData);
}

void MyDCOPObject::slotTimeout()
{
  qDebug("MyDCOPObject::slotTimeout() %d", countDownCount);
  countDownCount--;
  if (countDownCount == 0)
  {
     Q3CString replyType = "QString";
     QByteArray replyData;
     QDataStream reply( replyData, QIODevice::WriteOnly );
     reply << QString("Hello World");
     kapp->dcopClient()->endTransaction(countDownAction, replyType, replyData);
     countDownAction = 0;
  }
  else
  {
     QTimer::singleShot(1000, this, SLOT(slotTimeout()));
  }
}

void MyDCOPObject::slotTimeout2()
{
  qDebug("MyDCOPObject::slotTimeout2() %d", countDownCount2);
  countDownCount2--;
  if (countDownCount2 == 0)
  {
     Q3CString replyType = "QString";
     QByteArray replyData;
     QDataStream reply( replyData, QIODevice::WriteOnly );
     reply << QString("Hello World");
     kapp->dcopClient()->endTransaction(countDownAction2, replyType, replyData);
     countDownAction2 = 0;
  }
  else
  {
     QTimer::singleShot(1000, this, SLOT(slotTimeout2()));
  }
}

QCStringList MyDCOPObject::functions()
{
   QCStringList result = DCOPObject::functions();
   result << "QRect canLaunchRockets(QRect)";
   return result;
}

TestObject::TestObject(const Q3CString& app)
 :  m_app(app)
{
   QTimer::singleShot(2500, this, SLOT(slotTimeout()));
}

void TestObject::slotTimeout()
{
   Q3CString replyType;
   QByteArray data, reply;
   qWarning("#3 Calling countDown");

   if (!kapp->dcopClient()->call(m_app, "object1", "countDown()", data, replyType, reply))
      qDebug("#3 I couldn't call countDown");
   else
      qDebug("#3 countDown() return type was '%s'", replyType.data() ); 
   
}

void TestObject::slotCallBack(int callId, const Q3CString &replyType, const QByteArray &replyData)
{
   qWarning("Call Back! callId = %d", callId);
   qWarning("Type = %s", replyType.data());
   
   QDataStream args(replyData, QIODevice::ReadOnly);
   QString arg1;
   args >> arg1;
   
   qWarning("Value = %s", arg1.latin1());
}

int main(int argc, char **argv)
{
  KApplication app(argc, argv, "testdcop");

  Q3CString replyType;
  QByteArray data, reply;
  DCOPClient *client; client = app.dcopClient();

  if (argc == 2)
  {
      Q3CString app = argv[1];
      TestObject obj(app);
      qWarning("#1 Calling countDown");
      int result = kapp->dcopClient()->callAsync(app, "object1", "countDown()", data, &obj, SLOT(slotCallBack(int, const Q3CString&, const QByteArray&)));
      qDebug("#1 countDown() call id = %d", result);
      qWarning("#2 Calling countDown");
      result = kapp->dcopClient()->callAsync(app, "object1", "countDown()", data, &obj, SLOT(slotCallBack(int, const Q3CString&, const QByteArray&)));
      qDebug("#2 countDown() call id = %d", result);
      kapp->exec();
    
      return 0;
  }

//  client->attach(); // attach to the server, now we can use DCOP service

  client->registerAs( app.name(), false ); // register at the server, now others can call us.
  qDebug("I registered as '%s'", client->appId().data() );

  if ( client->isApplicationRegistered( app.name() ) )
      qDebug("indeed, we are registered!");

  QDataStream dataStream( data, QIODevice::WriteOnly );
  dataStream << (int) 43;
  client->emitDCOPSignal("alive(int,QCString)", data);

  MyDCOPObject *obj1 = new MyDCOPObject("object1");

  bool connectResult = client->connectDCOPSignal("", "alive(int , QCString)", "object1", "isAliveSlot(int)", false);
  qDebug("connectDCOPSignal returns %s", connectResult ? "true" : "false");

  QDataStream ds(data, QIODevice::WriteOnly);
  ds << QString("fourty-two") << 42;
  if (!client->call(app.name(), "object1", "aFunction(QString,int)", data, replyType, reply))
    qDebug("I couldn't call myself");
  else
      qDebug("return type was '%s'", replyType.data() ); 

  client->send(app.name(), "object1", "aFunction(QString,int)", data );

  int n = client->registeredApplications().count();
  qDebug("number of attached applications = %d", n );

  QObject::connect( client, SIGNAL( applicationRegistered( const Q3CString&)),
                    obj1, SLOT( registered( const Q3CString& )));

  QObject::connect( client, SIGNAL( applicationRemoved( const Q3CString&)),
                    obj1, SLOT( unregistered( const Q3CString& )));

  // Enable the above signals
  client->setNotifications( true );

  Q3CString foundApp;
  Q3CString foundObj;

  // Find a object called "object1" in any application that
  // meets the criteria "canLaunchRockets()"
//  bool boolResult = client->findObject( "", "object1", "canLaunchRockets()", data, foundApp, foundObj);
//  qDebug("findObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
//	foundApp.data(), foundObj.data());

  // Find an application that matches with "konqueror*"
  bool boolResult = client->findObject( "konqueror*", "", "", data, foundApp, foundObj);
  qDebug("findObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
	foundApp.data(), foundObj.data());

  // Find an object called "object1" in any application.
  boolResult = client->findObject( "", "ksycoca", "", data, foundApp, foundObj);
  qDebug("findObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
	foundApp.data(), foundObj.data());

  // Find ourselves in any application.
  boolResult = client->findObject( "testdcop", "ksycoca", "", data, foundApp, foundObj);
  qDebug("findObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
	foundApp.data(), foundObj.data());

  DCOPClient *client2 = new DCOPClient();
  client2->registerAs(app.name(), false);
  qDebug("I2 registered as '%s'", client2->appId().data() );

qDebug("Sending to object1");
  client2->send(app.name(), "object1", "aFunction(QString,int)", data );

qDebug("Calling object1");
  if (!client2->call(app.name(), "object1", "aFunction(QString,int)", data, replyType, reply))
    qDebug("I couldn't call myself");
  else
      qDebug("return type was '%s'", replyType.data() ); 

qDebug("Calling countDown() in object1");
  if (!client2->call(app.name(), "object1", "countDown()", data, replyType, reply))
    qDebug("I couldn't call myself");
  else
      qDebug("return type was '%s'", replyType.data() ); 

  // Find ourselves in any application.
  boolResult = client2->findObject( "testdcop", "object1", "", data, foundApp, foundObj);
  qDebug("findObject: result = %s, %s, %s\n", boolResult ? "true" : "false",
	foundApp.data(), foundObj.data());


  return app.exec();

  client->detach();
}

#include "testdcop.moc"
