/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2007 Alex Merry <huntedhacker@tiscali.co.uk>
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
 **/

#ifndef KDEPRINTDIALOG_H
#define KDEPRINTDIALOG_H

#include <kdeui_export.h>

#include <QtCore/QList>

class QPrintDialog;
class QPrinter;
class QWidget;

/**
 * Namespace for the KDE printing system
 */
namespace KdePrint
{
    /**
     * Creates a printer dialog for a QPrinter with the given custom widgets.
     *
     * Note that the custom widgets are only supported on X11
     * and will @b not be shown on Qt versions prior to 4.3.2.
     * On non-X11 systems it is preferred to provide the widgets 
     * within configuration dialog of the application.
     *
     * Setting the widgets will transfer their ownership to the print dialog
     * even on non-X11 systems.
     * The caller takes ownership of the dialog and is responsible
     * for deleting it.
     *
     * @param printer the QPrinter to apply settings to
     * @param parent the parent for the dialog
     * @param customTabs a list of custom widgets to show as tabs, the name printed on the tab will
     *      be taken from the widgets windowTitle().
     * @see QWidget::setWindowTitle()
     * @see QAbstractPrintDialog::setOptionTabs()
     */
    KDEUI_EXPORT QPrintDialog *createPrintDialog(QPrinter *printer,
                                                 const QList<QWidget*> &customTabs,
                                                 QWidget *parent = 0);

    /**
     * Creates a printer dialog for a QPrinter
     *
     * The caller takes ownership of the dialog and is responsible
     * for deleting it.
     *
     * @param printer the QPrinter to apply settings to
     * @param parent the parent for the dialog
     */
    KDEUI_EXPORT QPrintDialog *createPrintDialog(QPrinter *printer,
                                                 QWidget *parent = 0);
}


#endif // KDEPRINTDIALOG_H

