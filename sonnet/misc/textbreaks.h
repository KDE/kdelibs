/*  This file is part of the KDE libraries
    Copyright (c) 2006 Jacob R Rideout <kde@jacobrideout.net>

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

#ifndef TEXTBREAKS
#define TEXTBREAKS

#include <QList>
#include <QString>
#include <QHash>

class textBreaks
{
public:

    typedef QList<int> Positions;

    textBreaks( const QString & text = QString() );
    QString text() const;
    void setText( const QString & text );
    static Positions graphemeBreaks( const QString & text );
    static Positions wordBreaks( const QString & text );
    static Positions sentenceBreaks( const QString & text );
    Positions graphemeBreaks( ) const;
    Positions wordBreaks( ) const;
    Positions sentenceBreaks( ) const;

private:
    static void init();

    QString m_text;
    static QHash<quint32,qint8> data;
    static QHash<QString,qint8> catalog;
};
#endif
