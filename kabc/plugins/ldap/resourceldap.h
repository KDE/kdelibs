/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_RESOURCELDAP_H
#define KABC_RESOURCELDAP_H


#include <lber.h>
#include <ldap.h>

#include <kabc/resource.h>

class KConfig;

namespace KABC {

class ResourceLDAP : public Resource
{
  public:
    ResourceLDAP( const KConfig* );
    ResourceLDAP( const QString &user, const QString &passwd,
                  const QString &dn, const QString &host,
                  int port, const QString &filter, bool anonymous,
                  const QMap<QString, QString> &attributes );

    virtual void writeConfig( KConfig* );

    virtual bool doOpen();
    virtual void doClose();

    virtual Ticket *requestSaveTicket();
    virtual void releaseSaveTicket( Ticket* );

    virtual bool load();
    virtual bool asyncLoad();
    virtual bool save( Ticket * );

    virtual void removeAddressee( const Addressee& addr );

    void setUser( const QString &user );
    QString user() const;

    void setPassword( const QString &password );
    QString password() const;

    void setDn( const QString &dn );
    QString dn() const;

    void setHost( const QString &host );
    QString host() const;

    void setPort( int port );
    int port() const;

    void setFilter( const QString &filter );
    QString filter() const;

    void setIsAnonymous( bool value );
    bool isAnonymous() const;

    void setAttributes( const QMap<QString, QString> &attributes );
    QMap<QString, QString> attributes() const;

  protected:
    void init( const QString &user, const QString &passwd,
               const QString &dn, const QString &host,
               int port, const QString &filter, bool anonymous,
               const QMap<QString, QString> &attributes );

  private:
    QString mUser;
    QString mPassword;
    QString mDn;
    QString mHost;
    QString mFilter;
    int mPort;
    bool mAnonymous;
    QMap<QString, QString> mAttributes;

    LDAP *mLdap;
};

}

#endif
