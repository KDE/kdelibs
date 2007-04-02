/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

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

#include "providerloader.h"

#include "providerhandler.h"

#include <qbytearray.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>

using namespace KNS;

ProviderLoader::ProviderLoader()
{
}

void ProviderLoader::load(const QString &providersurl)
{
  kDebug(550) << "ProviderLoader::load()" << endl;

  m_providers.clear();
  m_jobdata.clear();

  kDebug(550) << "ProviderLoader::load(): providersUrl: " << providersurl << endl;
  
  KIO::TransferJob *job = KIO::get( KUrl( providersurl ), false, false );
  connect( job, SIGNAL( result( KJob * ) ),
           SLOT( slotJobResult( KJob * ) ) );
  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );
}

void ProviderLoader::slotJobData( KIO::Job *, const QByteArray &data )
{
  kDebug(550) << "ProviderLoader::slotJobData()" << endl;

  m_jobdata.append(data);
}

void ProviderLoader::slotJobResult( KJob *job )
{
  if ( job->error() ) {
    emit signalProvidersFailed();
    return;
  }

  kDebug(550) << "--PROVIDERS-START--" << endl;
  kDebug(550) << QString::fromUtf8(m_jobdata) << endl;
  kDebug(550) << "--PROVIDERS-END--" << endl;

  QDomDocument doc;
  if ( !doc.setContent( m_jobdata ) ) {
    emit signalProvidersFailed();
    return;
  }

  QDomElement providers = doc.documentElement();

  if ( providers.isNull() ) {
    kDebug(550) << "No document in Providers.xml." << endl;
  }

  QDomNode n;
  for ( n = providers.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement p = n.toElement();
 
    if ( p.tagName() == "provider" ) {
      ProviderHandler handler(p);
      m_providers.append(handler.providerptr());
    }
  }
 
  emit signalProvidersLoaded( m_providers );
}

#include "providerloader.moc"
