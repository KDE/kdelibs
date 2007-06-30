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
 * - Set up some default paths based on what is in kderc.
 *   Usage: set the following in /etc/kderc:
 *   AddDirectoriesToPath: /opt/kde4-deps/bin,/opt/kde4/bin
 *
 * @internal
*/

bool mac_initialized;

void KApplication_early_init_mac()
{
    if (mac_initialized)
        return;

    KConfigGroup g( KGlobal::config(), "General" );
    QStringList envVars, envAppend, path, newPath;
    envVars << "XDG_DATA_DIRS" << "XDG_CONFIG_DIRS" << "PATH";
    envAppend << "AddDirectoriesToXdgDataDirs" << "AddDirectoriesToXdgConfigDirs" << "AddDirectoriesToPath";

    for (int i = 0; i < envVars.size(); ++i) {
        newPath = g.readPathListEntry(envAppend.at(i));
        path = QFile::decodeName(getenv(envVars.at(i).toLocal8Bit())).split(":");
        for (int j = 0; j < path.size(); ++j) {
            newPath.append(path.at(j));
        }
        ::setenv(envVars.at(i).toLocal8Bit(), newPath.join(":").toLocal8Bit(), 1);
        kDebug() << envVars.at(i) << "=" << newPath.join(":") << endl;
    }

    /* temporary until we implement autolaunch for dbus  on Mac OS X */
    QString dbusSession;
    for (int i = 0; i < newPath.size(); ++i) {
        QString testSession = QString(newPath.at(i)).append("/start-session-bus.sh");
        kDebug() << "trying " << testSession << endl;
        if (QFile(testSession).exists()) {
            kDebug() << "found " << testSession << endl;
            dbusSession = testSession;
            break;
        }
    }

    if (!dbusSession.isEmpty()) {
        kDebug() << "running " << dbusSession << " --kde-mac" << endl;
        QString key, value, line;
        QStringList keyvals;
        QProcess qp;
        qp.setProcessChannelMode(QProcess::MergedChannels);
        qp.setTextModeEnabled(true);
        qp.start(dbusSession, QStringList() << "--kde-mac");
        if (!qp.waitForStarted(3000)) {
            kDebug() << dbusSession << " never started" << endl;
        } else {
            while (qp.waitForReadyRead(-1)) {
                while (qp.canReadLine()) {
                    line = qp.readLine().trimmed();
                    kDebug() << "line = " << line << endl;
                    keyvals = line.split('=');
                    key = keyvals.takeFirst();
                    value = keyvals.join("=");
                    kDebug() << "key = " << key << ", value = " << value << endl;
                    if (!key.isEmpty() && !value.isEmpty()) {
                        ::setenv(key.toLocal8Bit(), value.toLocal8Bit(), 1);
                        kDebug() << "setenv(" << key << "," << value << ",1)" << endl;
                    }
                }
            }
            qp.waitForFinished(-1);
        }
    }
    mac_initialized = true;
}
