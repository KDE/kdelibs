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
#include <qcombobox.h>
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <ksslall.h>


KCertPart::KCertPart(QWidget *parent, const char *name) 
          : KParts::ReadWritePart(parent, name) {
 KInstance *instance = new KInstance("KCertPart");
 QGridLayout *grid;
 setInstance(instance);

 _p12 = NULL;

 _frame = new QFrame(parent);
 grid = new QGridLayout(_frame, 9, 6, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 grid->addMultiCellWidget(new QLabel(i18n("KDE Secure Certificate Import"), _frame), 0, 0, 0, 5);
 grid->addWidget(new QLabel(i18n("Chain:"), _frame), 1, 0);
 _chain = new QComboBox(_frame);
 grid->addMultiCellWidget(_chain, 1, 1, 1, 4);
 connect(_chain, SIGNAL(activated(int)), SLOT(slotChain(int)));

 _subject = KSSLInfoDlg::certInfoWidget(_frame, QString(""));
 _issuer = KSSLInfoDlg::certInfoWidget(_frame, QString(""));
 grid->addMultiCellWidget(_subject, 2, 4, 0, 2);
 grid->addMultiCellWidget(_issuer, 2, 4, 3, 5);

 grid->addWidget(new QLabel(i18n("File:"), _frame), 5, 0);
 _filenameLabel = new QLabel("", _frame);
 grid->addWidget(_filenameLabel, 5, 1);
 grid->addWidget(new QLabel(i18n("File Format:"), _frame), 5, 3);
 grid->addWidget(new QLabel("PKCS#12", _frame), 5, 4);

 _validPeriod = new QLabel("", _frame);
 grid->addMultiCellWidget(_validPeriod, 6, 6, 0, 5);

 grid->addWidget(new QLabel(i18n("Serial Number:"), _frame), 7, 0);
 _serialNum = new QLabel("", _frame);
 grid->addWidget(_serialNum, 7, 1);

 grid->addWidget(new QLabel(i18n("State:"), _frame), 8, 0);
 _certState = new QLabel("", _frame);
 grid->addMultiCellWidget(_certState, 8, 8, 1, 5);

 setWidget(_frame);
 setReadWrite(true);
}


KCertPart::~KCertPart() {
if (_p12) delete _p12;
}


void KCertPart::setReadWrite(bool /*rw*/) {

}


bool KCertPart::saveFile() {

return false;
}


bool KCertPart::openFile() {
QCString pass;

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

return true;
}


void KCertPart::displayCert(KSSLCertificate *c) {
  // We have the file, lets work with it.
  _subject->setValues(c->getSubject());
  _issuer->setValues(c->getIssuer());

  // Set the valid period
         QPalette cspl = _validPeriod->palette();
         if (QDateTime::currentDateTime() < c->getQDTNotBefore()
            || QDateTime::currentDateTime() > c->getQDTNotAfter()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _validPeriod->setPalette(cspl);
 
         QString validText("Certificate is valid from %1 to %2.");
         _validPeriod->setText(validText.arg(c->getNotBefore()).arg(c->getNotAfter()));

  _serialNum->setText(c->getSerialNumber());
  _certState->setText(KSSLCertificate::verifyText(c->validate()));
}



void KCertPart::slotChain(int c) {

  if (c == 0) {
     displayCert(_p12->getCertificate());
     _certState->setText(KSSLCertificate::verifyText(_p12->validate()));
  } else {
     displayCert(_p12->getCertificate()->chain().getChain().at(c-1));
  }
}



#include "kcertpart.moc"

