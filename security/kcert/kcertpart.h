/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef _INCLUDE_KCERTPART_H
#define _INCLUDE_KCERTPART_H

#include <config.h>

#include <kparts/part.h>
#include <QtGui/QTreeWidget>
#include <QtCore/QVariantList>

class KSslCertificateBox;
class QFrame;
class QLabel;
class KCertPartPrivate;
class KComboBox;
class KSSLSigners;
class KSSLPKCS12;
class KSSLCertificate;
class KTabWidget;
class KTextEdit;
class KAboutData;
class QGridLayout;
class QPushButton;


class KX509Item : public QTreeWidgetItem {
	public:
		KX509Item(QTreeWidgetItem *parent, KSSLCertificate *x);
		KX509Item(QTreeWidget *parent, KSSLCertificate *x);
		void setup(KSSLCertificate *x);
		~KX509Item();
	KSSLCertificate *cert;
	QString _prettyName;
};


class KPKCS12Item : public QTreeWidgetItem {
	public:
		KPKCS12Item(QTreeWidgetItem *parent, KSSLPKCS12 *x);
		~KPKCS12Item();
	KSSLPKCS12 *cert;
	QString _prettyName;
};


class KCertPart : public KParts::ReadWritePart {
Q_OBJECT
public:
  explicit KCertPart(QWidget *parentWidget,
            QObject *parent = 0L,
	    const QVariantList &args = QVariantList() );
  virtual ~KCertPart();

  virtual void setReadWrite(bool rw);

  static KAboutData *createAboutData();

protected Q_SLOTS:
  void slotChain(int c);
  void slotImport();
  void slotSave();
  void slotDone();
  void slotLaunch();
  void slotSelectionChanged();
  void slotImportAll();

protected:

  virtual bool openFile();
  virtual bool saveFile();

  void displayPKCS12Cert(KSSLCertificate *c);
  void displayCACert(KSSLCertificate *c);

  QTreeWidget *_sideList;
  QTreeWidgetItem *_parentCA, *_parentP12;
  QFrame *_pkcsFrame, *_blankFrame, *_x509Frame, *_frame;

  // for the PKCS12 widget
  QLabel *_p12_filenameLabel, *_p12_validFrom, *_p12_validUntil,
         *_p12_serialNum, *_p12_certState;
  QLabel *_p12_digest;
  KComboBox *_p12_chain;
  KTextEdit*_p12_pubkey, *_p12_sig;
  KSslCertificateBox *_p12_subject, *_p12_issuer;

  // for the CA widget
  QLabel *_ca_filenameLabel, *_ca_validFrom, *_ca_validUntil,
         *_ca_serialNum, *_ca_certState;
  QLabel *_ca_digest;
  KTextEdit *_ca_pubkey, *_ca_sig;
  KSslCertificateBox *_ca_subject, *_ca_issuer;


  // The rest
  QPushButton *_import, *_save, *_done, *_launch, *_importAll;
  // Store the pointer to the current item
  KSSLPKCS12 *_p12;
  KSSLCertificate *_ca;
  KTabWidget *_tabs;
  QGridLayout *_baseGrid;
  KSSLSigners *_signers;
  bool _silentImport;
  QString _curName;

private:
  KCertPartPrivate* const d;
  void displayPKCS12();
};




#endif



