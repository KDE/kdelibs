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

#ifndef _TESTDCOP_H_
#define _TESTDCOP_H_

#include <kapp.h>
#include <qbitarray.h>
#include <dcopclient.h>
#include <dcopobject.h>

#include <qobject.h>

#include <stdio.h>
/**
 $QTDIR/bin/moc testdcop.cpp -o testdcop.moc
 g++ -o testdcop testdcop.cpp -I$KDEDIR/include -I$QTDIR/include -L$KDEDIR/lib -L$QTDIR/lib -lkdecore -ldl
 @internal

**/


class MyDCOPObject : public QObject, public DCOPObject
{
  Q_OBJECT
public:
  MyDCOPObject(const QCString &name) : DCOPObject(name) {}
  bool process(const QCString &fun, const QByteArray &data,
	       QCString& replyType, QByteArray &replyData);
  void function(const QString &arg1, int arg2) { qDebug("function got arg: %s and %d", arg1.utf8().data(), arg2); }
public slots:
  void registered(const QCString &appName)
     { printf("REGISTER: %s\n", appName.data()); }

  void unregistered(const QCString &appName)
     { printf("UNREGISTER: %s\n", appName.data()); }
  QCStringList functions();
};
#endif
