/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
 * Copyright (C) 2000 Malte Starostik <malte.starostik@t-online.de>
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
#include <qscrollview.h>
#include <qfile.h>

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kprocess.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kurllabel.h>
//#include <kstddirs.h>
//#include <krun.h>

class KSSLInfoDlg::KSSLInfoDlgPrivate {
private:
    friend class KSSLInfoDlg;
    bool m_secCon;
    QGridLayout *m_layout;
};

KSSLInfoDlg::KSSLInfoDlg(bool secureConnection, QWidget *parent, const char *name, bool modal)
 : KDialog(parent, name, modal, Qt::WDestructiveClose), d(new KSSLInfoDlgPrivate) {
    QVBoxLayout *topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    d->m_secCon = secureConnection;
    d->m_layout = new QGridLayout(topLayout, 3, 3, KDialog::spacingHint());
    d->m_layout->setColStretch(1, 1);
    d->m_layout->setColStretch(2, 1);
    QLabel *pixmap = new QLabel(this);
    d->m_layout->addWidget(pixmap, 0, 0);
    QLabel *info = new QLabel(this);
    d->m_layout->addWidget(info, 0, 1);
    if (KSSL::doesSSLWork()) {
        if (d->m_secCon) {
            pixmap->setPixmap(BarIcon("lock"));
            info->setText(i18n("Current connection is secured with SSL."));
        } else {
            pixmap->setPixmap(BarIcon("unlock"));
            info->setText(i18n("Current connection is not secured with SSL."));
        }
    } else {
        pixmap->setPixmap(BarIcon("unlock"));
        info->setText(i18n("SSL support is not available in this build of KDE."));
    }
    d->m_layout->addRowSpacing( 0, 50 ); // give minimum height to look better

    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout, KDialog::spacingHint());
    buttonLayout->addStretch( 1 );

    QPushButton *button = new QPushButton(i18n("Close"), this);
    connect(button, SIGNAL(clicked()), SLOT(close()));
    buttonLayout->addWidget( button );

    if (KSSL::doesSSLWork()) {
      button = new QPushButton(i18n("Cryptography Configuration..."), this);
      connect(button, SIGNAL(clicked()), SLOT(launchConfig()));
      buttonLayout->addWidget( button );
    }

    setCaption(i18n("KDE SSL Information"));
}


KSSLInfoDlg::~KSSLInfoDlg() {
    delete d;
}

void KSSLInfoDlg::launchConfig() {
KShellProcess p;
  p << "kcmshell" << "crypto";
  p.start(KProcess::DontCare);
}

void KSSLInfoDlg::setup( KSSL & ssl, const QString & ip, const QString & url )
{
    setup(
        ssl.peerInfo().getPeerCertificate().getSubject(),
        ssl.peerInfo().getPeerCertificate().getIssuer(),
        ip,
        url,
        ssl.connectionInfo().getCipher(),
        ssl.connectionInfo().getCipherDescription(),
        ssl.connectionInfo().getCipherVersion(),
        ssl.connectionInfo().getCipherUsedBits(),
        ssl.connectionInfo().getCipherBits(),
        ssl.peerInfo().getPeerCertificate().validate(),
        ssl.peerInfo().getPeerCertificate().getNotBefore(),
        ssl.peerInfo().getPeerCertificate().getNotAfter()
        );
}

void KSSLInfoDlg::setup(const QString& peername, const QString& issuer,
                        const QString& ip, const QString& url,
                        const QString& cipher, const QString& cipherdesc,
                        const QString& sslversion, int usedbits, int bits,
                        KSSLCertificate::KSSLValidation certState,
                        const QString& goodFrom, const QString& goodUntil) {
// Needed to put the GUI stuff here to get the layouting right
    QGridLayout *layout = new QGridLayout(2, 2, KDialog::spacingHint());
    layout->addWidget(new QLabel(i18n("Peer Certificate:"), this), 0, 0);
    layout->addWidget(buildCertInfo(peername), 1, 0);
    layout->addWidget(new QLabel(i18n("Issuer:"), this), 0, 1);
    layout->addWidget(buildCertInfo(issuer), 1, 1);
    d->m_layout->addMultiCell(layout, 1, 1, 0, 2);

    layout = new QGridLayout(7, 2, KDialog::spacingHint());
    layout->setColStretch(1, 1);
    layout->addWidget(new QLabel(i18n("IP Address:"), this), 0, 0);
    layout->addWidget(new QLabel(ip, this), 0, 1);
    layout->addWidget(new QLabel(i18n("URL:"), this), 1, 0);
    // truncate the label if it will be too long
    QString urllabel;
    if (url.length() > 80) {
      urllabel = url.left(80) + " ...";
    } else urllabel = url;
    KURLLabel *urlLabel = new KURLLabel(url, urllabel, this);
    layout->addWidget(urlLabel, 1, 1);
    connect(urlLabel, SIGNAL(leftClickedURL(const QString &)), SLOT(urlClicked(const QString &)));
    layout->addWidget(new QLabel(i18n("Certificate State:"), this), 2, 0);

    QLabel *csl;
    QPalette cspl;
    switch(certState) {
    case KSSLCertificate::Ok:
      layout->addWidget(csl = new QLabel(i18n("Certificate is valid from %1 until %2.").arg(goodFrom).arg(goodUntil), this), 2, 1);
      cspl = csl->palette();
      cspl.setColor(QColorGroup::Foreground, QColor(42,153,59));
      csl->setPalette(cspl);
    break;
    default:
      layout->addWidget(csl = new QLabel(KSSLCertificate::verifyText(certState), this), 2, 1);
      cspl = csl->palette();
      cspl.setColor(QColorGroup::Foreground, QColor(196,33,21));
      csl->setPalette(cspl);
    break;
    }
    update();

    layout->addWidget(new QLabel(i18n("Cipher in Use:"), this), 3, 0);
    layout->addWidget(new QLabel(cipher, this), 3, 1);
    layout->addWidget(new QLabel(i18n("Details:"), this), 4, 0);
    layout->addWidget(new QLabel(cipherdesc.simplifyWhiteSpace(), this), 4, 1);
    layout->addWidget(new QLabel(i18n("SSL Version:"), this), 5, 0);
    layout->addWidget(new QLabel(sslversion, this), 5, 1);
    layout->addWidget(new QLabel(i18n("Cipher Strength:"), this), 6, 0);
    layout->addWidget(new QLabel(i18n("%1 bits used of a %2 bit cipher").arg(usedbits).arg(bits), this), 6, 1);
    d->m_layout->addMultiCell(layout, 2, 2, 0, 2);
}

KSSLCertBox *KSSLInfoDlg::certInfoWidget(QWidget *parent, const QString &certName, QWidget *mailCatcher) {
    KSSLCertBox *result = new KSSLCertBox(parent);
    result->setValues(certName, mailCatcher);
    return result;
}


KSSLCertBox::KSSLCertBox(QWidget *parent, const char *name, WFlags f) 
:            QScrollView(parent, name, f)
{
    _frame = NULL;
}


void KSSLCertBox::setValues(QString certName, QWidget *mailCatcher) {
    KSSLX509Map cert(certName);
    QString tmp;

    if (_frame) {
       removeChild(_frame);
       delete _frame;
    }

    viewport()->setBackgroundMode(QWidget::PaletteButton);
    _frame = new QFrame(this);
    QGridLayout *grid = new QGridLayout(_frame, 1, 2, KDialog::marginHint(), KDialog::spacingHint());
    grid->setAutoAdd(true);
    QLabel *label;
    if (!(tmp = cert.getValue("O")).isEmpty()) {
        label = new QLabel(i18n("Organization:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        new QLabel(tmp, _frame);
    }
    if (!(tmp = cert.getValue("OU")).isEmpty()) {
        label = new QLabel(i18n("Organizational Unit:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        new QLabel(tmp, _frame);
    }
    if (!(tmp = cert.getValue("L")).isEmpty()) {
        label = new QLabel(i18n("Locality:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        new QLabel(tmp, _frame);
    }
    if (!(tmp = cert.getValue("ST")).isEmpty()) {
        label = new QLabel(i18n("State:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        new QLabel(tmp, _frame);
    }
    if (!(tmp = cert.getValue("C")).isEmpty()) {
        label = new QLabel(i18n("Country:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        new QLabel(tmp, _frame);
    }
    if (!(tmp = cert.getValue("CN")).isEmpty()) {
        label = new QLabel(i18n("Common Name:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        new QLabel(tmp, _frame);
    }
    if (!(tmp = cert.getValue("Email")).isEmpty()) {
        label = new QLabel(i18n("EMail:"), _frame);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        if (mailCatcher) {
           KURLLabel *mail = new KURLLabel(tmp, tmp, _frame);
           connect(mail, SIGNAL(leftClickedURL(const QString &)), mailCatcher, SLOT(mailClicked(const QString &)));
        } else {
           new QLabel(tmp, _frame);
        }
    }
    addChild(_frame);
    _frame->show();
    show();
}


QScrollView *KSSLInfoDlg::buildCertInfo(const QString &certName) {
return KSSLInfoDlg::certInfoWidget(this, certName, this);
}

void KSSLInfoDlg::urlClicked(const QString &url) {
    kapp->invokeBrowser(url);
}

void KSSLInfoDlg::mailClicked(const QString &url) {
    kapp->invokeMailer(url, QString::null);
}

#include "ksslinfodlg.moc"

