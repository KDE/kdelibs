/* This file is part of the KDE libraries
   Copyright (C) 2010 John Layt <john@layt.net>

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

#include "klocale_mac_p.h"
#include <CoreFoundation/CFString.h>

// Copied from kkernel_mac.cpp
QString convert_CFString_to_QString(CFStringRef str) {
	CFIndex length = CFStringGetLength(str);
	const UniChar *chars = CFStringGetCharactersPtr(str);
	if (chars)
		return QString(reinterpret_cast<const QChar *>(chars), length);

	QVarLengthArray<UniChar> buffer(length);
	CFStringGetCharacters(str, CFRangeMake(0, length), buffer.data());
	return QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
}

KLocaleMacPrivate::KLocaleMacPrivate(KLocale *q_ptr, KSharedConfig::Ptr config)
                  :KLocalePrivate(q_ptr)
{
    // Lock in the current Mac Locale settings
    m_macLocale = CFLocaleCopyCurrent();
    init(QString(), QString(), config, 0);
}

KLocaleMacPrivate::KLocaleMacPrivate(KLocale *q_ptr,
                                     const QString &language, const QString &country, KConfig *config)
                  :KLocalePrivate(q_ptr)
{
    // Lock in the current Mac Locale settings
    m_macLocale = CFLocaleCopyCurrent();
    init(language, country, KSharedConfig::Ptr(), config);
}

KLocaleMacPrivate::KLocaleMacPrivate( const KLocaleMacPrivate &rhs )
                  :KLocalePrivate( rhs )
{
    KLocalePrivate::copy( rhs );
    m_macLocale = rhs.m_macLocale;
}

KLocaleMacPrivate &KLocaleMacPrivate::operator=( const KLocaleMacPrivate &rhs )
{
    KLocalePrivate::copy( rhs );
    m_macLocale = rhs.m_macLocale;
    return *this;
}

KLocaleMacPrivate::~KLocaleMacPrivate()
{
}

QString KLocaleMacPrivate::macLocaleValue( CFStringRef key ) const
{
    return convert_CFString_to_QString( CFStringRef( CFLocaleGetValue( m_macLocale, key ) ) );
}

QString KLocaleMacPrivate::systemCountry() const
{
    return macLocaleValue( kCFLocaleCountryCode );
}

QByteArray KLocaleMacPrivate::systemCodeset() const
{
    return QByteArray( "UTF-8" );
}

/*
These functions are commented out for now until all required Date/Time functions are implemented
to ensure consistent behaviour, i.e. all KDE format or all Mac format, not some invalid mixture

QString KLocaleMacPrivate::macDateFormatterValue(CFStringRef key) const
{
    CFDateFormatterRef formatter = CFDateFormatterCreate(kCFAllocatorDefault, m_macLocale, kCFDateFormatterNoStyle, kCFDateFormatterNoStyle);
    return convert_CFString_to_QString(CFStringRef(CFDateFormatterCopyProperty(formatter, key)));
}

void KLocaleMacPrivate::initDayPeriods(const KConfigGroup &cg)
{
    QString amText = macDateFormatterValue(kCFDateFormatterAMSymbol);
    QString pmText = macDateFormatterValue(kCFDateFormatterPMSymbol);

    m_dayPeriods.clear();
    m_dayPeriods.append(KDayPeriod("am", amText, amText, amText
                                   QTime(0, 0, 0), QTime(11, 59, 59, 999), 0, 12));
    m_dayPeriods.append(KDayPeriod("pm", pmText, pmText, pmText,
                                   QTime(12, 0, 0), QTime(23, 59, 59, 999), 0, 12));
}
*/
