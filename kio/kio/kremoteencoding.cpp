/* This file is part of the KDE libraries
   Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <kdebug.h>
#include <kstringhandler.h>
#include "kremoteencoding.h"

KRemoteEncoding::KRemoteEncoding(const char *name)
  : codec(0L), d(0L)
{
  setEncoding(name);
}

KRemoteEncoding::~KRemoteEncoding()
{
  // delete d;		// not necessary yet
}

QString KRemoteEncoding::decode(const QByteArray& name) const
{
#ifdef CHECK_UTF8
  if (codec->mibEnum() == 106 && !KStringHandler::isUtf8(name))
    return QLatin1String(name);
#endif

  QString result = codec->toUnicode(name);
  if (codec->fromUnicode(result) != name)
    // fallback in case of decoding failure
    return QLatin1String(name);

  return result;
}

QByteArray KRemoteEncoding::encode(const QString& name) const
{
  QByteArray result = codec->fromUnicode(name);
  if (codec->toUnicode(result) != name)
    return name.latin1();
 
  return result;
}

QByteArray KRemoteEncoding::encode(const KURL& url) const
{
  return encode(url.path());
}

QByteArray KRemoteEncoding::directory(const KURL& url, bool ignore_trailing_slash) const
{
  QString dir = url.directory(true, ignore_trailing_slash);

  return encode(dir);
}

QByteArray KRemoteEncoding::fileName(const KURL& url) const
{
  return encode(url.fileName());
}

void KRemoteEncoding::setEncoding(const char *name)
{
  // don't delete codecs

  if (name)
    codec = QTextCodec::codecForName(name);

  if (codec == 0L)
    codec = QTextCodec::codecForMib(1);

  kdDebug() << k_funcinfo << "setting encoding " << codec->name() 
	    << " for name=" << name << endl;
}

void KRemoteEncoding::virtual_hook(int, void*)
{
}
