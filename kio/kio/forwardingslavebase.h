/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

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

#ifndef _FORWARDING_SLAVE_BASE_H_
#define _FORWARDING_SLAVE_BASE_H_

#include <kio/slavebase.h>
#include <kio/jobclasses.h>

#include <qobject.h>

namespace KIO
{

class ForwardingSlaveBase : public QObject, public SlaveBase
{
Q_OBJECT
public:
    ForwardingSlaveBase(const QCString &protocol,
                        const QCString &poolSocket,
                        const QCString &appSocket);
    virtual ~ForwardingSlaveBase();

    virtual void get(const KURL &url);

    virtual void put(const KURL &url, int permissions,
                     bool overwrite, bool resume);

    virtual void stat(const KURL &url);

    virtual void mimetype(const KURL &url);

    virtual void listDir(const KURL &url);

    virtual void mkdir(const KURL &url, int permissions);

    virtual void rename(const KURL &src, const KURL &dest, bool overwrite);

    virtual void symlink(const QString &target, const KURL &dest,
                         bool overwrite);

    virtual void chmod(const KURL &url, int permissions);

    virtual void copy(const KURL &src, const KURL &dest,
                      int permissions, bool overwrite);

    virtual void del(const KURL &url, bool isfile);

protected:
    /**
     * Rewrite an url to it's forwarded counterpart. It should return
     * true if everything was ok, and false otherwise.
     *
     * If a problem is detected it's up to this method to trigger error()
     * before returning. Returning false silently cancel the current
     * slave operation.
     */
    virtual bool rewriteURL(const KURL &url, KURL &newURL)=0;
  
private:
    bool internalRewriteURL(const KURL &url, KURL &newURL);
    
    void connectJob(Job *job);
    void connectSimpleJob(SimpleJob *job);
    void connectListJob(ListJob *job);
    void connectTransferJob(TransferJob *job);

private slots:
    // KIO::Job
    void slotResult(KIO::Job *job);
    void slotInfoMessage(KIO::Job *job, const QString &msg);
    void slotTotalSize(KIO::Job *job, KIO::filesize_t size);
    void slotProcessedSize(KIO::Job *job, KIO::filesize_t size);
    void slotSpeed(KIO::Job *job, unsigned long bytesPerSecond);

    // KIO::SimpleJob subclasses
    void slotRedirection(KIO::Job *job, const KURL &url);

    // KIO::ListJob
    void slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries);

    // KIO::TransferJob
    void slotData(KIO::Job *job, const QByteArray &data);
    void slotDataReq(KIO::Job *job, QByteArray &data);
    void slotMimetype (KIO::Job *job, const QString &type);
    void slotCanResume (KIO::Job *job, KIO::filesize_t offset);
};

}

#endif
