/*  This file is part of the KDE project
    Copyright (C) 2010 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "displaycertdialog_p.h"
#include <kpushbutton.h>
#include <kstandardguiitem.h>


DisplayCertDialog::DisplayCertDialog(QWidget *parent)
 : KDialog(parent),
   m_index(0)
{
    m_ui.setupUi(mainWidget());
    setButtons(KDialog::Ok | KDialog::User1 | KDialog::User2);
    QPair<KGuiItem, KGuiItem> bAndF = KStandardGuiItem::backAndForward();
    setButtonGuiItem(KDialog::User2, bAndF.first);
    setButtonGuiItem(KDialog::User1, bAndF.second);
    connect(button(KDialog::User2), SIGNAL(clicked()), SLOT(previousClicked()));
    connect(button(KDialog::User1), SIGNAL(clicked()), SLOT(nextClicked()));
}

void DisplayCertDialog::setCertificates(const QList<QSslCertificate> &certs)
{
    Q_ASSERT(!certs.isEmpty());
    m_certs = certs;
    m_index = 0;
    showCertificate(0);
    button(KDialog::User2)->setEnabled(certs.size() > 1);
    button(KDialog::User1)->setEnabled(certs.size() > 1);
}

void DisplayCertDialog::showCertificate(int index)
{
    const QSslCertificate &cert = m_certs.at(index);
    m_ui.subjectCertBox->setCertificate(cert, KSslCertificateBox::Subject);
    m_ui.issuerCertBox->setCertificate(cert, KSslCertificateBox::Issuer);
    
    QString vp = i18nc("%1 is the effective date of the certificate, %2 is the expiry date", "%1 to %2",
                       KGlobal::locale()->formatDateTime(cert.effectiveDate()),
                       KGlobal::locale()->formatDateTime(cert.expiryDate()));
    m_ui.validityPeriod->setText(vp);

    m_ui.serialNumber->setText(cert.serialNumber());
    m_ui.md5Digest->setText(cert.digest().toHex());
    m_ui.sha1Digest->setText(cert.digest(QCryptographicHash::Sha1).toHex());
}

//private slot
void DisplayCertDialog::nextClicked()
{
    if (m_index == m_certs.size() - 1) {
        m_index = 0;
    } else {
        m_index++;
    }
    showCertificate(m_index);
}

//private slot
void DisplayCertDialog::previousClicked()
{
    if (m_index == 0) {
        m_index = m_certs.size() - 1;
    } else {
        m_index--;
    }
    showCertificate(m_index);
}
