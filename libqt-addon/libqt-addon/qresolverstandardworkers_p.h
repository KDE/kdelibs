/*  -*- C++ -*-
 *  Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>
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

#ifndef QRESOLVERSTANDARDWORKERS_P_H
#define QRESOLVERSTANDARDWORKERS_P_H

#include <sys/types.h>
#include <netdb.h>

#include <qptrlist.h>
#include <qcstring.h>

#include "qresolver.h"
#include "qresolverworkerbase.h"
#include "qtaddon.h"

#include <config.h>

ADDON_START

namespace Internal
{
  extern void initStandardWorkers();

  /** @internal
   * Standard worker.
   */
  class QStandardWorker: public QResolverWorkerBase
  {
  protected:
    mutable QCString m_encodedName;
    Q_UINT16 port;
    int scopeid;
    QPtrList<QResolverResults> resultList;

  public:
    bool sanityCheck();

    virtual bool preprocess();
    virtual bool run();
    virtual bool postprocess();

    bool resolveScopeId();
    bool resolveService();
    bool resolveNumerically();

    QResolver::ErrorCodes addUnix();
  };

#if defined(HAVE_GETADDRINFO)
  /** @internal
   * Worker class based on getaddrinfo(3).
   *
   * This class does not do post-processing.
   */
  class QGetAddrinfoWorker: public QStandardWorker
  {
  public:
    QGetAddrinfoWorker()
    { }

    virtual ~QGetAddrinfoWorker();
    virtual bool preprocess();
    virtual bool run();
    virtual bool postprocess() { return true; }

    bool wantThis(int family);
  };
#endif // HAVE_GETADDRINFO

} // namespace Internal

ADDON_END

#endif
