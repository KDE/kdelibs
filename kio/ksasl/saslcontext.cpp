// $Id$

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <klibloader.h>
#include <ktrader.h>
#include "../kmdcodec.h"

#include "saslcontext.h"
#include "saslmodule.h"

KSASLContext::KSASLContext()
{
	m_lstModules.setAutoDelete( true );
	loadModules();
}

QString KSASLContext::chooseMethod (const QString &available, bool isBASE64)
{
	// It returns the selected authentication method as a QString
	QString operate_on;

	if (isBASE64) {
		operate_on=KBase64::decodeString(available.latin1());
	} else
		operate_on=available;

	QStringList server_side = QStringList::split(QString(" "), operate_on);
	QStringList supported;

	// Go thru the ones that the server supports, and compare them to the ones we support
	for (QStringList::Iterator it1 = server_side.begin(); it1 != server_side.end(); ++it1 ) {
		for (unsigned int x = 0; x < m_lstModules.count(); x++) {
			if (m_lstModules.at(x)->auth_method() ==  *it1) {
				supported += *it1;
			}
		}
	}

	if (!supported.count())
		m_sAuthMethod=QString::null;
	else if (supported.count() == 1)
		m_sAuthMethod=supported.first();
	else
		m_sAuthMethod=supported.last();
	return m_sAuthMethod;
}

void KSASLContext::setURL (const KURL &url)
{
	m_urlAuthPath=url;
}

QString KSASLContext::generateResponse (const QString &challenge, bool isBASE64)
{
	QString ret;
	unsigned int x;

	if (m_sAuthMethod == QString::null)
		return QString::null;

	for (x = 0; x < m_lstModules.count(); x++) {
		if (m_lstModules.at(x)->auth_method() ==  m_sAuthMethod) {
			break;
		}
	}
	ret=m_lstModules.at(x)->auth_response(challenge, m_urlAuthPath);

	if (isBASE64)
		return KBase64::encodeString(ret);
	return ret;
}


void KSASLContext::loadModules ()
{
	KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KSASL/Module") );
	KTrader::OfferList::ConstIterator it = offers.begin();
	KTrader::OfferList::ConstIterator end = offers.end();

	for (; it != end; ++it ) {
		if ((*it)->library().isEmpty()) continue;
         
		KLibFactory *factory = KLibLoader::self()->factory((*it)->library().latin1());
		if (!factory) continue;
		KSASLAuthModule *module = reinterpret_cast<KSASLAuthModule*>(factory->create(0, (*it)->desktopEntryName().latin1(), "KSASLModule"));
		if ( module ) {
			m_lstModules.append( module );
		}
	}
}
