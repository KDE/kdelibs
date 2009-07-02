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

#include <cstdio>
#include <syslog.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QTimer>

#include "HelperProxy.h"
#include "BackendsManager.h"

#include "helper.h"

void MyHelper::action()
{
    syslog(LOG_DEBUG, "Action executed by the helper. PID: %d, UID: %d", getpid(), getuid());
}

int main(int argc, char **argv)
{
    openlog("kauth_helper", 0, LOG_USER);
    MyHelper object;
    
    if(!BackendsManager::helperProxy()->initHelper("org.kde.auth"))
    {
        syslog(LOG_DEBUG, "initHelper() failed\n");
        return -1;
    }
    
    BackendsManager::helperProxy()->setHelperResponder(&object);
    
    QCoreApplication app(argc, argv);
    QTimer::singleShot(10000, &app, SLOT(quit()));
    app.exec();
    
    closelog();
    
    return 0;
}