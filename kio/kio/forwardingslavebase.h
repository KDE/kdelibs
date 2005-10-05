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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _FORWARDING_SLAVE_BASE_H_
#define _FORWARDING_SLAVE_BASE_H_

#include <kio/slavebase.h>
#include <kio/jobclasses.h>

#include <qobject.h>
#include <qeventloop.h>

namespace KIO
{

class ForwardingSlaveBasePrivate;

/**
 * This class should be used as a base for ioslaves acting as a
 * forwarder to other ioslaves. It has been designed to support only
 * local filesystem like ioslaves.
 *
 * If the resulting ioslave should be a simple proxy, you only need
 * to implement the ForwardingSlaveBase::rewriteURL() method.
 * 
 * For more advanced behavior, the classic ioslave methods should
 * be reimplemented, because their default behavior in this class
 * is to forward using the ForwardingSlaveBase::rewriteURL() method.
 * 
 * A possible code snippet for an advanced stat() behavior would look
 * like this in the child class:
 * 
 * \code
 *     void ChildProtocol::stat(const KURL &url)
 *     {
 *         bool is_special = false;
 *         
 *         // Process the URL to see if it should have
 *         // a special treatment
 *         
 *         if ( is_special )
 *         {
 *             // Handle the URL ourselves
 *             KIO::UDSEntry entry;
 *             // Fill entry with UDSAtom instances
 *             statEntry(entry);
 *             finished();
 *         }
 *         else
 *         {
 *             // Setup the ioslave internal state if
 *             // required by ChildProtocol::rewriteURL()
 *             ForwardingSlaveBase::stat(url);
 *         }
 *     }
 * \endcode
 * 
 * Of course in this case, you surely need to reimplement listDir()
 * and get() accordingly.
 * 
 * If you want view on directories to be correctly refreshed when
 * something changes on a forwarded URL, you'll need a companion kded
 * module to emit the KDirNotify Files*() DCOP signals.
 * 
 * This class was initially used for media:/ ioslave. This ioslave code
 * and the MediaDirNotify class of its companion kded module can be a
 * good source of inspiration.
 * 
 * @see ForwardingSlaveBase::rewriteURL()
 * @since 3.4
 * @author Kevin Ottens <ervin@ipsquad.net>
 */
class KIO_EXPORT ForwardingSlaveBase : public QObject, public SlaveBase
{
Q_OBJECT
public:
    ForwardingSlaveBase(const Q3CString &protocol,
                        const Q3CString &poolSocket,
                        const Q3CString &appSocket);
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
     *
     * @param url The URL as given during the slave call
     * @param newURL The new URL to forward the slave call to
     * @return true if the given url could be correctly rewritten
     */
    virtual bool rewriteURL(const KURL &url, KURL &newURL)=0;
    
    /**
     * Allow to modify a UDSEntry before it's sent to the ioslave enpoint.
     * This is the default implementation working in most case, but sometimes
     * you could make use of more forwarding black magic (for example
     * dynamically transform any desktop file into a fake directory...)
     *
     * @param entry the UDSEntry to post-process
     * @param listing indicate if this entry it created during a listDir
     *                operation
     */
    virtual void prepareUDSEntry(KIO::UDSEntry &entry,
                                 bool listing=false) const;
    
    /**
     * Return the URL being processed by the ioslave
     * Only access it inside prepareUDSEntry()
     */
    KURL processedURL() const { return m_processedURL; }

    /**
     * Return the URL asked to the ioslave
     * Only access it inside prepareUDSEntry()
     */
    KURL requestedURL() const { return m_requestedURL; }

private:
    KURL m_processedURL;
    KURL m_requestedURL;
    ForwardingSlaveBasePrivate *d;
    
    bool internalRewriteURL(const KURL &url, KURL &newURL);
    
    void connectJob(Job *job);
    void connectSimpleJob(SimpleJob *job);
    void connectListJob(ListJob *job);
    void connectTransferJob(TransferJob *job);

    QEventLoop eventLoop;

private slots:
    // KIO::Job
    void slotResult(KIO::Job *job);
    void slotWarning(KIO::Job *job, const QString &msg);
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
