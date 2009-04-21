/* This file is part of the KDE project
 *
 * Copyright (C) 2007, 2008 Andreas Hartmetz <ahartmetz@gmail.com>
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


#include "ksslcertificatemanager.h"
#include "ktcpsocket.h"
#include "ktcpsocket_p.h"
#include <ktoolinvocation.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksslinfodialog.h>

#include <QtDBus/QtDBus>

#include "kssld_interface.h"

/*
  Config file format:
[<MD5-Digest>]
<Host> = <Date> <List of ignored errors>
#for example
#mail.kdab.net =  ExpireUTC 2008-08-20T18:22:14, SelfSigned, Expired
#very.old.com =  ExpireUTC 2008-08-20T18:22:14, TooWeakEncryption <- not actually planned to implement
#clueless.admin.com =  ExpireUTC 2008-08-20T18:22:14, HostNameMismatch
#
#Wildcard syntax
#* = ExpireUTC 2008-08-20T18:22:14, SelfSigned
#*.kdab.net = ExpireUTC 2008-08-20T18:22:14, SelfSigned
#mail.kdab.net = ExpireUTC 2008-08-20T18:22:14, All <- not implemented
#* = ExpireUTC 9999-12-31T23:59:59, Reject  #we know that something is wrong with that certificate
CertificatePEM = <PEM-encoded certificate> #host entries are all lowercase, thus no clashes

 */

class KSslCertificateRulePrivate
{
public:
    QSslCertificate certificate;
    QString hostName;
    bool isRejected;
    QDateTime expiryDateTime;
    QList<KSslError::Error> ignoredErrors;
};


KSslCertificateRule::KSslCertificateRule(const QSslCertificate &cert, const QString &hostName)
 : d(new KSslCertificateRulePrivate())
{
    d->certificate = cert;
    d->hostName = hostName;
    d->isRejected = false;
}


KSslCertificateRule::KSslCertificateRule(const KSslCertificateRule &other)
 : d(new KSslCertificateRulePrivate())
{
    *d = *other.d;
}


KSslCertificateRule::~KSslCertificateRule()
{
    delete d;
}


KSslCertificateRule &KSslCertificateRule::operator=(const KSslCertificateRule &other)
{
    *d = *other.d;
    return *this;
}


QSslCertificate KSslCertificateRule::certificate() const
{
    return d->certificate;
}


QString KSslCertificateRule::hostName() const
{
    return d->hostName;
}


void KSslCertificateRule::setExpiryDateTime(const QDateTime &dateTime)
{
    d->expiryDateTime = dateTime;
}


QDateTime KSslCertificateRule::expiryDateTime() const
{
    return d->expiryDateTime;
}


void KSslCertificateRule::setRejected(bool rejected)
{
    d->isRejected = rejected;
}


bool KSslCertificateRule::isRejected() const
{
    return d->isRejected;
}


bool KSslCertificateRule::isErrorIgnored(KSslError::Error error) const
{
    foreach (KSslError::Error ignoredError, d->ignoredErrors)
        if (error == ignoredError)
            return true;

    return false;
}


void KSslCertificateRule::setIgnoredErrors(const QList<KSslError::Error> &errors)
{
    d->ignoredErrors.clear();
    //### Quadratic runtime, woohoo! Use a QSet if that should ever be an issue.
    foreach(KSslError::Error e, errors)
        if (!isErrorIgnored(e))
            d->ignoredErrors.append(e);
}


void KSslCertificateRule::setIgnoredErrors(const QList<KSslError> &errors)
{
    QList<KSslError::Error> el;
    foreach(const KSslError &e, errors)
        el.append(e.error());
    setIgnoredErrors(el);
}


QList<KSslError::Error> KSslCertificateRule::ignoredErrors() const
{
    return d->ignoredErrors;
}


QList<KSslError::Error> KSslCertificateRule::filterErrors(const QList<KSslError::Error> &errors) const
{
    QList<KSslError::Error> ret;
    foreach (KSslError::Error error, errors) {
        if (!isErrorIgnored(error))
            ret.append(error);
    }
    return ret;
}


QList<KSslError> KSslCertificateRule::filterErrors(const QList<KSslError> &errors) const
{
    QList<KSslError> ret;
    foreach (const KSslError &error, errors) {
        if (!isErrorIgnored(error.error()))
            ret.append(error);
    }
    return ret;
}


////////////////////////////////////////////////////////////////////

class KSslCertificateManagerContainer
{
public:
    KSslCertificateManager sslCertificateManager;
};

K_GLOBAL_STATIC(KSslCertificateManagerContainer, g_instance)


class KSslCertificateManagerPrivate
{
public:
    KSslCertificateManagerPrivate()
     : config("ksslcertificatemanager", KConfig::SimpleConfig), //TODO really good conf filename
       iface("org.kde.kded", "/modules/kssld", QDBusConnection::sessionBus())
    {
    }

    KConfig config;
    org::kde::KSSLDInterface iface;
    QHash<QString, KSslError::Error> stringToSslError;
    QHash<KSslError::Error, QString> sslErrorToString;
};


KSslCertificateManager::KSslCertificateManager()
 : d(new KSslCertificateManagerPrivate())
{
    // Make sure kded is running
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded")) {
        KToolInvocation::klauncher(); // this calls startKdeinit
    }
}


KSslCertificateManager::~KSslCertificateManager()
{
    delete d;
}


//static
KSslCertificateManager *KSslCertificateManager::self()
{
    return &g_instance->sslCertificateManager;
}


void KSslCertificateManager::setRule(const KSslCertificateRule &rule)
{
    d->iface.setRule(rule);
}


void KSslCertificateManager::clearRule(const KSslCertificateRule &rule)
{
    d->iface.clearRule(rule);
}


void KSslCertificateManager::clearRule(const QSslCertificate &cert, const QString &hostName)
{
    d->iface.clearRule(cert, hostName);
}


KSslCertificateRule KSslCertificateManager::rule(const QSslCertificate &cert, const QString &hostName) const
{
    return d->iface.rule(cert, hostName);
}


void KSslCertificateManager::setRootCertificates(const QList<QSslCertificate> &rootCertificates)
{
    d->iface.setRootCertificates(rootCertificates);
}


QList<QSslCertificate> KSslCertificateManager::rootCertificates() const
{
    return d->iface.rootCertificates();
}

//static
QList<KSslError> KSslCertificateManager::nonIgnorableErrors(const QList<KSslError> &/*e*/)
{
    QList<KSslError> ret;
    // ### add filtering here...
    return ret;
}

//static
QList<KSslError::Error> KSslCertificateManager::nonIgnorableErrors(const QList<KSslError::Error> &/*e*/)
{
    QList<KSslError::Error> ret;
    // ### add filtering here...
    return ret;
}


//static
bool KSslCertificateManager::askIgnoreSslErrors(const KTcpSocket *socket, RulesStorage storedRules)
{
    KSslErrorUiData uiData(socket);
    return askIgnoreSslErrors(uiData, storedRules);
}

//static
bool KSslCertificateManager::askIgnoreSslErrors(const KSslErrorUiData &uiData, RulesStorage storedRules)
{
    KSslErrorUiData::Private *const ud = uiData.d;
    if (ud->sslErrors.isEmpty()) {
        return true;
    }

    QList<KSslError> fatalErrors = KSslCertificateManager::nonIgnorableErrors(ud->sslErrors);
    if (!fatalErrors.isEmpty()) {
        //TODO message "sorry, fatal error, you can't override it"
        return false;
    }

    KSslCertificateManager *const cm = KSslCertificateManager::self();
    KSslCertificateRule rule(ud->certificateChain.first(), ud->host);
    if (storedRules & RecallRules) {
        rule = cm->rule(ud->certificateChain.first(), ud->host);
        // remove previously seen and acknowledged errors
        QList<KSslError> remainingErrors = rule.filterErrors(ud->sslErrors);
        if (remainingErrors.isEmpty()) {
            kDebug(7029) << "Error list empty after removing errors to be ignored. Continuing.";
            return true;
        }
    }

    //### We don't ask to permanently reject the certificate

    QString message = i18n("The server failed the authenticity check (%1).\n\n", ud->host);
    foreach (const KSslError &err, ud->sslErrors) {
        message.append(err.errorString());
        message.append('\n');
    }
    message = message.trimmed();

    int msgResult;
    do {
        msgResult = KMessageBox::warningYesNoCancel(0, message, i18n("Server Authentication"),
                                                    KGuiItem(i18n("&Details")),
                                                    KGuiItem(i18n("Co&ntinue")));
        if (msgResult == KMessageBox::Yes) {
            //Details was chosen - show the certificate and error details


            QList<QList<KSslError::Error> > meh;    // parallel list to cert list :/

            foreach (const QSslCertificate &cert, ud->certificateChain) {
                QList<KSslError::Error> errors;
                foreach(const KSslError &error, ud->sslErrors) {
                    if (error.certificate() == cert) {
                        // we keep only the error code enum here
                        errors.append(error.error());
                    }
                }
                meh.append(errors);
            }


            KSslInfoDialog *dialog = new KSslInfoDialog();
            dialog->setSslInfo(ud->certificateChain, ud->ip, ud->host, ud->sslProtocol,
                               ud->cipher, ud->usedBits, ud->bits, meh);
            dialog->exec();
        } else if (msgResult == KMessageBox::Cancel) {
            return false;
        }
        //fall through on KMessageBox::No
    } while (msgResult == KMessageBox::Yes);


    if (storedRules & StoreRules) {
        //Save the user's choice to ignore the SSL errors.

        msgResult = KMessageBox::warningYesNo(0,
                                i18n("Would you like to accept this "
                                    "certificate forever without "
                                    "being prompted?"),
                                i18n("Server Authentication"),
                                KGuiItem(i18n("&Forever")),
                                KGuiItem(i18n("&Current Sessions Only")));
        QDateTime ruleExpiry = QDateTime::currentDateTime();
        if (msgResult == KMessageBox::Yes) {
            //accept forever ("for a very long time")
            ruleExpiry = ruleExpiry.addYears(1000);
        } else {
            //accept "for a short time", half an hour.
            ruleExpiry = ruleExpiry.addSecs(30*60);
        }

        //TODO special cases for wildcard domain name in the certificate!
        //rule = KSslCertificateRule(d->socket.peerCertificateChain().first(), whatever);

        rule.setExpiryDateTime(ruleExpiry);
        rule.setIgnoredErrors(ud->sslErrors);
        cm->setRule(rule);
    }

    return true;
}


#include "kssld_interface.moc"
