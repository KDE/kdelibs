/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*                                                               
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or   
*   (at your option) any later version.                                 
*                                                                       
*   This program is distributed in the hope that it will be useful,     
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       
*   GNU General Public License for more details.                        
*                                                                       
*   You should have received a copy of the GNU General Public License   
*   along with this program; if not, write to the                       
*   Free Software Foundation, Inc.,                                     
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .      
*/

#include <cstdlib>
#include <syslog.h>

#include "BackendsManager.h"
#include "helper_debug.h"

static bool remote = false;

void init_debug_handler()
{
    openlog("kauth_helper", 0, LOG_USER);
    qInstallMsgHandler(&helper_debug_handler);
}

void enable_remote_debug()
{
    remote = true;
}

void helper_debug_handler(QtMsgType type, const char *msg)
{
    if(!remote)
    {
        int level;
        switch(type)
        {
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
    }else
    {
        //syslog(LOG_USER, "Sono in helper_debug_handler: %s", msg);
        BackendsManager::helperProxy()->sendDebugMessage(type, msg);
    }
    
    // Anyway I should follow the rule:
    if(type == QtFatalMsg)
        exit(-1);
}

void end_debug()
{
    closelog();
}