/*
 *  Copyright 2009 David Faure <faure@kde.org>
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

#include "klauncher_cmds.h"

const char* commandToString(int command)
{
    switch (command) {
    case LAUNCHER_EXEC:
        return "EXEC";
    case LAUNCHER_SETENV:
        return "LAUNCHER_SETENV";
    case LAUNCHER_CHILD_DIED:
        return "LAUNCHER_CHILD_DIED";
    case LAUNCHER_OK:
        return "LAUNCHER_OK";
    case LAUNCHER_ERROR:
        return "LAUNCHER_ERROR";
    case LAUNCHER_SHELL:
        return "SHELL";
    case LAUNCHER_TERMINATE_KDE:
        return "LAUNCHER_TERMINATE_KDE";
    case LAUNCHER_TERMINATE_KDEINIT:
        return "LAUNCHER_TERMINATE_KDEINIT";
    case LAUNCHER_DEBUG_WAIT:
        return "LAUNCHER_DEBUG_WAIT";
    case LAUNCHER_EXT_EXEC:
        return "EXT_EXEC";
    case LAUNCHER_KWRAPPER:
        return "KWRAPPER";
    case LAUNCHER_EXEC_NEW:
        return "EXEC_NEW";
    default:
        return "UNKNOWN COMMAND";
    }
}
