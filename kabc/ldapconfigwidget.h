/*
    This file is part of libkabc.
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef LDAPCONFIGWIDGET_H
#define LDAPCONFIGWIDGET_H

#include <qwidget.h>
#include <qmap.h>
#include <qstring.h>

#include <kabc/ldapurl.h>
#include <kabc/ldif.h>
#include <kio/job.h>

class QSpinBox;
class QPushButton;
class QCheckBox;
class QRadioButton;
class KComboBox;
class KLineEdit;
class KProgressDialog;

namespace KABC {

  /**
    @short LDAP Configuration widget
 
    This class can be used to query the user for LDAP connection parameters. 
    It's KConfigXT compatible, using widget names starting with kcfg_
  */

  class LdapConfigWidget : public QWidget
  {
    Q_OBJECT

    public:
    
      typedef enum _LCW_Flags {
        W_USER = 0x1,
        W_PASS = 0x2,
        W_BINDDN = 0x4,
        W_REALM = 0x8,
        W_HOST = 0x10,
        W_PORT = 0x20,
        W_VER = 0x40,
        W_DN = 0x80,
        W_FILTER = 0x100,
        W_SECBOX = 0x400,
        W_AUTHBOX = 0x800,
        W_TIMELIMIT = 0x1000,
        W_SIZELIMIT = 0x2000,
        W_ALL = 0xFFFFFFF
      } LCW_Flags;
      
      /** Constructs a configuration widget */    
      LdapConfigWidget( int flags = W_ALL, QWidget* parent = 0, 
        const char* name = 0, WFlags fl = 0 );
      /** Destructs a configuration widget */
      virtual ~LdapConfigWidget();

      /** Sets the user name. Kconfig widget name: kcfg_ldapuser */
      void setUser( const QString &user );
      /** Gets the user name. Kconfig widget name: kcfg_ldapuser */
      QString user() const;
    
      /** Sets the password. Kconfig widget name: kcfg_ldappassword */
      void setPassword( const QString &password );
      /** Gets the password. Kconfig widget name: kcfg_ldappassword */
      QString password() const;
    
      /** 
       * Sets the bind dn. Useful for SASL proxy auth. 
       * Kconfig widget name: kcfg_ldapbinddn 
       */
      void setBindDN( const QString &binddn );
      /** Gets the bind dn. Kconfig widget name: kcfg_ldapbinddn*/
      QString bindDN() const;
      
      /** Sets the SASL realm. Kconfig widget name: kcfg_ldaprealm */
      void setRealm( const QString &realm );
      /** Gets the SASL realm. Kconfig widget name: kcfg_ldaprealm */
      QString realm() const;
      
      /** Sets the host name. Kconfig widget name: kcfg_ldaphost */
      void setHost( const QString &host );
      /** Gets the host name. Kconfig widget name: kcfg_ldaphost */
      QString host() const;
    
      /** Sets the LDAP port. Kconfig widget name: kcfg_ldapport */
      void setPort( int port );
      /** Gets the LDAP port. Kconfig widget name: kcfg_ldapport */
      int port() const;
    
      /** Sets the LDAP protocol version. Kconfig widget name: kcfg_ldapver */
      void setVer( int ver );
      /** Gets the LDAP protocol version. Kconfig widget name: kcfg_ldapver */
      int ver() const;

      /** Sets the LDAP Base DN. Kconfig widget name: kcfg_ldapdn */
      void setDn( const QString &dn );
      /** Gets the LDAP Base DN. Kconfig widget name: kcfg_ldapdn */
      QString dn() const;
    
      /** Sets the LDAP Filter. Kconfig widget name: kcfg_ldapfilter */
      void setFilter( const QString &filter );
      /** Gets the LDAP Filter. Kconfig widget name: kcfg_ldapfilter */
      QString filter() const;
      
      /** Sets the SASL Mechanism. Kconfig widget name: kcfg_ldapsaslmech */
      void setMech( const QString &mech );
      /** Gets the SASL Mechanism. Kconfig widget name: kcfg_ldapsaslmech */
      QString mech() const;
      
      /** 
       * Sets the configuration to no transport security.
       * Kconfig widget name: kcfg_ldapnosec 
       */
      void setSecNO();
      /**
       * Returns true if no transport security selected.
       * Kconfig widget name: kcfg_ldapnosec 
       */
      bool isSecNO() const;
      
      /**
       * Sets the configuration to TLS.
       * Kconfig widget name: kcfg_ldaptls 
       */
      void setSecTLS();
      /**
       * Returns true if TLS selected.
       * Kconfig widget name: kcfg_ldaptls
       */
      bool isSecTLS() const;
      
      /**
       * Sets the configuration to SSL.
       * Kconfig widget name: kcfg_ldapssl
       */
      void setSecSSL();
      /**
       * Returns true if SSL selected.
       * Kconfig widget name: kcfg_ldapssl
       */
      bool isSecSSL() const;
      
      /**
       * Sets the authentication to anonymous.
       * Kconfig widget name: kcfg_ldapanon
       */
      void setAuthAnon();
      /**
       * Returns true if Anonymous authentication selected.
       * Kconfig widget name: kcfg_ldapanon
       */
      bool isAuthAnon() const;
      
      /**
       * Sets the authentication to simple.
       * Kconfig widget name: kcfg_ldapsimple
       */
      void setAuthSimple();
      /**
       * Returns true if Simple authentication selected.
       * Kconfig widget name: kcfg_ldapsimple
       */
      bool isAuthSimple() const;
      
      /**
       * Sets the authentication to SASL.
       * Kconfig widget name: kcfg_ldapsasl
       */
      void setAuthSASL();
      /**
       * Returns true if SASL authentication selected.
       * Kconfig widget name: kcfg_ldapsasl
       */
      bool isAuthSASL() const;
      
      /**
       * Sets the size limit.
       * KConfig widget name: kcfg_ldapsizelimit
       */
      void setSizeLimit( int sizelimit );
      /**
       * Returns the size limit.
       * KConfig widget name: kcfg_ldapsizelimit
       */
      int sizeLimit() const;
      
      /**
       * Sets the time limit.
       * KConfig widget name: kcfg_ldaptimelimit
       */
      void setTimeLimit( int timelimit );
      /**
       * Returns the time limit.
       * KConfig widget name: kcfg_ldaptimelimit
       */
      int timeLimit() const;
      
      /**
       * Returns a LDAP Url constructed from the settings given.
       * Extensions are filled for use in the LDAP ioslave
       */
      KABC::LDAPUrl url() const;
        
    private slots:
      void setLDAPPort();
      void setLDAPSPort();
      void setAnonymous( int state );
      void setSimple( int state );
      void setSASL( int state );
      void mQueryDNClicked();
      void mQueryMechClicked();
      void loadData( KIO::Job*, const QByteArray& );
      void loadResult( KIO::Job* );  
    private:

      LDIF mLdif;
      QStringList mQResult;
      QString mAttr;

      KLineEdit *mUser;
      KLineEdit *mPassword;
      KLineEdit *mHost;
      QSpinBox  *mPort, *mVer, *mSizeLimit, *mTimeLimit;
      KLineEdit *mDn, *mBindDN, *mRealm;
      KLineEdit *mFilter;
      QRadioButton *mAnonymous,*mSimple,*mSASL;
      QCheckBox *mSubTree;
      QPushButton *mEditButton;
      QPushButton *mQueryMech;
      QRadioButton *mSecNO,*mSecTLS,*mSecSSL;
      KComboBox *mMech;

      QString mErrorMsg;
      bool mCancelled;
      KProgressDialog *mProg;
    
      class LDAPConfigWidgetPrivate;
      LDAPConfigWidgetPrivate *d;
    
      void sendQuery();
  };
}

#endif
