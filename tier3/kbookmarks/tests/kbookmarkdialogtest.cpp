/*
    Copyright (c) 2013 David Faure <faure+bluesystems@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QLabel>
#include <QApplication>
#include <QLayout>
#include <QPushButton>

#include <kbookmarkdialog.h>
#include <kbookmarkmanager.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KBookmarkDialog dlg(KBookmarkManager::userBookmarksManager());
    dlg.addBookmark("KDE", QUrl("http://www.kde.org"), "www"); // calls exec()
    return 0;
}

