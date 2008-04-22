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
#include <kdeversion.h>

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
    dialog->setOptionTabs(customTabs);
    return dialog;
}


QPrintDialog *KdePrint::createPrintDialog(QPrinter *printer,
                                               QWidget *parent)
{
    return new QPrintDialog(printer, parent);
}

