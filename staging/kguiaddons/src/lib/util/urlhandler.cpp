/*********************************************************************************
 *                                                                               *
 * Copyright (C) 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>             *
 *                                                                               *
 * This library is free software; you can redistribute it and/or                 *
 * modify it under the terms of the GNU Lesser General Public                    *
 * License as published by the Free Software Foundation; either                  *
 * version 2.1 of the License, or (at your option) version 3, or any             *
 * later version accepted by the membership of KDE e.V. (or its                  *
 * successor approved by the membership of KDE e.V.), which shall                *
 * act as a proxy defined in Section 6 of version 3 of the license.              *
 *                                                                               *
 * This library is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
 * Lesser General Public License for more details.                               *
 *                                                                               *
 * You should have received a copy of the GNU Lesser General Public              *
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                               *
 *********************************************************************************/

#include <QObject>
#include <QUrl>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QProcess>
#include <QDesktopServices>

class UrlHandler : public QObject
{
    Q_OBJECT
public:
    explicit UrlHandler(QObject *parent = 0) : QObject(parent) {}

public Q_SLOTS:
    void openHelp(const QUrl &url)
    {
        QUrl u(url);
        if (u.path() == "/")
            u.setPath(QCoreApplication::applicationName());

        QString helpcenter = QStandardPaths::findExecutable("khelpcenter");
        if (helpcenter.isEmpty())
            QDesktopServices::openUrl(u);
        else
            QProcess::startDetached(helpcenter, QStringList(u.toString()));
    }
};

Q_GLOBAL_STATIC(UrlHandler, s_handler)

static void initializeGlobalSettings()
{
    QDesktopServices::setUrlHandler(QStringLiteral("help"), s_handler, "openHelp");
}

Q_COREAPP_STARTUP_FUNCTION(initializeGlobalSettings)

#include "urlhandler.moc"
