/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2008 John Layt <john@layt.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KCUPSOPTIONSJOBWIDGET_H
#define KCUPSOPTIONSJOBWIDGET_H

#include <qwidget.h>

#include "kcupsoptionswidget_p.h"
#include "ui_kcupsoptionsjobwidget.h"

class QString;
class QTime;

/**
 * A widget to add to QPrintDialog to enable extra CUPS options
 * Private, not part of KDE API, not to be accessed by apps
 * Use KDEPrintDialog to create a dialog with widget included.
 *
 * Note that the Cups Options cannot be updated until the dialog is
 * closed otherwise they get overwritten.
 */

class KCupsOptionsJobWidget : public KCupsOptionsWidget
{
    Q_OBJECT

public:

    /**
     * Enum for values of job-hold-until option
     */
    enum JobHoldUntil
    {
        NoHold = 0,  //CUPS Default
        Indefinite,
        DayTime,
        Night,
        SecondShift,
        ThirdShift,
        Weekend,
        SpecificTime
    };

    /**
     * Create a widget for extra CUPS options
     */
    explicit KCupsOptionsJobWidget( QPrintDialog *parent );

    /**
     * Destructor
     */
    ~KCupsOptionsJobWidget();

    /**
     * Set the Job Hold option
     */
    void setJobHold( JobHoldUntil jobHold = NoHold );

    /**
     * Returns the selected Job Hold option
     */
    JobHoldUntil jobHold() const;

    /**
     * Set the Job Hold Time
     */
    void setJobHoldTime( QTime holdUntilTime );

    /**
     * Returns the selected Job Hold Time
     */
    QTime jobHoldTime() const;

    /**
     * Set the Job Billing text
     */
    void setJobBilling( QString jobBilling = "" );

    /**
     * Returns the entered Job Billing text
     */
    QString jobBilling() const;

    /**
     * Set Job Priority
     */
    void setJobPriority( int priority = 50  );

    /**
     * Returns Job Priority
     */
    int jobPriority() const;

    /**
     * Set Job options
     */
    void setJobOptions( QStringList jobOptions );

    /**
     * Returns the entered Job Options
     */
    QStringList jobOptions() const;

protected:

    void setupCupsOptions( QStringList &cupsOptions );

private slots:

    void toggleJobHoldTime();

private:

    void initJobHold();
    void initJobBilling();
    void initJobPriority();
    void initJobOptions();

    Ui_KCupsOptionsJobWidget ui;
};

#endif  // KCUPSOPTIONSJOBWIDGET_H
