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
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <ksslall.h>


KCertPart::KCertPart(QWidget *parent, const char *name) 
          : KParts::ReadWritePart(parent, name) {
 KInstance *instance = new KInstance("KCertPart");
 QGridLayout *grid;
 setInstance(instance);


 _frame = new QFrame(parent);
 grid = new QGridLayout(_frame, 8, 6, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 grid->addMultiCellWidget(new QLabel(i18n("KDE Secure Certificate Import"), _frame), 0, 0, 0, 5);
 _subject = KSSLInfoDlg::certInfoWidget(_frame, QString(""));
 _issuer = KSSLInfoDlg::certInfoWidget(_frame, QString(""));
 grid->addMultiCellWidget(_subject, 1, 3, 0, 2);
 grid->addMultiCellWidget(_issuer, 1, 3, 3, 5);

 grid->addWidget(new QLabel(i18n("File:"), _frame), 4, 0);
 _filenameLabel = new QLabel("", _frame);
 grid->addWidget(_filenameLabel, 4, 1);
 grid->addWidget(new QLabel(i18n("File Format:"), _frame), 4, 3);
 grid->addWidget(new QLabel("PKCS#12", _frame), 4, 4);

 _validPeriod = new QLabel("", _frame);
 grid->addMultiCellWidget(_validPeriod, 5, 5, 0, 5);

 grid->addWidget(new QLabel(i18n("Serial Number:"), _frame), 6, 0);
 _serialNum = new QLabel("", _frame);
 grid->addWidget(_serialNum, 6, 1);

 grid->addWidget(new QLabel(i18n("State:"), _frame), 7, 0);
 _certState = new QLabel("", _frame);
 grid->addMultiCellWidget(_certState, 7, 7, 1, 5);

 setWidget(_frame);
 setReadWrite(true);
}



void KCertPart::setReadWrite(bool /*rw*/) {

}


bool KCertPart::saveFile() {

return false;
}


bool KCertPart::openFile() {
KSSLPKCS12 *p12 = KSSLPKCS12::loadCertFile(m_file);
QCString pass;

  while (!p12) {
     // try prompting for a password.
    int rc = KPasswordDialog::getPassword(pass, i18n("Certificate password"));
    if (rc != KPasswordDialog::Accepted) break;

    KSSLPKCS12 *p12 = KSSLPKCS12::loadCertFile(m_file, QString(pass));

    if (!p12) {
      rc = KMessageBox::warningYesNo(_frame, i18n("The certificate file could not be loaded.  Try a different password?"), i18n("Certificate Import"));
      if (rc == KMessageBox::Yes) continue;
      break;
    }
  }

  if (!p12) return false;

  KSSLCertificate *xc = p12->getCertificate();
  // We have the file, lets work with it.
  _subject->setValues(xc->getSubject());
  _issuer->setValues(xc->getIssuer());

  _filenameLabel->setText(m_file);
  // Set the valid period
         QPalette cspl = _validPeriod->palette();
         if (QDateTime::currentDateTime() < xc->getQDTNotBefore()
            || QDateTime::currentDateTime() > xc->getQDTNotAfter()) {
            cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
         } else {
            cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
         }
         _validPeriod->setPalette(cspl);
 
         QString validText("Certificate is valid from %1 to %2.");
         _validPeriod->setText(validText.arg(xc->getNotBefore()).arg(xc->getNotAfter()));

  _serialNum->setText(xc->getSerialNumber());
  _certState->setText(KSSLCertificate::verifyText(p12->validate()));

//delete p12;
return true;
}


