// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __kio_davjob_h__
#define __kio_davjob_h__

#include <kurl.h>

#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qguardedptr.h>
#include <qdom.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kio/jobclasses.h>
#include <kio/global.h>

class Observer;
class QTimer;

namespace KIO {

    class Slave;
    class SlaveInterface;

    /**
     * The transfer job pumps data into and/or out of a Slave.
     * Data is sent to the slave on request of the slave (@ref dataReq).
     * If data coming from the slave can not be handled, the
     * reading of data from the slave should be suspended.
     * @see KIO::davPropFind()
     * @see KIO::davPropPatch()
     * @see KIO::davSearch()
     * @since 3.1
     */
    class DavJob : public TransferJob {
    Q_OBJECT

    public:
        /**
	 * Use @ref KIO::davPropFind(), @ref KIO::davPropPatch() and
	 * @ref KIO::davSearch() to create a new DavJob.
	 */
        DavJob(const KURL& url, int method,
                    const QString& request, bool showProgressInfo);
        /**
	 * Returns the response as a @ref QDomDocument.
	 * @return the response document
	 */
	QDomDocument& response() { return m_response; }

    protected slots:
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);

    protected:
        bool m_suspended;
        TransferJob *m_subJob;
    private:
	class DavJobPrivate* d;
	QString m_str_response;
	QDomDocument m_response;
   };

   /**
    * Creates a new @ref DavJob that issues a PROPFIND command. PROPFIND retrieves
    * the properties of the resource identified by the given @p url.
    *
    * @param url the URL of the resource
    * @param properties a propfind document that describes the properties that
    *        should be retrieved
    * @param depth the depth of the request. Can be "0", "1" or "infinity"
    * @param showProgressInfo true to show progress information
    * @return the new DavJob
    */
   DavJob* davPropFind( const KURL& url, const QDomDocument& properties, QString depth, bool showProgressInfo=true );

   /**
    * Creates a new @ref DavJob that issues a PROPPATCH command. PROPPATCH sets
    * the properties of the resource identified by the given @p url.
    *
    * @param url the URL of the resource
    * @param properties a PROPPACTCH document that describes the properties that
    *        should be modified and its new values
    * @param showProgressInfo true to show progress information
    * @return the new DavJob
    */
   DavJob* davPropPatch( const KURL& url, const QDomDocument& properties, bool showProgressInfo=true );

   /**
    * Creates a new @ref DavJob that issues a SEARCH command.
    *
    * @param url the URL of the resource
    * @param nsURI the URI of the search method's qualified name
    * @param qName the local part of the search method's qualified name
    * @param query the search string
    * @param showProgressInfo true to show progress information
    * @return the new DavJob
    */
   DavJob* davSearch( const KURL &url, const QString& nsURI, const QString& qName, const QString& query, bool showProgressInfo=true ); 

};

#endif

