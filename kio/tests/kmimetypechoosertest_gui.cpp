/* This file is part of the KDE project
   Copyright (C) 2006 David Faure <faure@kde.org>

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

#include <QApplication>
#include <kmimetypechooser.h>
#include <QDebug>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QString text = QStringLiteral("Select the MimeTypes you want for this file type.");
    QStringList list;
    list << QStringLiteral("inode/directory") << QStringLiteral("text/plain");
    KMimeTypeChooserDialog dlg( QStringLiteral("Select Mime Types"), text, list, QStringLiteral("text"), QStringList(),
                                KMimeTypeChooser::Comments|KMimeTypeChooser::Patterns|KMimeTypeChooser::EditButton,
                                (QWidget*)0 );
    if ( dlg.exec() == QDialog::Accepted ) {
        qDebug() << dlg.chooser()->patterns();
        qDebug() << dlg.chooser()->mimeTypes().join(QLatin1Char(';'));
    }

    return 0; // app.exec();
}
