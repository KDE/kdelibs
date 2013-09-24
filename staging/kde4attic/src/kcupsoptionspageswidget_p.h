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

#ifndef KCUPSOPTIONSPAGESWIDGET_H
#define KCUPSOPTIONSPAGESWIDGET_H

#include <qwidget.h>

#include "kcupsoptionswidget_p.h"
#include "ui_kcupsoptionspageswidget.h"

/**
 * A widget to add to QPrintDialog to enable extra CUPS options
 * Private, not part of KDE API, not to be accessed by apps
 * Use KDEPrintDialog to create a dialog with widget included.
 *
 * Note that the Cups Options cannot be updated until the dialog is
 * closed otherwise they get overwritten.
 */

class KCupsOptionsPagesWidget : public KCupsOptionsWidget
{
    Q_OBJECT

public:

    /**
     * Enum for valid border styles for pages
     */
    enum PageBorder
    {
        NoBorder = 0,  //CUPS Default 'none'
        SingleLine,
        SingleThickLine,
        DoubleLine,
        DoubleThickLine
    };

    /**
     * Create a widget for extra CUPS options
     */
    explicit KCupsOptionsPagesWidget( QPrintDialog *parent );

    /**
     * Destructor
     */
    ~KCupsOptionsPagesWidget();

    /**
     * Set the border style of pages
     */
    void setPageBorder( PageBorder pageBorder = NoBorder  );

    /**
     * Returns the selected page border style
     */
    PageBorder pageBorder() const;

    /**
     * Set Page Label text
     */
    void setPageLabel( QString label = ""  );

    /**
     * Returns Page Label text
     */
    QString pageLabel() const;

    /**
     * Set mirror pages selected
     */
    void setMirrorPages( bool mirror = false  );

    /**
     * Returns if mirror pages selected
     */
    bool mirrorPages() const;

protected:

    void setupCupsOptions( QStringList &cupsOptions );

private:

    void initPageBorder();
    void initPageLabel();
    void initMirrorPages();

    Ui_KCupsOptionsPagesWidget ui;
};

#endif  // KCUPSOPTIONSPAGESWIDGET_H
