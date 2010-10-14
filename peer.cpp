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

#include <QFile>

#include <kdebug.h>

Peer::Peer( uint pid ) :
   m_pid( pid )
{
   kDebug() << "Peer " << pid;
   kDebug() << "   cmdLine = " << cmdLine();
   kDebug() << "   exePath = " << exePath();
}

QString Peer::procFileName() const
{
   return QString("/proc/%1").arg( m_pid );
}

QString Peer::cmdLine() const
{
   QFile procFile( QString("%s/cmd").arg( procFileName() ) );
   return procFile.readLine();
}

bool Peer::isStillRunning() const
{
   QFile procFile( procFileName() );
   return procFile.exists();
}

QString Peer::exePath() const 
{
   QFile procFile( QString("%s/exe").arg( procFileName() )) ;
   // TODO: add a watch an trigger signal when the peer process ends
   return procFile.symLinkTarget();
}
