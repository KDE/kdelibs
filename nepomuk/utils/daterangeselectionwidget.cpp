/*
   Copyright (c) 2009 Sebastian Trueg <trueg@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "daterangeselectionwidget.h"
#include "ui_daterangeselectionwidget.h"
#include "daterange.h"

#include <QtGui/QToolButton>
#include <QtGui/QCalendarWidget>
#include <QtGui/QButtonGroup>
#include <QtGui/QDateEdit>
#include <QtGui/QTextCharFormat>
#include <QtGui/QPalette>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtCore/QHash>


class DateRangeSelectionWidget::Private : public Ui::DateRangeSelectionWidgetBase
{
public:
    void _k_anytimeClicked();
    void _k_beforeClicked();
    void _k_afterClicked();
    void _k_rangeClicked();
    void _k_rangeClicked( QAction* );
    void _k_rangeStartEdited( const QDate& date );
    void _k_rangeEndEdited( const QDate& date );
    void _k_calendarDateClicked( const QDate& date );

    void checkButton( QAbstractButton* button );
    void setupPopupMenus();

    void updateCalendar( const DateRange& range );
    void updateEditBoxes( const DateRange& range );

    void setRange( const DateRange& range );

    QAction* findRangeAction( const DateRange& range );

    DateRange m_range;

    QAction* m_currentRangeAction;
    QHash<QAction*, DateRange> m_rangeActionMap;

    DateRangeSelectionWidget* q;
};


void DateRangeSelectionWidget::Private::_k_anytimeClicked()
{
    checkButton( m_anytimeButton );
    setRange( DateRange() );

    emit q->rangeChanged( m_range );
}


void DateRangeSelectionWidget::Private::_k_beforeClicked()
{
    if ( m_range.end().isValid() )
        setRange( DateRange( QDate(), m_range.end() ) );
    else if ( m_range.start().isValid() )
        setRange( DateRange( QDate(), m_range.start() ) );
    else
        setRange( DateRange( QDate(), QDate::currentDate() ) );

    emit q->rangeChanged( m_range );
}


void DateRangeSelectionWidget::Private::_k_afterClicked()
{
    if ( m_range.start().isValid() )
        setRange( DateRange( m_range.start(), QDate() ) );
    else if ( m_range.end().isValid() )
        setRange( DateRange( m_range.end(), QDate() ) );
    else
        setRange( DateRange( QDate::currentDate(), QDate() ) );

    emit q->rangeChanged( m_range );
}


void DateRangeSelectionWidget::Private::_k_rangeClicked()
{
    _k_rangeClicked( m_currentRangeAction );
}


void DateRangeSelectionWidget::Private::_k_rangeClicked( QAction* action )
{
    m_currentRangeAction = action;
    m_rangesButton->setText( action->text() );
    setRange( m_rangeActionMap[action] );

    emit q->rangeChanged( m_range );
}


void DateRangeSelectionWidget::Private::_k_rangeStartEdited( const QDate& date )
{
    if ( date > m_range.end() )
        setRange( DateRange( date, date ) );
    else
        setRange( DateRange( date, m_range.end() ) );

    emit q->rangeChanged( m_range );
}


void DateRangeSelectionWidget::Private::_k_rangeEndEdited( const QDate& date )
{
    if ( date < m_range.start() )
        setRange( DateRange( date, date ) );
    else
        setRange( DateRange( m_range.start(), date ) );

    emit q->rangeChanged( m_range );
}


void DateRangeSelectionWidget::Private::_k_calendarDateClicked( const QDate& date )
{
    if ( QApplication::keyboardModifiers() & Qt::ShiftModifier ) {
        if ( m_range.start().isValid() &&
             date < m_range.start() ) {
            setRange( DateRange( date, m_range.end().isValid() ? m_range.end() : m_range.start() ) );
            emit q->rangeChanged( m_range );
        }
        else if ( m_range.end().isValid() &&
                  date < m_range.end() ) {
            setRange( DateRange( date, m_range.end() ) );
            emit q->rangeChanged( m_range );
        }
        else if ( m_range.end().isValid() &&
                  date > m_range.end() ) {
            setRange( DateRange( m_range.start().isValid() ? m_range.start() : m_range.end(), date ) );
            emit q->rangeChanged( m_range );
        }
        else if ( m_range.start().isValid() &&
                  date > m_range.start() ) {
            setRange( DateRange( m_range.start(), date ) );
            emit q->rangeChanged( m_range );
        }
    }
    else if ( m_beforeButton->isChecked() ) {
        setRange( DateRange( QDate(), date ) );
        emit q->rangeChanged( m_range );
    }
    else if ( m_afterButton->isChecked() ) {
        setRange( DateRange( date, QDate() ) );
        emit q->rangeChanged( m_range );
    }
    else {
        setRange( DateRange( date, date ) );
        emit q->rangeChanged( m_range );
    }
}


void DateRangeSelectionWidget::Private::checkButton( QAbstractButton* button )
{
    foreach( QAbstractButton* b, m_buttonGroup->buttons() ) {
        b->setChecked( b == button );
    }
}


void DateRangeSelectionWidget::Private::setupPopupMenus()
{
    QMenu* menu = new QMenu(m_rangesButton);

    QAction* a = new QAction( menu );
    a->setText( i18n( "Today" ) );
    m_rangeActionMap.insert( a, DateRange::today() );
    menu->addAction( a );

    // today is the default
    m_currentRangeAction = a;

    a = new QAction( menu );
    a->setText( i18n( "This Week" ) );
    m_rangeActionMap.insert( a, DateRange::thisWeek() );
    menu->addAction( a );

    a = new QAction( menu );
    a->setText( i18n( "This Month" ) );
    m_rangeActionMap.insert( a, DateRange::thisMonth() );
    menu->addAction( a );

    m_rangesButton->setMenu( menu );
}


void DateRangeSelectionWidget::Private::updateCalendar( const DateRange& range )
{
    m_calendar->setDateTextFormat( QDate(), QTextCharFormat() );
    if ( range.start().isValid() )
        m_calendar->setSelectedDate( range.start() );
    else if ( range.end().isValid() )
        m_calendar->setSelectedDate( range.end() );
    else
        m_calendar->setSelectedDate( QDate::currentDate() );
    if ( range.isValid() ) {
        QTextCharFormat selectedFormat;
        selectedFormat.setBackground( q->palette().color( QPalette::Highlight ) );
        selectedFormat.setForeground( q->palette().color( QPalette::HighlightedText ) );
        for ( QDate date = range.start(); date <= range.end(); date = date.addDays( 1 ) ) {
            m_calendar->setDateTextFormat( date, selectedFormat );
        }
    }
}


void DateRangeSelectionWidget::Private::updateEditBoxes( const DateRange& range )
{
    m_rangeStartEdit->blockSignals( true );
    m_rangeEndEdit->blockSignals( true );

    m_rangeStartEdit->setDate( range.start() );
    m_rangeEndEdit->setDate( range.end() );
    m_rangeStartEdit->setEnabled( range.start().isValid() );
    m_rangeEndEdit->setEnabled( range.end().isValid() );

    m_rangeStartEdit->blockSignals( false );
    m_rangeEndEdit->blockSignals( false );
}


void DateRangeSelectionWidget::Private::setRange( const DateRange& range )
{
    m_range = range;

    if ( range.isValid() ) {
        if ( QAction* a = findRangeAction( range ) ) {
            m_currentRangeAction = a;
            m_rangesButton->setText( a->text() );
            checkButton( m_rangesButton );
        }
        else {
            checkButton( 0 );
        }
    }
    else if ( range.start().isValid() ) {
        checkButton( m_afterButton );
    }
    else if ( range.end().isValid() ) {
        checkButton( m_beforeButton );
    }
    else {
        checkButton( m_anytimeButton );
    }

    updateCalendar( range );
    updateEditBoxes( range );
}


QAction* DateRangeSelectionWidget::Private::findRangeAction( const DateRange& range )
{
    // We have only 3 elements in here, thus, looping over them is ok
    for ( QHash<QAction*, DateRange>::const_iterator it = m_rangeActionMap.constBegin();
          it != m_rangeActionMap.constEnd(); ++it ) {
        if ( it.value() == range )
            return it.key();
    }
    return 0;
}


DateRangeSelectionWidget::DateRangeSelectionWidget( QWidget* parent )
    : QWidget( parent ),
      d( new Private() )
{
    d->q = this;
    d->setupUi( this );
    d->setupPopupMenus();

    d->m_calendar->setFirstDayOfWeek( Qt::DayOfWeek( KGlobal::locale()->weekStartDay() ) );
    connect( d->m_anytimeButton, SIGNAL( clicked() ),
             this, SLOT( _k_anytimeClicked() ) );
    connect( d->m_beforeButton, SIGNAL( clicked() ),
             this, SLOT( _k_beforeClicked() ) );
    connect( d->m_afterButton, SIGNAL( clicked() ),
             this, SLOT( _k_afterClicked() ) );
    connect( d->m_rangesButton, SIGNAL( clicked() ),
             this, SLOT( _k_rangeClicked() ) );
    connect( d->m_rangesButton, SIGNAL( triggered(QAction*) ),
             this, SLOT( _k_rangeClicked(QAction*) ) );
    connect( d->m_rangeStartEdit, SIGNAL( dateChanged( QDate ) ),
             this, SLOT( _k_rangeStartEdited( QDate ) ) );
    connect( d->m_rangeEndEdit, SIGNAL( dateChanged( QDate ) ),
             this, SLOT( _k_rangeEndEdited( QDate ) ) );
    connect( d->m_calendar, SIGNAL( clicked( QDate ) ),
             this, SLOT( _k_calendarDateClicked( QDate ) ) );

    setRange( DateRange() );
}


DateRangeSelectionWidget::~DateRangeSelectionWidget()
{
}


DateRange DateRangeSelectionWidget::range() const
{
    return d->m_range;
}


void DateRangeSelectionWidget::setRange( const DateRange& range )
{
    d->setRange( range );
}

#include "daterangeselectionwidget.moc"
