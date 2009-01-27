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
