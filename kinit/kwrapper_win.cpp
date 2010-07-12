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

 TODO: There is an prelimary concept of setting KDEDIRS environment variable 
       from a cache file located on a well known path relative from the requested 
       application. 
       The recent implementation expects a file name 'kdedirs.cache' two level 
       above this executable which will be <ProgramFiles> in case kwrapper4 lives 
       in <Programfiles>/kde4/bin. 
       This works not in any case especially when running application inside the 
       build directory. 
       
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

bool verbose = 0;

int main(int argc, char **argv)
{
    QCoreApplication app(argc,argv);

    QStringList envPath; /// pathes for using in environment of started process
    QStringList searchPath; /// pathes for using to find executable
    QString exeToStart;
    QString myAppName = "kwrapper4:"; 
    QStringList exeParams;
    int firstParam = 1;

    if (QCoreApplication::arguments().size() == 1)
    {
        qDebug() << myAppName << "no application given";
        return 1;
    }
    
    if (QCoreApplication::arguments().at(1) == "--verbose") 
    {
        verbose = 1;
        firstParam = 2;
    }        

    exeToStart = QCoreApplication::arguments().at(firstParam);

    for(int i=firstParam+1; i < QCoreApplication::arguments().size(); i++)
        exeParams << QCoreApplication::arguments().at(i);

    QString path = QString::fromLocal8Bit(qgetenv("PATH")).toLower().replace('\\','/');

    /** add pathes from PATH environment
        - all to client path environment 
        - pathes not ending with lib to application search path
    */
    foreach(const QString &a, path.split(';'))
    {
        if (!envPath.contains(a))
            envPath << a;
        if (!a.endsWith(QLatin1String("/lib")) && !a.endsWith(QLatin1String("/lib/")) && !searchPath.contains(a))
            searchPath << a;
    }
    
    // add current install path 
    path = QCoreApplication::applicationDirPath().toLower().replace('\\','/');
    if (!envPath.contains(path))
        envPath << path;

    // detect directory where kdedirs.cache lives
    // this is not complete, KDEDIRS path should be used as base too 
    QFileInfo fi(path + "/../..");
    QString rootPath = fi.canonicalPath();

    if (verbose)
        qDebug() << "try to find kdedirs.cache in" << rootPath;

    // add current lib path to client path environment 
    path = path.replace("bin","lib");
    if (!envPath.contains(path))
        envPath << path;

    /**
      add bin and lib pathes from KDEDIRS 
        - bin/lib to client path environment 
        - bin to application search path
    */
    path = QString::fromLocal8Bit(qgetenv("KDEDIRS")).toLower().replace('\\','/');
    QStringList kdedirs;

    if (path.size() > 0)
        kdedirs = path.split(';'); 

    bool changedKDEDIRS = 0;
    // setup kdedirs if not present
    if (kdedirs.size() == 0) 
    {
        QStringList kdedirsCacheList;
#ifdef Q_CC_MSVC
        kdedirsCacheList << rootPath + "/kdedirs.cache.msvc";
#endif
        kdedirsCacheList << rootPath + "/kdedirs.cache";

        bool found = false;
        foreach(const QString &kdedirsCachePath,kdedirsCacheList)
        {
            QFile f(kdedirsCachePath);
            if (f.exists()) 
            {
                f.open(QIODevice::ReadOnly);
                QByteArray data = f.readAll();
                f.close();
                kdedirs = QString(data).split(';');
                if (verbose)
                    qDebug() << "load kdedirs cache from " << kdedirsCachePath <<  "values=" << kdedirs; 
                found = true;
				break;
            }            
        }
        if (!found)
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
    if (verbose)
        qDebug() << "found KDEDIRS\n\t" << kdedirs.join("\n\t");
    
    foreach(const QString &a, kdedirs)
    {
        if (!envPath.contains(a+"/bin"))
            envPath << a + "/bin";                
        if (!envPath.contains(a+"/lib"))
            envPath << a + "/lib";                
        if (!searchPath.contains(a+"/bin"))
            searchPath << a + "/bin";                
    }

    // find executable
    WCHAR _appName[MAX_PATH+1];
    
    if (verbose)
        qDebug() << "search " << exeToStart << "in";
    
    bool found = false;        
    foreach(const QString &a, searchPath)
    {
        if (verbose)
            qDebug() << "\t" << a;
#ifndef _WIN32_WCE
        if (SearchPathW((LPCWSTR)a.utf16(),(LPCWSTR)exeToStart.utf16(),
                        L".exe",MAX_PATH+1,(LPWSTR)_appName,NULL))
        {                        
            found = true;                        
            break;
        }
#else
        if (QFile::exists(a+"/"+exeToStart+".exe"))
        {
            found = true;                        
            break;
        }
#endif
    }
    QString appName = QString::fromUtf16((unsigned short*)_appName);

    if (!found)
    {
        qWarning() << myAppName << "application not found"; 
        return 3; 
    }

    if (verbose)
        qDebug() << "run" << exeToStart << "with params" << exeParams << "and PATH environment\n\t" << envPath.join("\n\t");

    // setup client process envirionment 
    QStringList env = QProcess::systemEnvironment();
    env.replaceInStrings(QRegExp("^PATH=(.*)", Qt::CaseInsensitive), QLatin1String("PATH=") + envPath.join(";"));
    if (changedKDEDIRS)
        env << QLatin1String("KDEDIRS=") + kdedirs.join(";");

    QProcess *process = new QProcess;
    process->setEnvironment(env);
    process->start(appName,exeParams); 
    if (process->state() == QProcess::NotRunning)
    {
        qWarning() << myAppName << "process not running"; 
        return 4;
    }
    process->waitForStarted();

    return 0;    
}
