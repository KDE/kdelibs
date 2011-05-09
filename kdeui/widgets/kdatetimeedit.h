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

#ifndef KDATETIMEWIDGET_H
#define KDATETIMEWIDGET_H

#include <kdeui_export.h>

#include <QtGui/QWidget>

#include "kdatetime.h"

class KDateTimeEditPrivate;

class KDEUI_EXPORT KDateTimeEdit : public QWidget
{
    Q_OBJECT

public:

    /**
     * Create a new KDateTimeEdit widget
     */
    explicit KDateTimeEdit(QWidget *parent = 0);

    /**
     * Destroy the widget
     */
    virtual ~KDateTimeEdit();

    /**
     * Return the currently selected date and time
     *
     * @return the currently selected date and time
     */
    QDateTime dateTime() const;

    /**
     * Return the currently selected date
     *
     * @return the currently selected date and time
     */
    QDate date() const;

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
     * Set the time interval to display in the time widget
     *
     * @param minutes the interval to display
     */
    void setTimeInterval(int minutes);

    /**
     * Return the displayed time interval
     *
     * @return the displayed time interval
     */
    int timeInterval() const;

    /**
     * Set the widget to be read only
     *
     * @param readOnly if th widget is read only
     */
    void setReadOnly( bool readOnly );

    /**
     * Return if the widget is read only
     *
     * @return if the widget is read only
     */
    bool isReadOnly() const;

Q_SIGNALS:


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

protected:

    virtual bool eventFilter(QObject* object, QEvent* event);
/*
    virtual void assignDateTime(const QDateTime &dateTime);
    virtual void assignDate(const QDate &date);
    virtual void assignTime(const QTime &time);
*/
private:

    KDateTimeEditPrivate *const d;
};

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
