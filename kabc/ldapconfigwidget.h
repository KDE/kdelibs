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
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qstring.h>

#include <klineedit.h>
#include <kcombobox.h>
#include <kprogress.h>

#include <kabc/ldapurl.h>
#include <kabc/ldif.h>
#include <kio/job.h>


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
        W_ALL = 0xFFFFFFF
      } LCW_Flags;
      
      /** Constructs a configuration widget */    
      LdapConfigWidget( int flags = W_ALL, QWidget* parent = 0, 
        const char* name = 0, WFlags fl = 0 );
      /** Destructs a configuration widget */
      virtual ~LdapConfigWidget();

      /** Sets the user name. Kconfig widget name: kcfg_ldapuser */
      void setUser( const QString &user ) { mUser->setText( user ); }
      /** Gets the user name. Kconfig widget name: kcfg_ldapuser */
      QString user() const { return mUser->text(); }
    
      /** Sets the password. Kconfig widget name: kcfg_ldappassword */
      void setPassword( const QString &password ) { mPassword->setText( password ); }
      /** Gets the password. Kconfig widget name: kcfg_ldappassword */
      QString password() const { return mPassword->text(); }
    
      /** Sets the bind dn. Useful for SASL proxy auth. 
       * Kconfig widget name: kcfg_ldapbinddn 
       */
      void setBindDN( const QString &binddn ) { mBindDN->setText( binddn ); }
      /** Gets the bind dn. Kconfig widget name: kcfg_ldapbinddn*/
      QString bindDN() const { return mBindDN->text(); }
      
      /** Sets the SASL realm. Kconfig widget name: kcfg_ldaprealm */
      void setRealm( const QString &realm ) { mRealm->setText( realm ); }
      /** Gets the SASL realm. Kconfig widget name: kcfg_ldaprealm*/
      QString realm() const { return mRealm->text(); }
      
      /** Sets the host name. Kconfig widget name: kcfg_ldaphost */
      void setHost( const QString &host ) { mHost->setText( host ); }
      /** Gets the host name. Kconfig widget name: kcfg_ldaphost */
      QString host() const { return mHost->text(); }
    
      /** Sets the LDAP port. Kconfig widget name: kcfg_ldapport */
      void setPort( int port ) { mPort->setValue( port ); }
      /** Gets the LDAP port. Kconfig widget name: kcfg_ldapport */
      int port() const { return mPort->value(); }
    
      /** Sets the LDAP protocol version. Kconfig widget name: kcfg_ldapver */
      void setVer( int ver ) { mVer->setValue( ver ); }
      /** Gets the LDAP protocol version. Kconfig widget name: kcfg_ldapver */
      int ver() const { return mVer->value(); }

      /** Sets the LDAP Base DN. Kconfig widget name: kcfg_ldapdn */
      void setDn( const QString &dn ) { mDn->setText( dn ); }
      /** Gets the LDAP Base DN. Kconfig widget name: kcfg_ldapdn */
      QString dn() const { return mDn->text(); }
    
      /** Sets the LDAP Filter. Kconfig widget name: kcfg_ldapfilter */
      void setFilter( const QString &filter ) { mFilter->setText( filter ); }
      /** Gets the LDAP Filter. Kconfig widget name: kcfg_ldapfilter */
      QString filter() const { return mFilter->text(); }
      
      /** Sets the SASL Mechanism. Kconfig widget name: kcfg_ldapsaslmech */
      void setMech( const QString &mech );
      /** Gets the SASL Mechanism. Kconfig widget name: kcfg_ldapsaslmech */
      QString mech() const { return mMech->currentText(); }
      
      /** 
       * Sets the configuration to no transport security.
       * Kconfig widget name: kcfg_ldapnosec 
       */
      void setSecNO() { mSecNO->setChecked( true ); }
      /**
       * Returns true if no transport security selected.
       * Kconfig widget name: kcfg_ldapnosec 
       */
      bool isSecNO() const { return mSecNO->isChecked(); }
      
      /**
       * Sets the configuration to TLS.
       * Kconfig widget name: kcfg_ldaptls 
       */
      void setSecTLS() { mSecTLS->setChecked( true ); }
      /**
       * Returns true if TLS selected.
       * Kconfig widget name: kcfg_ldaptls
       */
      bool isSecTLS() const { return mSecTLS->isChecked(); }
      
      /**
       * Sets the configuration to SSL.
       * Kconfig widget name: kcfg_ldapssl
       */
      void setSecSSL() { mSecSSL->setChecked( true ); }
      /**
       * Returns true if SSL selected.
       * Kconfig widget name: kcfg_ldapssl
       */
      bool isSecSSL() const { return mSecSSL->isChecked(); }
      
      /**
       * Sets the authentication to anonymous.
       * Kconfig widget name: kcfg_ldapanon
       */
      void setAuthAnon() { mAnonymous->setChecked( true ); }
      /**
       * Returns true if Anonymous authentication selected.
       * Kconfig widget name: kcfg_ldapanon
       */
      bool isAuthAnon() const { return mAnonymous->isChecked(); }
      
      /**
       * Sets the authentication to simple.
       * Kconfig widget name: kcfg_ldapsimple
       */
      void setAuthSimple() { mSimple->setChecked( true ); }
      /**
       * Returns true if Simple authentication selected.
       * Kconfig widget name: kcfg_ldapsimple
       */
      bool isAuthSimple() const { return mSimple->isChecked(); }
      
      /**
       * Sets the authentication to SASL.
       * Kconfig widget name: kcfg_ldapsasl
       */
      void setAuthSASL() { mSASL->setChecked( true ); }
      /**
       * Returns true if SASL authentication selected.
       * Kconfig widget name: kcfg_ldapsasl
       */
      bool isAuthSASL() const { return mSASL->isChecked(); }
      
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
      QSpinBox  *mPort, *mVer;
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
      KProgressDialog *prog;
    
      class LDAPConfigWidgetPrivate;
      LDAPConfigWidgetPrivate *d;
    
      void sendQuery();
  };
}

#endif
