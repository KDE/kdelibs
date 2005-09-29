/*
    This file is part of the KDE libraries

    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kidna.h"

#ifndef Q_WS_WIN //TODO kresolver not ported
#include "kresolver.h"
#endif
#include <kdebug.h>

#ifndef Q_WS_WIN //TODO knetwork not ported
using namespace KNetwork;
#endif

QCString KIDNA::toAsciiCString(const QString &idna)
{
#ifndef Q_WS_WIN //TODO kresolver not ported
	return KResolver::domainToAscii(idna);
#else
	return QCString();
#endif
}

QString KIDNA::toAscii(const QString &idna)
{
  if (idna.length() && (idna[0] == "."))
  {
     QString host = QString::fromLatin1(toAsciiCString(idna.mid(1)));
     if (host.isEmpty())
        return QString::null; // Error
     return idna[0] + host;
  }
  return QString::fromLatin1(toAsciiCString(idna));
}

QString KIDNA::toUnicode(const QString &idna)
{
#ifndef Q_WS_WIN //TODO kresolver not ported
  if (idna.length() && (idna[0] == "."))
     return idna[0] + KResolver::domainToUnicode(idna.mid(1));
  return KResolver::domainToUnicode(idna);
#else
	return QString::null;
#endif
}
