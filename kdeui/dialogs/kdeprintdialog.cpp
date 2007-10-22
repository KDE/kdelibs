/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2007 Alex Merry <huntedhacker@tiscali.co.uk>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
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

#include "kdeprintdialog.h"

#include <klocale.h>

#include <QLayout>
#include <fixx11h.h> // for enable-final
#include <QPrintDialog>
#include <QStyle>
#include <QTabWidget>

QPrintDialog *KdePrint::createPrintDialog(QPrinter *printer,
                                               const QList<QWidget*> &customTabs,
                                               QWidget *parent)
{
    QPrintDialog *dialog = createPrintDialog(printer, parent);
    if (! customTabs.isEmpty()) {
        QWidget *generalWidget = dialog->findChild<QWidget*>("generalWidget");
        QWidget *buttons = dialog->findChild<QWidget*>("buttonBox");
        if (generalWidget && buttons) {
            QTabWidget *tabs = new QTabWidget(dialog);
            tabs->addTab(generalWidget, i18n("General"));
            QStyle *style = dialog->style();
            generalWidget->layout()->setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                       style->pixelMetric(QStyle::PM_LayoutTopMargin),
                                       style->pixelMetric(QStyle::PM_LayoutRightMargin),
                                       style->pixelMetric(QStyle::PM_LayoutBottomMargin));
            dialog->layout()->addWidget(tabs);
            dialog->layout()->addWidget(buttons);

            foreach(QWidget* tab, customTabs)
                tabs->addTab(tab, tab->windowTitle());
        }
    }

    return dialog;
}


QPrintDialog *KdePrint::createPrintDialog(QPrinter *printer,
                                               QWidget *parent)
{
    return new QPrintDialog(printer, parent);
}

