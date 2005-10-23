/**
 * crypto.h
 *
 * Copyright (c) 2000-2005 George Staikos <staikos@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _CRYPTO_H
#define _CRYPTO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qcheckbox.h>
#include <qdatetime.h>
#include <qlineedit.h>
#include <q3listview.h>
#include <q3ptrlist.h>
#include <qtabwidget.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QLabel>

#include <kcmodule.h>
#include <ksimpleconfig.h>

class QGridLayout;
class Q3VButtonGroup;

class KComboBox;
class KCryptoConfig;
class KPushButton;
class KSSLCertBox;
class KSSLSigners;
class KURLLabel;
class KURLRequester;

class CipherItem : public Q3CheckListItem
{
public:
    CipherItem( Q3ListView *view, const QString& cipher, int bits, int maxBits,
		KCryptoConfig *module );
    ~CipherItem() {}

    void setCipher( const QString& cipher ) { m_cipher = cipher; }
    const QString& cipher() const { return m_cipher; }
    void setBits( int bits ) { m_bits = bits; }
    int bits() const { return m_bits; }

    QString configName() const;

protected:
    virtual void stateChange( bool );

private:
    int m_bits;
    QString m_cipher;
    KCryptoConfig *m_module; // just to call configChanged()
};



class OtherCertItem : public Q3ListViewItem
{
public:
    OtherCertItem(Q3ListView *view, const QString& sub, const QString& md5, bool perm, int policy, QDateTime exp, KCryptoConfig *module );
    ~OtherCertItem() {}

    QString configName() const;
    const QString& getSub() { return _sub; }
    int getPolicy() { return _policy; }
    const QString& getMD5() { return _md5; }
    bool isPermanent() { return _perm; }
    QDateTime getExpires() { return _exp; }
    void setPolicy(int x) { _policy = x; }
    void setPermanent(bool x) { _perm = x; }
    void setExpires(QDateTime x) { _exp = x; }

protected:
    virtual void stateChange( bool );

private:
    QString _sub, _md5;
    KCryptoConfig *m_module; // just to call configChanged()
    QDateTime _exp;
    bool _perm;
    int _policy;
};




class YourCertItem : public Q3ListViewItem
{
public:
    YourCertItem(Q3ListView *view, QString pkcs, QString pass, QString name, KCryptoConfig *module );
    ~YourCertItem() {}

    QString configName() const;
    QString& getPKCS() { return _pkcs; }
    void setPKCS(QString pkcs) { _pkcs = pkcs; }
    QString& getPass() { return _pass; }
    void setPass(QString pass) { _pass = pass; }
    QString& getName() { return _name; }
    void setName(QString name) { _name = name; }
    QString& getPassCache() { return _cpass; }
    void setPassCache(QString pass) { _cpass = pass; }

protected:
    virtual void stateChange( bool );

private:
    QString _pkcs;
    QString _pass;
    QString _cpass;
    QString _name;
    KCryptoConfig *m_module; // just to call configChanged()
};




class CAItem : public Q3ListViewItem
{
public:
    CAItem(Q3ListView *view, QString name, QString cert, bool site, bool email, bool code, KCryptoConfig *module );
    ~CAItem() {}

    QString configName() const;
    QString& getName() { return _name; }
    void setName(QString name) { _name = name; }

    inline QString getCert() const { return _cert; }
    inline bool getSite() const { return _site; }
    inline bool getEmail() const { return _email; }
    inline bool getCode() const { return _code; }
    inline void setSite(bool x) { _site = x; }
    inline void setEmail(bool x) { _email = x; }
    inline void setCode(bool x) { _code = x; }

    bool isNew, modified;

protected:
    virtual void stateChange( bool );

private:
    QString _name;
    QString _cert;
    bool _site, _email, _code;
    KCryptoConfig *m_module; // just to call configChanged()
};





class HostAuthItem : public Q3ListViewItem
{
public:
    HostAuthItem(Q3ListView *view, QString host, QString name, KCryptoConfig *module ) : Q3ListViewItem(view, QString::null ) {
                               _name = name;  _host = host;
                               m_module = module;
                               setText(0, _host);
                               setText(1, _name);
                               _oname = QString::null;
                              }
    ~HostAuthItem() {}

    void setAction(KSSLCertificateHome::KSSLAuthAction aa) {
                 _aa = aa;
                               switch (aa) {
                               case KSSLCertificateHome::AuthSend:
                                 setText(2, i18n("Send"));
                                break;
                               case KSSLCertificateHome::AuthDont:
                                 setText(2, i18n("Don't Send"));
                                break;
                               case KSSLCertificateHome::AuthPrompt:
                                 setText(2, i18n("Prompt"));
                                break;
                               default:
                                break;
                               }
    }
    KSSLCertificateHome::KSSLAuthAction getAction() const { return _aa; }
    QString configName() const { return _host; }
    QString getCertName() const { return _name; }
    void setCertName(QString name) { _name = name; setText(1, name); }
    void setHost(QString name) { _host = name; setText(0, name); }
    void setOriginalName(QString oname) { _oname = oname; }
    QString originalName() const { return _oname; }

protected:

private:
    QString _host;
    QString _name, _oname;
    KSSLCertificateHome::KSSLAuthAction _aa;
    KCryptoConfig *m_module; // just to call configChanged()
};





class KCryptoConfig : public KCModule
{
  Q_OBJECT
public:
  KCryptoConfig(QWidget *parent = 0L, const char *name = 0L, const QStringList &list = QStringList());
  virtual ~KCryptoConfig();

  void load();
  void save();
  void defaults();

#ifdef HAVE_SSL
  bool loadCiphers();
#endif

public slots:
  void configChanged();

  void slotGeneratePersonal();
  void slotUseEGD();
  void slotUseEFile();
  void slotSelectCipher(int id);

  void slotTestOSSL();

  void slotExportCert();
  void slotRemoveCert();
  void slotVerifyCert();
  void slotOtherCertSelect();
  void slotPolicyChanged(int id);
  void slotPermanent();
  void slotUntil();
  void slotDatePick();

  void slotYourImport();
  void slotYourExport();
  void slotYourVerify();
  void slotYourRemove();
  void slotYourUnlock();
  void slotYourPass();
  void slotYourCertSelect();

  void slotNewHostAuth();
  void slotRemoveHostAuth();
  void slotAuthItemChanged();
  void slotAuthText(const QString &t);
  void slotAuthButtons();
  void slotAuthCombo();

  void slotCAImport();
  void slotCARemove();
  void slotCARestore();
  void slotCAItemChanged();
  void slotCAChecked();

protected:
  void cwUS();
  void cwExp();
  void cwAll();

private:

  void offerImportToKMail( const QString& certFile );
  void setAuthCertLists();
  void genCAList();

  QTabWidget *tabs;
  QWidget *tabSSL, *tabOSSL;

  QWidget *tabYourSSLCert, *tabOtherSSLCert, *tabSSLCA, *tabSSLCOpts, *tabAuth;

  Q3ListView *SSLv3Box;
  QCheckBox *mWarnOnEnter, *mWarnOnLeave;

  /* EGD stuff */
  QLabel        *mEGDLabel;
  KURLRequester *mEGDPath;
  QCheckBox     *mUseEGD;
  QCheckBox     *mUseEFile;

  /* CipherWizards */
  QPushButton *mCWall, *mCWus, *mCWexp, *mCWcompatible;

  QCheckBox *mWarnOnUnencrypted, *mWarnOnMixed;
  Q3ListView *yourSSLBox, *otherSSLBox, *caList;
  QCheckBox *mWarnSelfSigned, *mWarnExpired, *mWarnRevoked;
  QPushButton *macAdd, *macRemove;
  KPushButton *macClear;
  Q3ListBox *macBox;
  QPushButton *otherSSLExport, *otherSSLView, *otherSSLRemove, *otherSSLVerify;
  QPushButton *yourSSLImport, *yourSSLPass, *yourSSLRemove, *yourSSLExport,
              *yourSSLUnlock, *yourSSLVerify;
  QRadioButton *yourSSLUseDefault, *yourSSLList, *yourSSLDont;
  QLineEdit *macCert;
  KSSLCertBox *oSubject, *oIssuer;
  KSSLCertBox *ySubject, *yIssuer;
  QGridLayout *oGrid;

  Q3VButtonGroup *policyGroup;
  Q3VButtonGroup *cacheGroup;
  QRadioButton *policyAccept, *policyReject, *policyPrompt;
  QRadioButton *cacheUntil, *cachePerm;
  QLabel *fromLabel, *untilLabel;
  QLabel *validFrom, *validUntil;
  QLabel *yValidFrom, *yValidUntil;
  KURLLabel *untilDate;

  Q3GroupBox  *oInfo;
  KURLRequester *oPath;
  QPushButton *oTest;
  Q3PtrList<OtherCertItem> otherCertDelList;
  Q3PtrList<YourCertItem> yourCertDelList;
  Q3PtrList<CAItem> caDelList;

  /* Personal Cert Policies tab */
  KComboBox *defCertBox;
  KComboBox *hostCertBox;
  Q3VButtonGroup *defCertBG;
  Q3HButtonGroup *hostCertBG;
  QRadioButton *defSend, *defPrompt, *defDont;
  QRadioButton *hostSend, *hostPrompt, *hostDont;
  Q3ListView *hostAuthList;
  QPushButton *authAdd, *authRemove;
  QLineEdit *authHost;
  Q3PtrList<HostAuthItem> authDelList;
  QLabel *yHash, *pHash;

  /* CA stuff */
  KSSLCertBox *caSubject, *caIssuer;
  QPushButton *caSSLImport, *caSSLRemove, *caSSLRestore;
  QCheckBox *caSite, *caEmail, *caCode;
  KSSLSigners *_signers;
  QLabel *cHash;

  KConfig *config;
  KSimpleConfig *policies, *pcerts, *authcfg;
  KConfig *cacfg;

  bool ___lehack;       // to hack around a lineedit problem
};

#endif
