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

#ifndef KCUPSOPTIONSWIDGET_H
#define KCUPSOPTIONSWIDGET_H

#include <qwidget.h>

class QString;
class QStringList;
class QPrinter;
class QPrintDialog;

/**
 * A widget to add to QPrintDialog to enable extra CUPS options
 * Private, not part of KDE API, not to be accessed by apps
 * Use KDEPrintDialog to create a dialog with widget included.
 *
 * Base widget all CUPS widgets should inherit from
 */

class KCupsOptionsWidget : public QWidget
{
    Q_OBJECT

public:

    /**
     * Create a widget for CUPS options
     */
    explicit KCupsOptionsWidget( QPrintDialog *parent );

    /**
     * Destructor
     */
    virtual ~KCupsOptionsWidget();

    /**
    * Return if Cups is running on system
    */
    static bool cupsAvailable();

public slots:

    /**
     * Setup the given QPrinter with the selected print options
     */
    void setupPrinter();

protected:

    virtual void setupCupsOptions( QStringList &cupsOptions );
    void setCupsOption( QStringList &cupsOptions, const QString option, const QString value );

    QPrintDialog *m_dialog;

    Q_DISABLE_COPY(KCupsOptionsWidget)
};

#endif  // KCUPSOPTIONSWIDGET_H
