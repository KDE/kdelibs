/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2013 Martin Klapetek <mklapetek@kde.org>
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

#include <QApplication>
#include <QtTest/QtTest>
#include <QPrinter>
#include <QPainter>

#include "kprintpreview.h"

int main( int argc, char **argv )
{
    QApplication::setApplicationName(QStringLiteral("kprintpreview_text"));
    QApplication app(argc, argv);

    QPrinter printer;
    KPrintPreview *preview = new KPrintPreview(&printer);

    QPainter painter;
    painter.begin(&printer);
    painter.drawText(printer.pageRect(), QStringLiteral("Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
    "Nam malesuada massa dolor, sed sollicitudin turpis congue eu. Pellentesque ac risus felis. Etiam scelerisque "
    "leo vitae mauris condimentum malesuada. Praesent tortor neque, egestas non elit vitae, accumsan mattis augue. "
    "Interdum et malesuada fames ac ante ipsum primis in faucibus. Quisque ligula ligula, convallis ac felis in, "
    "tincidunt pulvinar magna. Maecenas eu congue ipsum. Donec laoreet vulputate lorem, fermentum consectetur orci. "
    "Suspendisse sed mauris sit amet mauris molestie ullamcorper. Vivamus aliquet consectetur est sit amet fringilla. "
    "Nunc cursus risus velit, tempus imperdiet sapien tristique non. Pellentesque leo sapien, blandit et turpis eu, "
    "placerat placerat odio. Vestibulum auctor nulla ut felis venenatis, eleifend venenatis quam feugiat."

    "Duis magna enim, dignissim et convallis quis, ultrices quis nisi. Nulla nec rutrum tellus. Nunc in commodo orci. "
    "Praesent tincidunt sem consequat neque tincidunt ullamcorper. Pellentesque sit amet dui vitae lectus fermentum "
    "dignissim vel vel justo. Donec nec felis venenatis, hendrerit nunc vitae, iaculis leo. Duis vel purus pharetra, "
    "laoreet nisl non, ultricies mauris. Etiam adipiscing eget magna quis bibendum."

    "Curabitur felis nunc, lobortis nec tincidunt commodo, tempor id libero. Nulla sit amet ultrices erat. "
    "Suspendisse tempus lorem lacus, a fermentum sem euismod vitae. Proin sed adipiscing leo, sit amet "
    "vestibulum eros. Proin faucibus ut ante id facilisis. Nunc cursus ut purus sed ullamcorper. Donec "
    "turpis risus, pretium sed dui sed, accumsan dictum arcu. Vestibulum cursus sit amet enim ac vestibulum. "
    "Proin eget commodo enim. Nullam eu lacus nulla. Suspendisse malesuada tempor nulla, ac ullamcorper arcu. "
    "Nunc facilisis laoreet fringilla. Aliquam bibendum lacinia quam in ultrices. Donec non iaculis augue. "
    "Duis semper rhoncus arcu. Curabitur faucibus consectetur ultricies."));

    painter.end();

    preview->resize(600, 600);
    preview->show();

    return app.exec();
 }
