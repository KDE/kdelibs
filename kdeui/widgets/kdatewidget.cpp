/*  This file is part of the KDE libraries
    Copyright (C) 2001 Waldo Bastian (bastian@kde.org)
    Copyright 2007, 2010 John Layt <john@layt.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdatewidget.h"

#include <QtCore/QDate>
#include <QLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>

#include <kcombobox.h>

#include "kcalendarsystem.h"
#include "klocalizeddate.h"


class KDateWidgetSpinBox : public QSpinBox
{
public:
    KDateWidgetSpinBox( int min, int max, QWidget *parent ) : QSpinBox( parent )
    {
        setRange( qMin( min, max ), qMax( min, max ) );
        setSingleStep( 1 );
        lineEdit()->setAlignment( Qt::AlignRight );
    }
};

class KDateWidget::KDateWidgetPrivate
{
public:
    KDateWidgetSpinBox *m_day;
    KComboBox *m_month;
    KDateWidgetSpinBox *m_year;
    KLocalizedDate m_date;
    // Need to keep a QDate copy as the "const QDate &date() const;" method returns a reference
    // and returning m_date.date() creates a temporary leading to crashes.  Doh!
    QDate m_refDate;
};


KDateWidget::KDateWidget( QWidget *parent ) : QWidget( parent ), d( new KDateWidgetPrivate )
{
    initWidget( QDate::currentDate() );
}

KDateWidget::KDateWidget( const QDate &date, QWidget *parent )
            : QWidget( parent ), d( new KDateWidgetPrivate )
{
    initWidget( date );
}

void KDateWidget::initWidget( const QDate &date )
{
    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );
    const int spacingHint = style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    layout->setSpacing( spacingHint );

    d->m_day = new KDateWidgetSpinBox( 1, 31, this );
    d->m_month = new KComboBox( this );
    d->m_year = new KDateWidgetSpinBox( calendar()->year( calendar()->earliestValidDate() ),
                                        calendar()->year( calendar()->latestValidDate() ), this );

    layout->addWidget( d->m_day );
    layout->addWidget( d->m_month );
    layout->addWidget( d->m_year );

    connect( d->m_day, SIGNAL(valueChanged(int)), this, SLOT(slotDateChanged()) );
    connect( d->m_month, SIGNAL(activated(int)), this, SLOT(slotDateChanged()) );
    connect( d->m_year, SIGNAL(valueChanged(int)), this, SLOT(slotDateChanged()) );

    setFocusProxy(d->m_day);
    setFocusPolicy(Qt::StrongFocus);

    if ( calendar()->isValid( date ) ) {
        setDate( date );
    } else {
        setDate( QDate::currentDate() );
    }
}

KDateWidget::~KDateWidget()
{
    delete d;
}

bool KDateWidget::setDate( const QDate &date )
{
    if ( calendar()->isValid( date ) ) {
        bool dayBlocked = d->m_day->blockSignals( true );
        bool monthBlocked = d->m_month->blockSignals( true );
        bool yearBlocked = d->m_year->blockSignals( true );

        d->m_date.setDate( date );
        d->m_refDate = date;

        d->m_day->setMaximum( d->m_date.daysInMonth() );
        d->m_day->setValue( d->m_date.day() );

        d->m_month->clear();
        d->m_month->setMaxVisibleItems( d->m_date.monthsInYear() );
        for ( int m = 1; m <= d->m_date.monthsInYear(); ++m ) {
            d->m_month->addItem( calendar()->monthName( m, d->m_date.year() ) );
        }
        d->m_month->setCurrentIndex( d->m_date.month() - 1 );

        d->m_year->setValue( d->m_date.year() );

        d->m_day->blockSignals( dayBlocked );
        d->m_month->blockSignals( monthBlocked );
        d->m_year->blockSignals( yearBlocked );

        emit changed( d->m_refDate );
        return true;
    }
    return false;
}

const QDate& KDateWidget::date() const
{
    return d->m_refDate;
}

void KDateWidget::slotDateChanged( )
{
    KLocalizedDate date;
    int y, m, day;

    y = d->m_year->value();
    y = qMin( qMax( y, calendar()->year( calendar()->earliestValidDate() ) ),
              calendar()->year( calendar()->latestValidDate() ) );

    date.setDate( y, 1, 1 );
    m = d->m_month->currentIndex() + 1;
    m = qMin( qMax( m, 1 ), date.monthsInYear() );

    date.setDate( y, m, 1 );
    day = d->m_day->value();
    day = qMin( qMax( day, 1 ), date.daysInMonth() );

    date.setDate( y, m, day );
    setDate( date.date() );
}

const KCalendarSystem *KDateWidget::calendar() const
{
    return  d->m_date.calendar();
}

bool KDateWidget::setCalendar( KCalendarSystem *newCalendar )
{
    QDate oldDate = date();
    d->m_date = KLocalizedDate( oldDate, newCalendar );
    return setDate( oldDate );
}

bool KDateWidget::setCalendarSystem( KLocale::CalendarSystem newCalendarSystem )
{
    d->m_date.setCalendarSystem( newCalendarSystem );
    return true;
}

