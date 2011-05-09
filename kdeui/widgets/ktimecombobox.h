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

#ifndef KTIMECOMBOBOX_H
#define KTIMECOMBOBOX_H

#include <kdeui_export.h>

#include <QtGui/QWidget>

#include "kcombobox.h"
#include "klocale.h"

class KTimeComboBoxPrivate;

class KDEUI_EXPORT KTimeComboBox : public KComboBox
{
    Q_OBJECT

    Q_PROPERTY(QTime time READ time WRITE setTime NOTIFY timeChanged USER true)
    Q_PROPERTY(QTime minimumTime READ minimumTime RESET resetMinimumTime)
    Q_PROPERTY(QTime maximumTime READ maximumTime RESET resetMaximumTime)
    Q_PROPERTY(Options options READ options WRITE setOptions)
    //Q_PROPERTY(KLocale::TimeFormatOptions displayFormat READ displayFormat WRITE setDisplayFormat)

public:

    /**
     * Options provided by the widget
     * @see options
     * @see setOptions
     */
    enum Option {
        EditTime         = 0x0001,  /**< Allow the user to manually edit the time in the combo line edit */
        SelectTime       = 0x0002,  /**< Allow the user to select the time from a drop-down menu */
        ForceInterval    = 0x0004,  /**< The entered time can only be a selected interval */
        WarnOnInvalid    = 0x0010,  /**< Show a warning on focus out if the time is invalid */
        ErrorOnInvalid   = 0x0020   /**< Show an error on focus out if the time is invalid */
    };
    Q_DECLARE_FLAGS(Options, Option)

    /**
     * Create a new KTimeComboBox widget
     */
    explicit KTimeComboBox(QWidget *parent = 0);

    /**
     * Destroy the widget
     */
    virtual ~KTimeComboBox();

    /**
     * Return the currently selected time
     *
     * @return the currently selected time
     */
    QTime time() const;

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
     * Return the currently set time format
     *
     * By default this is the Short Time
     *
     * @return the currently set time format
     */
    KLocale::TimeFormatOptions displayFormat();

    /**
     * Return the current minimum time
     *
     * @return the current minimum time
     */
    QTime minimumTime() const;

    /**
     * Reset the minimum time to the default
     */
    void resetMinimumTime();

    /**
     * Return the current maximum time
     *
     * @return the current maximum time
     */
    QTime maximumTime() const;

    /**
     * Reset the maximum time to the default
     */
    void resetMaximumTime();

    /**
     * Set the minimum and maximum time range.
     *
     * To enable time range checking provide two valid times.
     * To disable time range checking provide two invalid times, or call
     * clearTimeRange;
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
     * Reset the minimum and maximum time to the default values.
     */
    void resetTimeRange();

    /**
     * Clear the minimum and maximum time, i.e. disable time range checking.
     */
    void clearTimeRange();

    /**
     * Return the time interval able to be selected
     *
     * @return the select time intervals in minutes
     */
    int timeInterval() const;

Q_SIGNALS:

    /**
     * Signal if the time has been manually entered or selected by the user.
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
     * Set the currently selected time
     *
     * You can set an invalid time or a time outside the valid range, validity
     * checking is only done via isValid().
     *
     * @param time the new time
     */
    void setTime(const QTime &time);

    /**
     * Set the new widget options
     *
     * @param options the new widget options
     */
    void setOptions(Options options);

    /**
     * Sets the time format to display.
     *
     * By default is the Short Time format.
     *
     * @param format the time format to use
     */
    void setDisplayFormat(KLocale::TimeFormatOptions formatOptions);

    /**
     * Set the time interval able to be selected in the time widget
     *
     * If the ForceInterval option is set then any entered date will be
     * forced to the nearest interval.
     *
     * This interval must be an exact divisor of 60 minutes.
     *
     * @param minutes the interval to display
     */
    void setTimeInterval(int minutes);

protected:

    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual void showPopup();
    virtual void hidePopup();
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

    /**
     * Assign the time for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules.
     *
     * @param time the new time
     */
    virtual void assignTime(const QTime &time);

private:

    friend class KTimeComboBoxPrivate;
    KTimeComboBoxPrivate *const d;

    Q_PRIVATE_SLOT(d, void selectTime(int index))
    Q_PRIVATE_SLOT(d, void editTime(const QString&))
    Q_PRIVATE_SLOT(d, void enterTime(const QTime&))
    Q_PRIVATE_SLOT(d, void parseTime())

};

Q_DECLARE_OPERATORS_FOR_FLAGS(KTimeComboBox::Options)

#endif // KTIMECOMBOBOX_H
