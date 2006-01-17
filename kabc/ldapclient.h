/* kldapclient.h - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


#ifndef KABC_LDAPCLIENT_H
#define KABC_LDAPCLIENT_H


#include <QObject>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QPointer>
#include <QTimer>

#include <kio/job.h>

namespace KABC {

class LdapClient;
typedef QList<QByteArray> LdapAttrValue;
typedef QMap<QString,LdapAttrValue > LdapAttrMap;

/**
  * This class is internal. Binary compatibiliy might be broken any time
  * without notification. Do not use it.
  *
  * We mean it!
  *
  */
class KABC_EXPORT LdapObject
{
  public:
    LdapObject()
      : dn( QString() ), client( 0 ) {}
    explicit LdapObject( const QString& _dn, LdapClient* _cl ) : dn( _dn ), client( _cl ) {}
    LdapObject( const LdapObject& that ) { assign( that ); }

    LdapObject& operator=( const LdapObject& that )
    {
      assign( that );
      return *this;
    }

    QString toString() const;

    void clear();

    QString dn;
    LdapAttrMap attrs;
    LdapClient* client;

  protected:
    void assign( const LdapObject& that );

  private:
    //class LdapObjectPrivate* d;
};

/**
  * This class is internal. Binary compatibiliy might be broken any time
  * without notification. Do not use it.
  *
  * We mean it!
  *
  */
class KABC_EXPORT LdapClient : public QObject
{
  Q_OBJECT

  public:
    LdapClient( QObject* parent = 0);
    virtual ~LdapClient();

    /*! returns true if there is a query running */
    bool isActive() const { return mActive; }

  Q_SIGNALS:
    /*! Emitted when the query is done */
    void done();

    /*! Emitted in case of error */
    void error( const QString& );

    /*! Emitted once for each object returned
     * from the query
     */
    void result( const KABC::LdapObject& );

  public Q_SLOTS:
    /*!
     * Set the name or IP of the LDAP server
     */
    void setHost( const QString& host );
    QString host() const { return mHost; }

    /*!
     * Set the port of the LDAP server
     * if using a nonstandard port
     */
    void setPort( const QString& port );
    QString port() const { return mPort; }

    /*!
     * Set the base DN
     */
    void setBase( const QString& base );
    QString base() const { return mBase; }

    /*!
     * Set the bind DN
     */
    void setBindDN( const QString& bindDN );
    QString bindDN() const;

    /*!
     * Set the bind password DN
     */
    void setPwdBindDN( const QString& pwdBindDN );
    QString pwdBindDN() const;

    /*! Set the attributes that should be
     * returned, or an empty list if
     * all attributes are wanted
     */
    void setAttrs( const QStringList& attrs );
    QStringList attrs() const { return mAttrs; }

    void setScope( const QString scope ) { mScope = scope; }

    /*!
     * Start the query with filter filter
     */
    void startQuery( const QString& filter );

    /*!
     * Abort a running query
     */
    void cancelQuery();

  protected Q_SLOTS:
    void slotData( KIO::Job*, const QByteArray &data );
    void slotInfoMessage( KIO::Job*, const QString &info );
    void slotDone();

  protected:
    void startParseLDIF();
    void parseLDIF( const QByteArray& data );
    void endParseLDIF();

    QString mHost;
    QString mPort;
    QString mBase;
    QString mScope;
    QStringList mAttrs;

    QPointer<KIO::SimpleJob> mJob;
    bool mActive;

    LdapObject mCurrentObject;
    QByteArray mBuf;
    QByteArray mLastAttrName;
    QByteArray mLastAttrValue;
    bool mIsBase64;

  private:
    class LdapClientPrivate;
    LdapClientPrivate* d;
};

/**
 * Structure describing one result returned by a LDAP query
 */
struct LdapResult {
  QString name;     ///< full name
  QString email;    ///< email
  int clientNumber; ///< for sorting
};
typedef QList<LdapResult> LdapResultList;


/**
  * This class is internal. Binary compatibiliy might be broken any time
  * without notification. Do not use it.
  *
  * We mean it!
  *
  */
class KABC_EXPORT LdapSearch : public QObject
{
  Q_OBJECT

  public:
    LdapSearch();

    void startSearch( const QString& txt );
    void cancelSearch();
    bool isAvailable() const;

  Q_SIGNALS:
    /// Results, assembled as "Full Name <email>"
    /// (This signal can be emitted many times)
    void searchData( const QStringList& );
    /// Another form for the results, with separate fields
    /// (This signal can be emitted many times)
    void searchData( const KABC::LdapResultList& );
    void searchDone();

  private Q_SLOTS:
    void slotLDAPResult( const KABC::LdapObject& );
    void slotLDAPError( const QString& );
    void slotLDAPDone();
    void slotDataTimer();

  private:
    void finish();
    void makeSearchData( QStringList& ret, LdapResultList& resList );
    QList< LdapClient* > mClients;
    QString mSearchText;
    QTimer mDataTimer;
    int mActiveClients;
    bool mNoLDAPLookup;
    QList< LdapObject > mResults;

  private:
    class LdapSearchPrivate* d;
};

}
#endif // KABC_LDAPCLIENT_H
