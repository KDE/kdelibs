/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>

   $Id$

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <assert.h>

#include <kdebug.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kurl.h>

#include "jobclasses.h"
#include "observer.h"

#include "uiserver_stub.h"

#include "passdlg.h"
#include "slavebase.h"
#include "observer_stub.h"
#include <kmessagebox.h>
#include <ksslinfodlg.h>
#include <ksslcertdlg.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>
#include <klocale.h>

using namespace KIO;

template class QIntDict<KIO::Job>;

Observer * Observer::s_pObserver = 0L;

const int KDEBUG_OBSERVER = 7007; // Should be 7028

Observer::Observer() : DCOPObject("KIO::Observer")
{
    // Register app as able to receive DCOP messages
    if (kapp && !kapp->dcopClient()->isAttached())
    {
        kapp->dcopClient()->attach();
    }

    if ( !kapp->dcopClient()->isApplicationRegistered( "kio_uiserver" ) )
    {
        kdDebug(KDEBUG_OBSERVER) << "Starting kio_uiserver" << endl;
        QString error;
        int ret = KApplication::startServiceByDesktopPath( "kio_uiserver.desktop",
                                                             QStringList(), &error );
        if ( ret > 0 )
        {
            kdError() << "Couldn't start kio_uiserver from kio_uiserver.desktop: " << error << endl;
        } else
            kdDebug(KDEBUG_OBSERVER) << "startServiceByDesktopPath returned " << ret << endl;

    }
    if ( !kapp->dcopClient()->isApplicationRegistered( "kio_uiserver" ) )
        kdDebug(KDEBUG_OBSERVER) << "The application kio_uiserver is STILL NOT REGISTERED" << endl;
    else
        kdDebug(KDEBUG_OBSERVER) << "kio_uiserver registered" << endl;

    m_uiserver = new UIServer_stub( "kio_uiserver", "UIServer" );
}

int Observer::newJob( KIO::Job * job, bool showProgress )
{
    // Tell the UI Server about this new job, and give it the application id
    // at the same time
    int progressId = m_uiserver->newJob( kapp->dcopClient()->appId(), showProgress );

    // Keep the result in a dict
    m_dctJobs.insert( progressId, job );

    return progressId;
}

void Observer::jobFinished( int progressId )
{
    m_uiserver->jobFinished( progressId );
    m_dctJobs.remove( progressId );
}

void Observer::killJob( int progressId )
{
    KIO::Job * job = m_dctJobs[ progressId ];
    if (!job)
    {
        kdWarning() << "Can't find job to kill ! There is no job with progressId=" << progressId << " in this process" << endl;
        return;
    }
    job->kill( false /* not quietly */ );
}

MetaData Observer::metadata( int progressId )
{
    KIO::Job * job = m_dctJobs[ progressId ];
    assert(job);
    if ( job->inherits("KIO::TransferJob") )
        return static_cast<KIO::TransferJob *>(job)->metaData();
    else
    {
        kdWarning() << "Observer::metaData(" << progressId << ") called on a job that is a " << job->className() << endl;
        return MetaData();
    }
}

void Observer::slotTotalSize( KIO::Job* job, KIO::filesize_t size )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotTotalSize " << job << " " << KIO::number(size) << endl;
  m_uiserver->totalSize64( job->progressId(), size );
}

void Observer::slotTotalFiles( KIO::Job* job, unsigned long files )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotTotalFiles " << job << " " << files << endl;
  m_uiserver->totalFiles( job->progressId(), files );
}

void Observer::slotTotalDirs( KIO::Job* job, unsigned long dirs )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotTotalDirs " << job << " " << dirs << endl;
  m_uiserver->totalDirs( job->progressId(), dirs );
}

void Observer::slotProcessedSize( KIO::Job* job, KIO::filesize_t size )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotProcessedSize " << job << " " << job->progressId() << " " << KIO::number(size) << endl;
  m_uiserver->processedSize64( job->progressId(), size );
}

void Observer::slotProcessedFiles( KIO::Job* job, unsigned long files )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotProcessedFiles " << job << " " << files << endl;
  m_uiserver->processedFiles( job->progressId(), files );
}

void Observer::slotProcessedDirs( KIO::Job* job, unsigned long dirs )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotProcessedDirs " << job << " " << dirs << endl;
  m_uiserver->processedDirs( job->progressId(), dirs );
}

void Observer::slotSpeed( KIO::Job* job, unsigned long bytes_per_second )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotSpeed " << job << " " << bytes_per_second << endl;
  m_uiserver->speed( job->progressId(), bytes_per_second );
}

void Observer::slotPercent( KIO::Job* job, unsigned long percent )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotPercent " << job << " " << percent << endl;
  m_uiserver->percent( job->progressId(), percent );
}

void Observer::slotInfoMessage( KIO::Job* job, const QString & msg )
{
  m_uiserver->infoMessage( job->progressId(), msg );
}

void Observer::slotCopying( KIO::Job* job, const KURL& from, const KURL& to )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotCopying " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->copying( job->progressId(), from, to );
}

void Observer::slotMoving( KIO::Job* job, const KURL& from, const KURL& to )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotMoving " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->moving( job->progressId(), from, to );
}

void Observer::slotDeleting( KIO::Job* job, const KURL& url )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotDeleting " << job << " " << url.url() << endl;
  m_uiserver->deleting( job->progressId(), url );
}

void Observer::slotTransferring( KIO::Job* job, const KURL& url )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotTransferring " << job << " " << url.url() << endl;
  m_uiserver->transferring( job->progressId(), url );
}

void Observer::slotCreatingDir( KIO::Job* job, const KURL& dir )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotCreatingDir " << job << " " << dir.url() << endl;
  m_uiserver->creatingDir( job->progressId(), dir );
}

void Observer::slotCanResume( KIO::Job* job, KIO::filesize_t offset )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotCanResume " << job << " " << KIO::number(offset) << endl;
  m_uiserver->canResume64( job->progressId(), offset );
}

void Observer::stating( KIO::Job* job, const KURL& url )
{
  m_uiserver->stating( job->progressId(), url );
}

void Observer::mounting( KIO::Job* job, const QString & dev, const QString & point )
{
  m_uiserver->mounting( job->progressId(), dev, point );
}

void Observer::unmounting( KIO::Job* job, const QString & point )
{
  m_uiserver->unmounting( job->progressId(), point );
}

bool Observer::openPassDlg( const QString& prompt, QString& user,
			    QString& pass, bool readOnly )
{
   AuthInfo info;
   info.prompt = prompt;
   info.username = user;
   info.password = pass;
   info.readOnly = readOnly;
   bool result = openPassDlg ( info );
   if ( result )
   {
     user = info.username;
     pass = info.password;
   }
   return result;
}

bool Observer::openPassDlg( KIO::AuthInfo& info )
{
    kdDebug(KDEBUG_OBSERVER) << "Observer::openPassDlg: User= " << info.username
                             << ", Message= " << info.prompt << endl;
    int result = KIO::PasswordDialog::getNameAndPassword( info.username, info.password,
                                                          &info.keepPassword, info.prompt,
                                                          info.readOnly, info.caption,
                                                          info.comment, info.commentLabel );
    if ( result == QDialog::Accepted )
    {
        info.setModified( true );
        return true;
    }
    return false;

#if 0
  kdDebug(KDEBUG_OBSERVER) << "Observer::openPassDlg: User= " << info.username
                << ", Message= " << info.prompt << endl;
  QCString replyType;
  QByteArray data, replyData;
  QDataStream stream( data, IO_WriteOnly );
  stream << info;
  if ( kapp->dcopClient()->call( "kio_uiserver", "UIServer",
                                 "openPassDlg(KIO::AuthInfo)",
                                 data, replyType, replyData, true ) &&
       replyType == "QByteArray" )
  {
    AuthInfo res_auth;
    QByteArray wrapper_data;
    QDataStream wrapper_stream( replyData, IO_ReadOnly );
    wrapper_stream >> wrapper_data;
    QDataStream data_stream( wrapper_data, IO_ReadOnly );
    data_stream >> res_auth;
    if( res_auth.isModified() )
    {
      info.username = res_auth.username;
      info.password = res_auth.password;
      return true;
    }
  }
  kdDebug(KDEBUG_OBSERVER) << "Observer::openPassDlg call failed!" << endl;
  return false;
#endif
}

int Observer::messageBox( int progressId, int type, const QString &text,
                          const QString &caption, const QString &buttonYes,
                          const QString &buttonNo )
{
    kdDebug() << "Observer::messageBox " << type << " " << text << " - " << caption << endl;
    int result = -1;

    switch (type) {
        case KIO::SlaveBase::QuestionYesNo:
            result = KMessageBox::questionYesNo( 0L, // parent ?
                                               text, caption, buttonYes, buttonNo );
            break;
        case KIO::SlaveBase::WarningYesNo:
            result = KMessageBox::warningYesNo( 0L, // parent ?
                                              text, caption, buttonYes, buttonNo );
            break;
        case KIO::SlaveBase::WarningContinueCancel:
            result = KMessageBox::warningContinueCancel( 0L, // parent ?
                                              text, caption, buttonYes );
            break;
        case KIO::SlaveBase::WarningYesNoCancel:
            result = KMessageBox::warningYesNoCancel( 0L, // parent ?
                                              text, caption, buttonYes, buttonNo );
            break;
        case KIO::SlaveBase::Information:
            KMessageBox::information( 0L, // parent ?
                                      text, caption );
            result = 1; // whatever
            break;
        case KIO::SlaveBase::SSLMessageBox:
        {
            QCString observerAppId = caption.utf8(); // hack, see slaveinterface.cpp
            // Contact the object "KIO::Observer" in the application <appId>
            // Yes, this could be the same application we are, but not necessarily.
            Observer_stub observer( observerAppId, "KIO::Observer" );

            KIO::MetaData meta = observer.metadata( progressId );
            KSSLInfoDlg *kid = new KSSLInfoDlg(meta["ssl_in_use"].upper()=="TRUE", 0L /*parent?*/, 0L, true);
            KSSLCertificate *x = KSSLCertificate::fromString(meta["ssl_peer_certificate"].local8Bit());
            if (x) {
               // Set the chain back onto the certificate
               QStringList cl =
                      QStringList::split(QString("\n"), meta["ssl_peer_chain"]);
               QPtrList<KSSLCertificate> ncl;

               ncl.setAutoDelete(true);
               for (QStringList::Iterator it = cl.begin(); it != cl.end(); ++it) {
                  KSSLCertificate *y = KSSLCertificate::fromString((*it).local8Bit());
                  if (y) ncl.append(y);
               }

               if (ncl.count() > 0)
                  x->chain().setChain(ncl);

               kid->setup( x,
                           meta["ssl_peer_ip"],
                           text, // the URL
                           meta["ssl_cipher"],
                           meta["ssl_cipher_desc"],
                           meta["ssl_cipher_version"],
                           meta["ssl_cipher_used_bits"].toInt(),
                           meta["ssl_cipher_bits"].toInt(),
                           KSSLCertificate::KSSLValidation(meta["ssl_cert_state"].toInt()));
               kdDebug(7024) << "Showing SSL Info dialog" << endl;
               kid->exec();
               delete x;
               kdDebug(7024) << "SSL Info dialog closed" << endl;
            } else {
               KMessageBox::information( 0L, // parent ?
                                         i18n("The peer SSL certificate appears to be corrupt."), i18n("SSL") );
            }
            // This doesn't have to get deleted.  It deletes on it's own.
            result = 1; // whatever
            break;
        }
        default:
            kdWarning() << "Observer::messageBox: unknown type " << type << endl;
            result = 0;
            break;
    }
    return result;
#if 0
    QByteArray data, replyData;
    QCString replyType;
    QDataStream arg( data, IO_WriteOnly );
    arg << progressId;
    arg << type;
    arg << text;
    arg << caption;
    arg << buttonYes;
    arg << buttonNo;
    if ( kapp->dcopClient()->call( "kio_uiserver", "UIServer", "messageBox(int,int,QString,QString,QString,QString)", data, replyType, replyData, true )
        && replyType == "int" )
    {
        int result;
        QDataStream _reply_stream( replyData, IO_ReadOnly );
        _reply_stream >> result;
        kdDebug(KDEBUG_OBSERVER) << "Observer::messageBox got result " << result << endl;
        return result;
    }
    kdDebug(KDEBUG_OBSERVER) << "Observer::messageBox call failed" << endl;
    return 0;
#endif
}

RenameDlg_Result Observer::open_RenameDlg( KIO::Job * job,
                                           const QString & caption,
                                           const QString& src, const QString & dest,
                                           RenameDlg_Mode mode, QString& newDest,
                                           KIO::filesize_t sizeSrc,
                                           KIO::filesize_t sizeDest,
                                           time_t ctimeSrc,
                                           time_t ctimeDest,
                                           time_t mtimeSrc,
                                           time_t mtimeDest
                                           )
{
  kdDebug(KDEBUG_OBSERVER) << "Observer::open_RenameDlg" << endl;
  /*
  QByteArray resultArgs = m_uiserver->open_RenameDlg64( job ? job->progressId() : 0, caption, src, dest, mode,
                                                      sizeSrc, sizeDest,
                                                      (unsigned long) ctimeSrc, (unsigned long) ctimeDest,
                                                      (unsigned long) mtimeSrc, (unsigned long) mtimeDest );
  if ( m_uiserver->ok() )  [...]
    QDataStream stream( resultArgs, IO_ReadOnly );
  */

  // We have to do it manually, to set useEventLoop to true.
  QByteArray data, replyData;
  QCString replyType;
  QDataStream arg( data, IO_WriteOnly );
  arg << (job ? job->progressId() : 0);
  arg << caption;
  arg << src;
  arg << dest;
  arg << (int) mode;
  arg << sizeSrc;
  arg << sizeDest;
  arg << (unsigned long) ctimeSrc;
  arg << (unsigned long) ctimeDest;
  arg << (unsigned long) mtimeSrc;
  arg << (unsigned long) mtimeDest;
  if ( kapp->dcopClient()->call( "kio_uiserver", "UIServer", "open_RenameDlg64(int,QString,QString,QString,int,KIO::filesize_t,KIO::filesize_t,unsigned long int,unsigned long int,unsigned long int,unsigned long int)", data, replyType, replyData, true ) &&
       replyType == "QByteArray" )
  {
    QDataStream stream( replyData, IO_ReadOnly );
    QByteArray res;
    stream >> res;

    QDataStream stream2( res, IO_ReadOnly );
    Q_UINT8 result;
    stream2 >> result >> newDest;
    kdDebug(KDEBUG_OBSERVER) << "UIServer::open_RenameDlg returned " << result << "," << newDest << endl;
    return (RenameDlg_Result) result;
  }
  kdDebug(KDEBUG_OBSERVER) << "open_RenameDlg call failed" << endl;
  return R_CANCEL;
}

SkipDlg_Result Observer::open_SkipDlg( KIO::Job * job,
                                       bool _multi,
                                       const QString& _error_text )
{
  kdDebug(KDEBUG_OBSERVER) << "Observer::open_SkipDlg" << endl;
  //int result = m_uiserver->open_SkipDlg( job ? job->progressId() : 0, (int)_multi, _error_text );
  int result;
  QByteArray data, replyData;
  QCString replyType;
  QDataStream arg( data, IO_WriteOnly );
  arg << ( job ? job->progressId() : 0 );
  arg << (int)_multi;
  arg << _error_text;
  if ( kapp->dcopClient()->call( "kio_uiserver", "UIServer", "open_SkipDlg(int,int,QString)", data, replyType, replyData, true )
       && replyType == "int" )
  {
    QDataStream stream( replyData, IO_ReadOnly );
    stream >> result;
    kdDebug(KDEBUG_OBSERVER) << "UIServer::open_SkipDlg returned " << result << endl;
    return (SkipDlg_Result) result;
  }
  kdDebug(KDEBUG_OBSERVER) << "open_SkipDlg call failed" << endl;
  return S_CANCEL;
}

void Observer::virtual_hook( int id, void* data )
{ DCOPObject::virtual_hook( id, data ); }

#include "observer.moc"
