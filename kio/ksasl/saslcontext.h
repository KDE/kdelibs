#ifndef SASLCONTEXT_H
#define SASLCONTEXT_H "$Id$"

#include <qobject.h>
#include <qstring.h>
#include <qdict.h>

#include <kurl.h>

#include "saslmodule.h"

/**
  * Provides a generic client side authentication API suitable for use 
  * with any SASL compatible protocol or authentication method.
  *
  * @author Alex Zepeda <jazepeda@pacbell.net>
  * @version $Revision$
  **/
class KSASLContext
{
public:

	/**
	  * Default ctor
	  **/
	KSASLContext ();

	/**
	  * Returns the best possible authentication method
	  *
	  * @param available A space separated list of authentication 
	  * methods supported by the server.
	  *
	  * @param isBASE64 Set to true if available is Base 64 encoded.
	  **/
	QString chooseMethod (const QString &available, bool isBASE64 = false);

	/**
	  * Instructs the current authentication context to operate on
	  * a given URL.
	  *
	  * @param url The URL.
	  **/
	void setURL (const KURL &url);

	/**
	  * An overloaded version of generateResponse.  Why it's here I don't know..
	  *
	  * @param challenge
	  * @param isBASE64
	  **/
	QString generateResponse (const QString &challenge = QString::null, bool isBASE64 = false);

	/**
	  * Returns a response based on the given URL, challenge, and 
	  * authentication scheme.  If isBASE64 is true, challenge is
	  * decoded from Base 64, and the returned value is Base 64 encoded.
	  *
	  * @param challenge
	  * @param isBASE64
	  **/
	QString generateResponse (const QString &challenge, bool isBASE64, bool &more);

protected:

	/**
	  * Load all of the available authentication modules into memory
	  * so we've got an idea what is supported on the client side.
	  **/
	void loadModules ();

	QDict<KSASLAuthModule> m_lstModules;
	KURL m_urlAuthPath;
	QString m_sAuthMethod;
};

#endif
