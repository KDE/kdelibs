#ifndef SASLMODULE_H
#define SASLMODULE_H "$Id$"

#include <qobject.h>
#include <qstring.h>

#include <kurl.h>

/**
  * Provides a generic client side authentication API suitable for use
  * with any SASL compatible protocol or authentication method.
  *
  * @author Alex Zepeda <jazepeda@pacbell.net>
  * @version $Revision$
  **/
class KSASLAuthModule
	: public QObject {

public:

	/**
	  * Default ctor.  Of no use really.
	  **/
	KSASLAuthModule (QObject *parent, const char *name)
		: QObject(parent, name) {;}

	/**
	  * Default destructor.
	  **/
	virtual ~KSASLAuthModule () {;}

	/**
	  * Returns the IANA sanctioned name for the authentication 
	  * method as per RFC2222.
	  **/
	virtual QString auth_method () = 0;

	/**
	  * If you need to do multi pass authentication you need to set the
	  * property "more" to true in all but the last pass, and to
	  * false in the last pass.
	  * e.g. with setProperty("more", QVariant(true,0));
	  **/
	virtual QString auth_response (const QString &challenge, const KURL &auth_url) = 0;

	/**
	  * Hmm.
	  **/
	virtual void get_usercredentials () = 0;
};

#endif
