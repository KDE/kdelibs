/* This file is part of the KDE project
   Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place -
   Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __KIO_SESSIONDATA_H
#define __KIO_SESSIONDATA_H

#include <qobject.h>

#include <kurl.h>

namespace KIO  {

class SlaveConfig;

/**
 * This class hanldes static io-slave config information.  Static
 * config information are options that rarely change and even when
 * they do they require user intervention. These kinds of options
 * are mostly modifiable by the end user from the control panel and
 * as such do not change as much as per session options, those that
 * can be modified programtically.  Some examples of static config
 * information are things such as locale settings, location of caching
 * directories, and state of the cookiejar.
 * 
 * Note that this class is only concerned with KDE configurations
 * that are needed by the io-slaves.
 *
 * @short Helper class that abstracts out static io-slave configuration
 *        infromation from the scheduler.
 *
 * @author Dawit Alemayehu <adawit@kde.org>
 */
class SessionData : public QObject
{
    Q_OBJECT

public:
    /** Constructor */
    SessionData();
    
    /** Destructor */
    ~SessionData();
    
    /** 
     * Sets any missing and/or required config data for the given
     * io-slave (protocol) and host.
     *
     * @param cfg     the configuration class passed to the io-slve
     * @param proto   the protocol (http, ftp, pop3) etc...
     * @param host    the host from which the resource is requested (eg: kde.org)
     */
    virtual void configDataFor( SlaveConfig* cfg, const QString& proto,
                                const QString& host = QString::null );
    /**
     * Re-reads configuration information.
     */
    virtual void reset();
    
    /**
     * Set the current request URL.
     *
     * This information, if supplied, is used to determine 
     * whether or not automatic login information is available
     * for the current request.
     *
     * @param url   current request URL.
     */
    void setRequestURL (const KURL& url);

public slots:
    
    /**
     * Slot that tracks and manages authentication information
     * using the supplied keys.
     *
     * @param
     * @param
     * @param
     */
    void slotAuthData( const QCString&, const QCString&, bool );
    
    /**
     * Removes the authenticion information stored using
     * the supplied key.
     *
     * @param key   key for authentication info to be deleted
     */
    void slotDelAuthData( const QCString& key );

private:
    struct AuthData;
    class AuthDataList;
    friend class AuthDataList;
    AuthDataList* authData;

    class SessionDataPrivate;
    SessionDataPrivate* d;
};

};

#endif
