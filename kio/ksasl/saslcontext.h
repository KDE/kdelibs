#ifndef SASLCONTEXT_H
#define SASLCONTEXT_H "$Id$"

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>

#include <kurl.h>

#include "saslmodule.h"

class KSASLContext
{
public:
	KSASLContext ();
	QString chooseMethod (const QString &available, bool isBASE64 = false);
	void setURL (const KURL &url);
	QString generateResponse (const QString &challenge = QString::null, bool isBASE64 = false);

protected:
	bool loadModule (const QString &mod_name);

	QList<KSASLAuthModule> modules;
	KURL auth_url;
	QString auth_method;
};

#endif
