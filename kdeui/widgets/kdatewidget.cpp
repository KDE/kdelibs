/*  This file is part of the KDE libraries
    Copyright (C) 2001 Waldo Bastian (bastian@kde.org)
    Copyright (c) 2007 John Layt <john@layt.net>
 
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
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleSpinBox>

#include <kcombobox.h>

#include "kcalendarsystem.h"
#include "kdialog.h"
#include "kglobal.h"
#include "klocale.h"


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
    QDate m_dat;
    KCalendarSystem *m_cal;
};


KDateWidget::KDateWidget( QWidget *parent ) : QWidget( parent ), d( new KDateWidgetPrivate )
{
    init( QDate() );
    setDate( QDate::currentDate() );
}

KDateWidget::KDateWidget( const QDate &date, QWidget *parent )
            : QWidget( parent ), d( new KDateWidgetPrivate )
{
    init( date );
    if ( ! setDate( date ) ) {
        setDate( QDate::currentDate() );
    }
}

void KDateWidget::init( const QDate &date )
{
    //set calendar system to default, i.e. global
    setCalendar();
    //make sure date is valid in calendar system
    QDate initDate;
    if ( calendar()->isValid( date ) ) {
        initDate = date;
    } else {
        initDate = QDate::currentDate();
    }

    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( KDialog::spacingHint() );
	// set the maximum day value in the day field, so that the day can
	// be editted when the KDateWidget is constructed with an empty date
    d->m_day = new KDateWidgetSpinBox( 1, 31, this );
    d->m_month = new KComboBox( this );
    d->m_month->setMaxVisibleItems( 12 );

    for ( int i = 1; ; ++i ) {
        const QString str = calendar()->monthName( i, calendar()->year( initDate ) );
        if ( str.isEmpty() ) {
            break;
        }
        d->m_month->addItem( str );
    }

    d->m_year = new KDateWidgetSpinBox( calendar()->year( calendar()->earliestValidDate() ),
                                        calendar()->year( calendar()->latestValidDate() ), this );
    layout->addWidget( d->m_day );
    layout->addWidget( d->m_month );
    layout->addWidget( d->m_year );

    connect( d->m_day, SIGNAL( valueChanged( int ) ), this, SLOT( slotDateChanged() ) );
    connect( d->m_month, SIGNAL( activated( int ) ), this, SLOT( slotDateChanged() ) );
    connect( d->m_year, SIGNAL( valueChanged( int ) ), this, SLOT( slotDateChanged() ) );

    setFocusProxy(d->m_day);
    setFocusPolicy(Qt::StrongFocus);

    d->m_dat = initDate;
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

        d->m_day->setMaximum( calendar()->daysInMonth( date ) );
        d->m_day->setValue( calendar()->day( date ) );
        d->m_month->setCurrentIndex( calendar()->month( date ) - 1 );
        d->m_year->setValue( calendar()->year( date ) );

        d->m_day->blockSignals( dayBlocked );
        d->m_month->blockSignals( monthBlocked );
        d->m_year->blockSignals( yearBlocked );

        d->m_dat = date;
        emit changed( d->m_dat );
        return true;
    }
    return false;
}

const QDate& KDateWidget::date() const
{
    return d->m_dat;
}

void KDateWidget::slotDateChanged( )
{
    QDate date;
    int y, m, day;

    y = d->m_year->value();
    y = qMin( qMax( y, calendar()->year( calendar()->earliestValidDate() ) ),
              calendar()->year( calendar()->latestValidDate() ) );

    calendar()->setYMD( date, y, 1, 1 );
    m = d->m_month->currentIndex() + 1;
    m = qMin( qMax( m, 1 ), calendar()->monthsInYear( date ) );

    calendar()->setYMD( date, y, m, 1 );
    day = d->m_day->value();
    day = qMin( qMax( day, 1 ), calendar()->daysInMonth( date ) );

    calendar()->setYMD( date, y, m, day );
    setDate( date );
}

const KCalendarSystem *KDateWidget::calendar() const
{
    if ( d->m_cal ) {
        return d->m_cal;
    }

    return  KGlobal::locale()->calendar();
}

bool KDateWidget::setCalendar( KCalendarSystem *calendar )
{
    d->m_cal = calendar;
    return true;
}

bool KDateWidget::setCalendar( const QString &calendarType )
{
    d->m_cal = KCalendarSystem::create( calendarType );
    return d->m_cal;
}


#include "kdatewidget.moc"
