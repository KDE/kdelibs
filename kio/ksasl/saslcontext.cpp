// $Id$

#include <qdict.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kmdcodec.h>

#include "saslcontext.h"
#include "saslmodule.h"

KSASLContext::KSASLContext()
{
	m_lstModules = QDict<KSASLAuthModule>(5, true);
	m_lstModules.setAutoDelete(true);

	loadModules();
}

QString KSASLContext::chooseMethod (const QString &available, bool isBASE64)
{
	// It returns the selected authentication method as a QString
	QString operate_on;

	if (isBASE64) {
		operate_on = KBase64::decodeString(available.stripWhiteSpace().latin1());
	} else {
		operate_on = available.stripWhiteSpace();
	}

	QStringList server_side = QStringList::split(QChar(' '), operate_on);
	QStringList supported;

	// Go thru the ones that the server supports, and compare them to the ones we support
	for (QStringList::Iterator it1 = server_side.begin(); it1 != server_side.end(); ++it1) {
		if (m_lstModules[*it1])
			supported += *it1;
	}

	if (!supported.count()) {
		m_sAuthMethod = QString::null;
	} else {
		m_sAuthMethod = supported.last();
	}

	return m_sAuthMethod;
}

void KSASLContext::setURL (const KURL &url)
{
	m_urlAuthPath = url;
}

QString KSASLContext::generateResponse (const QString &challenge, bool isBASE64)
{
	bool more = false;
	return generateResponse(challenge, isBASE64, more);
}

QString KSASLContext::generateResponse (const QString &challenge, bool isBASE64, bool &more)
{
	QString return_value, decoded_challenge;

	if (m_sAuthMethod == QString::null) {
		return QString::null;
	}

	if (isBASE64) {
		decoded_challenge = KBase64::decodeString(challenge);
	} else {
		decoded_challenge = challenge;
	}

        KSASLAuthModule *module = m_lstModules[m_sAuthMethod];

	if (!module) {
		return QString::null;
	}

	return_value = module->auth_response(decoded_challenge, m_urlAuthPath);
        more = module->property("more").toBool();

	if (isBASE64) {
		return KBase64::encodeString(return_value);
	}

	return return_value;
}


void KSASLContext::loadModules ()
{
	KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KSASL/Module") );
	KTrader::OfferList::ConstIterator it = offers.begin();
	KTrader::OfferList::ConstIterator end = offers.end();

	while (it != end) {

		if ((*it)->library().isEmpty()) {
			continue;
		}

		KLibFactory *factory = KLibLoader::self()->factory((*it)->library().latin1());

		if (!factory) {
			continue;
		}

		KSASLAuthModule *module = reinterpret_cast<KSASLAuthModule *>(factory->create(0, (*it)->desktopEntryName().latin1(), "KSASLModule"));

		if (module) {
			m_lstModules.insert(module->auth_method(), module);
		}

		++it;
	}
}
