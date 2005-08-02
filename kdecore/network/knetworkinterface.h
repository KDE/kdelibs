#ifndef KNETWORKINTERFACE_H
#define KNETWORKINTERFACE_H

#include <QSharedDataPointer>
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

    // getifaddrs info
    QString name() const;
    int index() const;
    int flags() const;

    KSocketAddress address() const;
    KSocketAddress netmask() const;
    KSocketAddress broadcastAddress() const;
    KSocketAddress destinationAddress() const;

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
