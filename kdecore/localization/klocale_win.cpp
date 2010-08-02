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

#include <windows.h>

#include <QtCore/QLocale>
#include <QtCore/QTextCodec>

KLocaleWindowsPrivate::KLocaleWindowsPrivate( KLocale *q_ptr, const QString &catalog, KConfig *config,
                                              const QString &language, const QString &country )
                      :KLocalePrivate( q_ptr, catalog, config, language, country )
{
}

KLocaleWindowsPrivate::KLocaleWindowsPrivate( const KLocaleWindowsPrivate &rhs )
                      :KLocalePrivate( rhs )
{
    KLocalePrivate::copy( rhs );
    strcpy( m_win32SystemEncoding, rhs.m_win32SystemEncoding );
}

KLocaleWindowsPrivate &KLocaleWindowsPrivate::operator=( const KLocaleWindowsPrivate &rhs )
{
    KLocalePrivate::copy( rhs );
    strcpy( m_win32SystemEncoding, rhs.m_win32SystemEncoding );
    return *this;
}

KLocaleWindowsPrivate::~KLocaleWindowsPrivate()
{
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
