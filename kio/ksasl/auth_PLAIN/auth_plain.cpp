// $Id$

#include <qstring.h>

#include <kdebug.h>

#include "../saslmodule.h"

DECLARE_SASL_MODULE("PLAIN", PlainAuth);

QString base64_encode_string(const QString &string);
QString base64_encode_auth_line(const QString &username, const QString &pass);

PlainAuth::PlainAuth()
	: KSASLAuthModule()
{
}

PlainAuth::~PlainAuth()
{
}

QString PlainAuth::auth_response(const QString &, const KURL &auth_url)
{
	return base64_encode_auth_line(auth_url.user(), auth_url.pass());
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
QString base64_encode_string( const QString &buf)
{
  char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  QString out;
  int inPos  = 0;
  int c1, c2, c3;
  unsigned int i;
  unsigned int len=buf.length();

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

QString base64_encode_auth_line(const QString &username, const QString &pass)
{
	QString ret, line;
	line.append(username);
	line.append(QChar(static_cast<char>(0)));
	line.append(username);
	line.append(QChar(static_cast<char>(0)));
	line.append(pass);
        ret=base64_encode_string(line);
        return ret;
}
