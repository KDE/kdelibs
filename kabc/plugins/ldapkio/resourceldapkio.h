/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

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

#ifndef KABC_RESOURCELDAP_H
#define KABC_RESOURCELDAP_H

#include <kabc/resource.h>
#include <kabc/ldif.h>
#include <kio/job.h>

class KConfig;

namespace KABC {

class KABC_EXPORT ResourceLDAPKIO : public Resource
{
  Q_OBJECT

  public:
    enum CachePolicy{ Cache_No, Cache_NoConnection, Cache_Always };

    ResourceLDAPKIO( const KConfig* );
    virtual ~ResourceLDAPKIO();
    /**
     *  Call this after you used one of the set... methods 
     */
    virtual void init();
    
    virtual void writeConfig( KConfig* );

    virtual bool doOpen();
    virtual void doClose();

    virtual Ticket *requestSaveTicket();
    virtual void releaseSaveTicket( Ticket* );

    virtual bool readOnly() const { return Resource::readOnly(); }
    virtual void setReadOnly( bool value );

    virtual bool load();
    virtual bool asyncLoad();
    virtual bool save( Ticket * );
    virtual bool asyncSave( Ticket * );

    virtual void removeAddressee( const Addressee& addr );

    void setUser( const QString &user );
    QString user() const;

    void setPassword( const QString &password );
    QString password() const;
    
    void setRealm( const QString &realm );
    QString realm() const;
    
    void setBindDN( const QString &binddn );
    QString bindDN() const;

    void setDn( const QString &dn );
    QString dn() const;

    void setHost( const QString &host );
    QString host() const;

    void setPort( int port );
    int port() const;

    void setVer( int ver );
    int ver() const;
    
    void setSizeLimit( int sizelimit );
    int sizeLimit();
    
    void setTimeLimit( int timelimit );
    int timeLimit();

    void setFilter( const QString &filter );
    QString filter() const;

    void setIsAnonymous( bool value );
    bool isAnonymous() const;

    void setAttributes( const QMap<QString, QString> &attributes );
    QMap<QString, QString> attributes() const;
    
    void setRDNPrefix( int value );
    int RDNPrefix() const;

    void setIsTLS( bool value );
    bool isTLS() const ;
    
    void setIsSSL( bool value );
    bool isSSL() const;
    
    void setIsSubTree( bool value );
    bool isSubTree() const ;

    void setIsSASL( bool value );
    bool isSASL() const ;

    void setMech( const QString &mech );
    QString mech() const;

    void setCachePolicy( int pol );
    int cachePolicy() const;

    void setAutoCache( bool value );
    bool autoCache();
    
    QString cacheDst() const;
    
protected slots:
    void entries( KIO::Job*, const KIO::UDSEntryList& );
    void data( KIO::Job*, const QByteArray& );
    void result( KIO::Job* );
    void listResult( KIO::Job* );
    void syncLoadSaveResult( KIO::Job* );
    void saveResult( KIO::Job* );
    void saveData( KIO::Job*, QByteArray& );
    void loadCacheResult( KIO::Job* );
  
  private:
    QString mUser;
    QString mPassword;
    QString mDn;
    QString mHost;
    QString mFilter;
    int mPort;
    bool mAnonymous;
    QMap<QString, QString> mAttributes;

    KURL mLDAPUrl;
    int mGetCounter; //KDE 4: remove
    bool mErrorOccured; //KDE 4: remove
    QString mErrorMsg;
    QMap<KIO::Job*, QByteArray> mJobMap; //KDE 4: remove

    KIO::Job *loadFromCache();
    void createCache();
    void activateCache();
    void enter_loop();
    QCString addEntry( const QString &attr, const QString &value, bool mod );
    QString findUid( const QString &uid );
    bool AddresseeToLDIF( QByteArray &ldif, const Addressee &addr, 
      const QString &olddn );
    
    class ResourceLDAPKIOPrivate;
    ResourceLDAPKIOPrivate *d;
};

}

#endif
