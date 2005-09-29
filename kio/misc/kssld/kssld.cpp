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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qtimer.h>

#include "kssld.h"
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <ksslcertchain.h>
#include <ksslcertificate.h>
#include <ksslcertificatehome.h>
#include <ksslpkcs12.h>
#include <ksslx509map.h>
#include <qptrlist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <qfile.h>
#include <qsortedlist.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <qdatetime.h>

#include <kmdcodec.h>
#include <kopenssl.h>

// See design notes at end

extern "C" {
	KDE_EXPORT KDEDModule *create_kssld(const QCString &name) {
		return new KSSLD(name);
	}

	KDE_EXPORT void *__kde_do_unload;
}


static void updatePoliciesConfig(KConfig *cfg) {
	QStringList groups = cfg->groupList();

	for (QStringList::Iterator i = groups.begin(); i != groups.end(); ++i) {
		if ((*i).isEmpty() || *i == "General") {
			continue;
		}

		cfg->setGroup(*i);

		// remove it if it has expired
		if (!cfg->readBoolEntry("Permanent") && cfg->readDateTimeEntry("Expires") < QDateTime::currentDateTime()) {
			cfg->deleteGroup(*i);
			continue;
		}

		QString encodedCertStr = cfg->readEntry("Certificate");
		QCString encodedCert = encodedCertStr.local8Bit();
	       	KSSLCertificate *newCert = KSSLCertificate::fromString(encodedCert);
		if (!newCert) {
			cfg->deleteGroup(*i);
			continue;
		}

		KSSLCertificateCache::KSSLCertificatePolicy policy = (KSSLCertificateCache::KSSLCertificatePolicy) cfg->readNumEntry("Policy");
		bool permanent = cfg->readBoolEntry("Permanent");
		QDateTime expires = cfg->readDateTimeEntry("Expires");
		QStringList hosts = cfg->readListEntry("Hosts");
		QStringList chain = cfg->readListEntry("Chain");
		cfg->deleteGroup(*i);

		cfg->setGroup(newCert->getMD5Digest());
		cfg->writeEntry("Certificate", encodedCertStr);
		cfg->writeEntry("Policy", policy);
		cfg->writeEntry("Permanent", permanent);
		cfg->writeEntry("Expires", expires);
		cfg->writeEntry("Hosts", hosts);
		cfg->writeEntry("Chain", chain);
		delete newCert;
	}

	cfg->setGroup("General");
	cfg->writeEntry("policies version", 2);

	cfg->sync();
}


KSSLD::KSSLD(const QCString &name) : KDEDModule(name)
{
// ----------------------- FOR THE CACHE ------------------------------------	
	cfg = new KSimpleConfig("ksslpolicies", false);
	cfg->setGroup("General");
	if (2 != cfg->readNumEntry("policies version", 0)) {
		::updatePoliciesConfig(cfg);
	}
	KGlobal::dirs()->addResourceType("kssl", KStandardDirs::kde_default("data") + "kssl");
	caVerifyUpdate();
	cacheLoadDefaultPolicies();
	certList.setAutoDelete(false);
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
KSSLCNode *node;

	cfg->setGroup("General");
	cfg->writeEntry("policies version", 2);

	for (node = certList.first(); node; node = certList.next()) {
		if (node->permanent ||
			node->expires > QDateTime::currentDateTime()) {
			// First convert to a binary format and then write the
			// kconfig entry write the (CN, policy, cert) to
			// KSimpleConfig
			cfg->setGroup(node->cert->getMD5Digest());
			cfg->writeEntry("Certificate", node->cert->toString());
			cfg->writeEntry("Policy", node->policy);
			cfg->writeEntry("Expires", node->expires);
			cfg->writeEntry("Permanent", node->permanent);
			cfg->writeEntry("Hosts", node->hosts);

			// Also write the chain
			QStringList qsl;
			QPtrList<KSSLCertificate> cl =
						node->cert->chain().getChain();
			for (KSSLCertificate *c = cl.first();
							c != 0;
							c = cl.next()) {
				//kdDebug() << "Certificate in chain: "
				//	    <<  c->toString() << endl;
				qsl << c->toString();
			}

			cl.setAutoDelete(true);
			cfg->writeEntry("Chain", qsl);
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
	cfg = new KSimpleConfig("ksslpolicies", false);
	cacheLoadDefaultPolicies();
}


void KSSLD::cacheClearList() {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		certList.remove(node);
		delete node;
	}

	skEmail.clear();
	skMD5Digest.clear();
}


void KSSLD::cacheLoadDefaultPolicies() {
QStringList groups = cfg->groupList();

	for (QStringList::Iterator i = groups.begin();
				i != groups.end();
				++i) {
		if ((*i).isEmpty() || *i == "General") {
			continue;
		}

		cfg->setGroup(*i);

		// remove it if it has expired
		if (!cfg->readBoolEntry("Permanent") &&
			cfg->readDateTimeEntry("Expires") <
				QDateTime::currentDateTime()) {
			cfg->deleteGroup(*i);
			continue;
		}

		QCString encodedCert;
		KSSLCertificate *newCert;

		encodedCert = cfg->readEntry("Certificate").local8Bit();
	       	newCert = KSSLCertificate::fromString(encodedCert);

		if (!newCert) {
		       continue;
		}

		KSSLCNode *n = new KSSLCNode;
		n->cert = newCert;
		n->policy = (KSSLCertificateCache::KSSLCertificatePolicy) cfg->readNumEntry("Policy");
		n->permanent = cfg->readBoolEntry("Permanent");
		n->expires = cfg->readDateTimeEntry("Expires");
		n->hosts = cfg->readListEntry("Hosts");
		newCert->chain().setCertChain(cfg->readListEntry("Chain"));
		certList.append(n); 
		searchAddCert(newCert);
	}
}


void KSSLD::cacheAddCertificate(KSSLCertificate cert, 
			KSSLCertificateCache::KSSLCertificatePolicy policy,
			bool permanent) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
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


KSSLCertificateCache::KSSLCertificatePolicy KSSLD::cacheGetPolicyByCN(QString cn) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			if (!node->permanent &&
				node->expires < QDateTime::currentDateTime()) {
				certList.remove(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				continue;
			}

			certList.remove(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return node->policy;
		}
	}

	cacheSaveToDisk();

return KSSLCertificateCache::Unknown;
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLD::cacheGetPolicyByCertificate(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {  
			if (!node->permanent &&
				node->expires < QDateTime::currentDateTime()) {
				certList.remove(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				cacheSaveToDisk();
				return KSSLCertificateCache::Unknown;
			}

			certList.remove(node);
			certList.prepend(node);
			return node->policy;
		}
	}

return KSSLCertificateCache::Unknown;
}


bool KSSLD::cacheSeenCN(QString cn) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			if (!node->permanent &&
				node->expires < QDateTime::currentDateTime()) {
				certList.remove(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				cacheSaveToDisk();
				continue;
			}

			certList.remove(node);
			certList.prepend(node);
			return true;
		}
	}

return false;
}


bool KSSLD::cacheSeenCertificate(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			if (!node->permanent &&
				node->expires < QDateTime::currentDateTime()) {
				certList.remove(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				cacheSaveToDisk();
				return false;
			}

			certList.remove(node);
			certList.prepend(node);
			return true;
		}
	}

return false;
}


bool KSSLD::cacheIsPermanent(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			if (!node->permanent && node->expires <
					QDateTime::currentDateTime()) {
				certList.remove(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				delete node;
				cacheSaveToDisk();
				return false;
			}

			certList.remove(node);
			certList.prepend(node);
			return node->permanent;
		}
	}

return false;
}


bool KSSLD::cacheRemoveBySubject(QString subject) {
KSSLCNode *node;
bool gotOne = false;

	for (node = certList.first(); node; node = certList.next()) {
		if (node->cert->getSubject() == subject) {
			certList.remove(node);
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

	for (node = certList.first(); node; node = certList.next()) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			certList.remove(node);
			cfg->deleteGroup(node->cert->getMD5Digest());
			searchRemoveCert(node->cert);
			delete node;
			gotOne = true;
		}
	}

	cacheSaveToDisk();

return gotOne;
}


bool KSSLD::cacheRemoveByCertificate(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			certList.remove(node);
			cfg->deleteGroup(node->cert->getMD5Digest());
			searchRemoveCert(node->cert);
			delete node;
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}


bool KSSLD::cacheModifyByCN(QString cn,
                            KSSLCertificateCache::KSSLCertificatePolicy policy,                             bool permanent,
                            QDateTime expires) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			node->permanent = permanent;
			node->expires = expires;
			node->policy = policy;
			certList.remove(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}


bool KSSLD::cacheModifyByCertificate(KSSLCertificate cert,
                             KSSLCertificateCache::KSSLCertificatePolicy policy,
			     bool permanent,
			     QDateTime expires) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			node->permanent = permanent;
			node->expires = expires;
			node->policy = policy;
			certList.remove(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}


QStringList KSSLD::cacheGetHostList(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			if (!node->permanent && node->expires <
				       QDateTime::currentDateTime()) {
				certList.remove(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				searchRemoveCert(node->cert);
				delete node;
				cacheSaveToDisk();
				return QStringList();
			}

			certList.remove(node);
			certList.prepend(node);
			return node->hosts;
		}
	}

return QStringList();
}


bool KSSLD::cacheAddHost(KSSLCertificate cert, QString host) {
KSSLCNode *node;

	if (host.isEmpty())
		return true;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			if (!node->permanent && node->expires <
				       	QDateTime::currentDateTime()) {
				certList.remove(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				searchRemoveCert(node->cert);
				delete node;
				cacheSaveToDisk();
				return false;
			}

			if (!node->hosts.contains(host)) {
				node->hosts << host;
			}

			certList.remove(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}


bool KSSLD::cacheRemoveHost(KSSLCertificate cert, QString host) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			if (!node->permanent && node->expires <
				       	QDateTime::currentDateTime()) {
				certList.remove(node);
				cfg->deleteGroup(node->cert->getMD5Digest());
				searchRemoveCert(node->cert);
				delete node;
				cacheSaveToDisk();
				return false;
			}
			node->hosts.remove(host);
			certList.remove(node);
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
	
	cfg->setGroup(QString::null);
	Q_UINT32 newStamp = KGlobal::dirs()->calcResourceHash("config", "ksslcalist", true);
	Q_UINT32 oldStamp = cfg->readUnsignedNumEntry("ksslcalistStamp");
	if (oldStamp != newStamp)
	{
		caRegenerate();
		cfg->writeEntry("ksslcalistStamp", newStamp);
		cfg->sync();
	}
}

bool KSSLD::caRegenerate() {
QString path = KGlobal::dirs()->saveLocation("kssl") + "/ca-bundle.crt";

QFile out(path);

	if (!out.open(IO_WriteOnly))
		return false;

KConfig cfg("ksslcalist", true, false);

QStringList x = cfg.groupList();

	for (QStringList::Iterator i = x.begin();
				   i != x.end();
				   ++i) {
		if ((*i).isEmpty() || *i == "<default>") continue;

		cfg.setGroup(*i);

		if (!cfg.readBoolEntry("site", false)) continue;

		QString cert = cfg.readEntry("x509", "");
		if (cert.length() <= 0) continue;

		unsigned int xx = cert.length() - 1;
		for (unsigned int j = 0; j < xx/64; j++) {
			cert.insert(64*(j+1)+j, '\n');
		}
		out.writeBlock("-----BEGIN CERTIFICATE-----\n", 28);
		out.writeBlock(cert.latin1(), cert.length());
		out.writeBlock("\n-----END CERTIFICATE-----\n\n", 28);
		out.flush();
	}

return true;
}



bool KSSLD::caAdd(QString certificate, bool ssl, bool email, bool code) {
KSSLCertificate *x = KSSLCertificate::fromString(certificate.local8Bit());

	if (!x) return false;

KConfig cfg("ksslcalist", false, false);

	cfg.setGroup(x->getSubject());
	cfg.writeEntry("x509", certificate);
	cfg.writeEntry("site", ssl);
	cfg.writeEntry("email", email);
	cfg.writeEntry("code", code);

	cfg.sync();
	delete x;

return true;
}


/**
  * @internal
  * Returns a list of certificates as QStrings read from the given file
  */
static QStringList caReadCerticatesFromFile(QString filename) {

	QStringList certificates;
	QString certificate, temp;
	QFile file(filename);

	if (!file.open(IO_ReadOnly))
		return certificates;

	while (!file.atEnd()) {
		file.readLine(temp, 999);
		if (temp.startsWith("-----BEGIN CERTIFICATE-----")) {
			certificate = QString::null;
			continue;
		}

		if (temp.startsWith("-----END CERTIFICATE-----")) {
			certificates.append(certificate);
			certificate = QString::null;
			continue;
		}

		certificate += temp.stripWhiteSpace();
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
		KSSLCertificate *x = KSSLCertificate::fromString(certificate.local8Bit());
		ok &= x && caRemove(x->getSubject());
		delete x;
	}

	return ok;
}


QStringList KSSLD::caList() {
QStringList x;
KConfig cfg("ksslcalist", true, false);

	x = cfg.groupList();
	x.remove("<default>");

return x;
}


bool KSSLD::caUseForSSL(QString subject) {
KConfig cfg("ksslcalist", true, false);

	if (!cfg.hasGroup(subject))
		return false;

	cfg.setGroup(subject);
return cfg.readBoolEntry("site", false);
}



bool KSSLD::caUseForEmail(QString subject) {
KConfig cfg("ksslcalist", true, false);

	if (!cfg.hasGroup(subject))
		return false;

	cfg.setGroup(subject);
return cfg.readBoolEntry("email", false);
}



bool KSSLD::caUseForCode(QString subject) {
KConfig cfg("ksslcalist", true, false);

	if (!cfg.hasGroup(subject))
		return false;

	cfg.setGroup(subject);
return cfg.readBoolEntry("code", false);
}


bool KSSLD::caRemove(QString subject) {
KConfig cfg("ksslcalist", false, false);
	if (!cfg.hasGroup(subject))
		return false;

	cfg.deleteGroup(subject);
	cfg.sync();

return true;
}


QString KSSLD::caGetCert(QString subject) {
KConfig cfg("ksslcalist", true, false);
	if (!cfg.hasGroup(subject))
		return QString::null;

	cfg.setGroup(subject);

return cfg.readEntry("x509", QString::null);
}


bool KSSLD::caSetUse(QString subject, bool ssl, bool email, bool code) {
KConfig cfg("ksslcalist", false, false);
	if (!cfg.hasGroup(subject))
		return false;

	cfg.setGroup(subject);

	cfg.writeEntry("site", ssl);
	cfg.writeEntry("email", email);
	cfg.writeEntry("code", code);
	cfg.sync();

return true;
}

///////////////////////////////////////////////////////////////////////////

void KSSLD::searchAddCert(KSSLCertificate *cert) {
	skMD5Digest.insert(cert->getMD5Digest(), cert, true);

	QStringList mails;
	cert->getEmails(mails);
	for(QStringList::const_iterator iter = mails.begin(); iter != mails.end(); ++iter) {
		QString email = static_cast<const QString &>(*iter).lower();
		QMap<QString, QPtrVector<KSSLCertificate> >::iterator it = skEmail.find(email);

		if (it == skEmail.end())
			it = skEmail.insert(email, QPtrVector<KSSLCertificate>());

		QPtrVector<KSSLCertificate> &elem = *it;
		
		if (elem.findRef(cert) == -1) {
			unsigned int n = 0;
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
		QMap<QString, QPtrVector<KSSLCertificate> >::iterator it = skEmail.find(static_cast<const QString &>(*iter).lower());

		if (it == skEmail.end())
		       break;

		QPtrVector<KSSLCertificate> &elem = *it;

		int n = elem.findRef(cert);
		if (n != -1)
			elem.remove(n);
	}
}	


QStringList KSSLD::getKDEKeyByEmail(const QString &email) {
	QStringList rc;
	QMap<QString, QPtrVector<KSSLCertificate> >::iterator it = skEmail.find(email.lower());

	kdDebug() << "GETKDEKey " << email.latin1() << endl;

	if (it == skEmail.end())
		return rc;

	QPtrVector<KSSLCertificate> &elem = *it;
	for (unsigned int n = 0; n < elem.size(); n++) {
		KSSLCertificate *cert = elem.at(n);
		if (cert) {
			rc.append(cert->getKDEKey());
		}
	}

	kdDebug() << "ergebnisse: " << rc.size() << " " << elem.size() << endl;
	return rc;
}


KSSLCertificate KSSLD::getCertByMD5Digest(const QString &key) {
	QMap<QString, KSSLCertificate *>::iterator iter = skMD5Digest.find(key);
	
	kdDebug() << "Searching cert for " << key.latin1() << endl;

	if (iter != skMD5Digest.end())
		return **iter;
	
	KSSLCertificate rc; // FIXME: Better way to return a not found condition?
	kdDebug() << "Not found: " << rc.toString().latin1() << endl;
	return rc;
}	


///////////////////////////////////////////////////////////////////////////

//
//  Certificate Home methods
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

