/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2007 Alex Merry <alex.merry@kdemail.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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

#ifndef KPRINTPREVIEW_H
#define KPRINTPREVIEW_H

#include <kutils_export.h>

#include <kdialog.h>

class KPrintPreviewPrivate;

/**
 * KPrintPreview provides a print preview dialog.
 *
 * Use it like this:
 *
 * @code
 * QPrinter printer;
 * KPrintPreview preview(&printer);
 * doPrint(printer); // draws to the QPrinter
 * preview.exec();
 * @endcode
 */
class KUTILS_EXPORT KPrintPreview : public KDialog
{
    Q_OBJECT

public:
    /**
     * Create a KPrintPreview object.
     *
     * This will change the settings on the QPrinter, so you
     * should not re-use the QPrinter object for printing
     * normally.
     *
     * @param printer pointer to a QPrinter to configure for
     *                print preview
     * @param parent  pointer to the parent widget for the dialog
     */
    explicit KPrintPreview(QPrinter *printer, QWidget *parent = 0);
    virtual ~KPrintPreview();

protected:
    void showEvent(QShowEvent *event);

private:
    KPrintPreviewPrivate * const d;
};


#endif // KPRINTPREVIEW_H

