/*
Copyright 2008 Roland Harnau <tau@gmx.eu>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hostinfo_p.h"

#include <kglobal.h>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QCache>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QThread>
#include <QtCore/QFutureWatcher>
#include <QtCore/QMetaType>
#include <QtCore/QtConcurrentRun>
#include <QtNetwork/QHostInfo>
#include "kdebug.h"

#ifdef Q_OS_UNIX
# include <QtCore/QFileInfo>
# include <netinet/in.h>
# include <arpa/nameser.h>
# include <resolv.h>            // for _PATH_RESCONF
# ifndef _PATH_RESCONF
#  define _PATH_RESCONF         "/etc/resolv.conf"
# endif
#endif

#define TTL 300

Q_DECLARE_METATYPE(QHostInfo)

namespace KIO
{
    class HostInfoAgentPrivate : public QObject
    {
        Q_OBJECT
    public:
        HostInfoAgentPrivate(int cacheSize = 100);
        virtual ~HostInfoAgentPrivate() {};
        void lookupHost(const QString& hostName, QObject* receiver, const char* member);
        QHostInfo lookupCachedHostInfoFor(const QString& hostName);
        void cacheLookup(const QHostInfo&);
        void setCacheSize(int s) { dnsCache.setMaxCost(s); }
        void setTTL(int _ttl) { ttl = _ttl; }
    private slots:
        void queryFinished(const QHostInfo&);
    private:
        class Result;
        class Query;

        QHash<QString, Query*> openQueries;
        QCache<QString, QPair<QHostInfo, QTime> > dnsCache;
        time_t resolvConfMTime;
        int ttl;
    };

    class HostInfoAgentPrivate::Result : public QObject
    {
        Q_OBJECT
    signals:
        void result(QHostInfo);
    private:
        friend class HostInfoAgentPrivate;
    };

    class HostInfoAgentPrivate::Query : public QObject
    {
        Q_OBJECT
    public:
        Query(): m_watcher(), m_hostName()
        {
            connect(&m_watcher, SIGNAL(finished()), this, SLOT(relayFinished()));
        }
        void start(const QString& hostName)
        {
            m_hostName = hostName;
            QFuture<QHostInfo> future = QtConcurrent::run(&QHostInfo::fromName, hostName);
            m_watcher.setFuture(future);
        }
        QString hostName() const
        {
            return m_hostName;
        }
    signals:
        void result(QHostInfo);
    private slots:
        void relayFinished()
        {
            emit result(m_watcher.result());
        }
    private:
        QFutureWatcher<QHostInfo> m_watcher;
        QString m_hostName;
    };

    class NameLookUpThread : public QThread
    {
    public:
        NameLookUpThread (const QString& name)
            :QThread (0), m_hostName(name), m_started(false)
        {
        }

        QHostInfo result() const
        {
          return m_hostInfo;
        }

        bool wasStarted() const
        {
            return m_started;
        }

        void run()
        {
            m_started = true;
            m_hostInfo = QHostInfo();

            // Do not perform a reverse lookup here...
            QHostAddress address (m_hostName);
            if (!address.isNull()) {
                QList<QHostAddress> addressList;
                addressList << address;
                m_hostInfo.setAddresses(addressList);
                return;
            }

            // Look up the name in the KIO/KHTML DNS cache...
            m_hostInfo = HostInfo::lookupCachedHostInfoFor(m_hostName);
            if (!m_hostInfo.hostName().isEmpty() && m_hostInfo.error() == QHostInfo::NoError) {
                return;
            }

            // Failing all of the above, do the lookup...
            m_hostInfo = QHostInfo::fromName(m_hostName);
            if (!m_hostInfo.hostName().isEmpty() && m_hostInfo.error() == QHostInfo::NoError) {
                HostInfo::cacheLookup(m_hostInfo); // cache the look up...
            }
        }

    private:
        QHostInfo m_hostInfo;
        QString m_hostName;
        bool m_started;
    };
}

using namespace KIO;

K_GLOBAL_STATIC(HostInfoAgentPrivate, hostInfoAgentPrivate)

void HostInfo::lookupHost(const QString& hostName, QObject* receiver,
    const char* member)
{
    hostInfoAgentPrivate->lookupHost(hostName, receiver, member);
}

QHostInfo HostInfo::lookupHost(const QString& hostName, unsigned long timeout)
{
    NameLookUpThread lookupThread (hostName);
    lookupThread.start();

    // Wait for it to start...
    while (!lookupThread.wasStarted()) {
       kDebug() << "Waiting for name lookup thread to start";
       lookupThread.wait(1000);
    }

    // Now wait for it to complete...
    if (!lookupThread.wait(timeout)) {
        kDebug() << "Name look up for" << hostName << "failed";
        lookupThread.terminate();
        return QHostInfo();
    }

    //kDebug(7022) << "Name look up succeeded for" << hostName;
    return lookupThread.result();
}

QHostInfo HostInfo::lookupCachedHostInfoFor(const QString& hostName)
{
    return hostInfoAgentPrivate->lookupCachedHostInfoFor(hostName);
}

void HostInfo::cacheLookup(const QHostInfo& info)
{
    hostInfoAgentPrivate->cacheLookup(info);
}

void HostInfo::prefetchHost(const QString& hostName)
{
    hostInfoAgentPrivate->lookupHost(hostName, 0, 0);
}

void HostInfo::setCacheSize(int s)
{
    hostInfoAgentPrivate->setCacheSize(s);
}

void HostInfo::setTTL(int ttl)
{
    hostInfoAgentPrivate->setTTL(ttl);
}

HostInfoAgentPrivate::HostInfoAgentPrivate(int cacheSize)
    : openQueries(),
      dnsCache(cacheSize),
      resolvConfMTime(0),
      ttl(TTL)
{
      qRegisterMetaType<QHostInfo>();
}

void HostInfoAgentPrivate::lookupHost(const QString& hostName,
    QObject* receiver, const char* member)
{
#ifdef _PATH_RESCONF
    QFileInfo resolvConf(QFile::decodeName(_PATH_RESCONF));
    time_t currentMTime = resolvConf.lastModified().toTime_t();
    if (resolvConf.exists() && currentMTime != resolvConfMTime) {
        // /etc/resolv.conf has been modified
        // clear our cache
        resolvConfMTime = currentMTime;
        dnsCache.clear();
    }
#endif

    if (QPair<QHostInfo, QTime>* info = dnsCache.object(hostName)) {
        if (QTime::currentTime() <= info->second.addSecs(ttl)) {
            Result result;
            if (receiver) {
                QObject::connect(&result, SIGNAL(result(QHostInfo)),receiver, member);
                emit result.result(info->first);
            }
            return;
        }
        dnsCache.remove(hostName);
    }

    if (Query* query = openQueries.value(hostName)) {
        if (receiver) {
            connect(query, SIGNAL(result(QHostInfo)), receiver, member);
        }
        return;
    }

    Query* query = new Query();
    openQueries.insert(hostName, query);
    connect(query, SIGNAL(result(QHostInfo)), this, SLOT(queryFinished(QHostInfo)));
    if (receiver) {
        connect(query, SIGNAL(result(QHostInfo)), receiver, member);
    }
    query->start(hostName);
}

QHostInfo HostInfoAgentPrivate::lookupCachedHostInfoFor(const QString& hostName)
{
    QPair<QHostInfo, QTime>* info = dnsCache.object(hostName);
    if (info && info->second.addSecs(ttl) >= QTime::currentTime())
        return info->first;

    return QHostInfo();
}

void HostInfoAgentPrivate::cacheLookup(const QHostInfo& info)
{
    if (info.hostName().isEmpty())
        return;

    if (info.error() != QHostInfo::NoError)
        return;

    dnsCache.insert(info.hostName(), new QPair<QHostInfo, QTime>(info, QTime::currentTime()));
}

void HostInfoAgentPrivate::queryFinished(const QHostInfo& info)
{
    Query* query = static_cast<Query* >(sender());
    openQueries.remove(query->hostName());
    if (info.error() == QHostInfo::NoError) {
        dnsCache.insert(query->hostName(),
            new QPair<QHostInfo, QTime>(info, QTime::currentTime()));
    }
    query->deleteLater();
}

#include "hostinfo.moc"
