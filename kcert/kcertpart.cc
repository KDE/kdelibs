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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include "kcertpart.h"
#include <kinstance.h>
#include <qframe.h>
#include <klocale.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qbutton.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <ksslall.h>
#include <kfiledialog.h>
#include <kprocess.h>
#include <qtabwidget.h>
#include <kseparator.h>
#include <qmultilinedit.h>




KCertPart::KCertPart(QWidget *parent, const char *name) 
          : KParts::ReadWritePart(parent, name) {
 KInstance *instance = new KInstance("KCertPart");
 QGridLayout *grid;
 setInstance(instance);

 _p12 = NULL;

 _frame = new QFrame(parent);
 grid = new QGridLayout(_frame, 15, 6, KDialog::marginHint(),
                                       KDialog::spacingHint() );
 grid->addMultiCellWidget(new QLabel(i18n("KDE Secure Certificate Import"), _frame), 0, 0, 0, 5);
 grid->addWidget(new QLabel(i18n("Chain:"), _frame), 1, 0);
 _chain = new QComboBox(_frame);
 grid->addMultiCellWidget(_chain, 1, 1, 1, 4);
 connect(_chain, SIGNAL(activated(int)), SLOT(slotChain(int)));

 grid->addWidget(new QLabel(i18n("Subject:"), _frame), 2, 0);
 grid->addWidget(new QLabel(i18n("Issued by:"), _frame), 2, 3);
 _subject = KSSLInfoDlg::certInfoWidget(_frame, QString(""));
 _issuer = KSSLInfoDlg::certInfoWidget(_frame, QString(""));
 grid->addMultiCellWidget(_subject, 3, 6, 0, 2);
 grid->addMultiCellWidget(_issuer, 3, 6, 3, 5);

 grid->addWidget(new QLabel(i18n("File:"), _frame), 7, 0);
 _filenameLabel = new QLabel("", _frame);
 grid->addWidget(_filenameLabel, 7, 1);
 grid->addWidget(new QLabel(i18n("File Format:"), _frame), 7, 3);
 grid->addWidget(new QLabel("PKCS#12", _frame), 7, 4);


 //
 //  Make the first tab
 //
 _tabs = new QTabWidget(_frame);
 grid->addMultiCellWidget(_tabs, 8, 12, 0, 5);

 QFrame *tab = new QFrame(_frame);
 QGridLayout *tabGrid = new QGridLayout(tab, 4, 5, KDialog::marginHint(),
                                                   KDialog::spacingHint() );
 tabGrid->addWidget(new QLabel(i18n("State:"), tab), 0, 0);
 _certState = new QLabel("", tab);
 tabGrid->addMultiCellWidget(_certState, 0, 0, 1, 4);

 tabGrid->addWidget(new QLabel(i18n("Valid From:"), tab), 1, 0);
 _validFrom = new QLabel("", tab);
 tabGrid->addMultiCellWidget(_validFrom, 1, 1, 1, 4);

 tabGrid->addWidget(new QLabel(i18n("Valid Until:"), tab), 2, 0);
 _validUntil = new QLabel("", tab);
 tabGrid->addMultiCellWidget(_validUntil, 2, 2, 1, 4);

 tabGrid->addWidget(new QLabel(i18n("Serial Number:"), tab), 3, 0);
 _serialNum = new QLabel("", tab);
 tabGrid->addWidget(_serialNum, 3, 1);
 _tabs->addTab(tab, i18n("State"));


 //
 // Make the second tab
 //
 tab = new QFrame(_frame);
 tabGrid = new QGridLayout(tab, 4, 5, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 tabGrid->addWidget(new QLabel(i18n("MD5 Digest:"), tab), 0, 0);
 _digest = new QLabel(tab);
 tabGrid->addMultiCellWidget(_digest, 0, 0, 1, 4);

 _tabs->addTab(tab, i18n("Signature"));


 //
 // Make the third tab
 //
 tab = new QFrame(_frame);
 tabGrid = new QGridLayout(tab, 4, 5, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 tabGrid->addWidget(new QLabel(i18n("Public Key:"), tab), 0, 0);
 _pubkey = new QMultiLineEdit(tab);
 tabGrid->addMultiCellWidget(_pubkey, 0, 3, 1, 4);
 _pubkey->setReadOnly(true);
 

 _tabs->addTab(tab, i18n("Public Key"));


 //
 // Finish it off
 //
 grid->addMultiCellWidget(new KSeparator(KSeparator::HLine, _frame), 13, 13, 0, 5);
 _launch = new QPushButton(i18n("&Crypto Manager..."), _frame);
 _import = new QPushButton(i18n("&Import..."), _frame);
 _save = new QPushButton(i18n("&Save..."), _frame);
 _done = new QPushButton(i18n("&Done"), _frame);
 grid->addMultiCellWidget(_launch, 14, 14, 0, 1);
 grid->addWidget(_import, 14, 3);
 grid->addWidget(_save, 14, 4);
 grid->addWidget(_done, 14, 5);
 connect(_launch, SIGNAL(clicked()), SLOT(slotLaunch()));
 connect(_import, SIGNAL(clicked()), SLOT(slotImport()));
 connect(_save, SIGNAL(clicked()), SLOT(slotSave()));
 connect(_done, SIGNAL(clicked()), SLOT(slotDone()));
 _import->setEnabled(false);
 _save->setEnabled(false);

 setWidget(_frame);
 setReadWrite(true);
}


KCertPart::~KCertPart() {
if (_p12) delete _p12;
}


void KCertPart::setReadWrite(bool rw) {
  _import->setEnabled(rw);
  _save->setEnabled(rw);
  ReadWritePart::setReadWrite(rw);
}


bool KCertPart::saveFile() {
  QString certFile = KFileDialog::getSaveFileName();
  if (certFile.isEmpty())
    return false;
 
  if (!_p12->toFile(certFile)) {
    KMessageBox::sorry(_frame, i18n("Save failed."), i18n("Certificate Import"));
    return false;
  }

return true;
}


bool KCertPart::openFile() {
QCString pass;

  emit completed();
  if (_p12) delete _p12;
  _p12 = KSSLPKCS12::loadCertFile(m_file);

  while (!_p12) {
     // try prompting for a password.
    int rc = KPasswordDialog::getPassword(pass, i18n("Certificate password"));
    if (rc != KPasswordDialog::Accepted) break;

    _p12 = KSSLPKCS12::loadCertFile(m_file, QString(pass));

    if (!_p12) {
      rc = KMessageBox::warningYesNo(_frame, i18n("The certificate file could not be loaded.  Try a different password?"), i18n("Certificate Import"));
      if (rc == KMessageBox::Yes) continue;
      break;
    }
  }

  if (!_p12) return false;

  KSSLCertificate *xc = _p12->getCertificate();
  _filenameLabel->setText(m_file);
  displayCert(xc);
  _certState->setText(KSSLCertificate::verifyText(_p12->validate()));

  // Set the chain if it's there
  if (xc->chain().depth() > 1) {
    QList<KSSLCertificate> cl = xc->chain().getChain();
    int cnt = 0;
    _chain->setEnabled(true);
    _chain->clear();
    _chain->insertItem(i18n("0 - Site Certificate"));
    for (KSSLCertificate *c = cl.first(); c != 0; c = cl.next()) {
       KSSLX509Map map(c->getSubject());
       _chain->insertItem(QString::number(++cnt)+" - "+map.getValue("CN"));
    }
    _chain->setCurrentItem(0);
  } else {
    _chain->clear();
    _chain->setEnabled(false);
  }

 _import->setEnabled(true);
 _save->setEnabled(true);
return true;
}


void KCertPart::displayCert(KSSLCertificate *c) {
  // We have the file, lets work with it.
  _subject->setValues(c->getSubject());
  _issuer->setValues(c->getIssuer());

  // Set the valid period
         QPalette cspl = _validFrom->palette();
         if (QDateTime::currentDateTime() < c->getQDTNotBefore()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _validFrom->setPalette(cspl);
         _validFrom->setText(c->getNotBefore());

         cspl = _validUntil->palette();
         if (QDateTime::currentDateTime() > c->getQDTNotAfter()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _validUntil->setPalette(cspl);
         _validUntil->setText(c->getNotAfter());

  _serialNum->setText(c->getSerialNumber());
         cspl = _certState->palette();
         if (!c->isValid()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _certState->setPalette(cspl);
  _certState->setText(KSSLCertificate::verifyText(c->validate()));

  _pubkey->setText(c->getPublicKeyText());
  _digest->setText(c->getMD5DigestText());
}



void KCertPart::slotChain(int c) {

  if (c == 0) {
     displayCert(_p12->getCertificate());
     _certState->setText(KSSLCertificate::verifyText(_p12->validate()));
  } else {
     displayCert(_p12->getCertificate()->chain().getChain().at(c-1));
  }
}


void KCertPart::slotImport() {
  KSimpleConfig cfg("ksslcertificates", false);

  if (cfg.hasGroup(_p12->getCertificate()->getSubject())) {
     int rc = KMessageBox::warningYesNo(_frame, i18n("A certificate with that name already exists.  Are you sure that you wish to replace it?"), i18n("Certificate Import"));
     if (rc == KMessageBox::No) {
        return;
     }
  }

  cfg.setGroup(_p12->getCertificate()->getSubject());
  cfg.writeEntry("PKCS12Base64", _p12->toString());
  cfg.writeEntry("Password", "");
  cfg.sync();
  KMessageBox::information(_frame, i18n("Certificate has been successfully imported into KDE.\nYou can manage your certificate settings from the KDE Control Center."), i18n("Certificate Import"));
}


void KCertPart::slotSave() {
  saveFile();
}


void KCertPart::slotDone() {
}


void KCertPart::slotLaunch() {
KShellProcess p;
  p << "kcmshell" << "crypto";
  p.start(KProcess::DontCare);
}


#include "kcertpart.moc"

