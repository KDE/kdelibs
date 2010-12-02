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

#ifndef KDATETABLE_H
#define KDATETABLE_H

#include <kdeui_export.h>

#include <QtGui/QValidator>
#include <QtGui/QLineEdit>
#include <QtCore/QDateTime>

class KMenu;
class KCalendarSystem;
class KColorScheme;

/**
 * Frame with popup menu behavior.
 * @author Tim Gilman, Mirko Boehm
 */
class KDEUI_EXPORT KPopupFrame : public QFrame
{
    Q_OBJECT
protected:
    /**
     * Catch key press events.
     */
    virtual void keyPressEvent( QKeyEvent *e );

public Q_SLOTS:
    /**
     * Close the popup window. This is called from the main widget, usually.
     * @p r is the result returned from exec().
     */
    void close( int r );

public:
    /**
     * The contructor. Creates a dialog without buttons.
     */
    KPopupFrame( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~KPopupFrame();

    /**
     * Set the main widget. You cannot set the main widget from the constructor,
     * since it must be a child of the frame itselfes.
     * Be careful: the size is set to the main widgets size. It is up to you to
     * set the main widgets correct size before setting it as the main
     * widget.
     */
    void setMainWidget( QWidget* m );

    /**
     * The resize event. Simply resizes the main widget to the whole
     * widgets client size.
     */
    virtual void resizeEvent( QResizeEvent *resize );

    /**
     * Open the popup window at position pos.
     */
    void popup( const QPoint &pos );

    /**
     * Execute the popup window.
     */
    int exec( const QPoint &p );

    /**
     * Execute the popup window.
     */
    int exec( int x, int y );

Q_SIGNALS:
    void leaveModality();

private:
    class KPopupFramePrivate;
    friend class KPopupFramePrivate;
    KPopupFramePrivate * const d;

    Q_DISABLE_COPY( KPopupFrame )
};

/**
* Validates user-entered dates.
*/
class KDEUI_EXPORT KDateValidator : public QValidator
{
public:
    KDateValidator( QWidget *parent = 0 );
    virtual State validate( QString &text, int &e ) const;
    virtual void fixup ( QString &input ) const;
    State date( const QString &text, QDate &date ) const;
private:
    class KDateValidatorPrivate;
    friend class KDateValidatorPrivate;
    KDateValidatorPrivate * const d;
};

/**
 * Date selection table.
 * This is a support class for the KDatePicker class.  It just
 * draws the calendar table without titles, but could theoretically
 * be used as a standalone.
 *
 * When a date is selected by the user, it emits a signal:
 * dateSelected(QDate)
 *
 * \image html kdatetable.png "KDE Date Selection Table"
 *
 * @internal
 * @author Tim Gilman, Mirko Boehm
 */
class KDEUI_EXPORT KDateTable : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QDate date READ date WRITE setDate )
//FIXME    Q_PROPERTY( KCalendarSystem calendar READ calendar WRITE setCalendar USER true )
    Q_PROPERTY( bool popupMenu READ popupMenuEnabled WRITE setPopupMenuEnabled )

public:
    /**
     * The constructor.
     */
    explicit KDateTable( QWidget* parent = 0 );

    /**
     * The constructor.
     */
    explicit KDateTable( const QDate&, QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~KDateTable();

    /**
     * Returns a recommended size for the widget.
     * To save some time, the size of the largest used cell content is
     * calculated in each paintCell() call, since all calculations have
     * to be done there anyway. The size is stored in maxCell. The
     * sizeHint() simply returns a multiple of maxCell.
     */
    virtual QSize sizeHint() const;

    /**
     * Set the font size of the date table.
     */
    void setFontSize( int size );

    /**
     * Select and display this date.
     */
    bool setDate( const QDate &date );

    /**
     * @returns the selected date.
     */
    const QDate &date() const;

    /**
     * Returns the currently selected calendar system.
     * 
     * @return a KCalendarSystem object
     */
    const KCalendarSystem *calendar() const;

    /**
     * Changes the calendar system to use.  Can use its own local locale if set.
     * 
     * @param calendar the calendar system object to use, defaults to global
     * 
     * @return @c true if the calendar system was successfully set, @c false otherwise
     */
    bool setCalendar( KCalendarSystem *calendar = 0 );

    /**
     * Changes the calendar system to use.  Will always use global locale.
     * 
     * @param calendarType the calendar system type to use
     * 
     * @return @c true if the calendar system was successfully set, @c false otherwise
     */
    bool setCalendar( const QString &calendarType );

    /**
     * Enables a popup menu when right clicking on a date.
     *
     * When it's enabled, this object emits a aboutToShowContextMenu signal
     * where you can fill in the menu items.
     */
    void setPopupMenuEnabled( bool enable );

    /**
     * Returns if the popup menu is enabled or not
     */
    bool popupMenuEnabled() const;

    enum BackgroundMode { NoBgMode = 0, RectangleMode, CircleMode };

    /**
     * Makes a given date be painted with a given foregroundColor, and background
     * (a rectangle, or a circle/ellipse) in a given color.
     */
    void setCustomDatePainting( const QDate &date, const QColor &fgColor, 
                                BackgroundMode bgMode = NoBgMode, const QColor &bgColor = QColor() );

    /**
     * Unsets the custom painting of a date so that the date is painted as usual.
     */
    void unsetCustomDatePainting( const QDate &date );

protected:
    /**
     * calculate the position of the cell in the matrix for the given date.
     * The result is the 0-based index.
     */
    virtual int posFromDate( const QDate &date );

    /**
     * calculate the date that is displayed at a given cell in the matrix. pos is the
     * 0-based index in the matrix. Inverse function to posForDate().
     */
    virtual QDate dateFromPos( int pos );

    virtual void paintEvent( QPaintEvent *e );

    /**
     * React on mouse clicks that select a date.
     */
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void wheelEvent( QWheelEvent *e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void focusInEvent( QFocusEvent *e );
    virtual void focusOutEvent( QFocusEvent *e );

    /**
     * Cell highlight on mouse hovering
     */
    virtual bool event(QEvent *e);

Q_SIGNALS:
    /**
     * The selected date changed.
     */
    void dateChanged( const QDate &date );

    /**
     * This function behaves essentially like the one above.
     * The selected date changed.
     * @param cur The current date
     * @param old The date before the date was changed
     */
    void dateChanged( const QDate &cur, const QDate &old );

    /**
     * A date has been selected by clicking on the table.
     */
    void tableClicked();

    /**
     * A popup menu for a given date is about to be shown (as when the user
     * right clicks on that date and the popup menu is enabled). Connect
     * the slot where you fill the menu to this signal.
     */
    void aboutToShowContextMenu( KMenu *menu, const QDate &date );

private:
    Q_PRIVATE_SLOT( d, void nextMonth() )
    Q_PRIVATE_SLOT( d, void previousMonth() )
    Q_PRIVATE_SLOT( d, void beginningOfMonth() )
    Q_PRIVATE_SLOT( d, void endOfMonth() )
    Q_PRIVATE_SLOT( d, void beginningOfWeek() )
    Q_PRIVATE_SLOT( d, void endOfWeek() )

private:
    class KDateTablePrivate;
    friend class KDateTablePrivate;
    KDateTablePrivate * const d;

    void initAccels();
    void paintCell( QPainter *painter, int row, int col, const KColorScheme &colorScheme );

    Q_DISABLE_COPY( KDateTable )
};

#endif // KDATETABLE_H
