/*
    Copyright 2011 John Layt <john@layt.net>

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

#ifndef KDATETIMEEDIT_H
#define KDATETIMEEDIT_H

#include <kdeui_export.h>

#include <QtGui/QWidget>

#include "klocale.h"
#include "kdatetime.h"

class KDateTimeEditPrivate;
class KCalendarSystem;

class KDEUI_EXPORT KDateTimeEdit : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged USER true)
    Q_PROPERTY(QTime time READ time WRITE setTime NOTIFY timeChanged USER true)
    Q_PROPERTY(int timeListInterval READ timeListInterval WRITE setTimeListInterval)
    Q_PROPERTY(Options options READ options WRITE setOptions)
    Q_FLAGS(Options)

public:

    /**
     * Options provided by the widget
     * @see options
     * @see setOptions
     */
    enum Option {
        ShowCalendar     = 0x00001,  /**< If the Calendar System edit is displayed */
        ShowDate         = 0x00002,  /**< If the Date is displayed */
        ShowTime         = 0x00004,  /**< If the Time is displayed */
        ShowTimeSpec     = 0x00008,  /**< If the Time Spec is displayed */
        //EditCalendar     = 0x00010,  /**< Allow the user to manually edit the calendar */
        EditDate         = 0x00020,  /**< Allow the user to manually edit the date */
        EditTime         = 0x00040,  /**< Allow the user to manually edit the time */
        //EditTimeSpec     = 0x00080,  /**< Allow the user to manually edit the time spec */
        SelectCalendar   = 0x00100,  /**< Allow the user to select a calendar */
        SelectDate       = 0x00200,  /**< Allow the user to select a date */
        SelectTime       = 0x00400,  /**< Allow the user to select a time */
        SelectTimeSpec   = 0x00800,  /**< Allow the user to select a time spec */
        DatePicker       = 0x01000,  /**< Show a date picker */
        DateKeywords     = 0x02000,  /**< Show date keywords */
        ForceTime        = 0x04000,  /**< The entered time can only be a selected time */
        WarnOnInvalid    = 0x08000   /**< Show a warning on focus out if the date or time is invalid */
    };
    Q_DECLARE_FLAGS(Options, Option)

    /**
     * Create a new KDateTimeEdit widget
     */
    explicit KDateTimeEdit(QWidget *parent = 0);

    /**
     * Destroy the widget
     */
    virtual ~KDateTimeEdit();

    /**
     * Return the currently set widget options
     *
     * @return the currently set widget options
     */
    Options options() const;

    /**
     * Return the currently selected date, time and time spec
     *
     * @return the currently selected date, time and time spec
     */
    KDateTime dateTime() const;

    /**
     * Returns the Calendar System type used by the widget
     *
     * @see KLocale::CalendarSystem
     * @see setCalendarSystem()
     * @return the Calendar System currently used
     */
    KLocale::CalendarSystem calendarSystem() const;

    /**
     * Returns a pointer to the Calendar System object used by this widget
     *
     * Usually this will be the Global Calendar System using the Global Locale,
     * but this may have been changed to a custom Calendar System possibly
     * using a custom Locale.
     *
     * Normally you will not need to access this object.
     *
     * @see KCalendarSystem
     * @see setCalendar
     * @return the current calendar system instance
     */
    const KCalendarSystem *calendar() const;

    /**
     * Return the currently selected date
     *
     * @return the currently selected date
     */
    QDate date() const;

    /**
     * Return the currently selected time
     *
     * @return the currently selected time
     */
    QTime time() const;

    /**
     * Return the currently selected time spec
     *
     * @return the currently selected time spec
     */
    KDateTime::Spec timeSpec() const;

    /**
     * Returns the list of Calendar Systems displayed.
     *
     * @param calendars the list of calendar systems to display
     */
    QList<KLocale::CalendarSystem> calendarSystemsList() const;

    /**
     * Return the current minimum date and time
     *
     * @return the current minimum date and time
     */
    KDateTime minimumDateTime() const;

    /**
     * Return the current maximum date and time
     *
     * @return the current maximum date and time
     */
    KDateTime maximumDateTime() const;

    /**
     * Return the currently set date display format
     *
     * By default this is the Short Date
     *
     * @return the currently set date format
     */
    KLocale::DateFormat dateDisplayFormat() const;

    /**
     * Return the map of dates listed in the drop-down and their displayed
     * string forms.
     *
     * @see setDateMap()
     * @return the select date map
     */
    QMap<QDate, QString> dateMap() const;

    /**
     * Return the currently set time format
     *
     * By default this is the Short Time
     *
     * @return the currently set time format
     */
    KLocale::TimeFormatOptions timeDisplayFormat() const;

    /**
     * Return the time list interval able to be selected
     *
     * @return the select time intervals in minutes
     */
    int timeListInterval() const;

    /**
     * Return the list of times able to be selected in the drop-down.
     *
     * @see setTimeList()
     * @see timeListInterval()
     * @see setTimeListInterval()
     * @return the select time list
     */
    QList<QTime> timeList() const;

    /**
     * Return the list of time zones able to be selected
     *
     * @param zones the time zones to display
     */
    KTimeZones::ZoneMap timeZones() const;

    /**
     * Return if the current user input is valid
     *
     * If the user input is null then it is not valid
     *
     * @see isNull()
     * @return if the current user input is valid
     */
    bool isValid() const;

    /**
     * Return if the current user input is null
     *
     * @see isValid()
     * @return if the current user input is null
     */
    bool isNull() const;

    /**
     * Return if the current user input date is valid
     *
     * If the user input date is null then it is not valid
     *
     * @see isNullDate()
     * @return if the current user input date is valid
     */
    bool isValidDate() const;

    /**
     * Return if the current user input date is null
     *
     * @see isValidDate()
     * @return if the current user input date is null
     */
    bool isNullDate() const;
    /**
     * Return if the current user input time is valid
     *
     * If the user input time is null then it is not valid
     *
     * @see isNullTime()
     * @return if the current user input time is valid
     */
    bool isValidTime() const;

    /**
     * Return if the current user input time is null
     *
     * @see isValidTime()
     * @return if the current user input time is null
     */
    bool isNullTime() const;

Q_SIGNALS:

    /**
     * Signal if the date or time has been manually entered by the user.
     *
     * The returned date and time may be invalid.
     *
     * @param dateTime the new date, time and time spec
     */
    void dateTimeEntered(const KDateTime &dateTime);

    /**
     * Signal if the date or time has been changed either manually by the user
     * or programatically.
     *
     * The returned date and time may be invalid.
     *
     * @param dateTime the new date, time and time spec
     */
    void dateTimeChanged(const KDateTime &dateTime);

    /**
     * Signal if the date or time is being manually edited by the user.
     *
     * The returned date and time may be invalid.
     *
     * @param dateTime the new date, time and time spec
     */
    void dateTimeEdited(const KDateTime &dateTime);

    /**
     * Signal if the Calendar System has been manually entered by the user.
     *
     * @param calendarSystem the new calendar system
     */
    void calendarEntered(KLocale::CalendarSystem calendarSystem);

    /**
     * Signal if the Calendar System has been changed either manually by the user
     * or programatically.
     *
     * @param calendarSystem the new calendar system
     */
    void calendarChanged(KLocale::CalendarSystem calendarSystem);

    /**
     * Signal if the date has been manually entered by the user.
     *
     * The returned date may be invalid.
     *
     * @param date the new date
     */
    void dateEntered(const QDate &date);

    /**
     * Signal if the date has been changed either manually by the user
     * or programatically.
     *
     * The returned date may be invalid.
     *
     * @param date the new date
     */
    void dateChanged(const QDate &date);

    /**
     * Signal if the date is being manually edited by the user.
     *
     * The returned date may be invalid.
     *
     * @param date the new date
     */
    void dateEdited(const QDate &date);

    /**
     * Signal if the time has been manually entered by the user.
     *
     * The returned time may be invalid.
     *
     * @param time the new time
     */
    void timeEntered(const QTime &time);

    /**
     * Signal if the time has been changed either manually by the user
     * or programatically.
     *
     * The returned time may be invalid.
     *
     * @param time the new time
     */
    void timeChanged(const QTime &time);

    /**
     * Signal if the time is being manually edited by the user.
     *
     * The returned time may be invalid.
     *
     * @param time the new time
     */
    void timeEdited(const QTime &time);

    /**
     * Signal if the time spec has been changed manually by the user.
     *
     * @param timeSpec the new time spec
     */
    void timeSpecEntered(const KDateTime::Spec &spec);

    /**
     * Signal if the time spec has been changed either manually by the user
     * or programatically.
     *
     * @param timeSpec the new time spec
     */
    void timeSpecChanged(const KDateTime::Spec &spec);

public Q_SLOTS:

    /**
     * Set the new widget options
     *
     * @param options the new widget options
     */
    void setOptions(Options options);

    /**
     * Set the currently selected date, time and time spec
     *
     * @param dateTime the new date, time and time spec
     */
    void setDateTime(const KDateTime &dateTime);

    /**
     * Set the Calendar System used for this widget.  Uses the global locale.
     *
     * @see KLocale::CalendarSystem
     * @see calendarSystem()
     * @param calendarSystem the Calendar System to use
     */
    void setCalendarSystem(KLocale::CalendarSystem calendarSystem);

    /**
     * Changes the calendar system to use.  Can use its own local locale if set.
     *
     * You retain ownership of the calendar object, it will not be destroyed with the widget.
     *
     * @param calendar the calendar system object to use, defaults to global
     */
    void setCalendar(KCalendarSystem *calendar = 0);

    /**
     * Set the currently selected date
     *
     * @param date the new date
     */
    void setDate(const QDate &date);

    /**
     * Set the currently selected time
     *
     * @param time the new time
     */
    void setTime(const QTime &time);

    /**
     * Set the current time spec
     *
     * @param spec the new spec
     */
    void setTimeSpec(const KDateTime::Spec &spec);

    /**
     * Set the minimum and maximum date and time range
     *
     * To enable range checking provide two valid dates.
     * To disable range checking provide two invalid dates, or call
     * clearDateRange;
     *
     * @param minDateTime the minimum date and time
     * @param maxDateTime the maximum date and time
     * @param minWarnMsg the minimum warning message
     * @param maxWarnMsg the maximum warning message
     */
    void setDateTimeRange(const KDateTime &minDateTime,
                          const KDateTime &maxDateTime,
                          const QString &minWarnMsg = QString(),
                          const QString &maxWarnMsg = QString());

    /**
     * Reset the minimum and maximum date and time to the default
     */
    void resetDateTimeRange();

    /**
     * Set the minimum allowed date.
     *
     * If the date is invalid, or more than current maximum,
     * then the minimum will not be set.
     *
     * @see setMaximumDateTime()
     * @see setDateRange()
     * @param maxDate the minimum date
     * @param maxWarnMsg the minimum warning message
     */
    void setMinimumDateTime(const KDateTime &minDateTime, const QString &minWarnMsg = QString());

    /**
     * Reset the minimum date and time to the default
     */
    void resetMinimumDateTime();

    /**
     * Set the maximum allowed date.
     *
     * If the date is invalid, or less than current minimum,
     * then the maximum will not be set.
     *
     * @see setMinimumDateTime()
     * @see setDateRange()
     * @param maxDate the maximum date
     * @param maxWarnMsg the maximum warning message
     */
    void setMaximumDateTime(const KDateTime &maxDateTime, const QString &maxWarnMsg = QString());

    /**
     * Reset the minimum date and time to the default
     */
    void resetMaximumDateTime();

    /**
     * Sets the date format to display.
     *
     * By default is the Short Date format.
     *
     * @param format the date format to use
     */
    void setDateDisplayFormat(KLocale::DateFormat format);

    /**
     * Set the list of Calendar Systems to display.
     *
     * @param calendars the list of calendar systems to display
     */
    void setCalendarSystemsList(QList<KLocale::CalendarSystem> calendars);

    /**
     * Set the list of dates able to be selected from the drop-down and the
     * string form to display for those dates, e.g. "2010-01-01" and "Yesterday".
     *
     * Any invalid or duplicate dates will be used, the list will NOT be
     * sorted, and the minimum and maximum date will not be affected.
     *
     * The @p dateMap is keyed by the date to be listed and the value is the
     * string to be displayed.  If you want the date to be displayed in the
     * default date format then the string should be null.  If you want a
     * separator to be displayed then set the string to "seperator".
     *
     * @see dateMap()
     * @param dateMap the map of dates able to be selected
     */
    void setDateMap(QMap<QDate, QString> dateMap);

    /**
     * Sets the time format to display.
     *
     * By default is the Short Time format.
     *
     * @param format the time format to use
     */
    void setTimeDisplayFormat(KLocale::TimeFormatOptions formatOptions);

    /**
     * Set the interval between times able to be selected from the drop-down.
     *
     * The combo drop-down will be populated with times every @param minutes
     * apart, starting from the minimumTime() and ending at maximumTime().
     *
     * If the ForceInterval option is set then any time manually typed into the
     * combo line edit will be forced to the nearest interval.
     *
     * This interval must be an exact divisor of the valid time range hours.
     * For example with the default 24 hour range @p interval must divide 1440
     * minutes exactly, meaning 1, 6 and 90 are valid but 7, 31 and 91 are not.
     *
     * Setting the time list interval will override any time list previously set
     * via setTimeList().
     *
     * @see timeListInterval()
     * @param minutes the time list interval to display
     */
    void setTimeListInterval(int minutes);

    /**
     * Set the list of times able to be selected from the drop-down.
     *
     * Setting the time list will override any time interval previously set via
     * setTimeListInterval().
     *
     * Any invalid or duplicate times will be ignored, and the list will be
     * sorted.
     *
     * The minimum and maximum time will automatically be set to the earliest
     * and latest value in the list.
     *
     * @see timeList()
     * @param timeList the list of times able to be selected
     * @param minWarnMsg the minimum warning message
     * @param maxWarnMsg the maximum warning message
     */
    void setTimeList(QList<QTime> timeList,
                     const QString &minWarnMsg = QString(),
                     const QString &maxWarnMsg = QString());

    /**
     * Set the time zones able to be selected
     *
     * @param zones the time zones to display
     */
    void setTimeZones(const KTimeZones::ZoneMap &zones);

protected:

    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

    /**
     * Assign the date, time and time spec for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules,
     * but reimplementations must call the parent method at the end.
     *
     * @param datetime the new date and time
     */
    virtual void assignDateTime(const KDateTime &dateTime);

    /**
     * Assign the date for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules,
     * but reimplementations must call the parent method at the end.
     *
     * @param date the new date
     */
    virtual void assignDate(const QDate &date);

    /**
     * Assign the calendar system for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules,
     * but reimplementations must call the parent method at the end.
     *
     * @param calendarSystem the new calendar system
     */
    void assignCalendarSystem(KLocale::CalendarSystem calendarSystem);

    /**
     * Assign the time for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules,
     * but reimplementations must call the parent method at the end.
     *
     * @param time the new time
     */
    virtual void assignTime(const QTime &time);

    /**
     * Assign the time spec for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules,
     * but reimplementations must call the parent method at the end.
     *
     * @param spec the new time spec
     */
    void assignTimeSpec(const KDateTime::Spec &spec);

private:

    friend class KDateTimeEditPrivate;
    KDateTimeEditPrivate *const d;

    Q_PRIVATE_SLOT(d, void selectCalendar(int))
    Q_PRIVATE_SLOT(d, void enterCalendar(KLocale::CalendarSystem))
    Q_PRIVATE_SLOT(d, void selectTimeZone(int))
    Q_PRIVATE_SLOT(d, void enterTimeZone(const QString&))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDateTimeEdit::Options)

#endif // KDATETIMEEDIT_H
