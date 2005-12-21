/*  -*- C++ -*-
 *  Copyright (C) 2005 Thiago Macieira <thiago@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KSRVRESOLVERWORKER_P_H
#define KSRVRESOLVERWORKER_P_H

#include <qobject.h>
#include <qdns.h>
#include <qsemaphore.h>
#include <qvaluelist.h>
#include <qdict.h>
#include "kresolver.h"
#include "kresolverworkerbase.h"

class QCustomEvent;

namespace KNetwork
{
  namespace Internal
  {
    /**
     * @internal
     * This class implements SRV-based resolution
     */
    class KSrvResolverWorker: public QObject,
			      public KNetwork::KResolverWorkerBase
    {
      Q_OBJECT

    public:
      struct Entry
      {
	QString name;
	Q_UINT16 port;
	Q_UINT16 weight;
	KNetwork::KResolver* resolver;
      };

      struct PriorityClass
      {
	PriorityClass() : totalWeight(0) { }

	QValueList<Entry> entries;
	Q_UINT16 totalWeight;
      };

    private:
      QDns *dns;
      QValueList<QDns::Server> rawResults;
      QCString encodedName;
      QSemaphore *sem;

      typedef QMap<Q_UINT16, PriorityClass> Results;
      Results myResults;

    public:
      virtual bool preprocess();
      virtual bool run();
      virtual bool postprocess();

      virtual void customEvent(QCustomEvent*);

    public slots:
      void dnsResultsReady();
    };
  }
}

#endif
