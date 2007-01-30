/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

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

#include "entryloader.h"

#include "entryhandler.h"

#include <qbytearray.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>

using namespace KNS;

EntryLoader::EntryLoader()
{
}

void EntryLoader::load(const QString &stuffurl)
{
  kDebug(550) << "EntryLoader::load()" << endl;

  mEntries.clear();
  mJobData = "";

  kDebug(550) << "EntryLoader::load(): stuffUrl: " << stuffurl << endl;
  
  KIO::TransferJob *job = KIO::get( KUrl( stuffurl ), false, false );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotJobResult( KIO::Job * ) ) );
  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );

//  job->dumpObjectInfo();
}

void EntryLoader::slotJobData( KIO::Job *, const QByteArray &data )
{
  kDebug(550) << "EntryLoader::slotJobData()" << endl;

  if ( data.size() == 0 ) return;

  mJobData.append( QString::fromUtf8( data ) );
// FIXME: utf-8 conversion only at the end!!!
}

void EntryLoader::slotJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    emit signalEntriesFailed();
    return;
  }

  kDebug(550) << "--ENTRIES-START--" << endl << mJobData << "--ENTRIES_END--"
            << endl;

  QDomDocument doc;
  if ( !doc.setContent( mJobData ) ) {
    emit signalEntriesFailed();
    return;
  }

  QDomElement entries = doc.documentElement();

  if ( entries.isNull() ) {
    kDebug(550) << "No document in stuff.xml." << endl;
  }

  QDomNode n;
  for ( n = entries.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();
 
    if ( e.tagName() == "stuff" ) {
      EntryHandler handler(e);
      mEntries.append(handler.entryptr());
    }
  }
  
  emit signalEntriesLoaded( &mEntries );
}

#include "entryloader.moc"
