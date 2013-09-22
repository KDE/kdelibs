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
#include <QDateTime>
#include <QDialogButtonBox>
#include <QPushButton>
#include <kstandardguiitem.h>


DisplayCertDialog::DisplayCertDialog(QWidget *parent)
 : QDialog(parent),
   m_index(0)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QWidget *mainWidget = new QWidget(this);
    m_ui.setupUi(mainWidget);
    layout->addWidget(mainWidget);

    QPair<KGuiItem, KGuiItem> bAndF = KStandardGuiItem::backAndForward();

    m_previousButton = new QPushButton;
    KGuiItem::assign(m_previousButton, bAndF.first);
    connect(m_previousButton, SIGNAL(clicked()), SLOT(previousClicked()));

    m_nextButton = new QPushButton;
    KGuiItem::assign(m_nextButton, bAndF.second);
    connect(m_nextButton, SIGNAL(clicked()), SLOT(nextClicked()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(m_previousButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_nextButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void DisplayCertDialog::setCertificates(const QList<QSslCertificate> &certs)
{
    Q_ASSERT(!certs.isEmpty());
    m_certs = certs;
    m_index = 0;
    showCertificate(0);
    m_previousButton->setEnabled(certs.size() > 1);
    m_nextButton->setEnabled(certs.size() > 1);
}

void DisplayCertDialog::showCertificate(int index)
{
    const QSslCertificate &cert = m_certs.at(index);
    m_ui.subjectCertBox->setCertificate(cert, KSslCertificateBox::Subject);
    m_ui.issuerCertBox->setCertificate(cert, KSslCertificateBox::Issuer);
    
    QString vp = i18nc("%1 is the effective date of the certificate, %2 is the expiry date", "%1 to %2",
                       cert.effectiveDate().toString(),
                       cert.expiryDate().toString());
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
