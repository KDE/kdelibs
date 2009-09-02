/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include "kauthhelpersupport.h"

#include <cstdlib>
#include <syslog.h>

#include <QCoreApplication>
#include <QTimer>

#include "BackendsManager.h"

namespace KAuth
{

    namespace HelperSupport {
        void helperDebugHandler(QtMsgType type, const char *msg);
    }

static bool remote_dbg = false;

int HelperSupport::helperMain(int argc, char **argv, const char *id, QObject *responder)
{
    openlog(id, 0, LOG_USER);
    qInstallMsgHandler(&HelperSupport::helperDebugHandler);

    if (!BackendsManager::helperProxy()->initHelper(id)) {
        syslog(LOG_DEBUG, "Helper initialization failed");
        return -1;
    }

    //closelog();
    remote_dbg = true;

    BackendsManager::helperProxy()->setHelperResponder(responder);

    QCoreApplication app(argc, argv);
    //QTimer::singleShot(10000, &app, SLOT(quit()));
    app.exec(); //krazy:exclude=crashy

    return 0;
}

void HelperSupport::helperDebugHandler(QtMsgType type, const char *msg)
{
    if (!remote_dbg) {
        int level = LOG_DEBUG;
        switch (type) {
        case QtDebugMsg:
            level = LOG_DEBUG;
            break;
        case QtWarningMsg:
            level = LOG_WARNING;
            break;
        case QtCriticalMsg:
        case QtFatalMsg:
            level = LOG_ERR;
            break;
        }
        syslog(level, "%s", msg);
    } else {
        BackendsManager::helperProxy()->sendDebugMessage(type, msg);
    }

    // Anyway I should follow the rule:
    if (type == QtFatalMsg) {
        exit(-1);
    }
}

void HelperSupport::progressStep(int step)
{
    BackendsManager::helperProxy()->sendProgressStep(step);
}

void HelperSupport::progressStep(const QVariantMap &data)
{
    BackendsManager::helperProxy()->sendProgressStep(data);
}

bool HelperSupport::isStopped()
{
    return BackendsManager::helperProxy()->hasToStopAction();
}

} // namespace Auth
