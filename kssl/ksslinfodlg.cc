/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
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

#include "ksslinfodlg.h"

#include <kssl.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qlabel.h>
#include <qfile.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <krun.h>


KSSLInfoDlg::KSSLInfoDlg(bool secureConnection, QWidget *parent, const char *name) 
 : KDialog(parent, name), m_secCon(secureConnection), m_peer(""), m_issuer("") {
  if (KSSL::doesSSLWork()) {
    if (m_secCon) {
      interfaceSSL();
    } else {
      interfaceSSLBlank();
    }
  } else {
    interfaceNoSSL();
  }

  setCaption(i18n("KDE SSL Information"));
}


KSSLInfoDlg::~KSSLInfoDlg() {

}


// If SSL isn't available
void KSSLInfoDlg::interfaceNoSSL() {
QGridLayout *grid = new QGridLayout(this, 4, 3);
QLabel *label = new QLabel(i18n("SSL support is not available in this build of KDE."), this);

grid->addMultiCellWidget(label, 1, 1, 1, 2);

QPushButton *button = new QPushButton(i18n("Cryptography Configuration..."), this);

connect(button, SIGNAL(clicked()), SLOT(launchConfig()));
grid->addWidget(button, 3, 1);

}

// If SSL is available but not in use
void KSSLInfoDlg::interfaceSSLBlank() {
QGridLayout *grid = new QGridLayout(this, 4, 3);
QLabel *label = new QLabel(i18n("Current connection is not secured with SSL."), this);

grid->addMultiCellWidget(label, 1, 1, 1, 2);

QPushButton *button = new QPushButton(i18n("Cryptography Configuration..."), this);

connect(button, SIGNAL(clicked()), SLOT(launchConfig()));
grid->addWidget(button, 3, 1);

}

// If SSL available
void KSSLInfoDlg::interfaceSSL() {
QGridLayout *grid = new QGridLayout(this, 18, 6);

// Add all the static labels first.

QLabel *label = new QLabel(i18n("Current connection is secured with SSL..."), this);
label->setAutoResize(true);

grid->addMultiCellWidget(label, 0, 0, 1, 5);

label = new QLabel(i18n("Peer Certificate"), this);
label->setAutoResize(true);
grid->addMultiCellWidget(label, 1, 1, 0, 1);

label = new QLabel(i18n("Organization:"), this);
label->setAutoResize(true);
grid->addWidget(label, 1, 2);

label = new QLabel(i18n("Org. Unit:"), this);
label->setAutoResize(true);
grid->addWidget(label, 1, 4);

label = new QLabel(i18n("Locality:"), this);
label->setAutoResize(true);
grid->addWidget(label, 2, 2);

label = new QLabel(i18n("Common Name:"), this);
label->setAutoResize(true);
grid->addWidget(label, 2, 4);

label = new QLabel(i18n("State:"), this);
label->setAutoResize(true);
grid->addWidget(label, 3, 2);

label = new QLabel(i18n("Email:"), this);
label->setAutoResize(true);
grid->addWidget(label, 3, 4);

label = new QLabel(i18n("Country:"), this);
label->setAutoResize(true);
grid->addWidget(label, 4, 4);

label = new QLabel(i18n("IP Address:"), this);
label->setAutoResize(true);
grid->addWidget(label, 5, 0);

label = new QLabel(i18n("URL:"), this);
label->setAutoResize(true);
grid->addWidget(label, 5, 2);

label = new QLabel(i18n("Issuer"), this);
label->setAutoResize(true);
grid->addMultiCellWidget(label, 7, 7, 0, 1);

label = new QLabel(i18n("Organization:"), this);
label->setAutoResize(true);
grid->addWidget(label, 7, 2);

label = new QLabel(i18n("Org. Unit:"), this);
label->setAutoResize(true);
grid->addWidget(label, 7, 4);

label = new QLabel(i18n("Locality:"), this);
label->setAutoResize(true);
grid->addWidget(label, 8, 2);

label = new QLabel(i18n("Common Name:"), this);
label->setAutoResize(true);
grid->addWidget(label, 8, 4);

label = new QLabel(i18n("State:"), this);
label->setAutoResize(true);
grid->addWidget(label, 9, 2);

label = new QLabel(i18n("Email:"), this);
label->setAutoResize(true);
grid->addWidget(label, 9, 4);

label = new QLabel(i18n("Country:"), this);
label->setAutoResize(true);
grid->addWidget(label, 10, 2);

label = new QLabel(i18n("Certificate State:"), this);
label->setAutoResize(true);
grid->addMultiCellWidget(label, 12, 12, 0, 1);

label = new QLabel(i18n("Cipher in Use:"), this);
label->setAutoResize(true);
grid->addWidget(label, 14, 0);

label = new QLabel(i18n("Details:"), this);
label->setAutoResize(true);
grid->addWidget(label, 14, 2);

label = new QLabel(i18n("SSL Version:"), this);
label->setAutoResize(true);
grid->addWidget(label, 15, 0);

label = new QLabel(i18n("Cipher Strength:"), this);
label->setAutoResize(true);
grid->addWidget(label, 15, 2);

m_lpO = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lpO, 1, 3);

m_lpOU = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lpOU, 1, 5);

m_lpL = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lpL, 2, 3);

m_lpCN = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lpCN, 2, 5);

m_lpST = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lpST, 3, 3);

m_lpEMAIL = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lpEMAIL, 3, 5);

m_lpC = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lpC, 4, 3);

m_lip = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lip, 5, 1);

m_lurl = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lurl, 5, 3);

m_liO = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_liO, 7, 3);

m_liOU = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_liOU, 7, 5);

m_liL = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_liL, 8, 3);

m_liCN = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_liCN, 8, 5);

m_liST = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_liST, 9, 3);

m_liEMAIL = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_liEMAIL, 9, 5);

m_liC = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_liC, 10, 3);

// FIXME - verified?

m_lcipher = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lcipher, 14, 1);

m_lcipherdesc = new QLabel("", this);
label->setAutoResize(true);
grid->addMultiCellWidget(m_lcipherdesc, 14, 14, 3, 5);

m_lsslversion = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lsslversion, 15, 1);

m_lbits = new QLabel("", this);
label->setAutoResize(true);
grid->addWidget(m_lbits, 15, 3);


QPushButton *button = new QPushButton(i18n("Cryptography Configuration..."), this);

connect(button, SIGNAL(clicked()), SLOT(launchConfig()));
grid->addWidget(button, 17, 5);

}


void KSSLInfoDlg::launchConfig() {
  KRun::runCommand("kcmshell crypto", "kcmshell", "");
}


void KSSLInfoDlg::setup(const QString& peername, const QString& issuer,
                        const QString& ip, const QString& url,
                        const QString& cipher, const QString& cipherdesc,
                        const QString& sslversion, int usedbits, int bits) {
  m_peer.reset(peername);
  m_issuer.reset(issuer);
  m_ip = ip;
  m_url = url;
  m_cipher = cipher;
  m_cipherdesc = cipherdesc;
  m_sslversion = sslversion;
  m_usedbits = usedbits;
  m_bits = bits;

// 
//Country                 C
//State                   ST
//Locality                L
//Organization            O
//Organizational Unit     OU
//Common Name             CN
//EMail                   Email

m_lpC->setText(m_peer.getValue("C"));
m_lpST->setText(m_peer.getValue("ST"));
m_lpL->setText(m_peer.getValue("L"));
m_lpO->setText(m_peer.getValue("O"));
m_lpOU->setText(m_peer.getValue("OU"));
m_lpCN->setText(m_peer.getValue("CN"));
m_lpEMAIL->setText(m_peer.getValue("Email"));
m_liC->setText(m_issuer.getValue("C"));
m_liST->setText(m_issuer.getValue("ST"));
m_liL->setText(m_issuer.getValue("L"));
m_liO->setText(m_issuer.getValue("O"));
m_liOU->setText(m_issuer.getValue("OU"));
m_liCN->setText(m_issuer.getValue("CN"));
m_liEMAIL->setText(m_issuer.getValue("Email"));
m_lip->setText(m_ip);
m_lurl->setText(m_url);
m_lcipherdesc->setText(m_cipherdesc);
m_lcipher->setText(m_cipher);
m_lsslversion->setText(m_sslversion);
QString tmp;
tmp.sprintf("%d bit, %d bits actually used.", m_bits, m_usedbits);
m_lbits->setText(tmp);

}



#include "ksslinfodlg.moc"

