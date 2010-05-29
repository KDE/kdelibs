/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "slaveinterface.h"
#include "slaveinterface_p.h"

#include "slavebase.h"
#include "connection.h"
#include "hostinfo_p.h"
#include <errno.h>
#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <klocale.h>
#include <kapplication.h>
#include <ksslinfodialog.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>
#include <kmessagebox.h>
#include <time.h>
#include <QtDBus/QtDBus>
#include <QtCore/QPointer>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslError>

using namespace KIO;


SlaveInterface::SlaveInterface(SlaveInterfacePrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
    connect(&d_ptr->speed_timer, SIGNAL(timeout()), SLOT(calcSpeed()));
}

SlaveInterface::~SlaveInterface()
{
    // Note: no kDebug() here (scheduler is deleted very late)

    delete d_ptr;
}

void SlaveInterface::setConnection( Connection* connection )
{
    Q_D(SlaveInterface);
    d->connection = connection;
}

Connection *SlaveInterface::connection() const
{
    const Q_D(SlaveInterface);
    return d->connection;
}

static KIO::filesize_t readFilesize_t(QDataStream &stream)
{
    KIO::filesize_t result;
    stream >> result;
    return result;
}

bool SlaveInterface::dispatch()
{
    Q_D(SlaveInterface);
    assert( d->connection );

    int cmd;
    QByteArray data;

    int ret = d->connection->read( &cmd, data );
    if (ret == -1)
      return false;

    return dispatch( cmd, data );
}

void SlaveInterface::calcSpeed()
{
    Q_D(SlaveInterface);
    if (d->slave_calcs_speed) {
        d->speed_timer.stop();
        return;
    }

    struct timeval tv;
    gettimeofday(&tv, 0);

    long diff = ((tv.tv_sec - d->start_time.tv_sec) * 1000000 +
                  tv.tv_usec - d->start_time.tv_usec) / 1000;
    if (diff - d->last_time >= 900) {
        d->last_time = diff;
        if (d->nums == max_nums) {
            // let's hope gcc can optimize that well enough
            // otherwise I'd try memcpy :)
            for (unsigned int i = 1; i < max_nums; ++i) {
                d->times[i-1] = d->times[i];
                d->sizes[i-1] = d->sizes[i];
            }
            d->nums--;
        }
        d->times[d->nums] = diff;
        d->sizes[d->nums++] = d->filesize - d->offset;

        KIO::filesize_t lspeed = 1000 * (d->sizes[d->nums-1] - d->sizes[0]) / (d->times[d->nums-1] - d->times[0]);

//      kDebug() << (long)d->filesize << diff
//          << long(d->sizes[d->nums-1] - d->sizes[0])
//          << d->times[d->nums-1] - d->times[0]
//          << long(lspeed) << double(d->filesize) / diff
//          << convertSize(lspeed)
//          << convertSize(long(double(d->filesize) / diff) * 1000);

        if (!lspeed) {
            d->nums = 1;
            d->times[0] = diff;
            d->sizes[0] = d->filesize - d->offset;
        }
        emit speed(lspeed);
    }
}

#ifndef KDE_USE_FINAL // already defined in slavebase.cpp
/*
 * Map pid_t to a signed integer type that makes sense for QByteArray;
 * only the most common sizes 16 bit and 32 bit are special-cased.
 */
template<int T> struct PIDType { typedef pid_t PID_t; } ;
template<> struct PIDType<2> { typedef qint16 PID_t; } ;
template<> struct PIDType<4> { typedef qint32 PID_t; } ;
#endif

bool SlaveInterface::dispatch(int _cmd, const QByteArray &rawdata)
{
    Q_D(SlaveInterface);
    //kDebug(7007) << "dispatch " << _cmd;

    QDataStream stream(rawdata);

    QString str1;
    qint32 i;
    qint8 b;
    quint32 ul;

    switch(_cmd) {
    case MSG_DATA:
        emit data(rawdata);
        break;
    case MSG_DATA_REQ:
        emit dataReq();
        break;
    case MSG_OPENED:
        emit open();
        break;
    case MSG_FINISHED:
        //kDebug(7007) << "Finished [this = " << this << "]";
        d->offset = 0;
        d->speed_timer.stop();
        emit finished();
        break;
    case MSG_STAT_ENTRY: {
        UDSEntry entry;
        stream >> entry;
        emit statEntry(entry);
        break;
    }
    case MSG_LIST_ENTRIES: {
        quint32 count;
        stream >> count;

        UDSEntryList list;
        UDSEntry entry;
        for (uint i = 0; i < count; i++) {
            stream >> entry;
            list.append(entry);
        }
        emit listEntries(list);
        break;
    }
    case MSG_RESUME: { // From the put job
        d->offset = readFilesize_t(stream);
        emit canResume(d->offset);
        break;
    }
    case MSG_CANRESUME: // From the get job
        d->filesize = d->offset;
        emit canResume(0); // the arg doesn't matter
        break;
    case MSG_ERROR:
        stream >> i >> str1;
        kDebug(7007) << "error " << i << " " << str1;
        emit error(i, str1);
        break;
    case MSG_SLAVE_STATUS: {
        PIDType<sizeof(pid_t)>::PID_t stream_pid;
        pid_t pid;
        QByteArray protocol;
        stream >> stream_pid >> protocol >> str1 >> b;
        pid = stream_pid;
        emit slaveStatus(pid, protocol, str1, (b != 0));
        break;
    }
    case MSG_CONNECTED:
        emit connected();
        break;
    case MSG_WRITTEN: {
        KIO::filesize_t size = readFilesize_t(stream);
        emit written(size);
        break;
    }
    case INF_TOTAL_SIZE: {
        KIO::filesize_t size = readFilesize_t(stream);
        gettimeofday(&d->start_time, 0);
        d->last_time = 0;
        d->filesize = d->offset;
        d->sizes[0] = d->filesize - d->offset;
        d->times[0] = 0;
        d->nums = 1;
        d->speed_timer.start(1000);
        d->slave_calcs_speed = false;
        emit totalSize(size);
        break;
    }
    case INF_PROCESSED_SIZE: {
        KIO::filesize_t size = readFilesize_t(stream);
        emit processedSize( size );
        d->filesize = size;
        break;
    }
    case INF_POSITION: {
        KIO::filesize_t pos = readFilesize_t(stream);
        emit position(pos);
        break;
    }
    case INF_SPEED:
        stream >> ul;
        d->slave_calcs_speed = true;
        d->speed_timer.stop();
        emit speed( ul );
        break;
    case INF_GETTING_FILE:
        break;
    case INF_ERROR_PAGE:
        emit errorPage();
        break;
    case INF_REDIRECTION: {
        KUrl url;
        stream >> url;
        emit redirection( url );
        break;
    }
    case INF_MIME_TYPE:
        stream >> str1;
        emit mimeType(str1);
        if (!d->connection->suspended())
            d->connection->sendnow(CMD_NONE, QByteArray());
        break;
    case INF_WARNING:
        stream >> str1;
        emit warning(str1);
        break;
    case INF_MESSAGEBOX: {
        kDebug(7007) << "needs a msg box";
        QString text, caption, buttonYes, buttonNo, dontAskAgainName;
        int type;
        stream >> type >> text >> caption >> buttonYes >> buttonNo;
        if (stream.atEnd()) {
            messageBox(type, text, caption, buttonYes, buttonNo);
        } else {
            stream >> dontAskAgainName;
            messageBox(type, text, caption, buttonYes, buttonNo, dontAskAgainName);
        }
        break;
    }
    case INF_INFOMESSAGE: {
        QString msg;
        stream >> msg;
        emit infoMessage(msg);
        break;
    }
    case INF_META_DATA: {
        MetaData meta_data;
        stream >> meta_data;
        d->m_incomingMetaData += meta_data;
//         kDebug(7007) << "incoming metadata now" << d->m_incomingMetaData
//                  << "\n newly arrived metadata is" << meta_data;
        emit metaData(meta_data);
        break;
    }
    case MSG_NET_REQUEST: {
        QString host;
        QString slaveid;
        stream >> host >> slaveid;
        requestNetwork(host, slaveid);
        break;
    }
    case MSG_NET_DROP: {
        QString host;
        QString slaveid;
        stream >> host >> slaveid;
        dropNetwork(host, slaveid);
        break;
    }
    case MSG_NEED_SUBURL_DATA: {
        emit needSubUrlData();
        break;
    }
    case MSG_HOST_INFO_REQ: {
        QString hostName;
        stream >> hostName;
        HostInfo::lookupHost(hostName, this, SLOT(slotHostInfo(QHostInfo)));
        break;
    }
    default:
        kWarning(7007) << "Slave sends unknown command (" << _cmd << "), dropping slave";
        return false;
    }
    return true;
}

void SlaveInterface::setOffset( KIO::filesize_t o)
{
    Q_D(SlaveInterface);
    d->offset = o;
}

KIO::filesize_t SlaveInterface::offset() const
{
    const Q_D(SlaveInterface);
    return d->offset;
}

void SlaveInterface::requestNetwork(const QString &host, const QString &slaveid)
{
    Q_D(SlaveInterface);
    kDebug(7007) << "requestNetwork " << host << slaveid;
    QByteArray packedArgs;
    QDataStream stream( &packedArgs, QIODevice::WriteOnly );
    stream << true;
    d->connection->sendnow( INF_NETWORK_STATUS, packedArgs );
}

void SlaveInterface::dropNetwork(const QString &host, const QString &slaveid)
{
    kDebug(7007) << "dropNetwork " << host << slaveid;
}

void SlaveInterface::sendResumeAnswer( bool resume )
{
    Q_D(SlaveInterface);
    kDebug(7007) << "ok for resuming:" << resume;
    d->connection->sendnow( resume ? CMD_RESUMEANSWER : CMD_NONE, QByteArray() );
}

void SlaveInterface::messageBox( int type, const QString &text, const QString &_caption,
                                 const QString &buttonYes, const QString &buttonNo )
{
    messageBox( type, text, _caption, buttonYes, buttonNo, QString() );
}

void SlaveInterface::messageBox( int type, const QString &text, const QString &caption,
                                 const QString &buttonYes, const QString &buttonNo, const QString &dontAskAgainName )
{
    Q_D(SlaveInterface);
    kDebug(7007) << "messageBox " << type << " " << text << " - " << caption << " " << dontAskAgainName;
    QByteArray packedArgs;
    QDataStream stream( &packedArgs, QIODevice::WriteOnly );

    QPointer<SlaveInterface> me = this;
    if (d->connection) d->connection->suspend();
    int result = d->messageBox( type, text, caption, buttonYes, buttonNo, dontAskAgainName );
    if ( me && d->connection ) // Don't do anything if deleted meanwhile
    {
        d->connection->resume();
        kDebug(7007) << this << " SlaveInterface result=" << result;
        stream << result;
        d->connection->sendnow( CMD_MESSAGEBOXANSWER, packedArgs );
    }
}

int SlaveInterfacePrivate::messageBox(int type, const QString &text,
                                      const QString &caption, const QString &buttonYes,
                                      const QString &buttonNo, const QString &dontAskAgainName)
{
    kDebug() << type << text << "caption=" << caption;
    int result = -1;
    KConfig *config = new KConfig("kioslaverc");
    KMessageBox::setDontShowAskAgainConfig(config);

    // SMELL: the braindead way to support button icons
    KGuiItem buttonYesGui, buttonNoGui;
    
    if (buttonYes == i18n("&Details"))
        buttonYesGui = KGuiItem(buttonYes, "help-about");
    else if (buttonYes == i18n("&Forever"))
        buttonYesGui = KGuiItem(buttonYes, "flag-green");
    else
        buttonYesGui = KGuiItem(buttonYes);

    if (buttonNo == i18n("Co&ntinue"))
        buttonNoGui = KGuiItem(buttonNo, "arrow-right");
    else if (buttonNo == i18n("&Current Session only"))
        buttonNoGui = KGuiItem(buttonNo, "chronometer");
    else
        buttonNoGui = KGuiItem(buttonNo);

    switch (type) {
    case KIO::SlaveBase::QuestionYesNo:
        result = KMessageBox::questionYesNo(
                     0, text, caption, buttonYesGui,
                     buttonNoGui, dontAskAgainName);
        break;
    case KIO::SlaveBase::WarningYesNo:
        result = KMessageBox::warningYesNo(
                     0, text, caption, buttonYesGui,
                     buttonNoGui, dontAskAgainName);
        break;
    case KIO::SlaveBase::WarningContinueCancel:
        result = KMessageBox::warningContinueCancel(
                     0, text, caption, buttonYesGui,
                     KStandardGuiItem::cancel(), dontAskAgainName);
        break;
    case KIO::SlaveBase::WarningYesNoCancel:
        result = KMessageBox::warningYesNoCancel(
                     0, text, caption, buttonYesGui, buttonNoGui,
                     KStandardGuiItem::cancel(), dontAskAgainName);
        break;
    case KIO::SlaveBase::Information:
        KMessageBox::information(0, text, caption, dontAskAgainName);
        result = 1; // whatever
        break;
    case KIO::SlaveBase::SSLMessageBox:
    {
        KIO::MetaData meta = m_incomingMetaData;
        KSslInfoDialog *kid = new KSslInfoDialog(0);
        //### this is boilerplate code and appears in khtml_part.cpp almost unchanged!
        QStringList sl = meta["ssl_peer_chain"].split('\x01', QString::SkipEmptyParts);
        QList<QSslCertificate> certChain;
        bool decodedOk = true;
        foreach (const QString &s, sl) {
            certChain.append(QSslCertificate(s.toAscii())); //or is it toLocal8Bit or whatever?
            if (certChain.last().isNull()) {
                decodedOk = false;
                break;
            }
        }

        if (decodedOk || true/*H4X*/) {
            kid->setSslInfo(certChain,
                            meta["ssl_peer_ip"],
                            text, // the URL
                            meta["ssl_protocol_version"],
                            meta["ssl_cipher"],
                            meta["ssl_cipher_used_bits"].toInt(),
                            meta["ssl_cipher_bits"].toInt(),
                            KSslInfoDialog::errorsFromString(meta["ssl_cert_errors"]));
            kDebug(7024) << "Showing SSL Info dialog";
            kid->exec();
            kDebug(7024) << "SSL Info dialog closed";
        } else {
            KMessageBox::information(0, i18n("The peer SSL certificate chain "
                                             "appears to be corrupt."),
                                     i18n("SSL"));
        }
        // KSslInfoDialog deletes itself (Qt::WA_DeleteOnClose).
        result = 1; // whatever
        break;
    }
    default:
        kWarning() << "unknown type" << type;
        result = 0;
        break;
    }
    KMessageBox::setDontShowAskAgainConfig(0);
    delete config;
    return result;
}

void SlaveInterfacePrivate::slotHostInfo(const QHostInfo& info)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream <<  info.hostName() << info.addresses() << info.error() << info.errorString();
    connection->send(CMD_HOST_INFO, data);
}

#include "slaveinterface.moc"
