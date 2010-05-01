/*
    Copyright 2010 John Layt <john@layt.net>

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

#include "kcalendarera_p.h"
#include "kcalendarsystem.h"
#include "kcalendarsystemprivate_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>

KCalendarEra::KCalendarEra()
{
}

KCalendarEra::~KCalendarEra()
{
}

bool KCalendarEra::isValid() const
{
    return m_startDate.isValid() &&
           m_startDate.isValid() &&
           !m_name.isEmpty() &&
           !m_shortName.isEmpty() &&
           !m_format.isEmpty();
}

QDate KCalendarEra::startDate() const
{
    return m_startDate;
}

QDate KCalendarEra::endDate() const
{
    return m_endDate;
}

QString KCalendarEra::name() const
{
    return m_name;
}

QString KCalendarEra::shortName() const
{
    return m_shortName;
}

QString KCalendarEra::format() const
{
    return m_format;
}

int KCalendarEra::direction() const
{
    return m_direction;
}

bool KCalendarEra::isInEra( const QDate &date ) const
{
    if ( m_endDate < m_startDate ) {
        return ( date >= m_endDate && date <= m_startDate );
    } else {
        return ( date >= m_startDate && date <= m_endDate );
    }
}

int KCalendarEra::yearInEra( int year ) const
{
    return ( ( year - m_startYear ) * m_direction ) + m_offset;
}

int KCalendarEra::year( int yearInEra ) const
{
    return ( ( yearInEra - m_offset ) / m_direction ) + m_startYear;
}
