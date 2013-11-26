/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
              (C) 2007 John Layt <john@layt.net>
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

#include "kdatevalidator.h"

#include <QDate>
#include <QLocale>

class KDateValidator::KDateValidatorPrivate
{
public:
    KDateValidatorPrivate( KDateValidator *q ): q( q )
    {
    }

    ~KDateValidatorPrivate()
    {
    }

    KDateValidator *q;
};

KDateValidator::KDateValidator( QObject *parent ) : QValidator( parent ), d( 0 )
{
}

QValidator::State KDateValidator::validate( QString &text, int &unused ) const
{
    Q_UNUSED( unused );

    QDate temp;
    // ----- everything is tested in date():
    return date( text, temp );
}

QValidator::State KDateValidator::date( const QString &text, QDate &d ) const
{
    QLocale::FormatType formats[] = { QLocale::LongFormat, QLocale::ShortFormat, QLocale::NarrowFormat };
    QLocale locale;

    for (int i = 0; i < 3; i++) {
        QDate tmp = locale.toDate(text, formats[i]);
        if (d.isValid()) {
            d = tmp;
            return Acceptable;
        }
    }

    return QValidator::Intermediate;
}

void KDateValidator::fixup( QString& ) const
{
}

