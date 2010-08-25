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

KLocaleWindowsPrivate::KLocaleWindowsPrivate( KLocale *q_ptr, const QString &catalog, KConfig *config,
                                              const QString &language, const QString &country )
                      :KLocalePrivate( q_ptr, catalog, config, language, country )
{
    // Lock in the current Windows Locale ID
    // Can we also lock in the actual settings like we do for Mac?
    m_winLocaleId = GetUserDefaultLCID();
    init( config );
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
        LPWSTR buffer;
        strcpy( m_win32SystemEncoding, "cp " );
        if ( GetLocaleInfoW( MAKELCID( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT ), SORT_DEFAULT ),
                             LOCALE_IDEFAULTANSICODEPAGE, buffer, sizeof( buffer ) ) ) {
            QString localestr = QString::fromUtf16( (const ushort*) buffer ) ;
            QByteArray localechar = localestr.toAscii();
            strcpy( m_win32SystemEncoding, localechar.data() + 3 );
            return m_win32SystemEncoding;
        }
    }
    return KLocalePrivate::encoding();
}
