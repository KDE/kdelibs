#ifndef SASLMODULE_H
#define SASLMODULE_H "$Id$"

#include <qobject.h>
#include <qstring.h>

#include <kurl.h>

class KSASLAuthModule
	: public QObject {
public:
	KSASLAuthModule(QObject *parent, const char *name):QObject(parent,name) {;}
	virtual ~KSASLAuthModule(){;}
	virtual QString auth_method()=0;

	// If you need to do multi pass authentication you need to set the
	// property "more" to true in all but the last pass, and to
	// false in the last pass.
	// E.g. with setProperty("more", QVariant(true,0));
	virtual QString auth_response(const QString &challenge, const KURL &auth_url)=0;
	virtual void get_usercredentials()=0;
};

#endif
