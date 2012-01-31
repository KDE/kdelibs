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

#include "davjob.h"

#include <kurl.h>

#include <QtCore/QObject>
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QPointer>
#include <QtXml/QDomDocument>

#include <sys/types.h>
#include <sys/stat.h>

#include <kdebug.h>
#include <kio/http.h>

#include "jobclasses.h"
#include "global.h"
#include "job.h"
#include "job_p.h"

#include "jobuidelegate.h"

using namespace KIO;

/** @internal */
class KIO::DavJobPrivate: public KIO::TransferJobPrivate
{
public:
    DavJobPrivate(const KUrl& url)
        : TransferJobPrivate(url, KIO::CMD_SPECIAL, QByteArray(), QByteArray())
        {}
    QByteArray savedStaticData;
    QByteArray str_response;
    QDomDocument m_response;
    //TransferJob *m_subJob;
    //bool m_suspended;

    Q_DECLARE_PUBLIC(DavJob)

    static inline DavJob *newJob(const KUrl &url, int method, const QString &request,
                                 JobFlags flags)
    {
        DavJob *job = new DavJob(*new DavJobPrivate(url), method, request);
        job->setUiDelegate(new JobUiDelegate);
        if (!(flags & HideProgressInfo))
            KIO::getJobTracker()->registerJob(job);
        return job;
    }
};

DavJob::DavJob(DavJobPrivate &dd, int method, const QString &request)
    : TransferJob(dd)
{
  // We couldn't set the args when calling the parent constructor,
  // so do it now.
  Q_D(DavJob);
  QDataStream stream( &d->m_packedArgs, QIODevice::WriteOnly );
  stream << (int) 7 << d->m_url << method;
  // Same for static data
  if ( ! request.isEmpty() ) {
    d->staticData = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" + request.toUtf8();
    d->staticData.truncate( d->staticData.size() - 1 );
    d->savedStaticData = d->staticData;
    stream << static_cast<qint64>( d->staticData.size() );
  }
  else {
    stream << static_cast<qint64>( -1 );
  }
}

QDomDocument& DavJob::response()
{
    return d_func()->m_response;
}

void DavJob::slotData( const QByteArray& data )
{
  Q_D(DavJob);
  if(d->m_redirectionURL.isEmpty() || !d->m_redirectionURL.isValid() || error()) {
    unsigned int oldSize = d->str_response.size();
    d->str_response.resize( oldSize + data.size() );
    memcpy( d->str_response.data() + oldSize, data.data(), data.size() );
  }
}

void DavJob::slotFinished()
{
  Q_D(DavJob);
  // kDebug(7113) << d->str_response;
	if (!d->m_redirectionURL.isEmpty() && d->m_redirectionURL.isValid() &&
            (d->m_command == CMD_SPECIAL)) {
		QDataStream istream( d->m_packedArgs );
		int s_cmd, s_method;
    qint64 s_size;
		KUrl s_url;
		istream >> s_cmd;
		istream >> s_url;
		istream >> s_method;
    istream >> s_size;
		// PROPFIND
		if ( (s_cmd == 7) && (s_method == (int)KIO::DAV_PROPFIND) ) {
			d->m_packedArgs.truncate(0);
			QDataStream stream( &d->m_packedArgs, QIODevice::WriteOnly );
			stream << (int)7 << d->m_redirectionURL << (int)KIO::DAV_PROPFIND << s_size;
		}
  } else if ( ! d->m_response.setContent( d->str_response, true ) ) {
		// An error occurred parsing the XML response
		QDomElement root = d->m_response.createElementNS( "DAV:", "error-report" );
		d->m_response.appendChild( root );

		QDomElement el = d->m_response.createElementNS( "DAV:", "offending-response" );
    QDomText textnode = d->m_response.createTextNode( d->str_response );
		el.appendChild( textnode );
		root.appendChild( el );
	}
  // kDebug(7113) << d->m_response.toString();
	TransferJob::slotFinished();
	d->staticData = d->savedStaticData; // Need to send DAV request to this host too
}

/* Convenience methods */

DavJob* KIO::davPropFind( const KUrl& url, const QDomDocument& properties, const QString &depth, JobFlags flags )
{
    DavJob *job = DavJobPrivate::newJob(url, (int) KIO::DAV_PROPFIND, properties.toString(), flags);
    job->addMetaData( "davDepth", depth );
    return job;
}


DavJob* KIO::davPropPatch( const KUrl& url, const QDomDocument& properties, JobFlags flags )
{
    return DavJobPrivate::newJob(url, (int) KIO::DAV_PROPPATCH, properties.toString(),
                                 flags);
}

DavJob* KIO::davSearch( const KUrl& url, const QString& nsURI, const QString& qName, const QString& query, JobFlags flags )
{
  QDomDocument doc;
  QDomElement searchrequest = doc.createElementNS( "DAV:", "searchrequest" );
  QDomElement searchelement = doc.createElementNS( nsURI, qName );
  QDomText text = doc.createTextNode( query );
  searchelement.appendChild( text );
  searchrequest.appendChild( searchelement );
  doc.appendChild( searchrequest );
  return DavJobPrivate::newJob(url, KIO::DAV_SEARCH, doc.toString(), flags);
}

DavJob* KIO::davReport( const KUrl& url, const QString& report, const QString &depth, JobFlags flags )
{
    DavJob *job = DavJobPrivate::newJob(url, (int) KIO::DAV_REPORT, report, flags);
    job->addMetaData( "davDepth", depth );
    return job;
}

