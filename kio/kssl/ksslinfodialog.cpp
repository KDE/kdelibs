/* This file is part of the KDE project
 *
 * Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
 * Copyright (C) 2000 Malte Starostik <malte@kde.org>
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

#include "ksslinfodialog.h"
#include "ui_sslinfo.h"
#include "ksslcertificatebox.h"

#include <kssl.h>

#include <QtGui/QFrame>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/Q_PID>
#include <QtNetwork/QSslCertificate>

#include <kcombobox.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <ksqueezedtextlabel.h>
#include <kstandardguiitem.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

#include "ksslcertificate.h"
#include "ksslcertchain.h"
#include "ksslsigners.h"
#include "ktcpsocket.h"


class KSSLInfoDialog::KSSLInfoDialogPrivate
{
public:
    QList<QSslCertificate> certificateChain;

    bool isMainPartEncrypted;
    bool auxPartsEncrypted;

    Ui::SslInfo ui;
    KSslCertificateBox *subject;
    KSslCertificateBox *issuer;
};



KSSLInfoDialog::KSSLInfoDialog(QWidget *parent)
 : KDialog(parent),
   d(new KSSLInfoDialogPrivate)
{
    setCaption(i18n("KDE SSL Information"));
    setAttribute(Qt::WA_DeleteOnClose);

    d->ui.setupUi(mainWidget());

    d->subject = new KSslCertificateBox(d->ui.certParties);
    d->issuer = new KSslCertificateBox(d->ui.certParties);
    d->ui.certParties->addTab(d->subject, i18n("Subject"));
    d->ui.certParties->addTab(d->issuer, i18n("Issuer"));

    d->isMainPartEncrypted = true;
    d->auxPartsEncrypted = true;
    updateWhichPartsEncrypted();

#if 0
    if (KSSL::doesSSLWork()) {
        if (d->m_secCon) {
            d->pixmap->setPixmap(BarIcon("security-high"));
            d->info->setText(i18n("Current connection is secured with SSL."));
        } else {
            d->pixmap->setPixmap(BarIcon("security-low"));
            d->info->setText(i18n("Current connection is not secured with SSL."));
        }
    } else {
        d->pixmap->setPixmap(BarIcon("security-low"));
        d->info->setText(i18n("SSL support is not available in this build of KDE."));
    }
#endif
}


KSSLInfoDialog::~KSSLInfoDialog()
{
    delete d;
}


//slot
void KSSLInfoDialog::launchConfig()
{
    QProcess::startDetached("kcmshell4", QStringList() << "crypto");
}


void KSSLInfoDialog::setMainPartEncrypted(bool mainEncrypted)
{
    d->isMainPartEncrypted = mainEncrypted;
    updateWhichPartsEncrypted();
}


void KSSLInfoDialog::setAuxiliaryPartsEncrypted(bool auxEncrypted)
{
    d->auxPartsEncrypted = auxEncrypted;
    updateWhichPartsEncrypted();
}


void KSSLInfoDialog::updateWhichPartsEncrypted()
{
    if (d->isMainPartEncrypted) {
        if (d->auxPartsEncrypted) {
            d->ui.encryptionIndicator->setPixmap(BarIcon("security-high"));
            d->ui.explanation->setText(i18n("Current connection is secured with SSL."));
        } else {
            d->ui.encryptionIndicator->setPixmap(BarIcon("security-medium"));
            d->ui.explanation->setText(i18n("The main part of this document is secured "
                                            "with SSL, but some parts are not."));
        }
    } else {
        if (d->auxPartsEncrypted) {
            d->ui.encryptionIndicator->setPixmap(BarIcon("security-medium"));
            d->ui.explanation->setText(i18n("Some of this document is secured with SSL, "
                                            "but the main part is not."));
        } else {
            d->ui.encryptionIndicator->setPixmap(BarIcon("security-low"));
            d->ui.explanation->setText(i18n("Current connection is not secured with SSL."));
        }
    }
}


void KSSLInfoDialog::setup(const KTcpSocket &socket, const QString &ip, const QString &url)
{
    Q_ASSERT(false); //TODO, or maybe not
#if 0
    setup(&ssl.peerInfo().getPeerCertificate(),
          ip, url,
          ssl.connectionInfo().getCipher(),
          ssl.connectionInfo().getCipherDescription(),
          ssl.connectionInfo().getCipherVersion(),
          ssl.connectionInfo().getCipherUsedBits(),
          ssl.connectionInfo().getCipherBits(),
          ssl.peerInfo().getPeerCertificate().validate());
#endif
}

void KSSLInfoDialog::setSslInfo(const QList<QSslCertificate> &certificateChain,
                                const QString &ip, const QString &url,
                                const QString &sslProtocol, const QString &cipher,
                                int usedBits, int bits,
                                const QList<QSslError::SslError> &validationErrors/*###*/) {

    d->certificateChain = certificateChain;
    d->ui.certSelector->clear();
    for (int i = 0; i < certificateChain.size(); i++) {
        const QSslCertificate &cert = certificateChain[i];
        QString name;
        static const QSslCertificate::SubjectInfo si[] = {
            QSslCertificate::CommonName,
            QSslCertificate::Organization,
            QSslCertificate::OrganizationalUnitName
        };
        for (int j = 0; j < 3 && name.isEmpty(); j++)
            name = cert.subjectInfo(si[j]);
        d->ui.certSelector->addItem(name);
    }
    if (certificateChain.size() < 2) {
        d->ui.certSelector->setEnabled(false);
    }
    connect(d->ui.certSelector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(displayFromChain(int)));
    if (d->certificateChain.isEmpty())
        d->certificateChain.append(QSslCertificate());
    displayFromChain(0);

    d->ui.ip->setText(ip);
    d->ui.address->setText(url);
    d->ui.sslVersion->setText(sslProtocol);

    const QStringList cipherInfo = cipher.split('\n', QString::SkipEmptyParts);
    if (cipherInfo.size() >= 4) {
        d->ui.encryption->setText(i18n("%1, using %2 bits of a %3 bit key",
                                         cipherInfo[0], QString::number(usedBits),
                                              QString::number(bits)));
        d->ui.details->setText(QString("Auth = %1, Kx = %2, MAC = %3")
                                      .arg(cipherInfo[1], cipherInfo[2],
                                           cipherInfo[3]));
    } else {
        d->ui.encryption->setText("");
        d->ui.details->setText("");
    }
}


#if 0 //###
void KSSLInfoDialog::displayCert(const QSslCertificate &x) {
    QPalette cspl;

    d->_serialNum->setText(x.getSerialNumber());

    cspl = d->_validFrom->palette();
    if (x->getQDTNotBefore() > QDateTime::currentDateTime().toUTC())
        cspl.setColor(QPalette::Foreground, QColor(196,33,21));
    else cspl.setColor(QPalette::Foreground, QColor(42,153,59));
    d->_validFrom->setPalette(cspl);
    d->_validFrom->setText(x.getNotBefore());

    cspl = d->_validUntil->palette();
    if (x->getQDTNotAfter() < QDateTime::currentDateTime().toUTC())
        cspl.setColor(QPalette::Foreground, QColor(196,33,21));
    else cspl.setColor(QPalette::Foreground, QColor(42,153,59));
    d->_validUntil->setPalette(cspl);
    d->_validUntil->setText(x->getNotAfter());

    cspl = palette();

    KSSLCertificate::KSSLValidation ksv;
    KSSLCertificate::KSSLValidationList ksvl;
    if ((x == d->_cert) && !d->_cert_ksvl.isEmpty()) {
        ksvl = d->_cert_ksvl;
        ksv = ksvl.first();
    } else {
        if (x == d->_cert)
            ksvl = d->_cert->validateVerbose(KSSLCertificate::SSLServer);
        else
            ksvl = d->_cert->validateVerbose(KSSLCertificate::SSLServer, x);

        if (ksvl.isEmpty())
            ksvl << KSSLCertificate::Ok;

        ksv = ksvl.first();

        if (ksv == KSSLCertificate::SelfSigned) {
            if (x->getQDTNotAfter() > QDateTime::currentDateTime().toUTC() &&
                    x->getQDTNotBefore() < QDateTime::currentDateTime().toUTC()) {
                if (KSSLSigners().useForSSL(*x))
                    ksv = KSSLCertificate::Ok;
            } else {
                ksv = KSSLCertificate::Expired;
            }
        }
    }

    if (ksv == KSSLCertificate::Ok) {
        cspl.setColor(QPalette::Foreground, QColor(42,153,59));
    } else if (ksv != KSSLCertificate::Irrelevant) {
        cspl.setColor(QPalette::Foreground, QColor(196,33,21));
    }
    d->_csl->setPalette(cspl);

    QString errorStr;
    for(KSSLCertificate::KSSLValidationList::ConstIterator it = ksvl.begin();
            it != ksvl.end(); ++it) {
        if (!errorStr.isEmpty())
            errorStr.append(QChar('\n'));
        errorStr += KSSLCertificate::verifyText(*it);
    }

    d->_csl->setText(errorStr);
    d->_csl->setMinimumSize(d->_csl->sizeHint());

    d->_subject->setValues(x->getSubject());
    d->_issuer->setValues(x->getIssuer());

    d->_digest->setText(x->getMD5DigestText());
}
#endif


void KSSLInfoDialog::displayFromChain(int i)
{
    const QSslCertificate &cert = d->certificateChain[i];
    d->ui.trusted->setText("TODO"); //TODO :)

    QString vp = "%1 to %2";
    vp = vp.arg(KGlobal::locale()->formatDateTime(cert.effectiveDate()));
    vp = vp.arg(KGlobal::locale()->formatDateTime(cert.expiryDate()));
    d->ui.validityPeriod->setText(vp);

    d->ui.serial->setText(cert.serialNumber());
    d->ui.digest->setText(cert.digest().toHex());

    d->subject->setCertificate(cert, KSslCertificateBox::Subject);
    d->issuer->setCertificate(cert, KSslCertificateBox::Issuer);
}

#if 0
void KSSLCertBox::setValues(const QString &certName, QWidget *mailCatcher) {
    if (certName.isEmpty()) {
        setWidget(new QFrame(this));
        show();
        return;
    }

    KSSLX509Map cert(certName);
    QString tmp;
    viewport()->setBackgroundRole(QPalette::Button);
    QFrame* _frame = new QFrame;
    QGridLayout *grid = new QGridLayout(_frame);
    grid->setMargin(KDialog::marginHint());
    grid->setSpacing(KDialog::spacingHint());
    int row = 0;
    QLabel *label = 0L;
    if (!(tmp = cert.getValue("O")).isEmpty()) {
        label = new QLabel(i18n("Organization:"));
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( label,row, 0);
        grid->addWidget( new QLabel(tmp), row, 1 );
        row++;
    }
    if (!(tmp = cert.getValue("OU")).isEmpty()) {
        label = new QLabel(i18n("Organizational unit:"));
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( label,row, 0);
        grid->addWidget( new QLabel(tmp), row, 1 );
        row++;
    }
    if (!(tmp = cert.getValue("L")).isEmpty()) {
        label = new QLabel(i18n("Locality:"));
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( label,row, 0);
        grid->addWidget( new QLabel(tmp), row, 1 );
        row++;
    }
    if (!(tmp = cert.getValue("ST")).isEmpty()) {
        label = new QLabel(i18nc("Federal State","State:"));
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( label,row, 0);
        grid->addWidget( new QLabel(tmp), row, 1 );
        row++;
    }
    if (!(tmp = cert.getValue("C")).isEmpty()) {
        label = new QLabel(i18n("Country:"));
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( label,row, 0);
        grid->addWidget( new QLabel(tmp), row, 1 );
        row++;
    }
    if (!(tmp = cert.getValue("CN")).isEmpty()) {
        label = new QLabel(i18n("Common name:"));
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        grid->addWidget( label,row, 0);
        grid->addWidget( new QLabel(tmp), row, 1 );
        row++;
    }
    if (!(tmp = cert.getValue("Email")).isEmpty()) {
        label = new QLabel(i18n("Email:"));
        grid->addWidget( label,row, 0);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        if (mailCatcher) {
            KUrlLabel *mail = new KUrlLabel(tmp, tmp);
            grid->addWidget( mail, row, 1 );
            connect(mail, SIGNAL(leftClickedURL(const QString &)), mailCatcher, SLOT(mailClicked(const QString &)));
        } else {
            label = new QLabel(tmp);
            grid->addWidget( label, row, 1 );
        }
    }
    if (label && viewport()) {
        viewport()->setBackgroundRole(label->backgroundRole());
    }
    setWidget(_frame);
    show();
}
#endif


#include "ksslinfodialog.moc"
