// $Id$

#include <sys/types.h>
#include <sys/uio.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream.h>

#include <qfile.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include <kmdbase.h>
#include <kdebug.h>
#include <kurl.h>

#include "auth_digest.h"

void GetA2(const QString &qop, const QString &uri, KMD5 &A2);
void GetA1(const QString &user, const QString &realm, const QString &pass, const QString &nonce, const QString &cnonce, KMD5 &A1);

DigestAuthModule::DigestAuthModule (QObject *parent, const char *name)
	: KSASLAuthModule (parent, name)
{
}

QString DigestAuthModule::auth_method()
{
	return QString::fromLatin1("DIGEST-MD5");
}

QString DigestAuthModule::auth_response(const QString &challenge, const KURL &auth_url)
{
	UseAuthString(challenge);
	char nc[9];
	QString uri, response;
	KMD5 A1, A2, FINAL;

	GetA1(auth_url.user(), keys[QString::fromLatin1("realm")], auth_url.pass(), keys[QString::fromLatin1("nonce")], QString(QString::fromLatin1(cnonce)), A1);

	uri=auth_url.protocol()+QChar('/')+auth_url.host()+QChar('/')+auth_url.host();
	GetA2(QString::fromLatin1("auth"), uri, A2);

        FINAL.update((unsigned char *)A1.hexDigest(), 32);
        FINAL.update((unsigned char *)":", 1);
        FINAL.update((unsigned char *)keys[QString::fromLatin1("nonce")].latin1(), strlen(keys[QString::fromLatin1("nonce")].latin1()));
        FINAL.update((unsigned char *)":", 1);
        sprintf(nc, "%.8d", ++nonce_count);
        FINAL.update((unsigned char *)nc, strlen(nc));
        FINAL.update((unsigned char *)":", 1);
        FINAL.update((unsigned char *)cnonce, strlen(cnonce));
        FINAL.update((unsigned char *)":", 1);
        FINAL.update((unsigned char *)"auth", strlen("auth"));
        FINAL.update((unsigned char *)":", 1);
        FINAL.update((unsigned char *)A2.hexDigest(), 32);
        FINAL.finalize();

	response=QString::fromLatin1("charset=utf-8,");
	response+=QString::fromLatin1("username=\"")+auth_url.user()+QString::fromLatin1("\",");
	response+=QString::fromLatin1("realm=\"")+keys[QString::fromLatin1("realm")]+QString::fromLatin1("\",");
	response+=QString::fromLatin1("nonce=\"")+keys[QString::fromLatin1("nonce")]+QString::fromLatin1("\",");
	response+=QString::fromLatin1("nc=")+QString::fromLatin1(nc)+QChar(',');
	response+=QString::fromLatin1("cnonce=\"")+QString::fromLatin1(cnonce)+QString::fromLatin1("\",");
	response+=QString::fromLatin1("digest-uri=\"")+uri+QString::fromLatin1("\",");
	response+=QString::fromLatin1("response=")+QString::fromLatin1(FINAL.hexDigest())+QChar(',');
	response+=QString::fromLatin1("qop=auth");
	return response;
}

void GetA1(const QString &user, const QString &realm, const QString &pass, const QString &nonce, const QString &cnonce, KMD5 &A1)
{
	KMD5 *md5;

	md5 = new KMD5;
	md5->update((unsigned char *)user.latin1(), strlen(user.latin1()));
	md5->update((unsigned char *)":", 1);
	md5->update((unsigned char *)realm.latin1(), strlen(realm.latin1()));
	md5->update((unsigned char *)":", 1);
	md5->update((unsigned char *)pass.latin1(), strlen(pass.latin1()));
	md5->finalize();

	A1.update(md5->raw_digest(), 16);
	A1.update((unsigned char *)":", 1);
	A1.update((unsigned char *)nonce.latin1(), strlen(nonce.latin1()));
	A1.update((unsigned char *)":", 1);
	A1.update((unsigned char *)cnonce.latin1(), strlen(cnonce.latin1()));
	A1.finalize();
	delete md5;
}

void GetA2(const QString &qop, const QString &uri, KMD5 &A2)
{
	A2.update((unsigned char *)"AUTHENTICATE", strlen("AUTHENTICATE"));
	A2.update((unsigned char *)":", 1);
	A2.update((unsigned char *)uri.latin1(), strlen(uri.latin1()));
	if ( (qop == "auth-int") ||  (qop == "auth-conf") ) {
		char buf[34];
		sprintf(buf, ":%.32d", 0);
		A2.update((unsigned char *)buf, strlen(buf));
	}
	A2.finalize();
}

void DigestAuthModule::Reset()
{
	memset(&cnonce, 0, 128);
	QFile f("/dev/random");
	if (f.open(IO_ReadOnly)) {
		::read(f.handle(), cnonce, 127); // Read up to 127 bits of randomness
		f.close();
	} else {
		sprintf(cnonce, "%s", "AHHYOUNEEDRANDOMINYOURLIFE");
	}
	keys.clear();
	nonce_count=0;
}

void DigestAuthModule::AddKey(const QString &k, const QString &v, QMap<QString, QString> &m)
{
	kdDebug() << "Adding " << k.latin1() << ":" << v.latin1() << endl;
	m.insert(k, v);
}

void DigestAuthModule::AddKey(const QString &line, QMap<QString, QString> &)
{
	bool open_quote=false;
	unsigned int i=0;
	QString authstr(line);
	while (authstr.length() >= i) {
		if (authstr.at(i) == '\"')
			if (open_quote) open_quote=false;
			else open_quote=true;
		else if (authstr.at(i) == '=') {
			if (!open_quote) {
				QString left, right;
			left=authstr.mid(0, i);
			right=authstr.mid(i+1, authstr.length());
			if (right.at(0) == '\"')
				right=right.mid(1, right.length());
			if (right.at(right.length()-1) == '\"' )
				right=right.mid(0, right.length()-1);
			AddKey(left, right, keys);	
			}
		}
		i++;
	}
}

void DigestAuthModule::UseAuthString(const QString &challenge)
{
	bool open_quote=false;
	unsigned int i=0, comma=0;
	while (challenge.length() >= i) {
		if (challenge.at(i) == '\"') {
			if (open_quote) open_quote=false;
			else open_quote=true;
		} else if (challenge.at(i) == ',') {
			if (!open_quote) {
				AddKey(challenge.mid(comma, i-comma), keys);
				comma=i+1;
				if (challenge.find(",", i+1) == -1) {
					AddKey(challenge.mid(i+1, challenge.length()), keys);
					break;
				}
			}
		}
		i++;
	}
}
