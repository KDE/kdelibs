/*
  This file is part of the KDE libraries
  Copyright (c) 2007 Ralf Habacker <ralf.habacker@freenet.de>

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

/*
  kde application starter 
  - allows starting kde application without any additional path settings [1]
  - supports multiple root installation which is often used by packagers 
    (adds bin/lib directories from KDEDIRS environment to PATH environment)
  - designed to start kde application from windows start menu entries 
  - support for reading KDEDIRS setting from flat file 
    (located in <path-of-kwrapper.exe>/../../kdedirs.cache)
  - planned: automatic KDEDIRS detection support 
  
[1] recent kde cmake buildsystem on win32 installs shared libraries 
    into lib instead of bin. This requires to have the lib directory 
    in the PATH environment variable too (required for all pathes in KDEDIRS)
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h>

#include <QString>
#include <QProcess>
#include <QtDebug>
#include <QFileInfo>
#include <QCoreApplication>
#include <QList>

int main(int argc, char **argv)
{
    QCoreApplication app(argc,argv);
    if (argc != 2) 
    {
        qDebug() << "kwrapper: no application given"; 
        return 1;
    }
    QString path = QString(getenv("PATH")).toLower().replace("\\","/");
    // add path from environment
    QStringList envPath = path.split(";");
    
    // add current install path 
    path = QCoreApplication::applicationDirPath().toLower().replace("\\","/");
    if (!envPath.contains(path))
        envPath << path;

    QFileInfo fi(path + "/../..");
    QString rootPath = fi.canonicalPath();
    qDebug() << rootPath;
    
    // add current lib path 
    path = path.replace("bin","lib");
    if (!envPath.contains(path))
        envPath << path;

    // add bin and lib pathes from KDEDIRS 
    path = QString(getenv("KDEDIRS")).toLower().replace("\\","/");
    QStringList kdedirs;

    if (path.size() > 0)
        QStringList kdedirs = path.split(";"); 

    bool changedKDEDIRS = 0;
    // setup kdedirs if not present
    if (kdedirs.size() == 0) 
    {
        QFile f(rootPath + "/kdedirs.cache");
        if (f.exists()) 
        {
            f.open(QIODevice::ReadOnly);
            QByteArray data = f.readAll();
            f.close();
            kdedirs = QString(data).split(";");
            qDebug() << "kwrapper: load kdedirs.cache from " << rootPath <<  kdedirs; 
        }            
        else
        {
/* 
            f.open(QIODevice::WriteOnly);
            // search all pathes one level above for a directory share/apps 
            // write entries into a cache 
            f.write(kdedirs.join(";").toAscii());
            f.close();
*/
        }            
        changedKDEDIRS = 1;
    }
    
    foreach(QString a, kdedirs)
    {
        if (!envPath.contains(a+"/bin"))
            envPath << a + "/bin";                
        if (!envPath.contains(a+"/lib"))
            envPath << a + "/lib";                
    }

    // find executable
    char appName[MAX_PATH+1];
    *appName = 0;    
    
    foreach(QString a, envPath)
    {
        if (SearchPath(a.toAscii().data(),argv[1],".exe",MAX_PATH+1,appName,NULL))
            break;
    }

    if (!*appName)
    {
        qWarning() << "kwrapper: application not found"; 
        return 3; 
    }

    // setup client process envirionment 
    QStringList env = QProcess::systemEnvironment();
    env.replaceInStrings(QRegExp("^PATH=(.*)", Qt::CaseInsensitive), "PATH=" + envPath.join(";"));
    if (changedKDEDIRS)
        env << "KDEDIRS=" + kdedirs.join(";");

    QProcess *process = new QProcess;
    process->setEnvironment(env);
    process->start(appName); 
    if (process->state() == QProcess::NotRunning)
    {
        qDebug() << "kwrapper: process not running"; 
        return 4;
    }
    process->waitForStarted();

    return 0;    
}
