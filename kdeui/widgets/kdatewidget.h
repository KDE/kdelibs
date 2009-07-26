/* This file is part of the KDE libraries
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

#ifndef KDATEWIDGET_H
#define KDATEWIDGET_H

#include <kdeui_export.h>

#include <QtGui/QWidget>

class KCalendarSystem;

class QDate;

/**
 * @short A date selection widget.
 *
 * This widget can be used to display or allow user selection of a date.
 *
 * \image html kdatewidget.png "KDE Date Widget"
 *
 * @see KDatePicker
 *
 * @author Waldo Bastian <bastian@kde.org>, John Layt <john@layt.net>
 */
class KDEUI_EXPORT KDateWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QDate date READ date WRITE setDate USER true )
//FIXME    Q_PROPERTY( KCalendarSystem calendar READ calendar WRITE setCalendar USER true )

public:
    /**
     * Constructs a date selection widget.
     */
    explicit KDateWidget( QWidget *parent = 0 );

    /**
     * Constructs a date selection widget with the initial date set to @p date.
     */
    explicit KDateWidget( const QDate &date, QWidget *parent = 0 );

    /**
     * Destructs the date selection widget.
     */
    virtual ~KDateWidget();

    /**
     * Returns the currently selected date.
     */
    const QDate& date() const;

    /**
     * Changes the selected date to @p date.
     * 
     * @return @c true if the date was successfully set, @c false otherwise
     */
    bool setDate( const QDate &date );

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

Q_SIGNALS:
    /**
     * Emitted whenever the date of the widget
     * is changed, either with setDate() or via user selection.
     */
    void changed( const QDate& date );

protected:
    void init( const QDate& date );

protected Q_SLOTS:
    void slotDateChanged();

private:
    class KDateWidgetPrivate;
    KDateWidgetPrivate * const d;
};

#endif // KDATEWIDGET_H

