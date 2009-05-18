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

#include "ksslsettings.h"

#include <config.h>
#include <ksslconfig.h>

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

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#undef crypt
#endif

#include <kopenssl.h>

#ifdef KSSL_HAVE_SSL
#define sk_new d->kossl->sk_new
#define sk_push d->kossl->sk_push
#define sk_free d->kossl->sk_free
#define sk_value d->kossl->sk_value
#define sk_num d->kossl->sk_num
#define sk_dup d->kossl->sk_dup
#define sk_pop d->kossl->sk_pop
#endif

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

KSSLSettings::KSSLSettings(bool readConfig)
	:d(new KSSLSettingsPrivate)
{
        m_cfg = new KConfig("cryptodefaults", KConfig::NoGlobals);

	if (!KGlobal::dirs()->addResourceType("kssl", "data", "kssl")) {
		//kDebug(7029) << "Error adding (kssl, share/apps/kssl)";
	}

	if (readConfig) load();
}


// we don't save settings incase it was a temporary object
KSSLSettings::~KSSLSettings() {
	delete m_cfg;
	delete d;
}


QString KSSLSettings::getCipherList() {
    QString clist;
    // TODO fill in list here (or just remove this method!)
    return clist;
}

// FIXME - sync these up so that we can use them with the control module!!
void KSSLSettings::load() {
	m_cfg->reparseConfiguration();

        KConfigGroup cfg(m_cfg, "Warnings");
	m_bWarnOnEnter = cfg.readEntry("OnEnter", false);
	m_bWarnOnLeave = cfg.readEntry("OnLeave", true);
	m_bWarnOnUnencrypted = cfg.readEntry("OnUnencrypted", false);
	m_bWarnOnMixed = cfg.readEntry("OnMixed", true);

	cfg.changeGroup("Validation");
	m_bWarnSelfSigned = cfg.readEntry("WarnSelfSigned", true);
	m_bWarnExpired = cfg.readEntry("WarnExpired", true);
	m_bWarnRevoked = cfg.readEntry("WarnRevoked", true);

	cfg.changeGroup("EGD");
	d->m_bUseEGD = cfg.readEntry("UseEGD", false);
	d->m_bUseEFile = cfg.readEntry("UseEFile", false);
	d->m_EGDPath = cfg.readPathEntry("EGDPath", QString());

	cfg.changeGroup("Auth");
	d->m_bSendX509 = ("send" == cfg.readEntry("AuthMethod", ""));
	d->m_bPromptX509 = ("prompt" == cfg.readEntry("AuthMethod", ""));

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
        KConfigGroup cfg(m_cfg, "Warnings");
	cfg.writeEntry("OnEnter", m_bWarnOnEnter);
	cfg.writeEntry("OnLeave", m_bWarnOnLeave);
	cfg.writeEntry("OnUnencrypted", m_bWarnOnUnencrypted);
	cfg.writeEntry("OnMixed", m_bWarnOnMixed);

	cfg.changeGroup("Validation");
	cfg.writeEntry("WarnSelfSigned", m_bWarnSelfSigned);
	cfg.writeEntry("WarnExpired", m_bWarnExpired);
	cfg.writeEntry("WarnRevoked", m_bWarnRevoked);

	cfg.changeGroup("EGD");
	cfg.writeEntry("UseEGD", d->m_bUseEGD);
	cfg.writeEntry("UseEFile", d->m_bUseEFile);
	cfg.writePathEntry("EGDPath", d->m_EGDPath);

	m_cfg->sync();
	// FIXME - ciphers
#if 0
#ifdef KSSL_HAVE_SSL
	cfg.setGroup("SSLv3");
	for (unsigned int i = 0; i < v3ciphers.count(); i++) {
		QString ciphername;
		ciphername.sprintf("cipher_%s", v3ciphers[i].ascii());
		if (v3selectedciphers.contains(v3ciphers[i])) {
			cfg.writeEntry(ciphername, true);
		} else cfg.writeEntry(ciphername, false);
	}
        m_cfg->sync();
#endif

	// insure proper permissions -- contains sensitive data
	QString cfgName(KGlobal::dirs()->findResource("config", "cryptodefaults"));
	if (!cfgName.isEmpty())
		KDE::chmod(cfgName, 0600);
#endif
}


bool KSSLSettings::warnOnEnter() const       { return m_bWarnOnEnter; }
void KSSLSettings::setWarnOnEnter(bool x)    { m_bWarnOnEnter = x; }
bool KSSLSettings::warnOnUnencrypted() const { return m_bWarnOnUnencrypted; }
void KSSLSettings::setWarnOnUnencrypted(bool x) { m_bWarnOnUnencrypted = x; }
bool KSSLSettings::warnOnLeave() const       { return m_bWarnOnLeave; }
void KSSLSettings::setWarnOnLeave(bool x)    { m_bWarnOnLeave = x; }
bool KSSLSettings::warnOnMixed() const       { return m_bWarnOnMixed; }
bool KSSLSettings::useEGD() const            { return d->m_bUseEGD;      }
bool KSSLSettings::useEFile() const          { return d->m_bUseEFile;    }
bool KSSLSettings::autoSendX509() const      { return d->m_bSendX509; }
bool KSSLSettings::promptSendX509() const    { return d->m_bPromptX509; }
QString& KSSLSettings::getEGDPath()       { return d->m_EGDPath; }

#ifdef KSSL_HAVE_SSL
#undef sk_new
#undef sk_push
#undef sk_free
#undef sk_value
#undef sk_num
#undef sk_pop
#undef sk_dup
#endif
