/*  -*- C++ -*-
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Thiago Macieira <thiagom@mail.com>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License.
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

#ifndef RESOLVERWORKERBASE_H
#define RESOLVERWORKERBASE_H

#include "resolver.h"		// knetwork/resolver.h

// forward declarations
class KDE::Internal::Resolver::WorkerThread;
class QString;
template <class T> class QValueList;

namespace KDE
{
  namespace Network
  {

    /** @internal
     * This class is the base functionality for a resolver worker. That is,
     * the class that does the actual work.
     *
     * In the future, this class might be exposed to allow plug-ins. So, try and 
     * make it binary compatible.
     *
     * Note that hostnames are still encoded in Unicode at this point. It's up to
     * the worker class to decide which encoding to use. In the case of DNS, 
     * an ASCII Compatible Encoding (ACE) must be used. 
     * See @ref Resolver::domainToAscii. 
     *
     * Also specially note that the run method in this class is called in a
     * thread that is not the program's main thread. So do not do anything there
     * that you shouldn't!
     *
     * @class ResolverWorkerBase resolverworkerbase.h knetwork/resolverworkerbase.h
     */
    class ResolverWorkerBase
    {
    private:
      // this will be like our d pointer
      KDE::Internal::Resolver::WorkerThread *th;
      /// @internal
      friend class KDE::Internal::Resolver::WorkerThread;

    protected:
      /**
       * Derived classes will put their resolved data in this list, or will
       * leave it empty in case of error.
       */
      KDE::Network::ResolverResults results;

    public:
      // default constructor
      ResolverWorkerBase();

      // virtual destructor
      virtual ~ResolverWorkerBase();

      /**
       * This is the encoded hostname to be looked for
       */
      QString nodeName() const;

      /**
       * And this is the encoded service name
       */
      QCString serviceName() const;

      /**
       * gets the family mask
       */
      int familyMask() const;

      /**
       * gets the socket type
       */
      int socketType() const;

      /**
       * gets the protocol number
       */
      int protocol() const;

      /**
       * gets the protocol name, if applicable
       */
      QCString protocolName() const;

    protected:
      // like a QThread
      /**
       * This is the function that should be overriden in derived classes.
       * 
       * Derived classes will do their blocking job in this function and return
       * either success or failure to work (not the lookup). That is, whether the
       * lookup result was a domain found or not, if we got our answer, we should
       * indicate success. The error itself must be set with @ref setError.
       *
       * \b Important: this function gets called in a separate thread!
       * 
       * @return true on success
       */
      virtual bool run() = 0;

      /**
       * Sets the error
       */
      inline void setError(int errorcode, int syserror = 0)
      { results.setError(errorcode, syserror); }

      /**
       * Request another resolution.
       *
       * Use this function to make the manager call for another resolution.
       * This is suitable for workers that do post-processing.
       *
       * For each request made there will be one ResolverResults object
       * in the list to be passed to the post-processing function.
       *
       * @param host		the nodename to resolve
       * @param service		the service to resolve
       * @param flags		flags to be used
       * @param families	the families to be searched
       */
      void requestResolution(const QString& host, const QString& service,
			     int flags = 0, int families = Resolver::AnyFamily);
    };

    /** @internal
     * This class is the base for a worker base that requires post-processing.
     * That is, a class that must do some post-processing after other jobs
     * have finished.
     *
     * The purpose of this class is to allow one worker class to enqueue other
     * requests and, when those are done and have been processed themselves, the
     * post-processing routine defined here will get called. 
     *
     * This is useful, for instance, in the case of SRV-based DNS lookups. In
     * those, the first lookup will return a list of domain names and port numbers
     * that have yet to be resolved. So, the worker class will queue other requests
     * for such hostnames and return to the caller.
     *
     * You should declare your post-processing worker this way:
     * \code
     *   class MyWorker: public ResolverWorkerBase, public PostProcessingWorker
     * \endocode
     *
     * @class PostProcessingWorker resolverworkerbase.h knetwork/resolverworkerbase.h
     */
    class PostProcessingWorker
    {
    protected:
      /**
       * This function gets called during post processing for this class and
       * you must override it.
       *
       * \b Important: this function gets called in a separate thread. And it MUST
       * NOT block.
       *
       * For each request queued with @ref requestResolution there is one entry
       * in the @p resultList list. The order in which they are presented is not defined.
       *
       * @param resultList	the list of requests
       * @return true on success
       */
      virtual bool postprocess(QValueList<ResolverResults>& resultList) = 0;
    };

  } // namespace KDE::Network
} // namespace KDE

#endif
