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

#include <kurl.h>

#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qguardedptr.h>
#include <qdom.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kdebug.h>
#include <kio/jobclasses.h>
#include <kio/global.h>
#include <kio/http.h>
#include <kio/davjob.h>
#include <kio/job.h>
#include <kio/slaveinterface.h>

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( packedArgs, IO_WriteOnly ); stream

using namespace KIO;

DavJob::DavJob( const KURL& url, int method, const QString& request, bool showProgressInfo )
  : TransferJob( url, KIO::CMD_SPECIAL, QByteArray(), QByteArray(), showProgressInfo )
{
  // We couldn't set the args when calling the parent constructor,
  // so do it now.
  QDataStream stream( m_packedArgs, IO_WriteOnly );
  stream << (int) 7 << url << method;
  // Same for static data
  staticData = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" + request.utf8();
  staticData.truncate( staticData.size() - 1 );
}

void DavJob::slotData( const QByteArray& data ) 
{
  m_str_response.append( QString( data ) );
}

void DavJob::slotFinished()
{
  kdDebug() << "DavJob::slotFinished()" << endl;
  kdDebug() << m_str_response << endl;
	 
  m_response.setContent( m_str_response, true );
  TransferJob::slotFinished();
}

/* Convenience methods */

DavJob* KIO::davPropFind( const KURL& url, const QDomDocument& properties, QString depth, bool showProgressInfo )
{
  DavJob *job = new DavJob( url, (int) KIO::DAV_PROPFIND, properties.toString(), showProgressInfo );
  job->addMetaData( "davDepth", depth );
  return job;
}


DavJob* KIO::davPropPatch( const KURL& url, const QDomDocument& properties, bool showProgressInfo )
{
  return new DavJob( url, (int) KIO::DAV_PROPPATCH, properties.toString(), showProgressInfo );
}

DavJob* KIO::davSearch( const KURL& url, const QString& nsURI, const QString& qName, const QString& query, bool showProgressInfo )
{
  QDomDocument doc;
  QDomElement searchrequest = doc.createElementNS( "DAV:", "searchrequest" );
  QDomElement searchelement = doc.createElementNS( nsURI, qName );
  QDomText text = doc.createTextNode( query );
  searchelement.appendChild( text );
  searchrequest.appendChild( searchelement );
  doc.appendChild( searchrequest );
  return new DavJob( url, KIO::DAV_SEARCH, doc.toString(), showProgressInfo );
}

