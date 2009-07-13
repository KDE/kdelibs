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

#ifndef HELPER_SUPPORT_H
#define HELPER_SUPPORT_H

#include <QObject>
#include <QVariant>

#define KDE4_AUTH_HELPER(ID, HelperClass) \
int main(int argc, char **argv) { return HelperSupport::helperMain(argc, argv, ID, new HelperClass()); }

class HelperSupport
{
    HelperSupport();
    
    static int helperMain(int argc, char **argv, const char *id, QObject *responder);
    static void helperDebugHandler(QtMsgType type, const char *msg);
    
public:
    static void progressStep(int step);
    static void progressStep(QVariantMap data);
    
    static bool isStopped();
    
    friend int main(int, char **);
};

#endif
