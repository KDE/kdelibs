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
      /** Constructs a configuration widget */    
      LdapConfigWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
      /** Destructs a configuration widget */
      virtual ~LdapConfigWidget();

      /** Sets the user name. Kconfig widget name: kcfg_ldapuser */
      void setUser( const QString &user ) { mUser->setText( user ); }
      /** Gets the user name. Kconfig widget name: kcfg_ldapuser */
      QString user() { return mUser->text(); }
    
      /** Sets the password. Kconfig widget name: kcfg_ldappassword */
      void setPassword( const QString &password ) { mPassword->setText( password ); }
      /** Gets the password. Kconfig widget name: kcfg_ldappassword */
      QString password() { return mPassword->text(); }
    
      /** Sets the host name. Kconfig widget name: kcfg_ldaphost */
      void setHost( const QString &host ) { mHost->setText( host ); }
      /** Gets the host name. Kconfig widget name: kcfg_ldaphost */
      QString host() { return mHost->text(); }
    
      /** Sets the LDAP port. Kconfig widget name: kcfg_ldapport */
      void setPort( int port ) { mPort->setValue( port ); }
      /** Gets the LDAP port. Kconfig widget name: kcfg_ldapport */
      int port() { return mPort->value(); }
    
      /** Sets the LDAP Base DN. Kconfig widget name: kcfg_ldapdn */
      void setDn( const QString &dn ) { mDn->setText( dn ); }
      /** Gets the LDAP Base DN. Kconfig widget name: kcfg_ldapdn */
      QString dn() { return mDn->text(); }
    
      /** Sets the LDAP Filter. Kconfig widget name: kcfg_ldapfilter */
      void setFilter( const QString &filter ) { mFilter->setText( filter ); }
      /** Gets the LDAP Filter. Kconfig widget name: kcfg_ldapfilter */
      QString filter() { return mFilter->text(); }
      
      /** Sets the SASL Mechanism. Kconfig widget name: kcfg_ldapsaslmech */
      void setMech( const QString &mech );
      /** Gets the SASL Mechanism. Kconfig widget name: kcfg_ldapsaslmech */
      QString mech() { return mMech->currentText(); }
      
      /** 
       * Sets the configuration to no transport security.
       * Kconfig widget name: kcfg_ldapnosec 
       */
      void setSecNO() { mSecNO->setChecked( true ); }
      /**
       * Returns true if no transport security selected.
       * Kconfig widget name: kcfg_ldapnosec 
       */
      bool isSecNO() { return mSecNO->isChecked(); }
      
      /**
       * Sets the configuration to TLS.
       * Kconfig widget name: kcfg_ldaptls 
       */
      void setSecTLS() { mSecTLS->setChecked( true ); }
      /**
       * Returns true if TLS selected.
       * Kconfig widget name: kcfg_ldaptls
       */
      bool isSecTLS() { return mSecTLS->isChecked(); }
      
      /**
       * Sets the configuration to SSL.
       * Kconfig widget name: kcfg_ldapssl
       */
      void setSecSSL() { mSecSSL->setChecked( true ); }
      /**
       * Returns true if SSL selected.
       * Kconfig widget name: kcfg_ldapssl
       */
      bool isSecSSL() { return mSecSSL->isChecked(); }
      
      /**
       * Sets the authentication to anonymous.
       * Kconfig widget name: kcfg_ldapanon
       */
      void setAuthAnon() { mAnonymous->setChecked( true ); setAnonymous();  }
      /**
       * Returns true if Anonymous authentication selected.
       * Kconfig widget name: kcfg_ldapanon
       */
      bool isAuthAnon() { return mAnonymous->isChecked(); }
      
      /**
       * Sets the authentication to simple.
       * Kconfig widget name: kcfg_ldapsimple
       */
      void setAuthSimple() { mSimple->setChecked( true ); setSimple(); }
      /**
       * Returns true if Simple authentication selected.
       * Kconfig widget name: kcfg_ldapsimple
       */
      bool isAuthSimple() { return mSimple->isChecked(); }
      
      /**
       * Sets the authentication to SASL.
       * Kconfig widget name: kcfg_ldapsasl
       */
      void setAuthSASL() { mSASL->setChecked( true ); setSASL(); }
      /**
       * Returns true if SASL authentication selected.
       * Kconfig widget name: kcfg_ldapsasl
       */
      bool isAuthSASL() { return mSASL->isChecked(); }
        
    private slots:
      void setLDAPPort();
      void setLDAPSPort();
      void setAnonymous();
      void setSimple();
      void setSASL();
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
      QSpinBox  *mPort;
      KLineEdit *mDn;
      KLineEdit *mFilter;
      QRadioButton *mAnonymous,*mSimple,*mSASL;
      QCheckBox *mSubTree;
      QPushButton *mEditButton;
      QPushButton *mQueryMech;
      QMap<QString, QString> mAttributes;
      QRadioButton *mSecNO,*mSecTLS,*mSecSSL;
      KComboBox *mMech;
    
      class LDAPConfigWidgetPrivate;
      LDAPConfigWidgetPrivate *d;
    
      void enter_loop();
      void sendQuery();
  };
}

#endif
