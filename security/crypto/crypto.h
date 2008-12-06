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

#ifndef CRYPTO_H
#define CRYPTO_H

#include <ksslconfig.h>
#include <ksslall.h>
#include <klocale.h>
#include <QCheckBox>
#include <QDateTime>
#include <QTreeWidget>
#include <QLabel>
#include <kcmodule.h>
#include <kconfig.h>

class QGridLayout;
class QRadioButton;
class QGroupBox;
class QButtonGroup;

class KComboBox;
class KCryptoConfig;
class KLineEdit;
class KPushButton;
class KSslCertificateBox;
class KSSLSigners;
class KTabWidget;
class KUrlLabel;
class KUrlRequester;

class CipherItem : public QTreeWidgetItem
{
public:
    CipherItem( QTreeWidget *view, const QString& cipher, int bits, int maxBits,
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



class OtherCertItem : public QTreeWidgetItem
{
public:
    OtherCertItem(QTreeWidget *view, const QString& sub, const QString& md5, bool perm, int policy, QDateTime exp, KCryptoConfig *module );
    ~OtherCertItem() {}

    QString configName() const;
    const QString& getSub() { return _sub; }
    int getPolicy() const { return _policy; }
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




class YourCertItem : public QTreeWidgetItem
{
public:
    YourCertItem(QTreeWidget *view, QString pkcs, QString pass, QString name, KCryptoConfig *module );
    ~YourCertItem() {}

    QString configName() const;
    QString& getPKCS() { return _pkcs; }
    void setPKCS(QString pkcs) { _pkcs = pkcs; }
    QString& getPass() { return _pass; }
    void setPass(QString pass) { _pass = pass; }
    QString& getName() { return _name; }
    void setName(const QString &name) { _name = name; }
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




class CAItem : public QTreeWidgetItem
{
public:
    CAItem(QTreeWidget *view, QString name, QString cert, bool site, bool email, bool code, KCryptoConfig *module );
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





class HostAuthItem : public QTreeWidgetItem
{
public:
    HostAuthItem(QTreeWidget *view, const QString &host, const QString &name, KCryptoConfig *module ) : QTreeWidgetItem(view) {
                               _name = name;  _host = host;
                               m_module = module;
                               setText(0, _host);
                               setText(1, _name);
                               _oname.clear();
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
    void setCertName(const QString &name) { _name = name; setText(1, name); }
    void setHost(const QString &name) { _host = name; setText(0, name); }
    void setOriginalName(const QString &oname) { _oname = oname; }
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
  explicit KCryptoConfig(QWidget *parent = 0L, const QVariantList &list = QVariantList());
  virtual ~KCryptoConfig();

  void load();
  void save();
  void defaults();

#ifdef KSSL_HAVE_SSL
  bool loadCiphers();
#endif

public Q_SLOTS:
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

  KTabWidget *tabs;
  QWidget *tabSSL, *tabOSSL;

  QWidget *tabYourSSLCert, *tabOtherSSLCert, *tabSSLCA, *tabSSLCOpts, *tabAuth;

  QTreeWidget *SSLv3Box;
  QCheckBox *mWarnOnEnter, *mWarnOnLeave;

  /* EGD stuff */
  QLabel        *mEGDLabel;
  KUrlRequester *mEGDPath;
  QCheckBox     *mUseEGD;
  QCheckBox     *mUseEFile;

  /* CipherWizards */
  QPushButton *mCWall, *mCWus, *mCWexp, *mCWcompatible;

  QCheckBox *mWarnOnUnencrypted, *mWarnOnMixed;
  QTreeWidget *yourSSLBox, *otherSSLBox, *caList;
  QCheckBox *mWarnSelfSigned, *mWarnExpired, *mWarnRevoked;
  QPushButton *macAdd, *macRemove;
  KPushButton *macClear;
  QTreeWidget *macBox;
  QPushButton *otherSSLExport, *otherSSLView, *otherSSLRemove, *otherSSLVerify;
  QPushButton *yourSSLImport, *yourSSLPass, *yourSSLRemove, *yourSSLExport,
              *yourSSLUnlock, *yourSSLVerify;
  QRadioButton *yourSSLUseDefault, *yourSSLList, *yourSSLDont;
  KLineEdit *macCert;
  KSslCertificateBox *oSubject, *oIssuer;
  KSslCertificateBox *ySubject, *yIssuer;

  QButtonGroup *policyGroup;
  QGroupBox *policyGroupBox;
  QButtonGroup *cacheGroup;
  QGroupBox *cacheGroupBox;
  QRadioButton *policyAccept, *policyReject, *policyPrompt;
  QRadioButton *cacheUntil, *cachePerm;
  QLabel *fromLabel, *untilLabel;
  QLabel *validFrom, *validUntil;
  QLabel *yValidFrom, *yValidUntil;
  KUrlLabel *untilDate;

  QGroupBox  *oInfo;
  KUrlRequester *oPath;
  QPushButton *oTest;
  QList<OtherCertItem*> otherCertDelList;
  QList<YourCertItem*> yourCertDelList;
  QList<CAItem*> caDelList;

  /* Personal Cert Policies tab */
  KComboBox *defCertBox;
  KComboBox *hostCertBox;
  QButtonGroup *defCertBG;
  QButtonGroup *hostCertBG;
  QGroupBox *hostCertGroupBox;
  QRadioButton *defSend, *defPrompt, *defDont;
  QRadioButton *hostSend, *hostPrompt, *hostDont;
  QTreeWidget *hostAuthList;
  QPushButton *authAdd, *authRemove;
  KLineEdit *authHost;
  QList<HostAuthItem*> authDelList;
  QLabel *yHash, *pHash;

  /* CA stuff */
  KSslCertificateBox *caSubject, *caIssuer;
  QPushButton *caSSLImport, *caSSLRemove, *caSSLRestore;
  QCheckBox *caSite, *caEmail, *caCode;
  KSSLSigners *_signers;
  QLabel *cHash;

  KConfig *config;
  KConfig *policies, *pcerts, *authcfg;
  KConfig *cacfg;

  bool ___lehack;       // to hack around a lineedit problem
};

#endif
