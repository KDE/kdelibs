/**
 * certexport.cpp
 *
 * Copyright (c) 2001 George Staikos <staikos@kde.org>
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "certexport.h"
#include <qpushbutton.h>
//Added by qt3to4:
#include <QGridLayout>
#include <klineedit.h>
#include <kfiledialog.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <ksslall.h>


KCertExport::KCertExport(QWidget *parent, const char *name) 
                             : KDialog(parent, name, true) {
QGridLayout *grid = new QGridLayout(this, 9, 6, marginHint(), spacingHint());

   setCaption(i18n("X509 Certificate Export"));

   Q3ButtonGroup *bg = new Q3ButtonGroup(i18n("Format"), this);
   bg->setOrientation(Qt::Vertical);
   _pem = new QRadioButton(i18n("&PEM"), bg);
   _netscape = new QRadioButton(i18n("&Netscape"), bg);
   _der = new QRadioButton(i18n("&DER/ASN1"), bg);
   _text = new QRadioButton(i18n("&Text"), bg);
   grid->addMultiCellWidget(bg, 0, 4, 0, 3);
   _pem->setChecked(true);

   grid->addMultiCellWidget(new QLabel(i18n("Filename:"), this), 5, 5, 0, 3);

   _filename = new KLineEdit(this);
   grid->addMultiCellWidget(_filename, 6, 6, 0, 4);
   connect(_filename, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged(const QString &)));
   connect(_filename, SIGNAL(returnPressed()), this, SLOT(slotExport()));

   _choose = new QPushButton("...", this);
   grid->addWidget(_choose, 6, 5);
   connect(_choose, SIGNAL(clicked()), this, SLOT(slotChoose()));
   
   _export = new QPushButton(i18n("&Export"), this);
   grid->addWidget(_export, 8, 4);
   connect(_export, SIGNAL(clicked()), this, SLOT(slotExport()));
   _export->setEnabled(false);

   _cancel = new QPushButton(i18n("&Cancel"), this);
   grid->addWidget(_cancel, 8, 5);
   connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
}


KCertExport::~KCertExport() {

}


void KCertExport::setCertificate(KSSLCertificate *c) {
   _c = c;
}


void KCertExport::slotExport() {
QByteArray cert;
QString certt;

   if (_filename->text().isEmpty()) return;

   if (!_c) {
      KMessageBox::sorry(this, i18n("Internal error. Please report to kfm-devel@kde.org."), i18n("SSL"));
      return;
   }

   if (_der->isChecked()) {
      cert = _c->toDer();
   } else if (_pem->isChecked()) {
      cert = _c->toPem();
   } else if (_text->isChecked()) {
      certt = _c->toText();
   } else {  // netscape
      cert = _c->toNetscape();
   }

      if ((!_text->isChecked() && cert.size() <= 0) && certt.isEmpty()) {
         KMessageBox::error(this, i18n("Error converting the certificate into the requested format."), i18n("SSL"));
         reject();
         return;
      }

      QFile outFile(_filename->text());

      if (!outFile.open(QIODevice::WriteOnly)) {
         KMessageBox::error(this, i18n("Error opening file for output."), i18n("SSL"));
         reject();
         return;
      }

      if (_text->isChecked())
        outFile.writeBlock(certt.toLocal8Bit(), certt.length());
      else outFile.writeBlock(cert);

      outFile.close();

accept();
}


void KCertExport::slotChoose() {
  //QString newFile = KFileDialog::getSaveFileName("::x509save", i18n("*.pem|Privacy Enhanced Mail Format\n*.der|DER/ASN1 Format"));
  QString newFile = KFileDialog::getSaveFileName(QString::null, "application/x-x509-ca-cert");

   //  Dunno about this one yet
   // \n*.ncert|Netscape certificate files");

  if (!newFile.isEmpty()) _filename->setText(newFile);
}


void KCertExport::slotTextChanged(const QString& x) {
  _export->setEnabled(!x.isEmpty());
}
  

#include "certexport.moc"

