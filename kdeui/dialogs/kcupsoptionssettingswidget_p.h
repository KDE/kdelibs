/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2009 John Layt <john@layt.net>
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

#ifndef KCUPSOPTIONSSETTINGSWIDGET_H
#define KCUPSOPTIONSSETTINGSWIDGET_H

#include <qwidget.h>
#include <QComboBox>

#include "kcupsoptionswidget_p.h"

/**
 * A widget to add to QPrintDialog to enable extra CUPS options
 * Private, not part of KDE API, not to be accessed by apps
 * Use KDEPrintDialog to create a dialog with widget included.
 *
 * Note that the Cups Options cannot be updated until the dialog is
 * closed otherwise they get overwritten.
 */

class KCupsOptionsSettingsWidget : public KCupsOptionsWidget
{
    Q_OBJECT

public:

    /**
    * Enum for valid page set
    */
    enum PageSet
    {
        AllPages = 0,  //CUPS Default
        OddPages,
        EvenPages
    };

    /**
     * Create a widget for extra CUPS options
     */
    explicit KCupsOptionsSettingsWidget( QPrintDialog *parent );

    /**
     * Destructor
     */
    ~KCupsOptionsSettingsWidget();

    /**
    * Set the Page Set
    */
    void setPageSet( PageSet pageSet = AllPages  );

    /**
    * Returns the selected Page Set
    */
    PageSet pageSet() const;

    /**
    * Set the Server Selects Pages Mode
    */

    void setSystemSelectsPages( bool systemSelectsPages = true  );

    /**
    * Returns if set to System Selects Pages mode
    */
    bool systemSelectsPages() const;

    protected:

    void setupCupsOptions( QStringList &cupsOptions );

private:

    void initPageSet();

    bool m_systemSelectsPages;
    QComboBox *m_pageSetCombo;
};

#endif  // KCUPSOPTIONSSETTINGSWIDGET_H
