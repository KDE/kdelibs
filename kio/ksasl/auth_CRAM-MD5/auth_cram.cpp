// $Id$

#define QSTRING_TO_MD5(x) reinterpret_cast<unsigned char *>(const_cast<char *>(x.latin1()))

#include <stdlib.h>

#include <iostream.h>

#include <qstring.h>

#include <kdebug.h>

#include "../saslmodule.h"

DECLARE_SASL_MODULE("CRAM-MD5", CramAuth);

CramAuth::CramAuth()
	: KSASLAuthModule()
{
}

CramAuth::~CramAuth()
{
}

#include "local_md5.h"

QString CramAuth::auth_response(const QString &challenge, const KURL &auth_url)
{
	char i_key[65], o_key[65];
	int i;
	MD5 *imd5, *md5;
	QString ret;

	memset(i_key, 0, 65);
	memset(o_key, 0, 65);
	if (auth_url.pass().length() > 64) {
		MD5 *kmd5 = new MD5;
 		kmd5->update(QSTRING_TO_MD5(auth_url.pass()), auth_url.pass().length());
		kmd5->finalize();
		memcpy(i_key, kmd5->raw_digest(), 16);
		memcpy(o_key, kmd5->raw_digest(), 16);
		delete kmd5;
	} else {
		memcpy(i_key, auth_url.pass().latin1(), auth_url.pass().length());
		memcpy(o_key, auth_url.pass().latin1(), auth_url.pass().length());
	}

	for (i=0; i < 64; i++) {
		i_key[i] ^= 0x36;
		o_key[i] ^= 0x5c;
	}

	imd5=new MD5;
	imd5->update((unsigned char *)i_key, 64);
	imd5->update(QSTRING_TO_MD5(challenge), challenge.length());
	imd5->finalize();

	md5= new MD5;
	md5->update(reinterpret_cast<unsigned char *>(o_key), 64);
	md5->update(imd5->raw_digest(), 16);
	md5->finalize();
	ret=auth_url.user();
	ret+=QString(" ")+md5->hex_digest();

	delete imd5;
	delete md5;
	return ret;
}
