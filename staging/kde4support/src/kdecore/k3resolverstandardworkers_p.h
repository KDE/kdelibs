/*  -*- C++ -*-
 *  Copyright (C) 2003,2005 Thiago Macieira <thiago@kde.org>
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

#ifndef KRESOLVERSTANDARDWORKERS_P_H
#define KRESOLVERSTANDARDWORKERS_P_H

#include <sys/types.h>
#include <netdb.h>

#include <QList>
#include <QByteArray>
#include <QStringList>

#include "k3resolver.h"
#include "k3resolverworkerbase.h"

#include <config.h>
#include <config-network.h>

namespace KNetwork { namespace Internal
{
  extern void initStandardWorkers() KDE_NO_EXPORT;

  /**
   * @internal
   * The blacklist worker.
   */
  class KBlacklistWorker: public KNetwork::KResolverWorkerBase
  {
  public:
    static QStringList blacklist;

    static void loadBlacklist();
    static void init();
    static bool isBlacklisted(const QString&);

    virtual bool preprocess();
    virtual bool run();
    virtual bool postprocess() { return true; }
  };

  /** @internal
   * Standard worker.
   */
  class KStandardWorker: public KNetwork::KResolverWorkerBase
  {
  protected:
    mutable QByteArray m_encodedName;
    quint16 port;
    int scopeid;
    QList<KNetwork::KResolverResults*> resultList;

  public:
    virtual ~KStandardWorker();
    bool sanityCheck();

    virtual bool preprocess();
    virtual bool run();
    virtual bool postprocess();

    bool resolveScopeId();
    bool resolveService();
    bool resolveNumerically();

    KNetwork::KResolver::ErrorCodes addUnix();
  };

#if defined(HAVE_GETADDRINFO)
  /** @internal
   * Worker class based on getaddrinfo(3).
   *
   * This class does not do post-processing.
   */
  class KGetAddrinfoWorker: public KStandardWorker
  {
  public:
    KGetAddrinfoWorker()
    { }

    virtual ~KGetAddrinfoWorker();
    virtual bool preprocess();
    virtual bool run();
    virtual bool postprocess() { return true; }

    bool wantThis(int family);
  };
#endif // HAVE_GETADDRINFO

} } // namespace KNetwork::Internal


#endif
