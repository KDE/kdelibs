/* This file is part of the KDE libraries
    Copyright (C) 2004,2005,2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2004,2005,2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2004,2005,2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2004,2005,2006 Erik L. Bunce <kde@bunce.us>

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

#ifndef KJSEGLOBAL_H
#define KJSEGLOBAL_H

#include <kjsembed_export.h>
#include <qglobal.h>

#if !defined(Q_OS_WIN)
#include <stdlib.h>
#include <ctype.h>

KJSEMBED_EXPORT void RedirectIOToConsole();

#endif

#include <QtCore/QTextStream>
namespace KJSEmbed {
KJSEMBED_EXPORT QTextStream *conin();
KJSEMBED_EXPORT QTextStream *conout();
KJSEMBED_EXPORT QTextStream *conerr();
}

#include <kjs/ustring.h>
#include <kjs/identifier.h>
#include <kjs/value.h>

namespace KJSEmbed {
	inline QString toQString(const KJS::UString& u)
	{
		return QString((QChar*)u.data(), u.size());
	}
	inline QString toQString(const KJS::Identifier& i)
	{
		return QString((QChar*)i.data(), i.size());
	}
	inline KJS::UString toUString(const QString& qs)
	{
		return KJS::UString((KJS::UChar*)qs.data(), qs.size());
	}
}

namespace KJS {
	inline KJS::JSCell* jsString(const QString& s)
	{
		return jsString(KJSEmbed::toUString(s));
	}
}

#ifndef QT_ONLY

/*
 * These are the normal definitions used when KDE is available.
 */

#include <QDebug>
#include <klocalizedstring.h>

#else // QT_ONLY

/*
 * These are the custom definitions used when we only have Qt.
 */

KJSEMBED_EXPORT QTextStream &kdDebug( int area=0 );
KJSEMBED_EXPORT QTextStream &kdWarning( int area=0 );

#ifndef NO_I18N
KJSEMBED_EXPORT QString i18n( const char *string );
#else
#define i18n(x) QString(x)
#endif // NO_I18N

inline KJSEMBED_EXPORT QString i18n( const QString& string, const QString& comment )
{
    return i18n(string.toUtf8().data(), comment.toUtf8().data());
}
template <typename A1>
inline QString i18n (const char *text, const A1 &a1)
{
    return i18n(text).arg(a1);
}
template <typename A1, typename A2>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2)
{
    return i18n(text).arg(a1).arg(a2);
}
template <typename A1, typename A2, typename A3>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return i18n(text).arg(a1).arg(a2).arg(a3);
}
template <typename A1, typename A2, typename A3, typename A4>
inline QString i18n (const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return i18n(text).arg(a1).arg(a2).arg(a3).arg(a4);
}

#endif // QT_ONLY

#endif // KJSEGLOBAL_H

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
