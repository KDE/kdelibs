/* This file is part of the KDE libraries
    Copyright (c) 2013 Aurélien Gâteau <agateau@kde.org>

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
#include <iostream>

#include <kmessagebox.h>

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kmessageboxwidtest"));
    QApplication app(argc, argv);
    if (argc != 2) {
        std::cerr << "Usage: " << qPrintable(QApplication::applicationName()) << " <window id>\n";
        std::cerr << '\n';
        std::cerr << "You can get a window id using a tool like `xwininfo`.\n";
        return 1;
    }

    bool ok;
    int winId = QByteArray(argv[1]).toInt(&ok, 0);
    if (!ok) {
        std::cerr << '"' << argv[1] << "\" is not a number\n";
        return 1;
    }

    int ret = KMessageBox::warningContinueCancelWId(winId,
        QStringLiteral("Are you sure you want to continue?"),
        QStringLiteral("Dangerous stuff"));

    std::cout << "Returned " << ret << '\n';
    return ret;
}
