/*  This file is part of the KDE libraries
 *  Copyright 2012 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "khelpclient.h"

#include "kdesktopfile.h"
#include <ktoolinvocation.h>
#include <klocalizedstring.h>

#include <QApplication>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QUrl>
#include <QMessageBox>
#include <QDirIterator>
#include <QDesktopServices>

void KHelpClient::invokeHelp(const QString& anchor,
                             const QString& _appname,
                             const QByteArray& startup_id)
{
    QString appname;
    if (_appname.isEmpty()) {
        appname = QCoreApplication::instance()->applicationName();
    } else {
        appname = _appname;
    }

    // Look for the .desktop file of the application

    // was:
    //KService::Ptr service(KService::serviceByDesktopName(appname));
    //if (service)
    //    docPath = service->docPath();
    // but we don't want to depend on KService here.

    QString docPath;
    const QStringList desktopDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    Q_FOREACH(const QString& dir, desktopDirs) {
        QDirIterator it(dir, QStringList() << appname + QLatin1String(".desktop"), QDir::NoFilter, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString desktopPath(it.next());
            KDesktopFile desktopFile(desktopPath);
            docPath = desktopFile.readDocPath();
            break;
        }
    }

    // docPath could be a path or a full URL, I think.

    QUrl url;
    if (!docPath.isEmpty()) {
        url = QUrl(QLatin1String("help:/")).resolved(QUrl::fromUserInput(docPath));
    } else {
        url = QUrl(QString::fromLatin1("help:/%1/index.html").arg(appname));
    }

    if (!anchor.isEmpty()) {
        url.addQueryItem(QString::fromLatin1("anchor"), anchor);
    }

    // launch a browser for URIs not handled by khelpcenter
    // (following KCMultiDialog::slotHelpClicked())
    if (!(url.scheme() == QLatin1String("help") || url.scheme() == QLatin1String("man") || url.scheme() == QLatin1String("info"))) {
        QDesktopServices::openUrl(url);
        return;
    }

    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.khelpcenter"))) {
        QString error;
#ifdef Q_OS_WIN
        // startServiceByDesktopName() does not work yet; KRun:processDesktopExec returned 'KRun: syntax error in command "khelpcenter %u" , service "KHelpCenter" '
        if (KToolInvocation::kdeinitExec(QLatin1String("khelpcenter"), QStringList() << url.toString(), &error, 0, startup_id))
#else
            if (KToolInvocation::startServiceByDesktopName(QLatin1String("khelpcenter"), url.toString(), &error, 0, 0, startup_id, false))
#endif
            {
                QMessageBox::warning(QApplication::activeWindow(),
                                     i18n("Could not Launch Help Center"),
                                     i18n("Could not launch the KDE Help Center:\n\n%1", error));
                return;
            }
    }
    QDBusInterface iface(QLatin1String("org.kde.khelpcenter"),
                         QLatin1String("/KHelpCenter"),
                         QLatin1String("org.kde.khelpcenter.khelpcenter"),
                         QDBusConnection::sessionBus());

    iface.call(QString::fromLatin1("openUrl"), url.toString(), startup_id );
}

