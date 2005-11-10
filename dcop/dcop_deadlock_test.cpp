/*****************************************************************

Copyright (c) 2004 Waldo Bastian <bastian@kde.org>
 
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

#include <dcop_deadlock_test.h>
#include <dcopref.h>
#include <qtimer.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

MyDCOPObject::MyDCOPObject(const QCString &name, const QCString &remoteName) 
: QObject(0, name), DCOPObject(name), m_remoteName(remoteName)
{
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

bool MyDCOPObject::process(const QCString &fun, const QByteArray &data,
QCString& replyType, QByteArray &replyData)
{
  if (fun == "function(QCString)") {
    QDataStream args( data, QIODevice::ReadOnly );
    args >>  m_remoteName;

    struct timeval tv;
    gettimeofday(&tv, 0);
qWarning("%s: function('%s') %d:%06d", name(), m_remoteName.data(), tv.tv_sec % 100, tv.tv_usec);

    replyType = "QString";
    QDataStream reply( replyData, QIODevice::WriteOnly );
    reply << QString("Hey");
    m_timer.start(1000, true);
    return true;
  }
  return DCOPObject::process(fun, data, replyType, replyData);
}

void MyDCOPObject::slotTimeout()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
qWarning("%s: slotTimeout() %d:%06d", name(), tv.tv_sec % 100, tv.tv_usec);

  m_timer.start(1000, true);
  QString result;
  DCOPRef(m_remoteName, m_remoteName).call("function", QCString(name())).get(result);
    gettimeofday(&tv, 0);
qWarning("%s: Got result '%s' %d:%06d", name(), result.latin1(), tv.tv_sec % 100, tv.tv_usec);
}

int main(int argc, char **argv)
{
  QCString myName = KApplication::dcopClient()->registerAs("testdcop", false);
  KApplication app(argc, argv, "testdcop");

  qWarning("%d:I am '%s'", getpid(), app.dcopClient()->appId().data());
  
  if (myName == "testdcop")
  {
      system("./dcop_deadlock_test testdcop&");
  }

  QCString remoteApp;
  if (argc == 2)
  {
      remoteApp = argv[1];
  }
  MyDCOPObject myObject(app.dcopClient()->appId(), remoteApp);

  if (!remoteApp.isEmpty())
     myObject.slotTimeout();
  app.exec();
}

#include "dcop_deadlock_test.moc"
