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
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>

#include <QtDBus/QtDBus>

#include "kssld/kssld_interface.h"


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
       iface("org.kde.kded", "/modules/kssld", QDBusConnection::sessionBus()),
       isCertListLoaded(false)
    {
        // set Qt's set to empty; this is protected by the lock in K_GLOBAL_STATIC.
        QSslSocket::setDefaultCaCertificates(QList<QSslCertificate>());
    }

    void loadDefaultCaCertificates()
    {
        if (isCertListLoaded) {
            // we've had a race between two threads trying to load the certs; don't actually
            // load them twice.
            return;
        }
        defaultCaCertificates.clear();

        if (!KGlobal::hasMainComponent()) {
            Q_ASSERT(false);
            return;                 // we need KGlobal::dirs() available
        }

        // set default CAs from KDE's own bundle
        QStringList bundles = KGlobal::dirs()->findAllResources("data", "kssl/ca-bundle.crt");
        foreach (const QString &bundle, bundles) {
            defaultCaCertificates += QSslCertificate::fromPath(bundle);
        }
        // We don't need to lock the mutex when retrieving the list without loading it first
        // because when the following flag is true the list is complete.
        isCertListLoaded = true;

        kDebug(7029) << "Loading" << defaultCaCertificates.count() << "CA certificates from" << bundles;
    }

    KConfig config;
    org::kde::KSSLDInterface iface;
    QHash<QString, KSslError::Error> stringToSslError;
    QHash<KSslError::Error, QString> sslErrorToString;

    QList<QSslCertificate> defaultCaCertificates;
    QMutex certListMutex;
    bool isCertListLoaded;
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
    // d->iface.setRootCertificates(rootCertificates);
    // TODO remove above stuff from the interface

    QMutexLocker certLocker(&d->certListMutex);
    d->defaultCaCertificates = rootCertificates;
    // don't do delayed initialization: the list *is* now initialized
    d->isCertListLoaded = true;
}


QList<QSslCertificate> KSslCertificateManager::rootCertificates() const
{
    // return d->iface.rootCertificates();
    // TODO remove from interface, see setRootCertificates()

    QMutexLocker certLocker(&d->certListMutex);
    if (!d->isCertListLoaded) {
        d->loadDefaultCaCertificates();
    }
    return d->defaultCaCertificates;
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


#include "kssld/kssld_interface.moc"
