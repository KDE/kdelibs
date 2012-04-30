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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIO_DAVJOB_H
#define KIO_DAVJOB_H

#include <kurl.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QPointer>
#include <QtXml/QDomDocument>

#include <sys/types.h>
#include <sys/stat.h>

#include "jobclasses.h"
#include "global.h"


namespace KIO {

    class Slave;

    class DavJobPrivate;
    /**
     * The transfer job pumps data into and/or out of a Slave.
     * Data is sent to the slave on request of the slave ( dataReq).
     * If data coming from the slave can not be handled, the
     * reading of data from the slave should be suspended.
     * @see KIO::davPropFind()
     * @see KIO::davPropPatch()
     * @see KIO::davSearch()
     */
    class KIO_EXPORT DavJob : public TransferJob
    {
        Q_OBJECT
    public:
        /**
	 * Returns the response as a QDomDocument.
	 * @return the response document
	 */
	QDomDocument& response();

    protected Q_SLOTS:
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);

    protected:
        DavJob(DavJobPrivate &dd, int, const QString &);
    private:
        Q_DECLARE_PRIVATE(DavJob)
    };

   /**
    * Creates a new DavJob that issues a PROPFIND command. PROPFIND retrieves
    * the properties of the resource identified by the given @p url.
    *
    * @param url the URL of the resource
    * @param properties a propfind document that describes the properties that
    *        should be retrieved
    * @param depth the depth of the request. Can be "0", "1" or "infinity"
    * @param flags: We support HideProgressInfo here
    * @return the new DavJob
    */
   KIO_EXPORT DavJob* davPropFind( const QUrl& url, const QDomDocument& properties, const QString &depth, JobFlags flags = DefaultFlags );

   /**
    * Creates a new DavJob that issues a PROPPATCH command. PROPPATCH sets
    * the properties of the resource identified by the given @p url.
    *
    * @param url the URL of the resource
    * @param properties a PROPPACTCH document that describes the properties that
    *        should be modified and its new values
    * @param flags: We support HideProgressInfo here
    * @return the new DavJob
    */
   KIO_EXPORT DavJob* davPropPatch( const QUrl& url, const QDomDocument& properties, JobFlags flags = DefaultFlags );

   /**
    * Creates a new DavJob that issues a SEARCH command.
    *
    * @param url the URL of the resource
    * @param nsURI the URI of the search method's qualified name
    * @param qName the local part of the search method's qualified name
    * @param query the search string
    * @param flags: We support HideProgressInfo here
    * @return the new DavJob
    */
   KIO_EXPORT DavJob* davSearch( const QUrl &url, const QString& nsURI, const QString& qName, const QString& query, JobFlags flags = DefaultFlags );

   /**
    * Creates a new DavJob that issues a REPORT command.
    *
    * @param url the URL of the resource
    * @param report a REPORT document that describes the request to make
    * @param depth the depth of the request. Can be "0", "1" or "infinity"
    * @param flags: We support HideProgressInfo here
    * @return the new DavJob
    * @since 4.4
    */
   KIO_EXPORT DavJob* davReport( const QUrl& url, const QString& report, const QString &depth, JobFlags flags = DefaultFlags );

}

#endif

