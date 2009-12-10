/*
   This file is part of the KDE libraries

   Copyright (c) 2007, 2008 Andreas Hartmetz <ahartmetz@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "kssld.h"

#include "ksslcertificatemanager.h"
#include "kssld_adaptor.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <QtCore/QFile>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <QtCore/QDate>
#include <kpluginfactory.h>
#include <kpluginloader.h>



K_PLUGIN_FACTORY(KSSLDFactory, registerPlugin<KSSLD>();)
K_EXPORT_PLUGIN(KSSLDFactory("kssld"))
//KDE_EXPORT void *__kde_do_unload; // TODO re-add support for this?


class KSSLDPrivate
{
public:
    KSSLDPrivate()
     : config("ksslcertificatemanager", KConfig::SimpleConfig)
    {
        struct strErr {
            const char *str;
            KSslError::Error err;
        };

        //hmmm, looks like these are all of the errors where it is possible to continue.
        const static strErr strError[] = {
            {"NoError", KSslError::NoError},
            {"UnknownError", KSslError::UnknownError},
            {"InvalidCertificateAuthority", KSslError::InvalidCertificateAuthorityCertificate},
            {"InvalidCertificate", KSslError::InvalidCertificate},
            {"CertificateSignatureFailed", KSslError::CertificateSignatureFailed},
            {"SelfSignedCertificate", KSslError::SelfSignedCertificate},
            {"RevokedCertificate", KSslError::RevokedCertificate},
            {"InvalidCertificatePurpose", KSslError::InvalidCertificatePurpose},
            {"RejectedCertificate", KSslError::RejectedCertificate},
            {"UntrustedCertificate", KSslError::UntrustedCertificate},
            {"ExpiredCertificate", KSslError::ExpiredCertificate},
            {"HostNameMismatch", KSslError::HostNameMismatch}
        };

        for (int i = 0; i < int(sizeof(strError)/sizeof(strErr)); i++) {
            QString s = QString::fromLatin1(strError[i].str);
            KSslError::Error e = strError[i].err;
            stringToSslError.insert(s, e);
            sslErrorToString.insert(e, s);
        }        
    }

    KConfig config;
    QHash<QString, KSslError::Error> stringToSslError;
    QHash<KSslError::Error, QString> sslErrorToString;
};



KSSLD::KSSLD(QObject* parent, const QVariantList&)
 : KDEDModule(parent),
   d(new KSSLDPrivate())
{
    new KSSLDAdaptor(this);
}


KSSLD::~KSSLD()
{
    delete d;
}


void KSSLD::setRule(const KSslCertificateRule &rule)
{
    if (rule.hostName().isEmpty()) {
        return;
    }
    KConfigGroup group = d->config.group(rule.certificate().digest().toHex());

    QStringList sl;

    QString dtString("ExpireUTC ");
    dtString.append(rule.expiryDateTime().toString(Qt::ISODate));
    sl.append(dtString);

    if (rule.isRejected()) {
        sl.append("Reject");
    } else {
        foreach (KSslError::Error e, rule.ignoredErrors())
            sl.append(d->sslErrorToString.value(e));
    }

    if (!group.hasKey("CertificatePEM"))
        group.writeEntry("CertificatePEM", rule.certificate().toPem());
#ifdef PARANOIA
    else
        if (group.readEntry("CertificatePEM") != rule.certificate().toPem())
            return;
#endif
    group.writeEntry(rule.hostName(), sl);
    group.sync();
}


void KSSLD::clearRule(const KSslCertificateRule &rule)
{
    clearRule(rule.certificate(), rule.hostName());
}


void KSSLD::clearRule(const QSslCertificate &cert, const QString &hostName)
{
    KConfigGroup group = d->config.group(cert.digest().toHex());
    group.deleteEntry(hostName);
    if (group.keyList().size() < 2) {
        group.deleteGroup();
    }
    group.sync();
}


KSslCertificateRule KSSLD::rule(const QSslCertificate &cert, const QString &hostName) const
{
    KConfigGroup group = d->config.group(cert.digest().toHex());

    //Find a rule for the hostname, either directly or with wildcards
    QString key = hostName;
    bool foundHostName = false;
    if (group.hasKey(key)) {
        foundHostName = true;
    } else {
        QString starDot("*.");
        while (!key.isEmpty()) {
            if (group.hasKey(starDot + key)) {
                foundHostName = true;
                break;
            }
            //now look for rules for higher-level domains
            int dotIndex = key.indexOf('.');
            if (dotIndex < 0)
                break;
            key.remove(0, dotIndex + 1);    //foo.bar -> bar
        }
        key.prepend(starDot);
    }
    if (!foundHostName) {
        //Don't make a rule with the failed wildcard pattern - use the original hostname.
        return KSslCertificateRule(cert, hostName);
    }

    //### This suprising behavior (rule.hostName() being the wildcard) needs documentation!
    KSslCertificateRule ret(cert, key);

#ifdef PARANOIA
    if (group.readEntry("CertificatePEM") != cert.toPem())
        return ret;
#endif
    //parse entry of the format "ExpireUTC <date>, Reject" or
    //"ExpireUTC <date>, HostNameMismatch, ExpiredCertificate, ..."
    QStringList sl = group.readEntry(key, QStringList());

    QString dtString = sl.takeFirst();
    if (!dtString.startsWith(QLatin1String("ExpireUTC ")))
        return ret;
    dtString.remove(0, 10/* length of "ExpireUTC " */);

    QDateTime expiryDt = QDateTime::fromString(dtString, Qt::ISODate);
    if (!expiryDt.isValid() || expiryDt < QDateTime::currentDateTime()) {
        //the entry is malformed or expired so we remove it
        group.deleteEntry(key);
        //the group is useless once only the CertificatePEM entry left
        if (group.keyList().size() < 2)
            group.deleteGroup();
        group.sync();
        return ret;
    }

    QList<KSslError::Error> ignoredErrors;
    bool isRejected = false;
    foreach (const QString &s, sl) {
        if (s == "Reject") {
            isRejected = true;
            ignoredErrors.clear();
            break;
        }
        if (!d->stringToSslError.contains(s))
            continue;
        ignoredErrors.append(d->stringToSslError.value(s));
    }

    //Everything is checked and we can make ret valid
    ret.setExpiryDateTime(expiryDt);
    ret.setRejected(isRejected);
    ret.setIgnoredErrors(ignoredErrors);
    return ret;
}


void KSSLD::setRootCertificates(const QList<QSslCertificate> &rootCertificates)
{
    Q_UNUSED(rootCertificates)
    //TODO
}


QList<QSslCertificate> KSSLD::rootCertificates() const
{
    //TODO
    return QList<QSslCertificate>();
}


#include "kssld.moc"
#include "kssld_adaptor.moc"
