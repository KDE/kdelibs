/*
    Copyright (c) 2009 David Faure <faure@kde.org>

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

#include <QDebug>
#include <browseropenorsavequestion.h>
#include <QApplication>

using namespace KParts;

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("openorsavequestion"));
    QApplication app(argc, argv);

    // A test for both 1) "unknown mimetype"  2) no associated app
    {
        BrowserOpenOrSaveQuestion questionOpenUnknownMimeType(0, QUrl(QStringLiteral("http://www.example.com/foo.foo")),
                                                              QString::fromLatin1("application/foo"));
        BrowserOpenOrSaveQuestion::Result res = questionOpenUnknownMimeType.askOpenOrSave();
        qDebug() << res;
    }

    // The normal case
    {
        BrowserOpenOrSaveQuestion questionOpen(0, QUrl(QStringLiteral("http://www.example.com/foo.pdf")),
                                               QString::fromLatin1("application/pdf"));
        questionOpen.setSuggestedFileName(QString::fromLatin1("file.pdf"));
        questionOpen.setFeatures(BrowserOpenOrSaveQuestion::ServiceSelection);
        BrowserOpenOrSaveQuestion::Result res = questionOpen.askOpenOrSave();
        qDebug() << res;
        if (res == BrowserOpenOrSaveQuestion::Open && questionOpen.selectedService())
            qDebug() << "Selected service:" << questionOpen.selectedService()->entryPath();
    }

    // Trying a case with only one app associated
    {
        BrowserOpenOrSaveQuestion questionOpen(0, QUrl(QStringLiteral("http://www.example.com/foo.zip")),
                                               QString::fromLatin1("application/zip"));
        questionOpen.setFeatures(BrowserOpenOrSaveQuestion::ServiceSelection);
        BrowserOpenOrSaveQuestion::Result res = questionOpen.askOpenOrSave();
        qDebug() << res;
        if (res == BrowserOpenOrSaveQuestion::Open && questionOpen.selectedService())
            qDebug() << "Selected service:" << questionOpen.selectedService()->entryPath();
    }

    return 0;
}
