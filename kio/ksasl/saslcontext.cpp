// $Id$

#include <ctype.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream.h>

#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>

#include "saslcontext.h"
#include "saslmodule.h"

typedef KSASLAuthModule* (*init_proto)(void);
typedef unsigned int (*ver_proto)(void);

QString base64_encode_string (const char *buf, unsigned int len);
QString base64_decode_string (const char *string, unsigned int len);

KSASLContext::KSASLContext()
{
	loadModule("./auth_plain.so");
	loadModule("./auth_cram.so");
}

QString KSASLContext::chooseMethod (const QString &available, bool isBASE64)
{
	// It returns the selected authentication method as a QString
	QString operate_on;

	if (isBASE64) {
		operate_on=base64_decode_string(available.latin1(), available.length());
	} else
		operate_on=available;

	QStringList server_side = QStringList::split(QString(" "), operate_on);
	QStringList supported;

	// Go thru the ones that the server supports, and compare them to the ones we support
	for (QStringList::Iterator it1 = server_side.begin(); it1 != server_side.end(); ++it1 ) {
		for (unsigned int x = 0; x < modules.count(); x++) {
			if (modules.at(x)->auth_method() ==  *it1) {
				supported += *it1;
			}
		}
	}

	if (!supported.count())
		auth_method=QString::null;
	else if (supported.count() == 1)
		auth_method=supported.first();
	else
		auth_method=supported.last();
	return auth_method;
}

void KSASLContext::setURL (const KURL &url)
{
	auth_url=url;
}

QString KSASLContext::generateResponse (const QString &challenge, bool isBASE64)
{
	QString ret;
	unsigned int x;

	if (auth_method == QString::null)
		return QString::null;

	for (x = 0; x < modules.count(); x++) {
		if (modules.at(x)->auth_method() ==  auth_method) {
			break;
		}
	}
	ret=modules.at(x)->auth_response(challenge, auth_url);
	if (!isBASE64)
		return ret;
	else
		return base64_encode_string(ret.latin1(), ret.length());
}


bool KSASLContext::loadModule (const QString &mod_name)
{
	void *handle;
	init_proto mod_init;
	ver_proto mod_ver;
	handle=dlopen(mod_name.latin1(), RTLD_LAZY);
	if (!handle) {
		cerr << "nohandle: " << dlerror()<<  endl;
		return false;
	}
	mod_ver=(ver_proto)dlsym(handle, "module_version");
	if (!mod_ver) {
		dlclose(handle);
		kdDebug() << "No module_version symbol found" << endl;
		return false;
	}
	if (mod_ver() != KSASL_MODULE_REV) {
		dlclose(handle);
		kdDebug() << "Module is of an incompatible type" << endl;
		return false;
	}
	mod_init=(init_proto)dlsym(handle, "auth_init");
	if (!mod_init) {
		dlclose(handle);
		kdDebug() << "No auth_init symbol found" << endl;
		return false;
	}
	modules.append(mod_init());
	kdDebug() << modules.getLast()->auth_method() << endl;
	return true;
}

/*
 * Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)
 *
 * Permission to use, copy, modify, and distribute this material
 * for any purpose and without fee is hereby granted, provided
 * that the above copyright notice and this permission notice
 * appear in all copies, and that the name of Bellcore not be
 * used in advertising or publicity pertaining to this
 * material without the specific, prior written permission
 * of an authorized representative of Bellcore.  BELLCORE
 * MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
 * OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
 */
QString base64_encode_string( const char *buf, unsigned int len )
{
  char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  QString out;
  int inPos  = 0;
  int c1, c2, c3;
  unsigned int i;

  /* Get three characters at a time and encode them. */
  for (i=0; i < len/3; ++i) {
      c1 = buf[inPos++] & 0xFF;
      c2 = buf[inPos++] & 0xFF;
      c3 = buf[inPos++] & 0xFF;
      out+= basis_64[(c1 & 0xFC) >> 2];
      out+= basis_64[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
      out+= basis_64[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
      out+= basis_64[c3 & 0x3F];
  }

  /* Encode the remaining one or two characters. */

  switch (len % 3) {
      case 0:
          break;
      case 1:
          c1 = buf[inPos] & 0xFF;
          out+= basis_64[(c1 & 0xFC) >> 2];
          out+= basis_64[((c1 & 0x03) << 4)];
          out+= '=';
          out+= '=';
          break;
      case 2:
          c1 = buf[inPos++] & 0xFF;
          c2 = buf[inPos] & 0xFF;
          out+= basis_64[(c1 & 0xFC) >> 2];
          out+= basis_64[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
          out+= basis_64[((c2 & 0x0F) << 2)];
          out+= '=';
          break;
  }
  return out.copy();
}


/* decode file as MIME base64 (RFC 1341) by John Walker http://www.fourmilab.ch/ This program is in the public domain. */
QString base64_decode_string(const char *string, unsigned int len)
{
	unsigned char dtable[256];	      /* Encode / decode table */
	int errcheck = TRUE;	      /* Check decode input for errors ? */
	QString ret;
	unsigned int i,z=0;

	for (i = 0; i < 255; i++)
		dtable[i] = 0x80;
	for (i = 'A'; i <= 'Z'; i++)
		dtable[i] = 0 + (i - 'A');
	for (i = 'a'; i <= 'z'; i++)
		dtable[i] = 26 + (i - 'a');
	for (i = '0'; i <= '9'; i++)
		dtable[i] = 52 + (i - '0');

	dtable['+'] = 62;
	dtable['/'] = 63;
	dtable['='] = 0;

	/*CONSTANTCONDITION*/
	while (TRUE) {
		unsigned char a[4], b[4], o[3];

		for (i = 0; i < 4; i++) {
			int c;

			if (z >= len) {
				if (errcheck && (i > 0)) {
					kdDebug() << "Input file incomplete." << endl;
					return QString::null;
				}
				return ret;
			}
			c=string[z]; z++;
			if (dtable[c] & 0x80) {
				if (errcheck) {
					kdDebug() << "Illegal character '" << c << "' in input file." << endl;
					return QString::null;
				}
				/* Ignoring errors: discard invalid character. */
				i--;
				continue;
			}
			a[i] = (unsigned char) c;
			b[i] = (unsigned char) dtable[c];
		}
		o[0] = (b[0] << 2) | (b[1] >> 4);
		o[1] = (b[1] << 4) | (b[2] >> 2);
		o[2] = (b[2] << 6) | b[3];
		i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);
		for (unsigned int w=0; w < i; w++)
			ret.append(o[w]);
		if (i < 3) {
			return ret;
		}
	}
}
