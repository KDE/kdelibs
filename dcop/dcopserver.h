/*
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
*/
#ifndef DCOPSERVER_H
#define DCOPSERVER_H "$Id$"

#include <qobject.h>
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

/**
 * @internal
 */
class DCOPServer : public QObject
{
    Q_OBJECT
public:
    DCOPServer();
    ~DCOPServer();

    void* watchConnection( IceConn iceConn );
    void removeConnection( void* data );
    void processMessage( IceConn iceConn, int opcode, unsigned long length, Bool swap);
    void ioError( IceConn iceConn );

    virtual bool receive(const QCString &app, const QCString &obj,
			 const QCString &fun, const QByteArray& data,
			 QCString& replyType, QByteArray &replyData, IceConn iceConn);

private slots:
    void newClient( int socket );
    void processData( int socket );

private:
    int majorOpcode;
    CARD32 time;
    QList<DCOPListener> listener;
    QDict<DCOPConnection> appIds;
    QPtrDict<DCOPConnection> clients;

    class DCOPServerPrivate;
    DCOPServerPrivate *d;
};

#endif
