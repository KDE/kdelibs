/*  -*- C++ -*-
 *  Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>
 *  Copyright (C) 2005 Matt Broadstone <mbroadst@gmail.com>
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included 
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef KNETWORKINTERFACE_H
#define KNETWORKINTERFACE_H

#include <QtCore/QSharedDataPointer>
#include <kdelibs_export.h>

namespace KNetwork
{

class KSocketAddress;
class KNetworkInterfacePrivate;

class KDECORE_EXPORT KNetworkInterface
{
/*
public:
    enum Flags {
        Up = 1,             //< Interface is up.
        Broadcast = 2,      //< Broadcast address (@ref broadcastAddress()) is valid..
        Loopback = 8,       //< Interface is a loopback interface.
        PointToPoint = 16,  //< Interface is a point-to-point interface.
        Running = 128,      //< Interface is running.
        Multicast = 65536   //< Interface is multicast-capable.
    };
*/
public:
    KNetworkInterface();
    KNetworkInterface(const QString &);
    KNetworkInterface(const KNetworkInterface &);
    virtual ~KNetworkInterface();

    KNetworkInterface& operator =(const KNetworkInterface&);

    bool isValid() const;
    void updateStats();

    // getifaddrs info
    QString name() const;
    int index() const;
    int flags() const;

    QList<KSocketAddress> address() const;
    QList<KSocketAddress> netmask() const;
    QList<KSocketAddress> broadcastAddress() const;
    QList<KSocketAddress> destinationAddress() const;

    // Stats
    int sendBytes() const;
    int receiveBytes() const;

    int sendPackets() const;
    int receivePackets() const;

    int sendErrors() const;
    int receiveErrors() const;

    int sendDropped() const;
    int receiveDropped() const;

    int sendFifoErrors() const;
    int receiveFifoErrors() const;

    int sendCompressed() const;
    int receiveCompressed() const;

    int receiveFrameErrors() const;
    int receiveMulticast() const;

    int sendColls() const;
    int sendCarrierErrors() const;

private:
    QSharedDataPointer<KNetworkInterfacePrivate> d;

};

}

#endif
