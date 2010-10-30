/*
 * Copyright 2010, Valentin Rusu <kde@rusu.info>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "peer.h"

#include <QtCore/QFile>
#include <QtCore/QSharedData>

#include <kdebug.h>

class PeerData : public QSharedData
{
public:
    PeerData() : m_valid(false), m_pid(0) {}
    PeerData(const PeerData &other) : QSharedData(other), m_pid(other.m_pid) {}
    ~PeerData() {}

    // designates whether this Peer is valid as it's not certain that
    // a pid of 0 can always denote an invalid peer.
    bool m_valid;

    // peer process pid
    uint m_pid;
};

Peer::Peer() : d(new PeerData)
{
}

Peer::Peer(const Peer &other) : d(other.d)
{
}

Peer &Peer::operator=(const Peer &other)
{
    d = other.d;
    return *this;
}

Peer::Peer(uint pid) : d(new PeerData)
{
    d->m_valid = true;
    d->m_pid = pid;

    kDebug() << "Peer " << pid;
    kDebug() << "   cmdLine = " << cmdLine();
    kDebug() << "   exePath = " << exePath();
}

Peer::~Peer()
{
}

QString Peer::procFileName() const
{
    Q_ASSERT(d->m_valid);
    return QString("/proc/%1").arg(d->m_pid);
}

QByteArray Peer::cmdLine() const
{
    if(!d->m_valid) {
        return QByteArray();
    } else {
        QFile procFile(QString("%1/cmd").arg(procFileName()));
        if(!procFile.open(QIODevice::ReadOnly | QIODevice::Text) ||
                procFile.atEnd()) {
            // file doesn't exist or is empty
            return QByteArray();
        } else {
            return procFile.readLine();
        }
    }
}

bool Peer::isValid() const
{
    return d->m_valid;
}

bool Peer::isStillRunning() const
{
    if(!d->m_valid) {
        return false;
    } else {
        return QFile::exists(procFileName());
    }
}

QString Peer::exePath() const
{
    if(!d->m_valid) {
        return QString();
    } else {
        // TODO: add a watch an trigger signal when the peer process ends
        return QFile::symLinkTarget(QString("%1/exe").arg(procFileName()));
    }
}
