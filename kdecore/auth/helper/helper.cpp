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

#include <QCoreApplication>

#include "HelperProxy.h"
#include "BackendsManager.h"

#include "helper.h"

void Class::action()
{
    FILE *file = fopen("~/org.kde.auth.example.helper.action.debug.txt", "rw");
    
    fprintf(file, "Hello");
    
    fclose(file);
}

int main(int argc, char **argv)
{
    Class object;
    
    if(!BackendsManager::helperProxy()->initHelper("org.kde.auth"))
    {
        printf("initHelper failed\n");
    }
    BackendsManager::helperProxy()->setHelperResponder(&object);
    
    QCoreApplication app(argc, argv);
    app.exec();
    
    return 0;
}