/*
   This file is part of the KDE libraries
   Copyright (C) 2007 Benjamin Reed <rangerrick@befunk.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <stdlib.h>
#include <unistd.h>

#include <QFile>
#include <QProcess>
#include <kstandarddirs.h>
#include <ksharedconfig.h>
#include <kconfig.h>
#include <kdebug.h>

/**
 * Mac OS X related actions for KApplication startup.
 *
 * @internal
*/

bool dbus_initialized;

void KApplication_early_init_mac()
{
    if (dbus_initialized)
        return;

    QString dbusVar = QFile::decodeName(getenv("DBUS_LAUNCHD_SESSION_BUS_SOCKET"));
    if (!dbusVar.isEmpty() && QFile::exists(dbusVar) && (QFile::permissions(dbusVar) & QFile::WriteUser)) {
        dbusVar = "unix:path=" + dbusVar;
        ::setenv("DBUS_SESSION_BUS_ADDRESS", dbusVar.toLocal8Bit(), 1);
        dbus_initialized = true;
        return;
    }

    /* temporary until we implement autolaunch for dbus on Mac OS X */
    QString dbusSession;
    QStringList path = QFile::decodeName(getenv("KDEDIRS")).split(':').replaceInStrings(QRegExp("$"), "/bin");
    path << QFile::decodeName(getenv("PATH")).split(':') << "/opt/kde4-deps/bin" << "/sw/bin" << "/usr/local/bin";
    for (int i = 0; i < path.size(); ++i) {
        // QString testSession = QString(newPath.at(i)).append("/start-session-bus.sh");
        QString testSession = QString(path.at(i)).append("/start-session-bus.sh");
        kDebug() << "trying " << testSession;
        if (QFile(testSession).exists()) {
            kDebug() << "found " << testSession;
            dbusSession = testSession;
            break;
        }
    }

    if (!dbusSession.isEmpty()) {
        kDebug() << "running " << dbusSession << " --kde-mac";
        QString key, value, line;
        QStringList keyvals;
        QProcess qp;
        qp.setProcessChannelMode(QProcess::MergedChannels);
        qp.setTextModeEnabled(true);
        qp.start(dbusSession, QStringList() << "--kde-mac");
        if (!qp.waitForStarted(3000)) {
            kDebug() << dbusSession << " never started";
        } else {
            while (qp.waitForReadyRead(-1)) {
                while (qp.canReadLine()) {
                    line = qp.readLine().trimmed();
                    kDebug() << "line = " << line;
                    keyvals = line.split('=');
                    key = keyvals.takeFirst();
                    value = keyvals.join("=");
                    kDebug() << "key = " << key << ", value = " << value;
                    if (!key.isEmpty() && !value.isEmpty()) {
                        ::setenv(key.toLocal8Bit(), value.toLocal8Bit(), 1);
                        kDebug() << "setenv(" << key << "," << value << ",1)";
                    }
                }
            }
            qp.waitForFinished(-1);
        }
    }
    dbus_initialized = true;
}
