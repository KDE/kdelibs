// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>

#include <kdebug.h>

#include <kinstance.h>
#include "../../kmdcodec.h"

#include "auth_cram.h"

CramAuthModule::CramAuthModule(QObject *parent, const char *name)
	: KSASLAuthModule(parent, name)
{
}

CramAuthModule::~CramAuthModule()
{
}

QString CramAuthModule::auth_method()
{
	return QString::fromLatin1("CRAM-MD5");
}

QString CramAuthModule::auth_response(const QString &challenge, const KURL &auth_url)
{
	char i_key[65], o_key[65];
	int i;
	KMD5 *imd5, *md5;
	QString ret;

	memset(i_key, 0, 65);
	memset(o_key, 0, 65);
	if (auth_url.pass().length() > 64) {
		KMD5 *kmd5 = new KMD5;
 		kmd5->update(auth_url.pass());
		kmd5->finalize();
		memcpy(i_key, kmd5->rawDigest(), 16);
		memcpy(o_key, kmd5->rawDigest(), 16);
		delete kmd5;
	} else {
		memcpy(i_key, auth_url.pass().latin1(), auth_url.pass().length());
		memcpy(o_key, auth_url.pass().latin1(), auth_url.pass().length());
	}

	for (i=0; i < 64; i++) {
		i_key[i] ^= 0x36;
		o_key[i] ^= 0x5c;
	}

	imd5=new KMD5;
	imd5->update((unsigned char *)i_key, 64);
	imd5->update(challenge);
	imd5->finalize();

	md5= new KMD5;
	md5->update(reinterpret_cast<unsigned char *>(o_key), 64);
	md5->update(imd5->rawDigest(), 16);
	md5->finalize();
	ret=auth_url.user();
	ret+=QString(" ")+md5->hexDigest();

	delete imd5;
	delete md5;
	return ret;
}

// Module factory stuff.. should be automated
CramAuthModuleFactory::CramAuthModuleFactory (QObject *parent, const char *name)
	: KLibFactory (parent, name)
{
	s_instance = new KInstance( "authcrammodule" );
}

CramAuthModuleFactory::~CramAuthModuleFactory()
{
	delete s_instance;
}

QObject *CramAuthModuleFactory::create (QObject *parent, const char *name, const char *, const QStringList &) {
	QObject *obj = new CramAuthModule (parent, name);
	emit objectCreated( obj );
	return obj;
}

KInstance *CramAuthModuleFactory::instance()
{
	return s_instance;
}

extern "C" {
        void *init_ksasl_auth_cram() {
                return new CramAuthModuleFactory;
        }
}

