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
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kdebug.h>
#include <knotification.h>
#include <kcalendarsystem.h>
#include <kshortcut.h>
#include <kstandardshortcut.h>
#include "kdatepicker.h"
#include "kmenu.h"
#include "kactioncollection.h"
#include "kaction.h"
#include <kdeversion.h>

#include <QtCore/QDate>
#include <QtCore/QCharRef>
#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionViewItem>
#include <QtGui/QDialog>
#include <QtGui/QActionEvent>
#include <QtCore/QHash>
#include <QtGui/QApplication>
#include <assert.h>

#include <cmath>

class KDateTable::KDateTablePrivate
{
public:
    KDateTablePrivate( KDateTable *q ): q( q )
    {
        popupMenuEnabled = false;
        useCustomColors = false;
        m_calendar = 0;
        hoveredPos = -1;
    }

    ~KDateTablePrivate()
    {
    }

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
    QDate mDate;

    /**
     * The weekday number of the first day in the month [1..daysInWeek()].
     */
    int weekDayFirstOfMonth;

    /**
     * The number of days in the current month.
     */
    int numDaysThisMonth;

    /**
     * Save the size of the largest used cell content.
     */
    QRectF maxCell;

    /**
     * How many week rows we are to draw.
     */
    int numWeekRows;

    /**
     * How many day columns we are to draw, i.e. days in a week.
     */
    int numDayColumns;

    bool popupMenuEnabled : 1;
    bool useCustomColors : 1;

    struct DatePaintingMode
    {
        QColor fgColor;
        QColor bgColor;
        BackgroundMode bgMode;
    };
    QHash <int, DatePaintingMode*> customPaintingModes;

    int hoveredPos;

    KCalendarSystem *m_calendar;

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
    QDate tmp = KGlobal::locale()->readDate( text );
    if ( !tmp.isNull() ) {
        d = tmp;
        return Acceptable;
    } else {
        return QValidator::Intermediate;
    }
}

void KDateValidator::fixup( QString& ) const
{
}

KDateTable::KDateTable( const QDate& date_, QWidget* parent )
           : QWidget( parent ), d( new KDateTablePrivate( this ) )
{
    d->numWeekRows = 7;
    d->numDayColumns = calendar()->daysInWeek( date_ );
    setFontSize( 10 );
    setFocusPolicy( Qt::StrongFocus );
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    if( !setDate( date_ ) ) {
        // this initializes weekDayFirstOfMonth, numDaysThisMonth
        setDate( QDate::currentDate() );
    }
    initAccels();
    setAttribute(Qt::WA_Hover, true);
}

KDateTable::KDateTable( QWidget *parent )
           : QWidget( parent ), d( new KDateTablePrivate( this ) )
{
    // JPL should we just call KDateTable( QDate::currentDate(), parent ) here to save duplication?
    // Or if that is a problem with base class instantiation move all to a private init()
    d->numWeekRows = 7;
    d->numDayColumns = calendar()->daysInWeek( QDate::currentDate() );
    setFontSize( 10 );
    setFocusPolicy( Qt::StrongFocus );
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    // this initializes weekDayFirstOfMonth, numDaysThisMonth
    setDate( QDate::currentDate() );
    initAccels();
    setAttribute(Qt::WA_Hover, true);
}

KDateTable::~KDateTable()
{
    delete d;
}

void KDateTable::initAccels()
{
    KActionCollection * localCollection = new KActionCollection( this );

    KAction* next = localCollection->addAction( QLatin1String( "next" ) );
    next->setShortcuts( KStandardShortcut::next() );
    connect( next, SIGNAL( triggered( bool ) ), SLOT( nextMonth() ) );

    KAction* prior = localCollection->addAction( QLatin1String( "prior" ) );
    prior->setShortcuts( KStandardShortcut::prior() );
    connect( prior, SIGNAL( triggered( bool ) ), SLOT( previousMonth() ) );

    KAction* beginMonth = localCollection->addAction( QLatin1String( "beginMonth" ) );
    beginMonth->setShortcuts( KStandardShortcut::begin() );
    connect( beginMonth, SIGNAL( triggered( bool ) ), SLOT( beginningOfMonth() ) );

    KAction* endMonth = localCollection->addAction( QLatin1String( "endMonth" ) );
    endMonth->setShortcuts( KStandardShortcut::end() );
    connect( endMonth, SIGNAL( triggered( bool ) ), SLOT( endOfMonth() ) );

    KAction* beginWeek = localCollection->addAction( QLatin1String( "beginWeek" ) );
    beginWeek->setShortcuts( KStandardShortcut::beginningOfLine() );
    connect( beginWeek, SIGNAL( triggered( bool ) ), SLOT( beginningOfWeek() ) );

    KAction* endWeek = localCollection->addAction( "endWeek" );
    endWeek->setShortcuts( KStandardShortcut::endOfLine() );
    connect( endWeek, SIGNAL( triggered( bool ) ), SLOT( endOfWeek() ) );

    localCollection->readSettings();
    localCollection->addAssociatedWidget( this );
    foreach (QAction* action, localCollection->actions())
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

int KDateTable::posFromDate( const QDate &date_ )
{
    int initialPosition = calendar()->day( date_ );
    int offset = ( d->weekDayFirstOfMonth - calendar()->weekStartDay() + d->numDayColumns ) % d->numDayColumns;

    // make sure at least one day of the previous month is visible.
    // adjust this < 1 if more days should be forced visible:
    if ( offset < 1 ) {
        offset += d->numDayColumns;
    }

    return initialPosition + offset;
}

QDate KDateTable::dateFromPos( int position )
{
    QDate cellDate;

    int offset = ( d->weekDayFirstOfMonth - calendar()->weekStartDay() + d->numDayColumns ) % d->numDayColumns;

    // make sure at least one day of the previous month is visible.
    // adjust this < 1 if more days should be forced visible:
    if ( offset < 1 ) {
        offset += d->numDayColumns;
    }

    if ( calendar()->setYMD( cellDate, calendar()->year( d->mDate ), calendar()->month( d->mDate ), 1 ) ) {
        cellDate = calendar()->addDays( cellDate, position - offset );
    } else {
        //If first of month is not valid, then that must be before earliestValid Date, so safe to assume next month ok
        if ( calendar()->setYMD( cellDate, calendar()->year( d->mDate ), calendar()->month( d->mDate ) + 1, 1 ) ) {
            cellDate = calendar()->addDays( cellDate, position - offset - calendar()->daysInMonth( d->mDate ) );
        }
    }
    return cellDate;
}

void KDateTable::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    KColorScheme colorScheme(palette().currentColorGroup(), KColorScheme::View);
    const QRect &rectToUpdate = e->rect();
    double cellWidth = width() / ( double ) d->numDayColumns;
    double cellHeight = height() / ( double ) d->numWeekRows;
    int leftCol = ( int )std::floor( rectToUpdate.left() / cellWidth );
    int topRow = ( int )std::floor( rectToUpdate.top() / cellHeight );
    int rightCol = ( int )std::ceil( rectToUpdate.right() / cellWidth );
    int bottomRow = ( int )std::ceil( rectToUpdate.bottom() / cellHeight );
    bottomRow = qMin( bottomRow, d->numWeekRows - 1 );
    rightCol = qMin( rightCol, d->numDayColumns - 1 );
    if ( layoutDirection() == Qt::RightToLeft ) {
        p.translate( ( d->numDayColumns - leftCol - 1 ) * cellWidth, topRow * cellHeight );
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
    double w = ( width() / ( double ) d->numDayColumns ) - 1;
    double h = ( height() / ( double ) d->numWeekRows ) - 1;
    QRectF cell = QRectF( 0, 0, w, h );
    QString cellText;
    QPen pen;
    QColor cellBackgroundColor, cellTextColor;
    QFont cellFont = KGlobalSettings::generalFont();
    bool workingDay = false;
    int cellWeekDay, pos;
    BackgroundMode cellBackgroundMode = RectangleMode;

    //Calculate the position of the cell in the grid
    pos = d->numDayColumns * ( row - 1 ) + col;

    //Calculate what day of the week the cell is
    if ( col + calendar()->weekStartDay() <= d->numDayColumns ) {
        cellWeekDay = col + calendar()->weekStartDay();
    } else {
        cellWeekDay = col + calendar()->weekStartDay() - d->numDayColumns;
    }

    //See if cell day is normally a working day
    if ( KGlobal::locale()->workingWeekStartDay() <= KGlobal::locale()->workingWeekEndDay() ) {
        if ( cellWeekDay >= KGlobal::locale()->workingWeekStartDay() &&
             cellWeekDay <= KGlobal::locale()->workingWeekEndDay() ) {
                workingDay = true;
        }
    } else {
        if ( cellWeekDay >= KGlobal::locale()->workingWeekStartDay() ||
             cellWeekDay <= KGlobal::locale()->workingWeekEndDay() ) {
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
        QDate cellDate = dateFromPos( pos );

        bool validDay = calendar()->isValid( cellDate );

        // Draw the day number in the cell, if the date is not valid then we don't want to show it
        if ( validDay ) {
            cellText = calendar()->dayString( cellDate, KCalendarSystem::ShortFormat );
        } else {
            cellText = "";
        }

        if( ! validDay || calendar()->month( cellDate ) != calendar()->month( d->mDate ) ) {
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
            bool selectedDay = ( cellDate == d->mDate );
            bool currentDay = ( cellDate == QDate::currentDate() );
            bool dayOfPray = ( calendar()->dayOfWeek( cellDate ) == KGlobal::locale()->weekDayOfPray() );
            bool customDay = ( d->useCustomColors && d->customPaintingModes.contains(cellDate.toJulianDay()) );

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
                KDateTablePrivate::DatePaintingMode * mode = d->customPaintingModes[cellDate.toJulianDay()];
                if ( mode->bgMode != NoBgMode ) {
                        cellBackgroundMode = mode->bgMode;
                        if (!selectedDay) cellBackgroundColor = mode->bgColor;
                }
                cellTextColor = mode->fgColor;
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
    } else if (cellBackgroundColor != palette().color(backgroundRole()) || pos == d->hoveredPos) {
        QStyleOptionViewItemV4 opt;
        opt.initFrom(this);
        opt.rect = cell.toRect();
        if (cellBackgroundColor != palette().color(backgroundRole())) {
            opt.palette.setBrush(QPalette::Highlight, cellBackgroundColor);
            opt.state |= QStyle::State_Selected;
        }
        if (pos == d->hoveredPos && opt.state & QStyle::State_Enabled) {
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
    if ( cell.width() > d->maxCell.width() ) d->maxCell.setWidth( cell.width() );
    if ( cell.height() > d->maxCell.height() ) d->maxCell.setHeight( cell.height() );
}

void KDateTable::KDateTablePrivate::nextMonth()
{
    // setDate does validity checking for us
    q->setDate( q->calendar()->addMonths( mDate, 1 ) );
}

void KDateTable::KDateTablePrivate::previousMonth()
{
    // setDate does validity checking for us
    q->setDate( q->calendar()->addMonths( mDate, -1 ) );
}

void KDateTable::KDateTablePrivate::beginningOfMonth()
{
    // setDate does validity checking for us
    q->setDate( q->calendar()->addDays( mDate, 1 - q->calendar()->day( mDate ) ) );
}

void KDateTable::KDateTablePrivate::endOfMonth()
{
    // setDate does validity checking for us
    q->setDate( q->calendar()->addDays( mDate,
                q->calendar()->daysInMonth( mDate ) - q->calendar()->day( mDate ) ) );
}

// JPL Do these make the assumption that first day of week is weekday 1? As it may not be.
void KDateTable::KDateTablePrivate::beginningOfWeek()
{
    // setDate does validity checking for us
    q->setDate( q->calendar()->addDays( mDate, 1 - q->calendar()->dayOfWeek( mDate ) ) );
}

// JPL Do these make the assumption that first day of week is weekday 1? As it may not be.
void KDateTable::KDateTablePrivate::endOfWeek()
{
    // setDate does validity checking for us
    q->setDate( q->calendar()->addDays( mDate,
                q->calendar()->daysInWeek( mDate ) - q->calendar()->dayOfWeek( mDate ) ) );
}

void KDateTable::keyPressEvent( QKeyEvent *e )
{
    switch( e->key() ) {
    case Qt::Key_Up:
        // setDate does validity checking for us
        setDate( calendar()->addDays( d->mDate, - d->numDayColumns ) );
        break;
    case Qt::Key_Down:
        // setDate does validity checking for us
        setDate( calendar()->addDays( d->mDate, d->numDayColumns ) );
        break;
    case Qt::Key_Left:
        // setDate does validity checking for us
        setDate( calendar()->addDays( d->mDate, -1 ) );
        break;
    case Qt::Key_Right:
        // setDate does validity checking for us
        setDate( calendar()->addDays( d->mDate, 1 ) );
        break;
    case Qt::Key_Minus:
        // setDate does validity checking for us
        setDate( calendar()->addDays( d->mDate, -1 ) );
        break;
    case Qt::Key_Plus:
        // setDate does validity checking for us
        setDate( calendar()->addDays( d->mDate, 1 ) );
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
    int count;
    QFontMetricsF metrics( fontMetrics() );
    QRectF rect;
    // ----- store rectangles:
    d->fontsize = size;
    // ----- find largest day name:
    d->maxCell.setWidth( 0 );
    d->maxCell.setHeight( 0 );
    for( count = 0; count < calendar()->daysInWeek( d->mDate ); ++count ) {
        rect = metrics.boundingRect( calendar()->weekDayName( count + 1, KCalendarSystem::ShortDayName ) );
        d->maxCell.setWidth( qMax( d->maxCell.width(), rect.width() ) );
        d->maxCell.setHeight( qMax( d->maxCell.height(), rect.height() ) );
    }
    // ----- compare with a real wide number and add some space:
    rect = metrics.boundingRect( QLatin1String( "88" ) );
    d->maxCell.setWidth( qMax( d->maxCell.width() + 2, rect.width() ) );
    d->maxCell.setHeight( qMax( d->maxCell.height() + 4, rect.height() ) );
}

void KDateTable::wheelEvent ( QWheelEvent * e )
{
    setDate( calendar()->addMonths( d->mDate, -( int )( e->delta() / 120 ) ) );
    e->accept();
}

bool KDateTable::event(QEvent *ev)
{
    switch (ev->type()) {
        case QEvent::HoverMove:
        {
            QHoverEvent *e = static_cast<QHoverEvent *>(ev);
            const int row = e->pos().y() * d->numWeekRows / height();
            int col;
            if ( layoutDirection() == Qt::RightToLeft ) {
                col = d->numDayColumns - ( e->pos().x() * d->numDayColumns / width() ) - 1;
            } else {
                col = e->pos().x() * d->numDayColumns / width();
            }

            const int pos = row < 1 ? -1 : (d->numDayColumns * (row - 1)) + col;

            if (pos != d->hoveredPos) {
                d->hoveredPos = pos;
                update();
            }
            break;
        }
        case QEvent::HoverLeave:
            if (d->hoveredPos != -1) {
                d->hoveredPos = -1;
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
    row = mouseCoord.y() * d->numWeekRows / height();
    if ( layoutDirection() == Qt::RightToLeft ) {
        col = d->numDayColumns - ( mouseCoord.x() * d->numDayColumns / width() ) - 1;
    } else {
        col = mouseCoord.x() * d->numDayColumns / width();
    }

    if( row < 1 || col < 0 ) { // the user clicked on the frame of the table
        return;
    }

    // Rows and columns are zero indexed.  The (row - 1) below is to avoid counting
    // the row with the days of the week in the calculation.

    // old selected date:
    temp = posFromDate( d->mDate );

    // new position and date
    pos = ( d->numDayColumns * ( row - 1 ) ) + col;
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

    if (  e->button() == Qt::RightButton && d->popupMenuEnabled ) {
        KMenu * menu = new KMenu();
        menu->addTitle( calendar()->formatDate( clickedDate ) );
        emit aboutToShowContextMenu( menu, clickedDate );
        menu->popup( e->globalPos() );
    }
}

bool KDateTable::setDate( const QDate& date_ )
{
    if( date_.isNull() || ! calendar()->isValid( date_ ) ) {
        return false;
    }

    bool changed = false;

    if( d->mDate != date_ ) {
        emit( dateChanged( d->mDate, date_ ) );
        d->mDate = date_;
        emit( dateChanged( d->mDate ) );
        changed = true;
    }

    // set weekday number of first day of this month, but this may not be a valid date so fake
    // it if needed e.g. in QDate Mon 1 Jan -4713 is not valid when it should be, so fake as day 1
    QDate firstDayOfMonth;
    if ( calendar()->setYMD( firstDayOfMonth,
                             calendar()->year( d->mDate ), calendar()->month( d->mDate ), 1 ) ) {
        d->weekDayFirstOfMonth = calendar()->dayOfWeek( firstDayOfMonth );
    } else {
        d->weekDayFirstOfMonth = calendar()->dayOfWeek( d->mDate ) -
                                 ( ( calendar()->day( d->mDate ) - 1 ) % d->numDayColumns );
        if ( d->weekDayFirstOfMonth <= 0 ) {
            d->weekDayFirstOfMonth = d->weekDayFirstOfMonth + d->numDayColumns;
        }
    }

    d->numDaysThisMonth = calendar()->daysInMonth( d->mDate );

    if( changed ) {
        update();
    }

    return true;
}

const QDate &KDateTable::date() const
{
    return d->mDate;
}

const KCalendarSystem *KDateTable::calendar() const
{
    if ( d->m_calendar ) {
        return d->m_calendar;
    }

    return  KGlobal::locale()->calendar();
}

bool KDateTable::setCalendar( KCalendarSystem *calendar_ )
{
    // Delete the old calendar first, provided it's not the global (better to be safe...)
    if ( d->m_calendar && d->m_calendar != KGlobal::locale()->calendar() ) {
        delete d->m_calendar;
    }

    d->m_calendar = 0;

    // Don't actually set calendar if it's the global, setting to 0 will cause global to be returned
    if ( calendar_ != KGlobal::locale()->calendar() ) {
        d->m_calendar = calendar_;

        // Need to redraw to display correct calendar
        d->numDayColumns = calendar()->daysInWeek( d->mDate );
        setDate( d->mDate );
        // JPL not 100% sure we need to emit
        emit( dateChanged( d->mDate, d->mDate ) );
        emit( dateChanged( d->mDate ) );
        update();
    }

    return true;
}

bool KDateTable::setCalendar( const QString &calendarType )
{
    // If type passed in is the same as the global, then use the global instead
    if ( calendarType != KGlobal::locale()->calendarType() ) {
        return( setCalendar( KCalendarSystem::create( calendarType ) ) );
    } else {
        // Delete the old calendar first, provided it's not the global (better to be safe...)
        if ( d->m_calendar && d->m_calendar != KGlobal::locale()->calendar() ) {
            delete d->m_calendar;
        }
        d->m_calendar = 0;
        return true;
    }
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
    if( d->maxCell.height() > 0 && d->maxCell.width() > 0 ) {
        return QSize( qRound( d->maxCell.width() * d->numDayColumns ),
                      ( qRound( d->maxCell.height() + 2 ) * d->numWeekRows ) );
    } else {
        kDebug() << "KDateTable::sizeHint: obscure failure - " << endl;
        return QSize( -1, -1 );
    }
}

void KDateTable::setPopupMenuEnabled( bool enable )
{
    d->popupMenuEnabled = enable;
}

bool KDateTable::popupMenuEnabled() const
{
    return d->popupMenuEnabled;
}

void KDateTable::setCustomDatePainting( const QDate &date, const QColor &fgColor, BackgroundMode bgMode, const QColor &bgColor )
{
    if ( !fgColor.isValid() ) {
        unsetCustomDatePainting( date );
        return;
    }

    KDateTablePrivate::DatePaintingMode *mode = new KDateTablePrivate::DatePaintingMode;
    mode->bgMode = bgMode;
    mode->fgColor = fgColor;
    mode->bgColor = bgColor;

    d->customPaintingModes.insert( date.toJulianDay(), mode );
    d->useCustomColors = true;
    update();
}

void KDateTable::unsetCustomDatePainting( const QDate &date )
{
    d->customPaintingModes.remove( date.toJulianDay() );
    if ( d->customPaintingModes.isEmpty() ) d->useCustomColors = false;
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
    QRect desktopGeometry = KGlobalSettings::desktopGeometry( pos );

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
    connect( this, SIGNAL( leaveModality() ),
             &eventLoop, SLOT( quit() ) );
    eventLoop.exec();

    hide();
    return d->result;
}

int KPopupFrame::exec( int x, int y )
{
    return exec( QPoint( x, y ) );
}

#include "kdatetable.moc"
