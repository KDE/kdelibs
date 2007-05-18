/*
 * This file is part of the KDE libraries
 * Copyright (c) 1999-2000 Waldo Bastian <bastian@kde.org>
 *           (c) 1999 Mario Weilguni <mweilguni@sime.com>
 *           (c) 2001 Lubos Lunak <l.lunak@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <QtCore/QString>
#include <QtCore/QLibrary>
#include <QtCore/QFile>
#ifdef Q_WS_WIN
#include <QStringList>
#include <QProcess>
#include <windows.h>
#endif

#ifndef Q_WS_WIN
/* These are to link libkio even if 'smart' linker is used */
#include <kio/authinfo.h>
extern "C" KIO::AuthInfo* _kioslave_init_kio() { return new KIO::AuthInfo(); }
#endif 

int main(int argc, char **argv)
{
     if (argc < 5)
     {
        fprintf(stderr, "Usage: kioslave <slave-lib> <protocol> <klauncher-socket> <app-socket>\n\nThis program is part of KDE.\n");
        exit(1);
     }
     QByteArray libpath = argv[1];     

     if (libpath.isEmpty())
     {
        fprintf(stderr, "library path is empty.\n");
        exit(1); 
     }

     QLibrary lib(QFile::decodeName(libpath.data()));
#ifdef Q_WS_WIN
     qDebug("trying to load '%s'",libpath.data());
#endif
     if (!lib.load() || !lib.isLoaded() )
     {
#ifdef Q_WS_WIN
        if (!getenv("KDEDIRS")) {
          qDebug("not able to find '%s' because KDEDIRS environment variable is not set.\n"
                 "Set KDEDIRS to the KDE installation root dir and restart klauncher to fix this problem.",libpath.data());
          exit(1);
        }
        QString pathes = getenv("KDEDIRS");
        QStringList pathlist = pathes.split(";");
        for (int i = 0; i < pathlist.size(); ++i) 
        {
          QString slave_path = pathlist.at(i) + "/lib/kde4/" + libpath.data();
          qDebug("trying to load '%s'",slave_path.toAscii().data());
          lib.setFileName(slave_path);
          if (lib.load() && lib.isLoaded() ) 
            break;
        }
        if (!lib.isLoaded()) 
        {
          fprintf(stderr, "could not open %s: %s", libpath.data(),
                  qPrintable (lib.errorString()) );
          exit(1);
        }
#else
        fprintf(stderr, "could not open %s: %s", libpath.data(),
                qPrintable (lib.errorString()) );
        exit(1);
#endif
     }  

     void* sym = lib.resolve("kdemain");
     if (!sym )
     {
        sym = lib.resolve("main");
        if (!sym )
        {
           fprintf(stderr, "Could not find main: %s\n", qPrintable(lib.errorString() ));
           exit(1);
        }
     }

#ifdef Q_WS_WIN
     // enter debugger in case debugging is actived 
     QString mSlaveDebug = getenv("KDE_SLAVE_DEBUG_WAIT");

     if (mSlaveDebug == argv[2]) 
#ifdef Q_CC_GNU
     {
        // gdb does not support win32 jit debug support, so implement it by ourself
        QByteArray buf(1024,0);
        GetModuleFileName(NULL,buf.data(),buf.size());
        QStringList params;
        params << buf;
        params << QString::number(GetCurrentProcessId());
        QProcess::startDetached("gdb",params);
        Sleep(1000);
     }
#else
        // msvc: windbg supports jit debugging, require install windbg jit with windbg -i
        DebugBreak();
#endif
#endif

     int (*func)(int, char *[]) = (int (*)(int, char *[])) sym;

     exit( func(argc-1, argv+1)); /* Launch! */
}
