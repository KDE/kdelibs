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
 grid = new QGridLayout(_frame, 7, 2, KDialog::marginHint(),
                                      KDialog::spacingHint() );
 _label = new QLabel("", _frame);
 grid->addMultiCellWidget(_label, 0, 0, 0, 1);

 setWidget(_frame);
 setReadWrite(true);
}



void KCertPart::setReadWrite(bool /*rw*/) {

}


bool KCertPart::saveFile() {

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

  // We have the file, lets work with it.
  _label->setText(p12->getCertificate()->getSubject());

delete p12;
return true;
}


