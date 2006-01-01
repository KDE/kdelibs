/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <qdom.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "knewstuff.h"
#include "downloaddialog.h"
#include "uploaddialog.h"
#include "providerdialog.h"

#include "engine.h"
#include "engine.moc"

using namespace KNS;

struct Engine::Private
{
    bool mIgnoreInstallResult;
    KNewStuff *mNewStuff;
};

Engine::Engine( KNewStuff *newStuff, const QString &type,
                QWidget *parentWidget ) :
  mParentWidget( parentWidget ), mDownloadDialog( 0 ),
  mUploadDialog( 0 ), mProviderDialog( 0 ), mUploadProvider( 0 ),
  d(new Private), mType( type )
{
  d->mNewStuff = newStuff;
  mProviderLoader = new ProviderLoader( mParentWidget );

}

Engine::Engine( KNewStuff *newStuff, const QString &type,
                const QString &providerList, QWidget *parentWidget ) :
                mParentWidget( parentWidget ),
		mDownloadDialog( 0 ), mUploadDialog( 0 ),
		mProviderDialog( 0 ), mUploadProvider( 0 ),
                mProviderList( providerList ), d(new Private),
		mType( type )
{
  d->mNewStuff = newStuff;
  d->mIgnoreInstallResult = false;
  mProviderLoader = new ProviderLoader( mParentWidget );
}

Engine::~Engine()
{
  qDeleteAll(mNewStuffList);
  mNewStuffList.clear();
  delete d;
  delete mProviderLoader;

  delete mUploadDialog;
  delete mDownloadDialog;
}

void Engine::download()
{
  kdDebug(5850) << "Engine::download()" << endl;

  connect( mProviderLoader,
           SIGNAL( providersLoaded( Provider::List * ) ),
           SLOT( getMetaInformation( Provider::List * ) ) );
  mProviderLoader->load( mType, mProviderList );
}

void Engine::getMetaInformation( Provider::List *providers )
{
  mProviderLoader->disconnect();

  mNewStuffJobData.clear();

  if ( !mDownloadDialog ) {
    mDownloadDialog = new DownloadDialog( this, mParentWidget );
    mDownloadDialog->show();
  }
  mDownloadDialog->clear();

  Provider *p;
  for (int i = 0; i < providers->size(); ++i) {
	p = providers->at(i);
    if ( p->downloadUrl().isEmpty() ) continue;

    KIO::TransferJob *job = KIO::get( p->downloadUrl() );
    connect( job, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotNewStuffJobResult( KIO::Job * ) ) );
    connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             SLOT( slotNewStuffJobData( KIO::Job *, const QByteArray & ) ) );

    mNewStuffJobData.insert( job, "" );
    mProviderJobs[ job ] = p;
  }
}

void Engine::slotNewStuffJobData( KIO::Job *job, const QByteArray &data )
{
  if ( data.isEmpty() ) return;

  kdDebug(5850) << "Engine:slotNewStuffJobData()" << endl;

  mNewStuffJobData[ job ].append( QString::fromUtf8( data ) ); // ####### The fromUtf8 conversion should be done at the end, not chunk by chunk
}

void Engine::slotNewStuffJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug(5850) << "Error downloading new stuff descriptions." << endl;
    job->showErrorDialog( mParentWidget );
  } else {
    QString knewstuffDoc = mNewStuffJobData[ job ];

    kdDebug(5850) << "---START---" << endl << knewstuffDoc << "---END---" << endl;

    mDownloadDialog->addProvider( mProviderJobs[ job ] );

    QDomDocument doc;
    if ( !doc.setContent( knewstuffDoc ) ) {
      kdDebug(5850) << "Error parsing knewstuff.xml." << endl;
      return;
    } else {
      QDomElement knewstuff = doc.documentElement();

      if ( knewstuff.isNull() ) {
        kdDebug(5850) << "No document in knewstuffproviders.xml." << endl;
      } else {
        QDomNode p;
        for ( p = knewstuff.firstChild(); !p.isNull(); p = p.nextSibling() ) {
          QDomElement stuff = p.toElement();
          if ( stuff.tagName() != "stuff" ) continue;
          if ( stuff.attribute("type", mType) != mType ) continue;

          Entry *entry = new Entry( stuff );
          mNewStuffList.append( entry );

          mDownloadDialog->show();

          mDownloadDialog->addEntry( entry );

          kdDebug(5850) << "KNEWSTUFF: " << entry->name() << endl;

          kdDebug(5850) << "  SUMMARY: " << entry->summary() << endl;
          kdDebug(5850) << "  VERSION: " << entry->version() << endl;
          kdDebug(5850) << "  RELEASEDATE: " << entry->releaseDate().toString() << endl;
          kdDebug(5850) << "  RATING: " << entry->rating() << endl;

          kdDebug(5850) << "  LANGS: " << entry->langs().join(", ") << endl;
        }
      }
    }
  }

  mNewStuffJobData.remove( job );
  mProviderJobs.remove( job );

  if ( mNewStuffJobData.count() == 0 ) {
    mDownloadDialog->show();
    mDownloadDialog->raise();
  }
}

void Engine::download( Entry *entry )
{
  kdDebug(5850) << "Engine::download(entry)" << endl;

  KURL source = entry->payload();
  mDownloadDestination = d->mNewStuff->downloadDestination( entry );

  if ( mDownloadDestination.isEmpty() ) {
    kdDebug(5850) << "Empty downloadDestination. Cancelling download." << endl;
    return;
  }

  KURL destination = KURL( mDownloadDestination );

  kdDebug(5850) << "  SOURCE: " << source.url() << endl;
  kdDebug(5850) << "  DESTINATION: " << destination.url() << endl;

  KIO::FileCopyJob *job = KIO::file_copy( source, destination, -1, true );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotDownloadJobResult( KIO::Job * ) ) );
}

void Engine::slotDownloadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug(5850) << "Error downloading new stuff payload." << endl;
    job->showErrorDialog( mParentWidget );
    return;
  }

  if ( d->mNewStuff->install( mDownloadDestination ) ) {
    if ( !d->mIgnoreInstallResult ) {
      KMessageBox::information( mParentWidget,
                                i18n("Successfully installed hot new stuff.") );
    }
  } else 
    if ( !d->mIgnoreInstallResult ){
      KMessageBox::error( mParentWidget,
                          i18n("Failed to install hot new stuff.") );
  }
}

void Engine::upload(const QString &fileName, const QString &previewName )
{
  mUploadFile = fileName;
  mPreviewFile = previewName;

  connect( mProviderLoader,
           SIGNAL( providersLoaded( Provider::List * ) ),
           SLOT( selectUploadProvider( Provider::List * ) ) );
  mProviderLoader->load( mType );
}

void Engine::selectUploadProvider( Provider::List *providers )
{
  kdDebug(5850) << "Engine:selectUploadProvider()" << endl;

  mProviderLoader->disconnect();

  if ( !mProviderDialog ) {
    mProviderDialog = new ProviderDialog( this, mParentWidget );
  }

  mProviderDialog->clear();

  mProviderDialog->show();
  mProviderDialog->raise();

  for (int i = 0; i < providers->size(); ++i) {
		  Provider *p = providers->at(i);
		mProviderDialog->addProvider( p );
  }
}

void Engine::requestMetaInformation( Provider *provider )
{
  mUploadProvider = provider;

  if ( !mUploadDialog ) {
    mUploadDialog = new UploadDialog( this, mParentWidget );
  }
  mUploadDialog->setPreviewFile( mPreviewFile );
  mUploadDialog->setPayloadFile( mUploadFile );
  mUploadDialog->show();
  mUploadDialog->raise();
}

void Engine::upload( Entry *entry )
{
  if ( mUploadFile.isNull()) {
    mUploadFile = entry->fullName();
    mUploadFile = locateLocal( "data", QString(kapp->instanceName()) + "/upload/" + mUploadFile );

    if ( !d->mNewStuff->createUploadFile( mUploadFile ) ) {
      KMessageBox::error( mParentWidget, i18n("Unable to create file to upload.") );
      emit uploadFinished( false );
      return;
    }
  }

  QString lang = entry->langs().first();
  QFileInfo fi( mUploadFile );
  entry->setPayload( KURL::fromPathOrURL( fi.fileName() ), lang );

  if ( !createMetaFile( entry ) ) {
    emit uploadFinished( false );
    return;
  } 

  QString text = i18n("The files to be uploaded have been created at:\n");
  text.append( i18n("Data file: %1\n").arg( mUploadFile) );
  if (!mPreviewFile.isEmpty()) {
    text.append( i18n("Preview image: %1\n").arg( mPreviewFile) );
  }
  text.append( i18n("Content information: %1\n").arg( mUploadMetaFile) );
  text.append( i18n("Those files can now be uploaded.\n") );
  text.append( i18n("Beware that any people might have access to them at any time.") );

  QString caption = i18n("Upload Files");

  if ( mUploadProvider->noUpload() ) {
    KURL noUploadUrl = mUploadProvider->noUploadUrl();
    if ( noUploadUrl.isEmpty() ) {
      text.append( i18n("Please upload the files manually.") );
      KMessageBox::information( mParentWidget, text, caption );
    } else {
      int result = KMessageBox::questionYesNo( mParentWidget, text, caption,
                                               i18n("Upload Info"),
                                               KStdGuiItem::close() );
      if ( result == KMessageBox::Yes ) {
        KToolInvocation::invokeBrowser( noUploadUrl.url() );
      }
    }
  } else {
    int result = KMessageBox::questionYesNo( mParentWidget, text, caption,
                                             i18n("&Upload"), KStdGuiItem::cancel() );
    if ( result == KMessageBox::Yes ) {
      KURL destination = mUploadProvider->uploadUrl();
      destination.setFileName( fi.fileName() );

      KIO::FileCopyJob *job = KIO::file_copy( KURL::fromPathOrURL( mUploadFile ), destination );
      connect( job, SIGNAL( result( KIO::Job * ) ),
               SLOT( slotUploadPayloadJobResult( KIO::Job * ) ) );
    } else {
      emit uploadFinished( false );
    }
  }
}

bool Engine::createMetaFile( Entry *entry )
{
  QDomDocument doc("knewstuff");
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement de = doc.createElement("knewstuff");
  doc.appendChild( de );

  entry->setType(type());
  de.appendChild( entry->createDomElement( doc, de ) );

  kdDebug(5850) << "--DOM START--" << endl << doc.toString()
            << "--DOM_END--" << endl;

  if ( mUploadMetaFile.isNull() ) {
    mUploadMetaFile = entry->fullName() + ".meta";
    mUploadMetaFile = locateLocal( "data", QString(kapp->instanceName()) + "/upload/" + mUploadMetaFile );
  }

  QFile f( mUploadMetaFile );
  if ( !f.open( QIODevice::WriteOnly ) ) {
    mUploadMetaFile.clear();
    return false;
  }

  QTextStream ts( &f );
  ts.setEncoding( QTextStream::UnicodeUTF8 );
  ts << doc.toString();

  f.close();

  return true;
}

void Engine::slotUploadPayloadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug(5850) << "Error uploading new stuff payload." << endl;
    job->showErrorDialog( mParentWidget );
    emit uploadFinished( false );
    return;
  }

  if (mPreviewFile.isEmpty()) {
    slotUploadPreviewJobResult(job);
    return;
  }

  QFileInfo fi( mPreviewFile );

  KURL previewDestination = mUploadProvider->uploadUrl();
  previewDestination.setFileName( fi.fileName() );

  KIO::FileCopyJob *newJob = KIO::file_copy( KURL::fromPathOrURL( mPreviewFile ), previewDestination );
  connect( newJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotUploadPreviewJobResult( KIO::Job * ) ) );
}

void Engine::slotUploadPreviewJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug(5850) << "Error uploading new stuff preview." << endl;
    job->showErrorDialog( mParentWidget );
    emit uploadFinished( true );
    return;
  }

  QFileInfo fi( mUploadMetaFile );

  KURL metaDestination = mUploadProvider->uploadUrl();
  metaDestination.setFileName( fi.fileName() );

  KIO::FileCopyJob *newJob = KIO::file_copy( KURL::fromPathOrURL( mUploadMetaFile ), metaDestination );
  connect( newJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotUploadMetaJobResult( KIO::Job * ) ) );
}

void Engine::slotUploadMetaJobResult( KIO::Job *job )
{
  mUploadMetaFile.clear();
  if ( job->error() ) {
    kdDebug(5850) << "Error uploading new stuff metadata." << endl;
    job->showErrorDialog( mParentWidget );
    emit uploadFinished( false );
    return;
  }

  KMessageBox::information( mParentWidget,
                            i18n("Successfully uploaded new stuff.") );
  emit uploadFinished( true );
}

void Engine::ignoreInstallResult(bool ignore)
{
  d->mIgnoreInstallResult = ignore;
}
