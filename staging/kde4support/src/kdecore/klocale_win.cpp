/*  This file is part of the KDE libraries
 *  Copyright 2005, 2008 Jaroslaw Staniek <staniek@kde.org>
 *  Copyright 2010 John Layt <john@layt.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "klocale_win_p.h"

#include <QtCore/QLocale>
#include <QtCore/QTextCodec>

KLocaleWindowsPrivate::KLocaleWindowsPrivate(KLocale *q_ptr, KSharedConfig::Ptr config)
                      :KLocalePrivate(q_ptr)
{
    // Lock in the current Windows Locale ID
    // Can we also lock in the actual settings like we do for Mac?
    m_winLocaleId = GetUserDefaultLCID();
    init(QString(), QString(), config, 0);
}

KLocaleWindowsPrivate::KLocaleWindowsPrivate(KLocale *q_ptr,
                                             const QString &language, const QString &country, KConfig *config)
                      :KLocalePrivate(q_ptr)
{
    // Lock in the current Windows Locale ID
    // Can we also lock in the actual settings like we do for Mac?
    m_winLocaleId = GetUserDefaultLCID();
    init(language, country, KSharedConfig::Ptr(), config);
}

KLocaleWindowsPrivate::KLocaleWindowsPrivate( const KLocaleWindowsPrivate &rhs )
                      :KLocalePrivate( rhs )
{
    KLocalePrivate::copy( rhs );
    m_winLocaleId = rhs.m_winLocaleId;
    strcpy( m_win32SystemEncoding, rhs.m_win32SystemEncoding );
}

KLocaleWindowsPrivate &KLocaleWindowsPrivate::operator=( const KLocaleWindowsPrivate &rhs )
{
    KLocalePrivate::copy( rhs );
    m_winLocaleId = rhs.m_winLocaleId;
    strcpy( m_win32SystemEncoding, rhs.m_win32SystemEncoding );
    return *this;
}

KLocaleWindowsPrivate::~KLocaleWindowsPrivate()
{
}

QString KLocaleWindowsPrivate::windowsLocaleValue( LCTYPE key ) const
{
    // Find out how big the buffer needs to be
    int size = GetLocaleInfoW( m_winLocaleId, key, 0, 0 );

    QString result;
    if ( size ) {
        wchar_t* buffer = new wchar_t[size];
        if ( GetLocaleInfoW( m_winLocaleId, key, buffer, size ) )
            result = QString::fromWCharArray( buffer );
        delete[] buffer;
    }
    return result;
}

QString KLocaleWindowsPrivate::systemCountry() const
{
    return windowsLocaleValue( LOCALE_SISO3166CTRYNAME );
}

QStringList KLocaleWindowsPrivate::systemLanguageList() const
{
    QStringList list;
    getLanguagesFromVariable( list, QLocale::system().name().toLocal8Bit().data() );
    return list;
}

QByteArray KLocaleWindowsPrivate::systemCodeset() const
{
    return QByteArray();
}

const QByteArray KLocaleWindowsPrivate::encoding()
{
    if ( qstrcmp( codecForEncoding()->name(), "System" ) == 0 ) {
        //win32 returns "System" codec name here but KDE apps expect a real name:
        strcpy( m_win32SystemEncoding, "cp " );
        // MSDN says the returned string for LOCALE_IDEFAULTANSICODEPAGE is max 6 char including '\0'
        char buffer[6];
        if ( GetLocaleInfoA( MAKELCID( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT ), SORT_DEFAULT ),
                             LOCALE_IDEFAULTANSICODEPAGE, buffer, sizeof( buffer ) ) ) {
            strcpy( m_win32SystemEncoding + 3, buffer );
            return m_win32SystemEncoding;
        }
    }
    return KLocalePrivate::encoding();
}

/*
These functions are commented out for now until all required Date/Time functions are implemented
to ensure consistent behaviour, i.e. all KDE format or all Windows format, not some invalid mixture

void KLocaleMacPrivate::initDayPeriods(const KConfigGroup &cg)
{
    QString amText = windowsLocaleValue(LOCALE_S1159);
    QString pmText = windowsLocaleValue(LOCALE_S2359);

    m_dayPeriods.clear();
    m_dayPeriods.append(KDayPeriod("am", amText, amText, amText
                                   QTime(0, 0, 0), QTime(11, 59, 59, 999), 0, 12));
    m_dayPeriods.append(KDayPeriod("pm", pmText, pmText, pmText,
                                   QTime(12, 0, 0), QTime(23, 59, 59, 999), 0, 12));
}

*/
