// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>

#include <kdebug.h>
#include <kinstance.h>
#include <kmdbase.h>

#include "auth_plain.h"

PlainAuthModule::PlainAuthModule(QObject *parent, const char *name)
	: KSASLAuthModule(parent, name)
{
}

PlainAuthModule::~PlainAuthModule()
{
}

QString PlainAuthModule::auth_method()
{
	return QString::fromLatin1("PLAIN");
}

QString PlainAuthModule::auth_response(const QString &, const KURL &auth_url)
{
	QString line;
	line.append(auth_url.user());
	line.append(QChar(static_cast<char>(0)));
	line.append(auth_url.user());
	line.append(QChar(static_cast<char>(0)));
	line.append(auth_url.pass());
	return KBase64::encodeString(line);
}

// Module factory stuff.. should be automated

PlainAuthModuleFactory::PlainAuthModuleFactory (QObject *parent, const char *name)
	: KLibFactory (parent, name)
{
	s_instance = new KInstance( "authplainmodule" );
}

PlainAuthModuleFactory::~PlainAuthModuleFactory()
{
	delete s_instance;
}

QObject *PlainAuthModuleFactory::create (QObject *parent, const char *name, const char *, const QStringList &)
{
	QObject *obj = new PlainAuthModule (parent, name);
	emit objectCreated( obj );
	return obj;
}

KInstance *PlainAuthModuleFactory::instance()
{
	return s_instance;
}

extern "C" {
	void *init_ksasl_auth_plain() {
		return new PlainAuthModuleFactory;
	}
}

