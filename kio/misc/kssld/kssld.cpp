/*
   This file is part of the KDE libraries

   Copyright (c) 2001-2005 George Staikos <staikos@kde.org>

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

#include <config.h>

#include <QtCore/QTimer>

#include <ksharedconfig.h>
#include <kconfig.h>
#include <ksslcertchain.h>
#include <ksslcertificate.h>
#include <ksslcertificatehome.h>
#include <ksslpkcs12.h>
#include <ksslx509map.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <QtCore/QFile>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <QtCore/QDate>

#include <kcodecs.h>
#include <kopenssl.h>

// See design notes at end

extern "C" {
	KDE_EXPORT KDEDModule *create_kssld() {
		return new KSSLD();
	}

	KDE_EXPORT void *__kde_do_unload;
}

template <> inline
void KConfigGroup::writeEntry( const char *pKey,
                               const KSSLCertificateCache::KSSLCertificatePolicy& aValue,
                               KConfigGroup::WriteConfigFlags flags)
{
    writeEntry(pKey, int(aValue), flags);
}

static void updatePoliciesConfig(KConfig *cfg) {
	QStringList groups = cfg->groupList();

	for (QStringList::Iterator i = groups.begin(); i != groups.end(); ++i) {
		if ((*i).isEmpty() || *i == "General") {
			continue;
		}

                KConfigGroup cg(cfg, *i);

		// remove it if it has expired
		if (!cg.readEntry("Permanent", false) &&
					 cg.readEntry("Expires", QDateTime()) < QDateTime::currentDateTime()) {
			cfg->deleteGroup(*i);
			continue;
		}

		QString encodedCertStr = cg.readEntry("Certificate");
		QByteArray encodedCert = encodedCertStr.toLocal8Bit();
			KSSLCertificate *newCert = KSSLCertificate::fromString(encodedCert);
		if (!newCert) {
			cfg->deleteGroup(*i);
			continue;
		}

		KSSLCertificateCache::KSSLCertificatePolicy policy = (KSSLCertificateCache::KSSLCertificatePolicy) cg.readEntry("Policy", 0);
		bool permanent = cg.readEntry("Permanent", false);
		QDateTime expires = cg.readEntry("Expires", QDateTime());
		QStringList hosts = cg.readEntry("Hosts", QStringList());
		QStringList chain = cg.readEntry("Chain", QStringList());
		cfg->deleteGroup(*i);

		cg.changeGroup(newCert->getMD5Digest());
		cg.writeEntry("Certificate", encodedCertStr);
		cg.writeEntry("Policy", policy);
		cg.writeEntry("Permanent", permanent);
		cg.writeEntry("Expires", expires);
		cg.writeEntry("Hosts", hosts);
		cg.writeEntry("Chain", chain);
		delete newCert;
	}

	KConfigGroup cg(cfg, "General");
	cg.writeEntry("policies version", 2);

	cg.sync();
}


KSSLD::KSSLD() : KDEDModule()
{
// ----------------------- FOR THE CACHE ------------------------------------
    cfg = new KConfig("ksslpolicies", KConfig::SimpleConfig);
    KConfigGroup cg(cfg, "General");
    if (2 != cg.readEntry("policies version", 0)) {
        ::updatePoliciesConfig(cfg);
    }
    KGlobal::dirs()->addResourceType("kssl", "data", "kssl");
    caVerifyUpdate();
    cacheLoadDefaultPolicies();
    kossl = KOSSL::self();

// ----------------------- FOR THE HOME -------------------------------------
}


KSSLD::~KSSLD()
{
// ----------------------- FOR THE CACHE ------------------------------------
	cacheClearList();
	delete cfg;

// ----------------------- FOR THE HOME -------------------------------------
}




// A node in the cache
class KSSLCNode {
	public:
		KSSLCertificate *cert;
		KSSLCertificateCache::KSSLCertificatePolicy policy;
		bool permanent;
		QDateTime expires;
		QStringList hosts;
		KSSLCNode() { cert = 0L;
				policy = KSSLCertificateCache::Unknown;
				permanent = true;
			}
		~KSSLCNode() { delete cert; }
};



void KSSLD::cacheSaveToDisk() {

    KConfigGroup cg(cfg, "General");
	cg.writeEntry("policies version", 2);

	Q_FOREACH( const KSSLCNode *node , certList ) {
		if (node->permanent ||
			node->expires > QDateTime::currentDateTime()) {
			// First convert to a binary format and then write the
			// kconfig entry write the (CN, policy, cert) to
			// KConfig
			cg.changeGroup(node->cert->getMD5Digest());
			cg.writeEntry("Certificate", node->cert->toString());
			cg.writeEntry("Policy", (int)node->policy);   // cast to avoid ICE in msvc
			cg.writeEntry("Expires", node->expires);
			cg.writeEntry("Permanent", node->permanent);
			cg.writeEntry("Hosts", node->hosts);

			// Also write the chain
			QStringList qsl;
			QList<KSSLCertificate *> cl =
						node->cert->chain().getChain();
			foreach (KSSLCertificate *c, cl) {
				//kDebug() << "Certificate in chain: "
				//		<<	c->toString() << endl;
				qsl << c->toString();
			}

			qDeleteAll(cl);
			cg.writeEntry("Chain", qsl);
		}
	}

	cfg->sync();

	// insure proper permissions -- contains sensitive data
	QString cfgName(KGlobal::dirs()->findResource("config", "ksslpolicies"));

	if (!cfgName.isEmpty()) {
		::chmod(QFile::encodeName(cfgName), 0600);
	}
}


void KSSLD::cacheReload() {
	cacheClearList();
	delete cfg;
	cfg = new KConfig("ksslpolicies", KConfig::SimpleConfig);
	cacheLoadDefaultPolicies();
}


void KSSLD::cacheClearList() {
KSSLCNode *node;

	Q_FOREACH( node , certList ) {
		certList.removeAll(node);
		delete node;
	}

	skEmail.clear();
	skMD5Digest.clear();
}


void KSSLD::cacheLoadDefaultPolicies() {
	const QStringList groups = cfg->groupList();

	for (QStringList::const_iterator i = groups.begin();
				i != groups.end();
				++i) {
		if ((*i).isEmpty() || *i == "General") {
			continue;
		}

                KConfigGroup cg(cfg, *i);

		// remove it if it has expired
		if (!cg.readEntry("Permanent", false) &&
			cg.readEntry("Expires", QDateTime()) <
				QDateTime::currentDateTime()) {
			cfg->deleteGroup(*i);
			continue;
		}

		QByteArray encodedCert;
		KSSLCertificate *newCert;

		encodedCert = cg.readEntry("Certificate").toLocal8Bit();
			newCert = KSSLCertificate::fromString(encodedCert);

		if (!newCert) {
			   continue;
		}

		KSSLCNode *n = new KSSLCNode;
		n->cert = newCert;
		n->policy = (KSSLCertificateCache::KSSLCertificatePolicy) cg.readEntry("Policy", 0);
		n->permanent = cg.readEntry("Permanent", false);
		n->expires = cg.readEntry("Expires", QDateTime());
		n->hosts = cg.readEntry("Hosts", QStringList());
		newCert->chain().setCertChain(cg.readEntry("Chain", QStringList()));
		certList.append(n);
		searchAddCert(newCert);
	}
}

void KSSLD::cacheAddCertificate(QByteArray certData, int policy, bool permanent) {
	QDataStream stream(&certData, QIODevice::ReadOnly);
	KSSLCertificate cert;
	stream >> cert;
	cacheAddCertificate(cert, KSSLCertificateCache::KSSLCertificatePolicy(policy), permanent);
}

void KSSLD::cacheAddCertificate(KSSLCertificate cert,
			KSSLCertificateCache::KSSLCertificatePolicy policy,
			bool permanent) {
KSSLCNode *node;
	Q_FOREACH( node , certList ) {
		if (cert == *(node->cert)) {
			node->policy = policy;
			node->permanent = permanent;

			if (!permanent) {
				node->expires = QDateTime::currentDateTime();
				// FIXME: make this configurable
				node->expires = node->expires.addSecs(3600);
			}

			cacheSaveToDisk();
			return;
		}
	}

	KSSLCNode *n = new KSSLCNode;
	n->cert = cert.replicate();
	n->policy = policy;
	n->permanent = permanent;
	// remove the old one
	cacheRemoveByCertificate(*(n->cert));
	certList.prepend(n);

	if (!permanent) {
		n->expires = QDateTime::currentDateTime();
		n->expires = n->expires.addSecs(3600);
	}

	searchAddCert(n->cert);
	cacheSaveToDisk();
}


int KSSLD::cacheGetPolicyByCN(QString cn) {
KSSLCNode *node;
	Q_FOREACH( node , certList ) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			if (!node->permanent &&
				node->expires < QDateTime::currentDateTime()) {
				certList.removeAll(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				continue;
			}

			certList.removeAll(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return node->policy;
		}
	}

	cacheSaveToDisk();

return KSSLCertificateCache::Unknown;
}

int KSSLD::cacheGetPolicyByCertificate(QByteArray certData) {
	QDataStream stream(&certData, QIODevice::ReadOnly);
	KSSLCertificate cert;
	stream >> cert;
	return cacheGetPolicyByCertificate(cert);
}

KSSLCertificateCache::KSSLCertificatePolicy KSSLD::cacheGetPolicyByCertificate(KSSLCertificate cert) {
KSSLCNode *node;
	Q_FOREACH( node , certList ) {
		if (cert == *(node->cert)) {
			if (!node->permanent &&
				node->expires < QDateTime::currentDateTime()) {
				certList.removeAll(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				cacheSaveToDisk();
				return KSSLCertificateCache::Unknown;
			}

			certList.removeAll(node);
			certList.prepend(node);
			return node->policy;
		}
	}

return KSSLCertificateCache::Unknown;
}


bool KSSLD::cacheSeenCN(QString cn) {
KSSLCNode *node;
	Q_FOREACH( node , certList ) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			if (!node->permanent &&
				node->expires < QDateTime::currentDateTime()) {
				certList.removeAll(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				cacheSaveToDisk();
				continue;
			}

			certList.removeAll(node);
			certList.prepend(node);
			return true;
		}
	}

return false;
}

bool KSSLD::cacheSeenCertificate(QByteArray certData) {
	QDataStream stream(&certData, QIODevice::ReadOnly);
	KSSLCertificate cert;
	stream >> cert;
	return cacheSeenCertificate(cert);
}

bool KSSLD::cacheSeenCertificate(KSSLCertificate cert) {
KSSLCNode *node;
	Q_FOREACH( node , certList ) {
		if (cert == *(node->cert)) {
			if (!node->permanent &&
				node->expires < QDateTime::currentDateTime()) {
				certList.removeAll(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				cacheSaveToDisk();
				return false;
			}

			certList.removeAll(node);
			certList.prepend(node);
			return true;
		}
	}

return false;
}

bool KSSLD::cacheIsPermanent(QByteArray certData) {
	QDataStream stream(&certData, QIODevice::ReadOnly);
	KSSLCertificate cert;
	stream >> cert;
	return cacheIsPermanent(cert);
}

bool KSSLD::cacheIsPermanent(KSSLCertificate cert) {
KSSLCNode *node;
	Q_FOREACH( node , certList ) {
		if (cert == *(node->cert)) {
			if (!node->permanent && node->expires <
					QDateTime::currentDateTime()) {
				certList.removeAll(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				cacheSaveToDisk();
				return false;
			}

			certList.removeAll(node);
			certList.prepend(node);
			return node->permanent;
		}
	}

return false;
}


bool KSSLD::cacheRemoveBySubject(QString subject) {
KSSLCNode *node;
bool gotOne = false;
	Q_FOREACH( node , certList ) {
		if (node->cert->getSubject() == subject) {
			certList.removeAll(node);
			cfg->deleteGroup(node->cert->getMD5Digest());
			searchRemoveCert(node->cert);
			delete node;
			gotOne = true;
		}
	}

	cacheSaveToDisk();

return gotOne;
}


bool KSSLD::cacheRemoveByCN(QString cn) {
KSSLCNode *node;
bool gotOne = false;
	Q_FOREACH( node , certList ) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			certList.removeAll(node);
			cfg->deleteGroup(node->cert->getMD5Digest());
			searchRemoveCert(node->cert);
			delete node;
			gotOne = true;
		}
	}

	cacheSaveToDisk();

return gotOne;
}

bool KSSLD::cacheRemoveByCertificate(QByteArray certData) {
	QDataStream stream(&certData, QIODevice::ReadOnly);
	KSSLCertificate cert;
	stream >> cert;
	return cacheRemoveByCertificate(cert);
}

bool KSSLD::cacheRemoveByCertificate(KSSLCertificate cert) {
KSSLCNode *node;

	Q_FOREACH( node , certList ) {
		if (cert == *(node->cert)) {
			certList.removeAll(node);
			cfg->deleteGroup(node->cert->getMD5Digest());
			searchRemoveCert(node->cert);
			delete node;
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}

bool KSSLD::cacheModifyByCN(QString cn, int policy, bool permanent, qlonglong expires) {
	QDateTime dt;
	dt.setTime_t(expires);
	return cacheModifyByCN(cn, KSSLCertificateCache::KSSLCertificatePolicy(policy), permanent, dt);
}

bool KSSLD::cacheModifyByCN(QString cn,
							KSSLCertificateCache::KSSLCertificatePolicy policy,								bool permanent,
							QDateTime expires) {
KSSLCNode *node;
	Q_FOREACH( node , certList ) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			node->permanent = permanent;
			node->expires = expires;
			node->policy = policy;
			certList.removeAll(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}

bool KSSLD::cacheModifyByCertificate(QByteArray certData, int policy, bool permanent, qlonglong expires) {
	QDataStream stream(&certData, QIODevice::ReadOnly);
	KSSLCertificate cert;
	stream >> cert;
	QDateTime dt;
	dt.setTime_t(expires);
	return cacheModifyByCertificate(cert, KSSLCertificateCache::KSSLCertificatePolicy(policy),
									permanent, dt);
}

bool KSSLD::cacheModifyByCertificate(KSSLCertificate cert,
							 KSSLCertificateCache::KSSLCertificatePolicy policy,
				 bool permanent,
				 QDateTime expires) {
KSSLCNode *node;

	Q_FOREACH( node , certList ) {
		if (cert == *(node->cert)) {
			node->permanent = permanent;
			node->expires = expires;
			node->policy = policy;
			certList.removeAll(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}

QStringList KSSLD::cacheGetHostList(QByteArray certData) {
	QDataStream stream(&certData, QIODevice::ReadOnly);
	KSSLCertificate cert;
	stream >> cert;
	return cacheGetHostList(cert);
}

QStringList KSSLD::cacheGetHostList(KSSLCertificate cert) {
KSSLCNode *node;

	Q_FOREACH( node , certList ) {
		if (cert == *(node->cert)) {
			if (!node->permanent && node->expires <
					   QDateTime::currentDateTime()) {
				certList.removeAll(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				searchRemoveCert(node->cert);
				delete node;
				cacheSaveToDisk();
				return QStringList();
			}

			certList.removeAll(node);
			certList.prepend(node);
			return node->hosts;
		}
	}

return QStringList();
}

bool KSSLD::cacheAddHost(QByteArray certData, QString host) {
	QDataStream stream(&certData, QIODevice::ReadOnly);
	KSSLCertificate cert;
	stream >> cert;
	return cacheAddHost(cert, host);
}

bool KSSLD::cacheAddHost(KSSLCertificate cert, QString host) {
KSSLCNode *node;

	if (host.isEmpty())
		return true;

	Q_FOREACH( node , certList ) {
		if (cert == *(node->cert)) {
			if (!node->permanent && node->expires <
						QDateTime::currentDateTime()) {
				certList.removeAll(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				searchRemoveCert(node->cert);
				delete node;
				cacheSaveToDisk();
				return false;
			}

			if (!node->hosts.contains(host)) {
				node->hosts << host;
			}

			certList.removeAll(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}


bool KSSLD::cacheRemoveHost(QByteArray certData, QString host) {
	QDataStream stream(&certData, QIODevice::ReadOnly);
	KSSLCertificate cert;
	stream >> cert;
	return cacheRemoveHost(cert, host);
}

bool KSSLD::cacheRemoveHost(KSSLCertificate cert, QString host) {
KSSLCNode *node;

	Q_FOREACH( node , certList ) {
		if (cert == *(node->cert)) {
			if (!node->permanent && node->expires <
						QDateTime::currentDateTime()) {
				certList.removeAll(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				searchRemoveCert(node->cert);
				delete node;
				cacheSaveToDisk();
				return false;
			}
			node->hosts.removeAll(host);
			certList.removeAll(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}




///////////////////////////////////////////////////////////////////////////

void KSSLD::caVerifyUpdate() {
	QString path = KGlobal::dirs()->saveLocation("kssl") + "/ca-bundle.crt";
	if (!QFile::exists(path))
		return;

        KConfigGroup cg(cfg, QString());
	quint32 newStamp = KGlobal::dirs()->calcResourceHash("config", "ksslcalist",
                                                             KStandardDirs::Recursive);
	quint32 oldStamp = cg.readEntry("ksslcalistStamp", 0);
	if (oldStamp != newStamp)
	{
		caRegenerate();
		cg.writeEntry("ksslcalistStamp", newStamp);
		cg.sync();
	}
}

bool KSSLD::caRegenerate() {
QString path = KGlobal::dirs()->saveLocation("kssl") + "/ca-bundle.crt";

QFile out(path);

	if (!out.open(QIODevice::WriteOnly))
		return false;

        KConfig cfg("ksslcalist", KConfig::NoGlobals);

        const QStringList x = cfg.groupList();

	for (QStringList::const_iterator i = x.begin();
				   i != x.end();
				   ++i) {
		if ((*i).isEmpty() || *i == "<default>") continue;

                KConfigGroup cg(&cfg, *i);

		if (!cg.readEntry("site", false)) continue;

		QString cert = cg.readEntry("x509", "");
		if (cert.length() <= 0) continue;

		unsigned int xx = cert.length() - 1;
		for (unsigned int j = 0; j < xx/64; j++) {
			cert.insert(64*(j+1)+j, '\n');
		}
		out.write("-----BEGIN CERTIFICATE-----\n", 28);
		out.write(cert.toLatin1(), cert.length());
		out.write("\n-----END CERTIFICATE-----\n\n", 28);
		out.flush();
	}

return true;
}



bool KSSLD::caAdd(QString certificate, bool ssl, bool email, bool code) {
        KSSLCertificate *x = KSSLCertificate::fromString(certificate.toLocal8Bit());

	if (!x) return false;

        KConfig cfgFile("ksslcalist", KConfig::NoGlobals);

        KConfigGroup cfg(&cfgFile, x->getSubject());
	cfg.writeEntry("x509", certificate);
	cfg.writeEntry("site", ssl);
	cfg.writeEntry("email", email);
	cfg.writeEntry("code", code);

	cfgFile.sync();
	delete x;

        return true;
}


/**
  * @internal
  * Returns a list of certificates as QStrings read from the given file
  */
static QStringList caReadCerticatesFromFile(QString filename) {

	QStringList certificates;
	QString certificate;
	QFile file(filename);
		QByteArray temp(1000, 0);

	if (!file.open(QIODevice::ReadOnly))
		return certificates;

	while (!file.atEnd()) {
		file.readLine(temp.data(), 999);
		if (temp.startsWith("-----BEGIN CERTIFICATE-----")) {
			certificate.clear();
			continue;
		}

		if (temp.startsWith("-----END CERTIFICATE-----")) {
			certificates.append(certificate);
			certificate.clear();
			continue;
		}

		certificate += temp.trimmed();
	}

	file.close();

	return certificates;
}

bool KSSLD::caAddFromFile(QString filename, bool ssl, bool email, bool code) {

	QStringList certificates;
	certificates = caReadCerticatesFromFile(filename);
	if (certificates.isEmpty())
		return false;

	bool ok = true;

	for (QStringList::Iterator it = certificates.begin();
					it != certificates.end(); ++it ) {
		ok &= caAdd(*it, ssl, email, code);
	}

	return ok;
}

bool KSSLD::caRemoveFromFile(QString filename) {

	QStringList certificates;
	certificates = caReadCerticatesFromFile(filename);
	if (certificates.isEmpty())
		return false;

	bool ok = true;

	for (QStringList::Iterator it = certificates.begin();
					it != certificates.end(); ++it ) {
		QString certificate = *it;
		KSSLCertificate *x = KSSLCertificate::fromString(certificate.toLocal8Bit());
		ok &= x && caRemove(x->getSubject());
		delete x;
	}

	return ok;
}


QStringList KSSLD::caList() {
QStringList x;
KConfig cfg("ksslcalist", KConfig::NoGlobals);

	x = cfg.groupList();
	x.removeAll("<default>");

return x;
}


bool KSSLD::caUseForSSL(QString subject) {
    KConfigGroup cg = KSharedConfig::openConfig("ksslcalist", KConfig::NoGlobals)->group(subject);

    return cg.readEntry("site", false);
}



bool KSSLD::caUseForEmail(QString subject) {
    KConfigGroup cg = KSharedConfig::openConfig("ksslcalist", KConfig::NoGlobals)->group(subject);

    return cg.readEntry("email", false);
}



bool KSSLD::caUseForCode(QString subject) {
    KConfigGroup cg = KSharedConfig::openConfig("ksslcalist", KConfig::NoGlobals)->group(subject);

    return cg.readEntry("code", false);
}


bool KSSLD::caRemove(QString subject) {
    KConfigGroup cg = KSharedConfig::openConfig("ksslcalist", KConfig::NoGlobals)->group(subject);

    if (!cg.exists())
        return false;

    cg.deleteGroup();
    cg.sync();

    return true;
}


QString KSSLD::caGetCert(QString subject) {
    KConfigGroup cg = KSharedConfig::openConfig("ksslcalist", KConfig::NoGlobals)->group(subject);

    return cg.readEntry("x509", QString());
}


bool KSSLD::caSetUse(QString subject, bool ssl, bool email, bool code) {
    KConfigGroup cg = KSharedConfig::openConfig("ksslcalist", KConfig::NoGlobals)->group(subject);

    if (!cg.exists())
        return false;

    cg.writeEntry("site", ssl);
    cg.writeEntry("email", email);
    cg.writeEntry("code", code);
    cg.sync();

    return true;
}

///////////////////////////////////////////////////////////////////////////

void KSSLD::searchAddCert(KSSLCertificate *cert) {
	skMD5Digest.insert(cert->getMD5Digest(), cert);

	QStringList mails;
	cert->getEmails(mails);
	for(QStringList::const_iterator iter = mails.begin(); iter != mails.end(); ++iter) {
		QString email = static_cast<const QString &>(*iter).toLower();
		QMap<QString, QVector<KSSLCertificate*> >::iterator it = skEmail.find(email);

		if (it == skEmail.end())
			it = skEmail.insert(email, QVector<KSSLCertificate*>());

		QVector<KSSLCertificate*> &elem = *it;

		if (elem.indexOf(cert) == -1) {
			int n = 0;
			for(; n < elem.size(); n++) {
				if (!elem.at(n)) {
					elem.insert(n, cert);
					break;
				}
			}
			if (n == elem.size()) {
				elem.resize(n+1);
				elem.insert(n, cert);
			}
		}
	}
}


void KSSLD::searchRemoveCert(KSSLCertificate *cert) {
	skMD5Digest.remove(cert->getMD5Digest());

	QStringList mails;
	cert->getEmails(mails);
	for(QStringList::const_iterator iter = mails.begin(); iter != mails.end(); ++iter) {
		QMap<QString, QVector<KSSLCertificate*> >::iterator it = skEmail.find(static_cast<const QString &>(*iter).toLower());

		if (it == skEmail.end())
			   break;

		QVector<KSSLCertificate*> &elem = *it;

		int n = elem.indexOf(cert);
		if (n != -1)
			elem.remove(n);
	}
}


QStringList KSSLD::getKDEKeyByEmail(const QString &email) {
	QStringList rc;
	QMap<QString, QVector<KSSLCertificate*> >::iterator it = skEmail.find(email.toLower());

	kDebug() << "GETKDEKey " << email.toLatin1();

	if (it == skEmail.end())
		return rc;

	QVector<KSSLCertificate*> &elem = *it;
	for (int n = 0; n < elem.size(); n++) {
		KSSLCertificate *cert = elem.at(n);
		if (cert) {
			rc.append(cert->getKDEKey());
		}
	}

	kDebug() << "ergebnisse: " << rc.size() << " " << elem.size();
	return rc;
}


KSSLCertificate KSSLD::getCertByMD5Digest(const QString &key) {
	QMap<QString, KSSLCertificate *>::iterator iter = skMD5Digest.find(key);

	kDebug() << "Searching cert for " << key.toLatin1();

	if (iter != skMD5Digest.end())
		return **iter;

	KSSLCertificate rc; // FIXME: Better way to return a not found condition?
	kDebug() << "Not found: " << rc.toString().toLatin1();
	return rc;
}


///////////////////////////////////////////////////////////////////////////

//
//	Certificate Home methods
//

QStringList KSSLD::getHomeCertificateList() {
	return KSSLCertificateHome::getCertificateList();
}

bool KSSLD::addHomeCertificateFile(QString filename, QString password, bool storePass) {
	return KSSLCertificateHome::addCertificate(filename, password, storePass);
}

bool KSSLD::addHomeCertificatePKCS12(QString base64cert, QString passToStore) {
	bool ok;
	KSSLPKCS12 *pkcs12 = KSSLPKCS12::fromString(base64cert, passToStore);
	ok = KSSLCertificateHome::addCertificate(pkcs12, passToStore);
	delete pkcs12;
	return ok;
}

bool KSSLD::deleteHomeCertificateByFile(QString filename, QString password) {
	return KSSLCertificateHome::deleteCertificate(filename, password);
}

bool KSSLD::deleteHomeCertificateByPKCS12(QString base64cert, QString password) {
	bool ok;
	KSSLPKCS12 *pkcs12 = KSSLPKCS12::fromString(base64cert, password);
	ok = KSSLCertificateHome::deleteCertificate(pkcs12);
	delete pkcs12;
	return ok;
}

bool KSSLD::deleteHomeCertificateByName(QString name) {
	return KSSLCertificateHome::deleteCertificateByName(name);
}



///////////////////////////////////////////////////////////////////////////

#include "kssld.moc"


/*

  DESIGN     - KSSLCertificateCache
  ------

  This is the first implementation and I think this cache actually needs
  experimentation to determine which implementation works best.  My current
  options are:

   (1) Store copies of the X509 certificates in a QPtrList using a self
       organizing heuristic as described by Munro and Suwanda.
   (2) Store copies of the X509 certificates in a tree structure, perhaps
       a redblack tree, avl tree, or even just a simple binary tree.
   (3) Store the CN's in a tree or list and use them as a hash to retrieve
       the X509 certificates.
   (4) Create "nodes" containing the X509 certificate and place them in
       two structures concurrently, one organized by CN, the other by
       X509 serial number.

  This implementation uses (1).  (4) is definitely attractive, but I don't
  think it will be necessary to go so crazy with performance, and perhaps
  end up performing poorly in situations where there are very few entries in
  the cache (which is most likely the case most of the time).  The style of
  heuristic is move-to-front, not swap-forward.  This seems to make more
  sense because the typical user will hit a site at least a few times in a
  row before moving to a new one.

  What I worry about most with respect to performance is that cryptographic
  routines are expensive and if we have to perform them on each X509
  certificate until the right one is found, we will perform poorly.

  All in all, this code is actually quite crucial for performance on SSL
  website, especially those with many image files loaded via SSL.  If a
  site loads 15 images, we will have to run through this code 15 times.
  A heuristic for self organization will make each successive lookup faster.
  Sounds good, doesn't it?

  DO NOT ATTEMPT TO GUESS WHICH CERTIFICATES ARE ACCEPTIBLE IN YOUR CODE!!
  ALWAYS USE THE CACHE.  IT MAY CHECK THINGS THAT YOU DON'T THINK OF, AND
  ALSO IF THERE IS A BUG IN THE CHECKING CODE, IF IT IS ALL CONTAINED IN
  THIS LIBRARY, A MINOR FIX WILL FIX ALL APPLICATIONS.
 */

