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

#include "kdatetable.h"

#include <kconfig.h>
#include <kcolorscheme.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <knotification.h>
#include <kcalendarsystem.h>
#include <klocalizeddate.h>
#include <kshortcut.h>
#include <kstandardshortcut.h>
#include "kdatepicker.h"
#include "kactioncollection.h"
#include <kdeversion.h>

#include <QAction>
#include <QtCore/QDate>
#include <QtCore/QCharRef>
#include <QPen>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QDialog>
#include <QActionEvent>
#include <QtCore/QHash>
#include <QApplication>
#include <QMenu>
#include <assert.h>

#include <cmath>

class KDateTable::KDateTablePrivate
{
public:
    KDateTablePrivate( KDateTable *q ): q( q )
    {
        m_popupMenuEnabled = false;
        m_useCustomColors = false;
        m_hoveredPos = -1;
        setDate( QDate::currentDate() );
    }

    ~KDateTablePrivate()
    {
    }

    void setDate( const QDate& date );
    void nextMonth();
    void previousMonth();
    void beginningOfMonth();
    void endOfMonth();
    void beginningOfWeek();
    void endOfWeek();

    KDateTable *q;

    /**
    * The font size of the displayed text.
    */
    int fontsize;

    /**
    * The currently selected date.
    */
    KLocalizedDate m_date;
    // Need to keep a QDate copy as the "const QDate &date() const;" method returns a reference
    // and returning m_date.date() creates a temporary leading to crashes.  Doh!
    QDate m_refDate;

    /**
     * The weekday number of the first day in the month [1..daysInWeek()].
     */
    int m_weekDayFirstOfMonth;

    /**
     * The number of days in the current month.
     */
    int m_numDaysThisMonth;

    /**
     * Save the size of the largest used cell content.
     */
    QRectF m_maxCell;

    /**
     * How many week rows we are to draw.
     */
    int m_numWeekRows;

    /**
     * How many day columns we are to draw, i.e. days in a week.
     */
    int m_numDayColumns;

    bool m_popupMenuEnabled;
    bool m_useCustomColors;

    struct DatePaintingMode
    {
        QColor fgColor;
        QColor bgColor;
        BackgroundMode bgMode;
    };
    QHash <int, DatePaintingMode> m_customPaintingModes;

    int m_hoveredPos;
};


class KPopupFrame::KPopupFramePrivate
{
public:
    KPopupFramePrivate( KPopupFrame *q );
    ~KPopupFramePrivate();

    KPopupFrame *q;

    /**
     * The result. It is returned from exec() when the popup window closes.
     */
    int result;

    /**
     * The only subwidget that uses the whole dialog window.
     */
    QWidget *main;

    // ### KDE 5: Remove this, add a hideEvent() reimplementation instead.
    class OutsideClickCatcher;
    OutsideClickCatcher *outsideClickCatcher;
};


class KPopupFrame::KPopupFramePrivate::OutsideClickCatcher
    : public QObject
{
public:
    OutsideClickCatcher(QObject *parent = 0)
        : QObject(parent), m_popup(0) { }
    ~OutsideClickCatcher() { }

    void setPopupFrame(KPopupFrame *popup)
    {
        m_popup = popup;
        popup->installEventFilter(this);
    }

    KPopupFrame *m_popup;

    bool eventFilter(QObject *object, QEvent *event)
    {
        Q_UNUSED(object);

        // To catch outside clicks, it is sufficient to check for
        // hide events on Qt::Popup type widgets
        if (event->type() == QEvent::Hide && m_popup) {
            // do not set d->result here, because the popup
            // hides itself after leaving the event loop.
            emit m_popup->leaveModality();
        }
        return false;
    }
};


KPopupFrame::KPopupFramePrivate::KPopupFramePrivate( KPopupFrame *q ):
    q( q ),
    result( 0 ), // rejected
    main( 0 ),
    outsideClickCatcher(new OutsideClickCatcher)
{
    outsideClickCatcher->setPopupFrame(q);
}

KPopupFrame::KPopupFramePrivate::~KPopupFramePrivate()
{
    delete outsideClickCatcher;
}


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

KDateValidator::KDateValidator( QWidget *parent ) : QValidator( parent ), d( 0 )
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
    //FIXME This is wrong if the widget is not using the global!
    QDate tmp = KLocale::global()->readDate( text );
    if ( KLocale::global()->calendar()->isValid( tmp ) ) {
        d = tmp;
        return Acceptable;
    } else {
        return QValidator::Intermediate;
    }
}

void KDateValidator::fixup( QString& ) const
{
}

KDateTable::KDateTable( const QDate& date, QWidget* parent )
           : QWidget( parent ),
             d( new KDateTablePrivate( this ) )
{
    initWidget( date );
}

KDateTable::KDateTable( QWidget *parent )
           : QWidget( parent ),
             d( new KDateTablePrivate( this ) )
{
    initWidget( QDate::currentDate() );
}

KDateTable::~KDateTable()
{
    delete d;
}

void KDateTable::initWidget( const QDate &date )
{
    d->m_numWeekRows = 7;

    setFontSize( 10 );
    setFocusPolicy( Qt::StrongFocus );
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    initAccels();
    setAttribute(Qt::WA_Hover, true);

    setDate( date );
}

void KDateTable::initAccels()
{
    KActionCollection * localCollection = new KActionCollection( this );

    QAction* next = localCollection->addAction( QLatin1String( "next" ) );
    next->setShortcuts( KStandardShortcut::next() );
    connect( next, SIGNAL(triggered(bool)), SLOT(nextMonth()) );

    QAction* prior = localCollection->addAction( QLatin1String( "prior" ) );
    prior->setShortcuts( KStandardShortcut::prior() );
    connect( prior, SIGNAL(triggered(bool)), SLOT(previousMonth()) );

    QAction* beginMonth = localCollection->addAction( QLatin1String( "beginMonth" ) );
    beginMonth->setShortcuts( KStandardShortcut::begin() );
    connect( beginMonth, SIGNAL(triggered(bool)), SLOT(beginningOfMonth()) );

    QAction* endMonth = localCollection->addAction( QLatin1String( "endMonth" ) );
    endMonth->setShortcuts( KStandardShortcut::end() );
    connect( endMonth, SIGNAL(triggered(bool)), SLOT(endOfMonth()) );

    QAction* beginWeek = localCollection->addAction( QLatin1String( "beginWeek" ) );
    beginWeek->setShortcuts( KStandardShortcut::beginningOfLine() );
    connect( beginWeek, SIGNAL(triggered(bool)), SLOT(beginningOfWeek()) );

    QAction* endWeek = localCollection->addAction( "endWeek" );
    endWeek->setShortcuts( KStandardShortcut::endOfLine() );
    connect( endWeek, SIGNAL(triggered(bool)), SLOT(endOfWeek()) );

    localCollection->readSettings();
    localCollection->addAssociatedWidget( this );
    foreach (QAction* action, localCollection->actions()) {
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    }
}

int KDateTable::posFromDate( const QDate &date )
{
    int initialPosition = calendar()->day( date );
    int offset = ( d->m_weekDayFirstOfMonth - calendar()->weekStartDay() + d->m_numDayColumns ) % d->m_numDayColumns;

    // make sure at least one day of the previous month is visible.
    // adjust this < 1 if more days should be forced visible:
    if ( offset < 1 ) {
        offset += d->m_numDayColumns;
    }

    return initialPosition + offset;
}

QDate KDateTable::dateFromPos( int position )
{
    int offset = ( d->m_weekDayFirstOfMonth - calendar()->weekStartDay() + d->m_numDayColumns ) % d->m_numDayColumns;

    // make sure at least one day of the previous month is visible.
    // adjust this < 1 if more days should be forced visible:
    if ( offset < 1 ) {
        offset += d->m_numDayColumns;
    }

    return d->m_date.firstDayOfMonth().addDays( position - offset ).date();
}

void KDateTable::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    KColorScheme colorScheme(palette().currentColorGroup(), KColorScheme::View);
    const QRect &rectToUpdate = e->rect();
    double cellWidth = width() / ( double ) d->m_numDayColumns;
    double cellHeight = height() / ( double ) d->m_numWeekRows;
    int leftCol = ( int )std::floor( rectToUpdate.left() / cellWidth );
    int topRow = ( int )std::floor( rectToUpdate.top() / cellHeight );
    int rightCol = ( int )std::ceil( rectToUpdate.right() / cellWidth );
    int bottomRow = ( int )std::ceil( rectToUpdate.bottom() / cellHeight );
    bottomRow = qMin( bottomRow, d->m_numWeekRows - 1 );
    rightCol = qMin( rightCol, d->m_numDayColumns - 1 );
    if ( layoutDirection() == Qt::RightToLeft ) {
        p.translate( ( d->m_numDayColumns - leftCol - 1 ) * cellWidth, topRow * cellHeight );
    } else {
        p.translate( leftCol * cellWidth, topRow * cellHeight );
    }
    for ( int i = leftCol; i <= rightCol; ++i ) {
        for ( int j = topRow; j <= bottomRow; ++j ) {
            paintCell( &p, j, i, colorScheme );
            p.translate( 0, cellHeight );
        }
        if ( layoutDirection() == Qt::RightToLeft ) {
            p.translate( -cellWidth, 0 );
        } else {
            p.translate( cellWidth, 0 );
        }
        p.translate( 0, -cellHeight * ( bottomRow - topRow + 1 ) );
    }
}

void KDateTable::paintCell( QPainter *painter, int row, int col, const KColorScheme &colorScheme )
{
    double w = ( width() / ( double ) d->m_numDayColumns ) - 1;
    double h = ( height() / ( double ) d->m_numWeekRows ) - 1;
    QRectF cell = QRectF( 0, 0, w, h );
    QString cellText;
    QPen pen;
    QColor cellBackgroundColor, cellTextColor;
    QFont cellFont = KGlobalSettings::generalFont();
    bool workingDay = false;
    int cellWeekDay, pos;
    BackgroundMode cellBackgroundMode = RectangleMode;

    //Calculate the position of the cell in the grid
    pos = d->m_numDayColumns * ( row - 1 ) + col;

    //Calculate what day of the week the cell is
    if ( col + calendar()->weekStartDay() <= d->m_numDayColumns ) {
        cellWeekDay = col + calendar()->weekStartDay();
    } else {
        cellWeekDay = col + calendar()->weekStartDay() - d->m_numDayColumns;
    }

    //FIXME This is wrong if the widget is not using the global!
    //See if cell day is normally a working day
    if ( KLocale::global()->workingWeekStartDay() <= KLocale::global()->workingWeekEndDay() ) {
        if ( cellWeekDay >= KLocale::global()->workingWeekStartDay() &&
             cellWeekDay <= KLocale::global()->workingWeekEndDay() ) {
                workingDay = true;
        }
    } else {
        if ( cellWeekDay >= KLocale::global()->workingWeekStartDay() ||
             cellWeekDay <= KLocale::global()->workingWeekEndDay() ) {
                workingDay = true;
        }
    }

    if( row == 0 ) {

        //We are drawing a header cell

        //If not a normal working day, then use "do not work today" color
        if ( workingDay ) {
            cellTextColor = palette().color(QPalette::WindowText);
        } else {
            KColorScheme colorScheme(palette().currentColorGroup(), KColorScheme::Window);
            cellTextColor = colorScheme.foreground(KColorScheme::NegativeText).color();
        }
        cellBackgroundColor = palette().color(QPalette::Window);

        //Set the text to the short day name and bold it
        cellFont.setBold( true );
        cellText = calendar()->weekDayName( cellWeekDay, KCalendarSystem::ShortDayName );

    } else {

        //We are drawing a day cell

        //Calculate the date the cell represents
        //Copy current date to get same calendar system & locale
        KLocalizedDate cellDate = d->m_date;
        cellDate = dateFromPos( pos );

        bool validDay = cellDate.isValid();

        // Draw the day number in the cell, if the date is not valid then we don't want to show it
        if ( validDay ) {
            cellText = cellDate.formatDate( KLocale::Day, KLocale::ShortNumber );
        } else {
            cellText = "";
        }

        if( ! validDay || cellDate.month() != d->m_date.month() ) {
            // we are either
            // ° painting an invalid day
            // ° painting a day of the previous month or
            // ° painting a day of the following month or
            cellBackgroundColor = palette().color(backgroundRole());
            cellTextColor = colorScheme.foreground(KColorScheme::InactiveText).color();
        } else {
            //Paint a day of the current month

            // Background Colour priorities will be (high-to-low):
            // * Selected Day Background Colour
            // * Customized Day Background Colour
            // * Normal Day Background Colour

            // Background Shape priorities will be (high-to-low):
            // * Customized Day Shape
            // * Normal Day Shape

            // Text Colour priorities will be (high-to-low):
            // * Customized Day Colour
            // * Day of Pray Colour (Red letter)
            // * Selected Day Colour
            // * Normal Day Colour

            //Determine various characteristics of the cell date
            bool selectedDay = ( cellDate == date() );
            bool currentDay = ( cellDate == QDate::currentDate() );
            bool dayOfPray = ( cellDate.dayOfWeek() == calendar()->locale()->weekDayOfPray() );
            bool customDay = ( d->m_useCustomColors && d->m_customPaintingModes.contains(cellDate.toJulianDay()) );

            //Default values for a normal cell
            cellBackgroundColor = palette().color( backgroundRole() );
            cellTextColor = palette().color( foregroundRole() );

            // If we are drawing the current date, then draw it bold and active
            if ( currentDay ) {
                cellFont.setBold( true );
                cellTextColor = colorScheme.foreground(KColorScheme::ActiveText).color();
            }

            // if we are drawing the day cell currently selected in the table
            if ( selectedDay ) {
                // set the background to highlighted
                cellBackgroundColor = palette().color( QPalette::Highlight );
                cellTextColor = palette().color( QPalette::HighlightedText );
            }

            //If custom colors or shape are required for this date
            if ( customDay ) {
                KDateTablePrivate::DatePaintingMode mode = d->m_customPaintingModes[cellDate.toJulianDay()];
                if ( mode.bgMode != NoBgMode ) {
                        cellBackgroundMode = mode.bgMode;
                        if (!selectedDay) cellBackgroundColor = mode.bgColor;
                }
                cellTextColor = mode.fgColor;
            }

            //If the cell day is the day of religious observance, then always color text red unless Custom overrides
            if ( ! customDay && dayOfPray ) {
                KColorScheme colorScheme(palette().currentColorGroup(),
                                         selectedDay ? KColorScheme::Selection : KColorScheme::View);
                cellTextColor = colorScheme.foreground(KColorScheme::NegativeText).color();
            }

        }
    }

    //Draw the background
    if (row == 0) {
        painter->setPen( cellBackgroundColor );
        painter->setBrush( cellBackgroundColor );
        painter->drawRect( cell );
    } else if (cellBackgroundColor != palette().color(backgroundRole()) || pos == d->m_hoveredPos) {
        QStyleOptionViewItemV4 opt;
        opt.initFrom(this);
        opt.rect = cell.toRect();
        if (cellBackgroundColor != palette().color(backgroundRole())) {
            opt.palette.setBrush(QPalette::Highlight, cellBackgroundColor);
            opt.state |= QStyle::State_Selected;
        }
        if (pos == d->m_hoveredPos && opt.state & QStyle::State_Enabled) {
            opt.state |= QStyle::State_MouseOver;
        } else {
            opt.state &= ~QStyle::State_MouseOver;
        }
        opt.showDecorationSelected = true;
        opt.viewItemPosition = QStyleOptionViewItemV4::OnlyOne;
        style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, this);
    }

    //Draw the text
    painter->setPen( cellTextColor );
    painter->setFont( cellFont );
    painter->drawText( cell, Qt::AlignCenter, cellText, &cell );

    //Draw the base line
    if (row == 0) {
        painter->setPen( palette().color(foregroundRole()) );
        painter->drawLine( QPointF( 0, h ), QPointF( w, h ) );
    }

    // If the day cell we just drew is bigger than the current max cell sizes,
    // then adjust the max to the current cell
    if ( cell.width() > d->m_maxCell.width() ) d->m_maxCell.setWidth( cell.width() );
    if ( cell.height() > d->m_maxCell.height() ) d->m_maxCell.setHeight( cell.height() );
}

void KDateTable::KDateTablePrivate::nextMonth()
{
    // setDate does validity checking for us
    q->setDate( m_date.addMonths( 1 ).date() );
}

void KDateTable::KDateTablePrivate::previousMonth()
{
    // setDate does validity checking for us
    q->setDate( m_date.addMonths( -1 ).date() );
}

void KDateTable::KDateTablePrivate::beginningOfMonth()
{
    // setDate does validity checking for us
    q->setDate( m_date.firstDayOfMonth().date() );
}

void KDateTable::KDateTablePrivate::endOfMonth()
{
    // setDate does validity checking for us
    q->setDate( m_date.lastDayOfMonth().date() );
}

// JPL Do these make the assumption that first day of week is weekday 1? As it may not be.
void KDateTable::KDateTablePrivate::beginningOfWeek()
{
    // setDate does validity checking for us
    q->setDate( m_date.addDays( 1 - m_date.dayOfWeek() ).date() );
}

// JPL Do these make the assumption that first day of week is weekday 1? As it may not be.
void KDateTable::KDateTablePrivate::endOfWeek()
{
    // setDate does validity checking for us
    q->setDate( m_date.addDays( m_date.daysInWeek() - m_date.dayOfWeek() ).date() );
}

void KDateTable::keyPressEvent( QKeyEvent *e )
{
    switch( e->key() ) {
    case Qt::Key_Up:
        // setDate does validity checking for us
        setDate( d->m_date.addDays( - d->m_numDayColumns ).date() );
        break;
    case Qt::Key_Down:
        // setDate does validity checking for us
        setDate( d->m_date.addDays( d->m_numDayColumns ).date() );
        break;
    case Qt::Key_Left:
        // setDate does validity checking for us
        setDate( d->m_date.addDays( -1 ).date() );
        break;
    case Qt::Key_Right:
        // setDate does validity checking for us
        setDate( d->m_date.addDays( 1 ).date() );
        break;
    case Qt::Key_Minus:
        // setDate does validity checking for us
        setDate( d->m_date.addDays( -1 ).date() );
        break;
    case Qt::Key_Plus:
        // setDate does validity checking for us
        setDate( d->m_date.addDays( 1 ).date() );
        break;
    case Qt::Key_N:
        // setDate does validity checking for us
        setDate( QDate::currentDate() );
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        emit tableClicked();
        break;
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_Shift:
        // Don't beep for modifiers
        break;
    default:
        if ( !e->modifiers() ) { // hm
            KNotification::beep();
        }
    }
}

void KDateTable::setFontSize( int size )
{
    QFontMetricsF metrics( fontMetrics() );
    QRectF rect;
    // ----- store rectangles:
    d->fontsize = size;
    // ----- find largest day name:
    d->m_maxCell.setWidth( 0 );
    d->m_maxCell.setHeight( 0 );
    for( int weekday = 1; weekday <= d->m_date.daysInWeek(); ++weekday ) {
        rect = metrics.boundingRect( calendar()->weekDayName( weekday, KCalendarSystem::ShortDayName ) );
        d->m_maxCell.setWidth( qMax( d->m_maxCell.width(), rect.width() ) );
        d->m_maxCell.setHeight( qMax( d->m_maxCell.height(), rect.height() ) );
    }
    // ----- compare with a real wide number and add some space:
    rect = metrics.boundingRect( QLatin1String( "88" ) );
    d->m_maxCell.setWidth( qMax( d->m_maxCell.width() + 2, rect.width() ) );
    d->m_maxCell.setHeight( qMax( d->m_maxCell.height() + 4, rect.height() ) );
}

void KDateTable::wheelEvent ( QWheelEvent * e )
{
    setDate( d->m_date.addMonths( -( int )( e->delta() / 120 ) ).date() );
    e->accept();
}

bool KDateTable::event(QEvent *ev)
{
    switch (ev->type()) {
        case QEvent::HoverMove:
        {
            QHoverEvent *e = static_cast<QHoverEvent *>(ev);
            const int row = e->pos().y() * d->m_numWeekRows / height();
            int col;
            if ( layoutDirection() == Qt::RightToLeft ) {
                col = d->m_numDayColumns - ( e->pos().x() * d->m_numDayColumns / width() ) - 1;
            } else {
                col = e->pos().x() * d->m_numDayColumns / width();
            }

            const int pos = row < 1 ? -1 : (d->m_numDayColumns * (row - 1)) + col;

            if (pos != d->m_hoveredPos) {
                d->m_hoveredPos = pos;
                update();
            }
            break;
        }
        case QEvent::HoverLeave:
            if (d->m_hoveredPos != -1) {
                d->m_hoveredPos = -1;
                update();
            }
            break;
        default:
            break;
    }
    return QWidget::event(ev);
}

void KDateTable::mousePressEvent( QMouseEvent *e )
{
    if( e->type() != QEvent::MouseButtonPress ) { // the KDatePicker only reacts on mouse press events:
        return;
    }

    if( !isEnabled() ) {
        KNotification::beep();
        return;
    }

    int row, col, pos, temp;

    QPoint mouseCoord = e->pos();
    row = mouseCoord.y() * d->m_numWeekRows / height();
    if ( layoutDirection() == Qt::RightToLeft ) {
        col = d->m_numDayColumns - ( mouseCoord.x() * d->m_numDayColumns / width() ) - 1;
    } else {
        col = mouseCoord.x() * d->m_numDayColumns / width();
    }

    if( row < 1 || col < 0 ) { // the user clicked on the frame of the table
        return;
    }

    // Rows and columns are zero indexed.  The (row - 1) below is to avoid counting
    // the row with the days of the week in the calculation.

    // old selected date:
    temp = posFromDate( date() );

    // new position and date
    pos = ( d->m_numDayColumns * ( row - 1 ) ) + col;
    QDate clickedDate = dateFromPos( pos );

    // set the new date. If it is in the previous or next month, the month will
    // automatically be changed, no need to do that manually...
    // validity checking done inside setDate
    setDate( clickedDate );

    // This could be optimized to only call update over the regions
    // of old and new cell, but 99% of times there is also a call to
    // setDate that already calls update() so no need to optimize that
    // much here
    update();

    emit tableClicked();

    if (  e->button() == Qt::RightButton && d->m_popupMenuEnabled ) {
        QMenu * menu = new QMenu();
        menu->addSection( d->m_date.formatDate() );
        emit aboutToShowContextMenu( menu, clickedDate );
        menu->popup( e->globalPos() );
    }
}

void KDateTable::KDateTablePrivate::setDate( const QDate& date )
{
    m_date.setDate( date );
    m_refDate = date;
    m_weekDayFirstOfMonth = m_date.firstDayOfMonth().dayOfWeek();
    m_numDaysThisMonth = m_date.daysInMonth();
    m_numDayColumns = m_date.daysInWeek();
}

bool KDateTable::setDate( const QDate& toDate )
{
    if ( !calendar()->isValid( toDate ) ) {
        return false;
    }

    if ( toDate == date() ) {
        return true;
    }

    QDate oldDate = date();
    d->setDate( toDate );
    emit( dateChanged( date(), oldDate ) );
    emit( dateChanged( date() ) );
    update();

    return true;
}

const QDate &KDateTable::date() const
{
    return d->m_refDate;
}

const KCalendarSystem *KDateTable::calendar() const
{
    return  d->m_date.calendar();
}

bool KDateTable::setCalendar( KCalendarSystem *newCalendar )
{
    QDate oldDate = date();
    d->m_date = KLocalizedDate( oldDate, newCalendar );
    return setDate( oldDate );
}

bool KDateTable::setCalendarSystem( KLocale::CalendarSystem newCalendarSystem )
{
    d->m_date.setCalendarSystem( newCalendarSystem );
    return true;
}

void KDateTable::focusInEvent( QFocusEvent *e )
{
    QWidget::focusInEvent( e );
}

void KDateTable::focusOutEvent( QFocusEvent *e )
{
    QWidget::focusOutEvent( e );
}

QSize KDateTable::sizeHint() const
{
    if( d->m_maxCell.height() > 0 && d->m_maxCell.width() > 0 ) {
        return QSize( qRound( d->m_maxCell.width() * d->m_numDayColumns ),
                      ( qRound( d->m_maxCell.height() + 2 ) * d->m_numWeekRows ) );
    } else {
        kDebug() << "KDateTable::sizeHint: obscure failure - " << endl;
        return QSize( -1, -1 );
    }
}

void KDateTable::setPopupMenuEnabled( bool enable )
{
    d->m_popupMenuEnabled = enable;
}

bool KDateTable::popupMenuEnabled() const
{
    return d->m_popupMenuEnabled;
}

void KDateTable::setCustomDatePainting( const QDate &date, const QColor &fgColor, BackgroundMode bgMode, const QColor &bgColor )
{
    if ( !fgColor.isValid() ) {
        unsetCustomDatePainting( date );
        return;
    }

    KDateTablePrivate::DatePaintingMode mode;
    mode.bgMode = bgMode;
    mode.fgColor = fgColor;
    mode.bgColor = bgColor;

    d->m_customPaintingModes.insert( date.toJulianDay(), mode );
    d->m_useCustomColors = true;
    update();
}

void KDateTable::unsetCustomDatePainting( const QDate &date )
{
    d->m_customPaintingModes.remove( date.toJulianDay() );
    if ( d->m_customPaintingModes.isEmpty() ) d->m_useCustomColors = false;
    update();
}


// JPL Shouldn't this be in own file as is used in a couple of places?  Or moved to private in KDE5?

KPopupFrame::KPopupFrame( QWidget* parent )
            : QFrame( parent, Qt::Popup ), d( new KPopupFramePrivate( this ) )
{
    setFrameStyle( QFrame::Box | QFrame::Raised );
    setMidLineWidth( 2 );
}

KPopupFrame::~KPopupFrame()
{
    delete d;
}

void KPopupFrame::keyPressEvent( QKeyEvent* e )
{
    if( e->key() == Qt::Key_Escape ) {
        d->result = 0; // rejected
        emit leaveModality();
        //qApp->exit_loop();
    }
}

void KPopupFrame::close( int r )
{
    d->result = r;
    emit leaveModality();
    //qApp->exit_loop();
}

void KPopupFrame::setMainWidget( QWidget *m )
{
    d->main = m;
    if( d->main ) {
        resize( d->main->width() + 2 * frameWidth(), d->main->height() + 2 * frameWidth() );
    }
}

void KPopupFrame::resizeEvent( QResizeEvent *e )
{
    Q_UNUSED( e );

    if( d->main ) {
        d->main->setGeometry( frameWidth(), frameWidth(),
                              width() - 2 * frameWidth(), height() - 2 * frameWidth() );
    }
}

void KPopupFrame::popup( const QPoint &pos )
{
    // Make sure the whole popup is visible.
    QRect desktopGeometry = QApplication::desktop()->screenGeometry( pos );

    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();
    if ( x + w > desktopGeometry.x() + desktopGeometry.width() ) {
        x = desktopGeometry.width() - w;
    }
    if ( y + h > desktopGeometry.y() + desktopGeometry.height() ) {
        y = desktopGeometry.height() - h;
    }
    if ( x < desktopGeometry.x() ) {
        x = 0;
    }
    if ( y < desktopGeometry.y() ) {
        y = 0;
    }

    // Pop the thingy up.
    move( x, y );
    show();
    d->main->setFocus();
}

int KPopupFrame::exec( const QPoint &pos )
{
    popup( pos );
    repaint();
    d->result = 0; // rejected
    QEventLoop eventLoop;
    connect( this, SIGNAL(leaveModality()),
             &eventLoop, SLOT(quit()) );
    eventLoop.exec();

    hide();
    return d->result;
}

int KPopupFrame::exec( int x, int y )
{
    return exec( QPoint( x, y ) );
}

#include "moc_kdatetable.cpp"
