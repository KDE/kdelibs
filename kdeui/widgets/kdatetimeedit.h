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

    //Q_PROPERTY(KDateTime dateTime READ dateTime WRITE setDateTime NOTIFY dateTimeChanged USER true)
    Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime NOTIFY dateTimeChanged USER true)
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged USER true)
    Q_PROPERTY(QTime time READ time WRITE setTime NOTIFY timeChanged USER true)
    Q_PROPERTY(KDateTime::Spec timeSpec READ timeSpec WRITE setTimeSpec)
    Q_PROPERTY(Options options READ options WRITE setOptions)
    Q_PROPERTY(QDateTime minimumDateTime READ minimumDateTime WRITE setMinimumDateTime RESET clearMinimumDateTime)
    Q_PROPERTY(QDateTime maximumDateTime READ maximumDateTime WRITE setMaximumDateTime RESET clearMaximumDateTime)
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate RESET clearMinimumDate)
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate RESET clearMaximumDate)
    Q_PROPERTY(QTime minimumTime READ minimumTime WRITE setMinimumTime RESET clearMinimumTime)
    Q_PROPERTY(QTime maximumTime READ maximumTime WRITE setMaximumTime RESET clearMaximumTime)

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
        ShowTimeSpec     = 0x00008,  /**< If the Timezone is displayed */
        EditCalendar     = 0x00010,  /**< Allow the user to edit the calendar */
        EditDate         = 0x00020,  /**< Allow the user to edit the date */
        EditTime         = 0x00040,  /**< Allow the user to edit the time */
        EditTimeSpec     = 0x00080,  /**< Allow the user to edit the time spec */
        SelectCalendar   = 0x00100,  /**< Allow the user to select a calendar */
        SelectDate       = 0x00200,  /**< Allow the user to select a date */
        SelectTime       = 0x00400,  /**< Allow the user to select a time */
        SelectTimeSpec   = 0x00800,  /**< Allow the user to select a time */
        CustomDateFormat = 0x01000,  /**< If the Date edit uses a custom KLocale::DateFormat */
        FancyDate        = 0x02000,  /**< If the Date edit accepts 'Fancy' dates, e.g. 'Tomorrow' */
        CustomTimeFormat = 0x04000,  /**< If the Time edit uses a custom KLocale::TimeFormat */
        ShowSeconds      = 0x08000,  /**< If the Time edit shows seconds */
        ShowMSecs        = 0x10000,  /**< If the Time edit shows milliseconds */
        ApplyMinMax      = 0x20000,  /**< If the widget applies teh min/max dates/times */
        AcceptInvalid    = 0x30000   /**< If the widget accepts invalid dates/times */
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
     * Return the currently selected date, time and timezone
     *
     * @return the currently selected date, time and timezone
     */
    KDateTime dateTimeSpec() const;

    /**
     * Return the currently selected date and time
     *
     * @return the currently selected date and time
     */
    QDateTime dateTime() const;

    /**
     * Return the currently selected date
     *
     * @return the currently selected date
     */
    QDate date() const;

    /**
     * Returns the Calendar System type used by the widget
     *
     * @see KLocale::CalendarSystem
     * @see setCalendarSystem()
     * @return the Calendar System currently used
     */
    KLocale::CalendarSystem calendarSystem();

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
     * Return the currently selected time
     *
     * @return the currently selected time
     */
    QTime time() const;

    /**
     * Return the currently selected timezone
     *
     * @return the currently selected timezone
     */
    KDateTime::Spec timeSpec() const;

    /**
     * Return if the current user input is valid
     *
     * @return if the current user input is valid
     */
    bool isValid()const;

    /**
     * Return the currently set widget options
     *
     * @return the currently set widget options
     */
    Options options() const;

    /**
     * Return the current minimum date and time
     *
     * @return the current minimum date and time
     */
    QDateTime minimumDateTime() const;

    /**
     * Set the minimum date and time
     *
     * @param minDateTime the minimum date and time
     * @param minErrorMsg the minimum error message
     */
    void setMinimumDateTime(const QDateTime &minDateTime,
                            const QString &minErrorMsg = QString());

    /**
     * Reset the minimum date and time to the default
     */
    void clearMinimumDateTime();

    /**
     * Return the current maximum date and time
     *
     * @return the current maximum date and time
     */
    QDateTime maximumDateTime() const;

    /**
     * Set the maximum date and time
     *
     * @param maxDateTime the maximum date and time
     * @param maxErrorMsg the maximum error message
     */
    void setMaximumDateTime(const QDateTime &maxDateTime,
                            const QString &maxErrorMsg = QString());

    /**
     * Reset the minimum date and time to the default
     */
    void clearMaximumDateTime();

    /**
     * Set the minimum and maximum date and time range
     *
     * @param minDateTime the minimum date and time
     * @param maxDateTime the maximum date and time
     * @param minErrorMsg the minimum error message
     * @param maxErrorMsg the maximum error message
     */
    void setDateTimeRange(const QDateTime &minDateTime,
                          const QDateTime &maxDateTime,
                          const QString &minErrorMsg = QString(),
                          const QString &maxErrorMsg = QString());

    /**
     * Reset the minimum and maximum date and time to the default
     */
    void clearDateTimeRange();


    /**
     * Return the current minimum date
     *
     * @return the current minimum date
     */
    QDate minimumDate() const;

    /**
     * Set the minimum date
     *
     * @param minDate the minimum date
     * @param minErrorMsg the minimum error message
     */
    void setMinimumDate(const QDate &minDate,
                        const QString &minErrorMsg = QString());

    /**
     * Reset the minimum date to the default
     */
    void clearMinimumDate();


    /**
     * Return the current maximum date
     *
     * @return the current maximum date
     */
    QDate maximumDate() const;

    /**
     * Set the maximum date
     *
     * @param maxDate the maximum date
     * @param maxErrorMsg the maximum error message
     */
    void setMaximumDate(const QDate &maxDate,
                        const QString &maxErrorMsg = QString());

    /**
     * Reset the maximum date to the default
     */
    void clearMaximumDate();

    /**
     * Set the minimum and maximum date range
     *
     * @param minDate the minimum date
     * @param maxDate the maximum date
     * @param minErrorMsg the minimum error message
     * @param maxErrorMsg the maximum error message
     */
    void setDateRange(const QDate &minDate,
                      const QDate &maxDate,
                      const QString &minErrorMsg = QString(),
                      const QString &maxErrorMsg = QString());

    /**
     * Reset the minimum and maximum date to the default
     */
    void clearDateRange();

    /**
     * Return the current minimum time
     *
     * @return the current minimum time
     */
    QTime minimumTime() const;

    /**
     * Set the minimum time
     *
     * @param minDateTime the minimum time
     * @param minErrorMsg the minimum error message
     */
    void setMinimumTime(const QTime &minTime,
                        const QString &minErrorMsg = QString());

    /**
     * Reset the minimum time to the default
     */
    void clearMinimumTime();

    /**
     * Return the current maximum time
     *
     * @return the current maximum time
     */
    QTime maximumTime() const;

    /**
     * Set the maximum time
     *
     * @param maxTime the maximum time
     * @param maxErrorMsg the maximum error message
     */
    void setMaximumTime(const QTime &maxTime,
                        const QString &maxErrorMsg = QString());

    /**
     * Reset the maximum time to the default
     */
    void clearMaximumTime();

    /**
     * Set the minimum and maximum time range
     *
     * @param minTime the minimum time
     * @param maxTime the maximum time
     * @param minErrorMsg the minimum error message
     * @param maxErrorMsg the maximum error message
     */
    void setTimeRange(const QTime &minTime,
                      const QTime &maxTime,
                      const QString &minErrorMsg = QString(),
                      const QString &maxErrorMsg = QString());

    /**
     * Reset the minimum and maximum time to the default
     */
    void clearTimeRange();

    /**
     * Set the time interval able to be selected in the time widget
     *
     * @param minutes the interval to display
     */
    void setSelectTimeInterval(int minutes);

    /**
     * Return the time interval able to be selected
     *
     * @return the select time intervals in minutes
     */
    int selectTimeInterval() const;

    /**
     * Changes the calendar system to use.  Can use its own local locale if set.
     *
     * You retain ownership of the calendar object, it will not be destroyed with the widget.
     *
     * @param calendar the calendar system object to use, defaults to global
     */
    void setCalendar(KCalendarSystem *calendar = 0);

    /**
     * Set the timezones able to be selected
     *
     * @param zones the timezones to display
     */
    void setTimeZones(const KTimeZones::ZoneMap &zones);

Q_SIGNALS:

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
     * Signal if the date or time has been manually entered by the user.
     *
     * The returned date and time may be invalid.
     *
     * @param dateTime the new date, time and timezone
     */
    void dateTimeEntered(const KDateTime &dateTime);

    /**
     * Signal if the date or time has been changed either manually by the user
     * or programatically.
     *
     * The returned date and time may be invalid.
     *
     * @param dateTime the new date, time and timezone
     */
    void dateTimeChanged(const KDateTime &dateTime);

    /**
     * Signal if the date or time is being manually edited by the user.
     *
     * The returned date and time may be invalid.
     *
     * @param dateTime the new date, time and timezone
     */
    void dateTimeEdited(const KDateTime &dateTime);

    /**
     * Signal if the date or time has been manually entered by the user.
     *
     * The returned date and time may be invalid.
     *
     * @param dateTime the new date and time
     */
    void dateTimeEntered(const QDateTime &dateTime);

    /**
     * Signal if the date or time has been changed either manually by the user
     * or programatically.
     *
     * The returned date and time may be invalid.
     *
     * @param dateTime the new date and time
     */
    void dateTimeChanged(const QDateTime &dateTime);

    /**
     * Signal if the date or time is being manually edited by the user.
     *
     * The returned date and time may be invalid.
     *
     * @param dateTime the new date and time
     */
    void dateTimeEdited(const QDateTime &dateTime);

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
     * Signal if the time spec has been manually entered by the user.
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
     * Set the currently selected date, time and timezone
     *
     * @param dateTime the new date, time and timezone
     */
    void setDateTime(const KDateTime &dateTime);

    /**
     * Set the currently selected date and time
     *
     * @param dateTime the new date and time
     */
    void setDateTime(const QDateTime &dateTime);

    /**
     * Set the currently selected date
     *
     * @param date the new date
     */
    void setDate(const QDate &date);

    /**
     * Set the Calendar System used for this widget.  Uses the global locale.
     *
     * @see KLocale::CalendarSystem
     * @see calendarSystem()
     * @param calendarSystem the Calendar System to use
     */
    void setCalendarSystem(KLocale::CalendarSystem calendarSystem);

    /**
     * Set the currently selected time
     *
     * @param time the new time
     */
    void setTime(const QTime &time);

    /**
     * Set the currently selected timezone spec
     *
     * @param spec the new spec
     */
    void setTimeSpec(const KDateTime::Spec &spec);

    /**
     * Set the new widget options
     *
     * @param options the new widget options
     */
    void setOptions(Options options);

protected:

    virtual bool eventFilter(QObject *object, QEvent *event);

    /**
     * Assign the date, time and timezone for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules.
     *
     * @param datetime the new date and time
     */
    virtual void assignDateTime(const KDateTime &dateTime);

    /**
     * Assign the date and time for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules.
     *
     * @param datetime the new date and time
     */
    virtual void assignDateTime(const QDateTime &dateTime);

    /**
     * Assign the date for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules.
     *
     * @param date the new date
     */
    virtual void assignDate(const QDate &date);

    /**
     * Assign the calendar system for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules.
     *
     * @param time the new time
     */
    void assignCalendarSystem(KLocale::CalendarSystem calendarSystem);

    /**
     * Assign the time for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules.
     *
     * @param time the new time
     */
    virtual void assignTime(const QTime &time);

    /**
     * Assign the timezone for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules.
     *
     * @param time the new timezone
     */
    void assignTimeSpec(const KDateTime::Spec &spec);

private:

    friend class KDateTimeEditPrivate;
    KDateTimeEditPrivate *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDateTimeEdit::Options)

class KDEUI_EXPORT KTimeEdit : public KDateTimeEdit
{
    Q_OBJECT
public:
    KTimeEdit(QWidget *parent = 0);
};

class KDEUI_EXPORT KDateEdit : public KDateTimeEdit
{
    Q_OBJECT
public:
    KDateEdit(QWidget *parent = 0);
};

#endif // KDATETIMEEDIT_H
