/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Thiago Macieira <thiagom@mail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef KEXTSOCKLOOKUP_H
#define KEXTSOCKLOOKUP_H

/**
 * @internal
 * What is this class for?
 *
 * Instead of writing the whole lookup code in
 * KExtendedSocket, I preferred writing a separate
 * class to do the dirty work.
 *
 * @author Thiago Macieira <thiagom@mail.com>
 * @short internal lookup class used by @ref KExtendedSocket
 */
class KExtendedSocketLookup: public QObject
{
  Q_OBJECT
public:
  QDns dnsIpv4, dnsIpv6;
  int workingCount;		// number of QDns at work
  const QString& servname;
  addrinfo hint;

  KExtendedSocketLookup(const QString& hostname, const QString& servname, const addrinfo& hint) :
    dnsIpv4(hostname, QDns::A), dnsIpv6(hostname, QDns::Aaaa), workingCount(2),
    servname(servname), hint(hint)
  {
    connect(&dnsIpv4, SIGNAL(resultsReady()), this, SLOT(slotResultsReady()));
    connect(&dnsIpv6, SIGNAL(resultsReady()), this, SLOT(slotResultsReady()));
  }

  inline bool isWorking() const
  { return workingCount; }

  kde_addrinfo *results();
  static void freeresults(kde_addrinfo* res);

public slots:
  void slotResultsReady()
  {
    if (--workingCount == 0)
      emit resultsReady();
  }

signals:
  void resultsReady();

public:
  friend class KExtendedSocket;
};

#endif
