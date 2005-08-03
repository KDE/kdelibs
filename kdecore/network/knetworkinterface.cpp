#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>

#include <QString>
#include <QStringList>
#include <QList>
#include <QMultiHash>
#include <QFile>

#include "kdebug.h"
#include <ksocketaddress.h>
#include "knetworkinterface.h"

using namespace KNetwork;

class KNetwork::KNetworkInterfacePrivate : public QSharedData
{
    public:
        KNetworkInterfacePrivate();
        KNetworkInterfacePrivate(const QString& _name,
                                 int _index,
                                 int _flags,
                                 KSocketAddress _address,
                                 KSocketAddress _netmask,
                                 KSocketAddress _broadcast,
                                 KSocketAddress _destination);

        virtual ~KNetworkInterfacePrivate() {}
        KNetworkInterfacePrivate& operator= (const KNetworkInterfacePrivate& interface);

// These all need to be moved to a QCache
        void init();
        void insert(const QString& _name,
                    int _index,
                    int _flags,
                    KSocketAddress _address,
                    KSocketAddress _netmask,
                    KSocketAddress _broadcast,
                    KSocketAddress _destination);

        KNetworkInterfacePrivate *find(const QString& ifName);
        KNetworkInterfacePrivate *find(int ifIndex);

        void readStats();

        QString name;
        int index;
        int flags;

        struct InterfaceStats
        {
            InterfaceStats() :
                rxBytes(0),
                rxPackets(0),
                rxErrors(0),
                rxDropped(0),
                rxFifoErrors(0),
                rxFrameErrors(0),
                rxCompressed(0),
                rxMulticast(0),
                txBytes(0),
                txPackets(0),
                txErrors(0),
                txDropped(0),
                txFifoErrors(0),
                txColls(0),
                txCarrierErrors(0),
                txCompressed(0)
            {
            }

            // Rx
            int rxBytes,
                rxPackets,
                rxErrors,
                rxDropped,
                rxFifoErrors,
                rxFrameErrors,
                rxCompressed,
                rxMulticast;

            // Tx
            int txBytes,
                txPackets,
                txErrors,
                txDropped,
                txFifoErrors,
                txColls,
                txCarrierErrors,
                txCompressed;
        } stats;

        QList<KSocketAddress> address;
        QList<KSocketAddress> netmask;
        QList<KSocketAddress> broadcast;
        QList<KSocketAddress> destination;

        QHash<QString, KNetworkInterfacePrivate*> nameMap;
        QHash<int, KNetworkInterfacePrivate*> indexMap;
};

KNetworkInterfacePrivate::KNetworkInterfacePrivate()
    : index(0),
      flags(0)
{
}

KNetworkInterfacePrivate::KNetworkInterfacePrivate(const QString& _name,
                                                   int _index,
                                                   int _flags,
                                                   KSocketAddress _address,
                                                   KSocketAddress _netmask,
                                                   KSocketAddress _broadcast,
                                                   KSocketAddress _destination)
    : name(_name),
      index(_index),
      flags(_flags)
{
      address.append(_address);
      netmask.append(_netmask);
      broadcast.append(_broadcast);
      destination.append(_destination);
}

void KNetworkInterfacePrivate::insert(const QString& _name,
                                      int _index,
                                      int _flags,
                                      KSocketAddress _address,
                                      KSocketAddress _netmask,
                                      KSocketAddress _broadcast,
                                      KSocketAddress _destination)
{
    KNetworkInterfacePrivate *nameMapPrivate = find(_name);
    if (!nameMapPrivate)
    {
        KNetworkInterfacePrivate *interface =
            new KNetworkInterfacePrivate(_name, _index, _flags,
                                         _address, _netmask, _broadcast,
                                         _destination);
        nameMap.insert(interface->name, interface);
        indexMap.insert(interface->index, interface);
    }
    else
    {
        // no need to go through indexMap because they both hold
        // pointers to the same thing..
        nameMapPrivate->address.append(_address);
        nameMapPrivate->netmask.append(_netmask);
        nameMapPrivate->broadcast.append(_broadcast);
        nameMapPrivate->destination.append(_broadcast);
    }
}


KNetworkInterfacePrivate& KNetworkInterfacePrivate::operator= (const KNetworkInterfacePrivate& interface)
{
    name = interface.name;
    index = interface.index;
    flags = interface.flags;
    stats = interface.stats;
    address = interface.address;
    netmask = interface.netmask;
    broadcast = interface.broadcast;
    destination = interface.destination;

    return *this;
}

void KNetworkInterfacePrivate::init()
{
    nameMap.clear();
    indexMap.clear();

    struct ifaddrs *ads;
    struct ifaddrs *a;

    if (getifaddrs(&ads))
    {
        return;
    }

    a = ads;
    while (a)
    {
        if (a->ifa_addr->sa_family == AF_INET)
        {
            insert(QString::fromUtf8(a->ifa_name),
                   if_nametoindex(a->ifa_name),
                   a->ifa_flags,
                   KSocketAddress(a->ifa_addr, sizeof(struct sockaddr_in)),
                   KSocketAddress(a->ifa_netmask, sizeof(struct sockaddr_in)),
                   KSocketAddress(a->ifa_broadaddr, sizeof(struct sockaddr_in)),
                   KSocketAddress(a->ifa_dstaddr, sizeof(struct sockaddr_in)));
        }
        if (a->ifa_addr->sa_family == AF_INET6)
        {
            insert(QString::fromUtf8(a->ifa_name),
                   if_nametoindex(a->ifa_name),
                   a->ifa_flags,
                   KSocketAddress(a->ifa_addr, sizeof(struct sockaddr_in6)),
                   KSocketAddress(a->ifa_netmask, sizeof(struct sockaddr_in6)),
                   KSocketAddress(a->ifa_broadaddr, sizeof(struct sockaddr_in6)),
                   KSocketAddress(a->ifa_dstaddr, sizeof(struct sockaddr_in6)));
        }
        a = a->ifa_next;
    }

    freeifaddrs(ads);

}

KNetworkInterfacePrivate *KNetworkInterfacePrivate::find(const QString& ifName)
{
    QHash<QString, KNetworkInterfacePrivate*>::iterator it = nameMap.find(ifName);
    if (it != nameMap.end())
    {
        return it.value();
    }

    return 0;
}

KNetworkInterfacePrivate *KNetworkInterfacePrivate::find(int ifIndex)
{
    QHash<int, KNetworkInterfacePrivate*>::iterator it = indexMap.find(ifIndex);
    if (it != indexMap.end())
    {
        return it.value();
    }

    return 0;
}

void KNetworkInterfacePrivate::readStats()
{
    // Read basic info
    QFile proc( "/proc/net/dev" );
    if (!proc.open(QIODevice::ReadOnly))
    {
        qWarning("Can't open /proc/net/dev. No interfaces...");
        return;
    }

    // Chuck out the first two lines
    proc.readLine();
    proc.readLine();

    QString line = proc.readLine();
    while (!line.isEmpty())
    {
        QString netif = line.section(":", 0, 0);
        if (netif.simplified() == name)
        {
            QString info = line.mid(netif.size() + 1, line.size());
            QStringList stat = info.split(" ", QString::SkipEmptyParts);

            // There's probably a better way to do this..
            stats.rxBytes = stat[0].toInt();
            stats.rxPackets = stat[1].toInt();
            stats.rxErrors = stat[2].toInt();
            stats.rxDropped = stat[3].toInt();
            stats.rxFifoErrors = stat[4].toInt();
            stats.rxFrameErrors = stat[5].toInt();
            stats.rxCompressed = stat[6].toInt();
            stats.rxMulticast = stat[7].toInt();

            stats.txBytes = stat[8].toInt();
            stats.txPackets = stat[9].toInt();
            stats.txErrors = stat[10].toInt();
            stats.txDropped = stat[11].toInt();
            stats.txFifoErrors = stat[12].toInt();
            stats.txColls = stat[13].toInt();
            stats.txCarrierErrors = stat[14].toInt();
            stats.txCompressed = stat[15].toInt();
        }
        line = proc.readLine();
    }
}

///////////////////////////////////
//  Constructors
//
//
KNetworkInterface::KNetworkInterface()
    : d(new KNetworkInterfacePrivate)
{
}

KNetworkInterface::KNetworkInterface(const QString& ifname)
    : d(new KNetworkInterfacePrivate)
{
    d->init();
    d = d->find(ifname);
    d->readStats();
}

KNetworkInterface::KNetworkInterface(const KNetworkInterface &interface)
    : d(0)
{
    *this = interface;
}

KNetworkInterface::~KNetworkInterface()
{
}

KNetworkInterface& KNetworkInterface::operator= (const KNetworkInterface& interface)
{
  d = interface.d;
  return *this;
}

///////////////////////////////////
//  Accessors
//
//
bool KNetworkInterface::isValid() const
{
    return (d == 0);
}

QString KNetworkInterface::name() const
{
    return d->name;
}

int KNetworkInterface::index() const
{
    return d->index;
}

int KNetworkInterface::flags() const
{
    return d->flags;
}

QList<KSocketAddress> KNetworkInterface::address() const
{
    return d->address;
}

QList<KSocketAddress> KNetworkInterface::netmask() const
{
    return d->netmask;
}

QList<KSocketAddress> KNetworkInterface::broadcastAddress() const
{
    return d->broadcast;
}

QList<KSocketAddress> KNetworkInterface::destinationAddress() const
{
    return d->destination;
}

int KNetworkInterface::sendBytes() const
{
    return d->stats.txBytes;
}

int KNetworkInterface::receiveBytes() const
{
    return d->stats.rxBytes;
}

int KNetworkInterface::sendPackets() const
{
    return d->stats.txPackets;
}

int KNetworkInterface::receivePackets() const
{
    return d->stats.rxPackets;
}

int KNetworkInterface::sendErrors() const
{
    return d->stats.txErrors;
}

int KNetworkInterface::receiveErrors() const
{
    return d->stats.rxErrors;
}

int KNetworkInterface::sendDropped() const
{
    return d->stats.txDropped;
}

int KNetworkInterface::receiveDropped() const
{
    return d->stats.rxDropped;
}

int KNetworkInterface::sendFifoErrors() const
{
    return d->stats.txFifoErrors;
}

int KNetworkInterface::receiveFifoErrors() const
{
    return d->stats.rxFifoErrors;
}

int KNetworkInterface::sendCompressed() const
{
    return d->stats.txCompressed;
}

int KNetworkInterface::receiveCompressed() const
{
    return d->stats.rxCompressed;
}

int KNetworkInterface::receiveFrameErrors() const
{
    return d->stats.rxFrameErrors;
}

int KNetworkInterface::receiveMulticast() const
{
    return d->stats.rxMulticast;
}

int KNetworkInterface::sendColls() const
{
    return d->stats.txColls;
}

int KNetworkInterface::sendCarrierErrors() const
{
    return d->stats.txCarrierErrors;
}
