/*
    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999 David Faure (faure@kde.org)

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

#include "netaccess.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <cstring>

#include <QtCore/QCharRef>
#include <QtGui/QApplication>
#include <QtCore/QFile>
#include <QtCore/QMetaClassInfo>
#include <QtCore/QTextIStream>

#include <kapplication.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <kurl.h>
#include <kstandarddirs.h>

#include "job.h"
#include "copyjob.h"
#include "deletejob.h"
#include "jobuidelegate.h"
#include "scheduler.h"

namespace KIO
{
    class NetAccessPrivate
    {
    public:
        NetAccessPrivate()
            : m_metaData(0)
            , bJobOK(true)
        {}
        UDSEntry m_entry;
        QString m_mimetype;
        QByteArray m_data;
        KUrl m_url;
        QMap<QString, QString> *m_metaData;

        /**
         * Whether the download succeeded or not
         */
        bool bJobOK;
    };

} // namespace KIO

using namespace KIO;


/**
 * List of temporary files
 */
static QStringList* tmpfiles;

static QString* lastErrorMsg = 0;
static int lastErrorCode = 0;

NetAccess::NetAccess() :
    d( new NetAccessPrivate )
{
}

NetAccess::~NetAccess()
{
    delete d;
}

bool NetAccess::download(const KUrl& u, QString & target, QWidget* window)
{
  if (u.isLocalFile()) {
    // file protocol. We do not need the network
    target = u.toLocalFile();
    bool accessible = KStandardDirs::checkAccess(target, R_OK);
    if(!accessible)
    {
        if(!lastErrorMsg)
            lastErrorMsg = new QString;
        *lastErrorMsg = i18n("File '%1' is not readable", target);
        lastErrorCode = ERR_COULD_NOT_READ;
    }
    return accessible;
  }

  if (target.isEmpty())
  {
      KTemporaryFile tmpFile;
      tmpFile.setAutoRemove(false);
      tmpFile.open();
      target = tmpFile.fileName();
      if (!tmpfiles)
          tmpfiles = new QStringList;
      tmpfiles->append(target);
  }

  NetAccess kioNet;
  KUrl dest;
  dest.setPath( target );
  return kioNet.filecopyInternal( u, dest, -1, KIO::Overwrite, window, false /*copy*/);
}

bool NetAccess::upload(const QString& src, const KUrl& target, QWidget* window)
{
  if (target.isEmpty())
    return false;

  // If target is local... well, just copy. This can be useful
  // when the client code uses a temp file no matter what.
  // Let's make sure it's not the exact same file though
  if (target.isLocalFile() && target.toLocalFile() == src)
    return true;

  NetAccess kioNet;
  KUrl s;
  s.setPath(src);
  return kioNet.filecopyInternal( s, target, -1, KIO::Overwrite, window, false /*copy*/ );
}

bool NetAccess::file_copy( const KUrl & src, const KUrl & target, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.filecopyInternal( src, target, -1, KIO::DefaultFlags,
                                  window, false /*copy*/ );
}

bool NetAccess::copy( const KUrl& src, const KUrl& target, QWidget* window )
{
    return file_copy( src, target, window );
}

// bool NetAccess::file_copy( const KUrl& src, const KUrl& target, int permissions,
//                            bool overwrite, bool resume, QWidget* window )
// {
//   NetAccess kioNet;
//   return kioNet.filecopyInternal( src, target, permissions, overwrite, resume,
//                                   window, false /*copy*/ );
// }


// bool NetAccess::file_move( const KUrl& src, const KUrl& target, int permissions,
//                            bool overwrite, bool resume, QWidget* window )
// {
//   NetAccess kioNet;
//   return kioNet.filecopyInternal( src, target, permissions, overwrite, resume,
//                                   window, true /*move*/ );
// }

bool NetAccess::dircopy( const KUrl & src, const KUrl & target, QWidget* window )
{
  KUrl::List srcList;
  srcList.append( src );
  return NetAccess::dircopy( srcList, target, window );
}

bool NetAccess::dircopy( const KUrl::List & srcList, const KUrl & target, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( srcList, target, window, false /*copy*/ );
}

bool NetAccess::move( const KUrl& src, const KUrl& target, QWidget* window )
{
  KUrl::List srcList;
  srcList.append( src );
  NetAccess kioNet;
  return kioNet.dircopyInternal( srcList, target, window, true /*move*/ );
}

bool NetAccess::move( const KUrl::List& srcList, const KUrl& target, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( srcList, target, window, true /*move*/ );
}

bool NetAccess::exists( const KUrl & url, bool source, QWidget* window )
{
  if ( url.isLocalFile() )
    return QFile::exists( url.toLocalFile() );
  NetAccess kioNet;
  return kioNet.statInternal( url, 0 /*no details*/,
                              source ? SourceSide : DestinationSide, window );
}

bool NetAccess::exists( const KUrl & url, StatSide side, QWidget* window )
{
  if ( url.isLocalFile() )
    return QFile::exists( url.toLocalFile() );
  NetAccess kioNet;
  return kioNet.statInternal( url, 0 /*no details*/, side, window );
}

bool NetAccess::stat( const KUrl & url, KIO::UDSEntry & entry, QWidget* window )
{
  NetAccess kioNet;
  bool ret = kioNet.statInternal( url, 2 /*all details*/, SourceSide, window );
  if (ret)
    entry = kioNet.d->m_entry;
  return ret;
}

KUrl NetAccess::mostLocalUrl(const KUrl & url, QWidget* window)
{
  if ( url.isLocalFile() )
  {
    return url;
  }

  KIO::UDSEntry entry;
  if (!stat(url, entry, window))
  {
    return url;
  }

  const QString path = entry.stringValue( KIO::UDSEntry::UDS_LOCAL_PATH );
  if ( !path.isEmpty() )
  {
    KUrl new_url;
    new_url.setPath(path);
    return new_url;
  }

  return url;
}

bool NetAccess::del( const KUrl & url, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.delInternal( url, window );
}

bool NetAccess::mkdir( const KUrl & url, QWidget* window, int permissions )
{
  NetAccess kioNet;
  return kioNet.mkdirInternal( url, permissions, window );
}

QString NetAccess::fish_execute( const KUrl & url, const QString &command, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.fish_executeInternal( url, command, window );
}

bool NetAccess::synchronousRun( Job* job, QWidget* window, QByteArray* data,
                                KUrl* finalURL, QMap<QString, QString>* metaData )
{
  NetAccess kioNet;
  // Disable autodeletion until we are back from this event loop (#170963)
  // We just have to hope people don't mess with setAutoDelete in slots connected to the job, though.
  const bool wasAutoDelete = job->isAutoDelete();
  job->setAutoDelete(false);
  const bool ok = kioNet.synchronousRunInternal(job, window, data, finalURL, metaData);
  if (wasAutoDelete) {
    job->deleteLater();
  }
  return ok;
}

QString NetAccess::mimetype( const KUrl& url, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.mimetypeInternal( url, window );
}

QString NetAccess::lastErrorString()
{
    return lastErrorMsg ? *lastErrorMsg : QString();
}

int NetAccess::lastError()
{
    return lastErrorCode;
}

void NetAccess::removeTempFile(const QString& name)
{
  if (!tmpfiles)
    return;
  if (tmpfiles->contains(name))
  {
    unlink(QFile::encodeName(name));
    tmpfiles->removeAll(name);
  }
}

bool NetAccess::filecopyInternal(const KUrl& src, const KUrl& target, int permissions,
                                 KIO::JobFlags flags, QWidget* window, bool move)
{
  d->bJobOK = true; // success unless further error occurs

  KIO::Scheduler::checkSlaveOnHold(true);
  KIO::Job * job = move
                   ? KIO::file_move( src, target, permissions, flags )
                   : KIO::file_copy( src, target, permissions, flags );
  job->ui()->setWindow (window);
  connect( job, SIGNAL( result (KJob *) ),
           this, SLOT( slotResult (KJob *) ) );

  enter_loop();
  return d->bJobOK;
}

bool NetAccess::dircopyInternal(const KUrl::List& src, const KUrl& target,
                                QWidget* window, bool move)
{
  d->bJobOK = true; // success unless further error occurs

  KIO::Job * job = move
                   ? KIO::move( src, target )
                   : KIO::copy( src, target );
  job->ui()->setWindow (window);
  connect( job, SIGNAL( result (KJob *) ),
           this, SLOT( slotResult (KJob *) ) );

  enter_loop();
  return d->bJobOK;
}

bool NetAccess::statInternal( const KUrl & url, int details, StatSide side,
                              QWidget* window )
{
  d->bJobOK = true; // success unless further error occurs
  KIO::JobFlags flags = url.isLocalFile() ? KIO::HideProgressInfo : KIO::DefaultFlags;
  KIO::StatJob * job = KIO::stat( url, flags );
  job->ui()->setWindow (window);
  job->setDetails( details );
  job->setSide( side == SourceSide ? StatJob::SourceSide : StatJob::DestinationSide );
  connect( job, SIGNAL( result (KJob *) ),
           this, SLOT( slotResult (KJob *) ) );
  enter_loop();
  return d->bJobOK;
}

bool NetAccess::delInternal( const KUrl & url, QWidget* window )
{
  d->bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::del( url );
  job->ui()->setWindow (window);
  connect( job, SIGNAL( result (KJob *) ),
           this, SLOT( slotResult (KJob *) ) );
  enter_loop();
  return d->bJobOK;
}

bool NetAccess::mkdirInternal( const KUrl & url, int permissions,
                               QWidget* window )
{
  d->bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::mkdir( url, permissions );
  job->ui()->setWindow (window);
  connect( job, SIGNAL( result (KJob *) ),
           this, SLOT( slotResult (KJob *) ) );
  enter_loop();
  return d->bJobOK;
}

QString NetAccess::mimetypeInternal( const KUrl & url, QWidget* window )
{
  d->bJobOK = true; // success unless further error occurs
  d->m_mimetype = QLatin1String("unknown");
  KIO::Job * job = KIO::mimetype( url );
  job->ui()->setWindow (window);
  connect( job, SIGNAL( result (KJob *) ),
           this, SLOT( slotResult (KJob *) ) );
  connect( job, SIGNAL( mimetype (KIO::Job *, const QString &) ),
           this, SLOT( slotMimetype (KIO::Job *, const QString &) ) );
  enter_loop();
  return d->m_mimetype;
}

void NetAccess::slotMimetype( KIO::Job *, const QString & type  )
{
  d->m_mimetype = type;
}

QString NetAccess::fish_executeInternal(const KUrl & url, const QString &command, QWidget* window)
{
  QString target, remoteTempFileName, resultData;
  KUrl tempPathUrl;
  KTemporaryFile tmpFile;
  tmpFile.open();

  if( url.protocol() == "fish" )
  {
    // construct remote temp filename
    tempPathUrl = url;
    remoteTempFileName = tmpFile.fileName();
    // only need the filename KTempFile adds some KDE specific dirs
    // that probably does not exist on the remote side
    int pos = remoteTempFileName.lastIndexOf('/');
    remoteTempFileName = "/tmp/fishexec_" + remoteTempFileName.mid(pos + 1);
    tempPathUrl.setPath( remoteTempFileName );
    d->bJobOK = true; // success unless further error occurs
    QByteArray packedArgs;
    QDataStream stream( &packedArgs, QIODevice::WriteOnly );

    stream << int('X') << tempPathUrl << command;

    KIO::Job * job = KIO::special( tempPathUrl, packedArgs );
    job->ui()->setWindow( window );
    connect( job, SIGNAL( result (KJob *) ),
             this, SLOT( slotResult (KJob *) ) );
    enter_loop();

    // since the KIO::special does not provide feedback we need to download the result
    if( NetAccess::download( tempPathUrl, target, window ) )
    {
      QFile resultFile( target );

      if (resultFile.open( QIODevice::ReadOnly ))
      {
        QTextStream ts( &resultFile ); // default encoding is Locale
        resultData = ts.readAll();
        resultFile.close();
        NetAccess::del( tempPathUrl, window );
      }
    }
  }
  else
  {
    resultData = i18n( "ERROR: Unknown protocol '%1'", url.protocol() );
  }
  return resultData;
}

bool NetAccess::synchronousRunInternal( Job* job, QWidget* window, QByteArray* data,
                                        KUrl* finalURL, QMap<QString,QString>* metaData )
{
  if ( job->ui() ) job->ui()->setWindow( window );

  d->m_metaData = metaData;
  if ( d->m_metaData ) {
      for ( QMap<QString, QString>::iterator it = d->m_metaData->begin(); it != d->m_metaData->end(); ++it ) {
          job->addMetaData( it.key(), it.value() );
      }
  }

  if ( finalURL ) {
      SimpleJob *sj = qobject_cast<SimpleJob*>( job );
      if ( sj ) {
          d->m_url = sj->url();
      }
  }

  connect( job, SIGNAL( result (KJob *) ),
           this, SLOT( slotResult (KJob *) ) );

  const QMetaObject* meta = job->metaObject();

  static const char dataSignal[] = "data(KIO::Job*,QByteArray)";
  if ( meta->indexOfSignal( dataSignal ) != -1 ) {
      connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
               this, SLOT(slotData(KIO::Job*,const QByteArray&)) );
  }

  static const char redirSignal[] = "redirection(KIO::Job*,KUrl)";
  if ( meta->indexOfSignal( redirSignal ) != -1 ) {
      connect( job, SIGNAL(redirection(KIO::Job*,const KUrl&)),
               this, SLOT(slotRedirection(KIO::Job*, const KUrl&)) );
  }

  enter_loop();

  if ( finalURL )
      *finalURL = d->m_url;
  if ( data )
      *data = d->m_data;

  return d->bJobOK;
}

void NetAccess::enter_loop()
{
    QEventLoop eventLoop;
    connect(this, SIGNAL(leaveModality()),
            &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void NetAccess::slotResult( KJob * job )
{
  lastErrorCode = job->error();
  d->bJobOK = !job->error();
  if ( !d->bJobOK )
  {
    if ( !lastErrorMsg )
      lastErrorMsg = new QString;
    *lastErrorMsg = job->errorString();
  }
  KIO::StatJob* statJob = qobject_cast<KIO::StatJob *>( job );
  if ( statJob )
    d->m_entry = statJob->statResult();

  KIO::Job* kioJob = qobject_cast<KIO::Job *>( job );
  if ( kioJob && d->m_metaData )
    *d->m_metaData = kioJob->metaData();

  emit leaveModality();
}

void NetAccess::slotData( KIO::Job*, const QByteArray& data )
{
  if ( data.isEmpty() )
    return;

  unsigned offset = d->m_data.size();
  d->m_data.resize( offset + data.size() );
  std::memcpy( d->m_data.data() + offset, data.data(), data.size() );
}

void NetAccess::slotRedirection( KIO::Job*, const KUrl& url )
{
  d->m_url = url;
}

#include "netaccess.moc"
