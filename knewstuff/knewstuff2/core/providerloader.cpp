/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2006 Josef Spillner <spillner@kde.org>

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

#include <kconfig.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klocale.h>

using namespace KNS;

ProviderLoader::ProviderLoader( QWidget *parentWidget ) :
  mParentWidget( parentWidget )
{
  mProviders.setAutoDelete( true );
}

void ProviderLoader::load( const QString &type, const QString &providersList )
{
  kdDebug(5850) << "ProviderLoader::load()" << endl;

  mProviders.clear();
  mJobData = "";

  KConfig *cfg = KGlobal::config();
  cfg->setGroup("KNewStuff");

  QString providersUrl = providersList;
  if( providersUrl.isEmpty() )
  	providersUrl = cfg->readEntry( "ProvidersUrl" );

  if ( providersUrl.isEmpty() ) {
    // TODO: Replace the default by the real one.
    QString server = cfg->readEntry( "MasterServer",
                                     "http://korganizer.kde.org" );
  
    providersUrl = server + "/knewstuff/" + type + "/providers.xml";
  }

  kdDebug(5850) << "ProviderLoader::load(): providersUrl: " << providersUrl << endl;
  
  KIO::TransferJob *job = KIO::get( KURL( providersUrl ), false, false );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotJobResult( KIO::Job * ) ) );
  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );

//  job->dumpObjectInfo();
}

void ProviderLoader::slotJobData( KIO::Job *, const QByteArray &data )
{
  kdDebug(5850) << "ProviderLoader::slotJobData()" << endl;

  if ( data.size() == 0 ) return;

  QCString str( data, data.size() + 1 );

  mJobData.append( QString::fromUtf8( str ) );
}

void ProviderLoader::slotJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( mParentWidget );
  }

  kdDebug(5850) << "--PROVIDERS-START--" << endl << mJobData << "--PROV_END--"
            << endl;

  QDomDocument doc;
  if ( !doc.setContent( mJobData ) ) {
    KMessageBox::error( mParentWidget, i18n("Error parsing providers list.") );
    return;
  }

  QDomElement providers = doc.documentElement();

  if ( providers.isNull() ) {
    kdDebug(5850) << "No document in Providers.xml." << endl;
  }

  QDomNode n;
  for ( n = providers.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement p = n.toElement();
 
    if ( p.tagName() == "provider" ) {
      ProviderHandler handler(p);
      mProviders.append(handler.providerptr());
    }
  }
  
  emit providersLoaded( &mProviders );
}

#include "providerloader.moc"
