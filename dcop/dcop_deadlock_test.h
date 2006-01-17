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

#ifndef _TESTDCOP_H_
#define _TESTDCOP_H_

#include <kapplication.h>
#include <dcopclient.h>
#include <dcopobject.h>

#include <qobject.h>
#include <qtimer.h>

#include <stdio.h>

class MyDCOPObject : public QObject, public DCOPObject
{
  Q_OBJECT
public:
  MyDCOPObject(const QCString &name, const QCString &remoteName);
  bool process(const QCString &fun, const QByteArray &data,
	       QCString& replyType, QByteArray &replyData);
public Q_SLOTS:
  void slotTimeout();

private:
  QCString m_remoteName;
  QTimer m_timer;
};
#endif
