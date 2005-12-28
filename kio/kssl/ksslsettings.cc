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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
 
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include <qfile.h>
#include <q3sortedlist.h>

#include "ksslsettings.h"
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#undef crypt
#endif

#include <kopenssl.h>

class CipherNode {
	public:
		CipherNode(const char *_name, int _keylen) : 
			name(_name), keylen(_keylen) {}
		QString name;
		int keylen;
		inline int operator==(CipherNode &x) 
		{ return ((x.keylen == keylen) && (x.name == name)); }
		inline int operator< (CipherNode &x) { return keylen < x.keylen;  }
		inline int operator<=(CipherNode &x) { return keylen <= x.keylen; }
		inline int operator> (CipherNode &x) { return keylen > x.keylen;  }
		inline int operator>=(CipherNode &x) { return keylen >= x.keylen; }
};


class KSSLSettingsPrivate {
	public:
		KSSLSettingsPrivate() {
			kossl = 0L;   // try to delay this as long as possible
		}
		~KSSLSettingsPrivate() {

		}

		KOSSL *kossl;
		bool m_bUseEGD;
		bool m_bUseEFile;
		QString m_EGDPath;
		bool m_bSendX509;
		bool m_bPromptX509;
};

//
// FIXME
// Implementation note: for now, we only read cipher settings from disk,
//                      and do not store them in memory.  This should change.
//

KSSLSettings::KSSLSettings(bool readConfig) {
	d = new KSSLSettingsPrivate;
	m_cfg = new KConfig("cryptodefaults", false, false);

	if (!KGlobal::dirs()->addResourceType("kssl", KStandardDirs::kde_default("data") + "kssl")) {
		//kdDebug(7029) << "Error adding (kssl, share/apps/kssl)" << endl;
	}

	if (readConfig) load();
}


// we don't save settings incase it was a temporary object
KSSLSettings::~KSSLSettings() {
	delete m_cfg;
	delete d;
}


// FIXME: we should make a default list available if this fails
//        since OpenSSL seems to just choose any old thing if it's given an
//        empty list.  This behavior is not confirmed though.
QString KSSLSettings::getCipherList() {
	QString clist;
#ifdef KSSL_HAVE_SSL
	QString tcipher;
	bool firstcipher = true;
	SSL_METHOD *meth = 0L;
	Q3SortedList<CipherNode> cipherSort;

	cipherSort.setAutoDelete(true);

	if (!d->kossl)
		d->kossl = KOSSL::self();

	m_cfg->setGroup("SSLv3");
	meth = d->kossl->SSLv3_client_method();
	for(int i = 0; ; i++) {
		SSL_CIPHER *sc = (meth->get_cipher)(i);
		if (!sc)
			break;
		tcipher.sprintf("cipher_%s", sc->name);
		int bits = d->kossl->SSL_CIPHER_get_bits(sc, 0L);

		if (m_cfg->readBoolEntry(tcipher, bits >= 56)) {
			CipherNode *xx = new CipherNode(sc->name,bits);
			if (!cipherSort.contains(xx)) {
				cipherSort.prepend(xx);
			} else {
				delete xx;
			}
		}
	}

	// Remove any ADH ciphers as per RFC2246
	for (unsigned int i = 0; i < cipherSort.count(); i++) {
		CipherNode *j = 0L;
		while ((j = cipherSort.at(i)) != 0L) {
			if (j->name.contains("ADH-")) {
				cipherSort.remove(j);
			} else {
				break;
			}
		}
	} 

	// now assemble the list  cipher1:cipher2:cipher3:...:ciphern
	while (!cipherSort.isEmpty()) {
		if (firstcipher)
			firstcipher = false;
		else clist.append(":");
		clist.append(cipherSort.getLast()->name);
		cipherSort.removeLast();
	} // while

	kdDebug(7029) << "Cipher list is: " << clist << endl;

#endif
	return clist;
}

// FIXME - sync these up so that we can use them with the control module!!
void KSSLSettings::load() {
	m_cfg->reparseConfiguration();

	m_cfg->setGroup("Warnings");
	m_bWarnOnEnter = m_cfg->readEntry("OnEnter", QVariant(false)).toBool();
	m_bWarnOnLeave = m_cfg->readEntry("OnLeave", QVariant(true)).toBool();
	m_bWarnOnUnencrypted = m_cfg->readEntry("OnUnencrypted", QVariant(true)).toBool();
	m_bWarnOnMixed = m_cfg->readEntry("OnMixed", QVariant(true)).toBool();

	m_cfg->setGroup("Validation");
	m_bWarnSelfSigned = m_cfg->readEntry("WarnSelfSigned", QVariant(true)).toBool();
	m_bWarnExpired = m_cfg->readEntry("WarnExpired", QVariant(true)).toBool();
	m_bWarnRevoked = m_cfg->readEntry("WarnRevoked", QVariant(true)).toBool();

	m_cfg->setGroup("EGD");
	d->m_bUseEGD = m_cfg->readEntry("UseEGD", QVariant(false)).toBool();
	d->m_bUseEFile = m_cfg->readEntry("UseEFile", QVariant(false)).toBool();
	d->m_EGDPath = m_cfg->readPathEntry("EGDPath");

	m_cfg->setGroup("Auth");
	d->m_bSendX509 = ("send" == m_cfg->readEntry("AuthMethod", ""));
	d->m_bPromptX509 = ("prompt" == m_cfg->readEntry("AuthMethod", ""));

#ifdef KSSL_HAVE_SSL



#endif
}


void KSSLSettings::defaults() {
	m_bWarnOnEnter = false;
	m_bWarnOnLeave = true;
	m_bWarnOnUnencrypted = true;
	m_bWarnOnMixed = true;
	m_bWarnSelfSigned = true;
	m_bWarnExpired = true;
	m_bWarnRevoked = true;
	d->m_bUseEGD = false;
	d->m_bUseEFile = false;
	d->m_EGDPath = "";
}


void KSSLSettings::save() {
	m_cfg->setGroup("Warnings");
	m_cfg->writeEntry("OnEnter", m_bWarnOnEnter);
	m_cfg->writeEntry("OnLeave", m_bWarnOnLeave);
	m_cfg->writeEntry("OnUnencrypted", m_bWarnOnUnencrypted);
	m_cfg->writeEntry("OnMixed", m_bWarnOnMixed);

	m_cfg->setGroup("Validation");
	m_cfg->writeEntry("WarnSelfSigned", m_bWarnSelfSigned);
	m_cfg->writeEntry("WarnExpired", m_bWarnExpired);
	m_cfg->writeEntry("WarnRevoked", m_bWarnRevoked);

	m_cfg->setGroup("EGD");
	m_cfg->writeEntry("UseEGD", d->m_bUseEGD);
	m_cfg->writeEntry("UseEFile", d->m_bUseEFile);
	m_cfg->writePathEntry("EGDPath", d->m_EGDPath);

	m_cfg->sync();
	// FIXME - ciphers
#if 0
#ifdef KSSL_HAVE_SSL
	m_cfg->setGroup("SSLv3");
	for (unsigned int i = 0; i < v3ciphers.count(); i++) {
		QString ciphername;
		ciphername.sprintf("cipher_%s", v3ciphers[i].ascii());
		if (v3selectedciphers.contains(v3ciphers[i])) {
			m_cfg->writeEntry(ciphername, true);
		} else m_cfg->writeEntry(ciphername, false);
	}
#endif 

	m_cfg->sync();

	// insure proper permissions -- contains sensitive data
	QString cfgName(KGlobal::dirs()->findResource("config", "cryptodefaults"));
	if (!cfgName.isEmpty())
		::chmod(QFile::encodeName(cfgName), 0600);
#endif
}


bool KSSLSettings::warnOnEnter() const       { return m_bWarnOnEnter; }
void KSSLSettings::setWarnOnEnter(bool x)    { m_bWarnOnEnter = x; }
bool KSSLSettings::warnOnUnencrypted() const { return m_bWarnOnUnencrypted; }
void KSSLSettings::setWarnOnUnencrypted(bool x) { m_bWarnOnUnencrypted = x; }
bool KSSLSettings::warnOnLeave() const       { return m_bWarnOnLeave; }
void KSSLSettings::setWarnOnLeave(bool x)    { m_bWarnOnLeave = x; }
bool KSSLSettings::warnOnMixed() const       { return m_bWarnOnMixed; }
bool KSSLSettings::warnOnSelfSigned() const  { return m_bWarnSelfSigned; }
bool KSSLSettings::warnOnRevoked() const     { return m_bWarnRevoked; }
bool KSSLSettings::warnOnExpired() const     { return m_bWarnExpired; }
bool KSSLSettings::useEGD() const            { return d->m_bUseEGD;      }
bool KSSLSettings::useEFile() const          { return d->m_bUseEFile;    }
bool KSSLSettings::autoSendX509() const      { return d->m_bSendX509; }
bool KSSLSettings::promptSendX509() const    { return d->m_bPromptX509; }
QString& KSSLSettings::getEGDPath()       { return d->m_EGDPath; }

