/* This file is part of the KDE libraries
   Copyright (C) 2007 Christian Ehrlicher <ch.ehrlicher@gmx.de>

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

#include <config.h>
#include <QtCore/QBool>

#ifdef Q_OS_WIN


#include <QDir>
#include <windows.h>

static HINSTANCE g_hInstance = NULL;

/**
 * The dll entry point - get the instance handle for GetModuleFleNameW
 * Maybe also some special initialization / cleanup can be done here
 **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID)
{
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            g_hInstance = hinstDLL;
            break;
        default:
            break;
    }
    return true;
}

// can't use QCoreApplication::applicationDirPath() because sometimes we
// don't have an instantiated QCoreApplication
QString getKde4Prefix()
{
    static QString modFilePath;
    if(modFilePath.isEmpty()) {
        wchar_t module_name[256];
        GetModuleFileNameW(g_hInstance, module_name, sizeof(module_name) / sizeof(wchar_t));
        modFilePath = QString::fromUtf16((ushort *)module_name);
        int idx = modFilePath.lastIndexOf('\\');
        if(idx != -1)
            modFilePath = modFilePath.left(idx);
        modFilePath = QDir(modFilePath + "/../").canonicalPath();
    }
    return modFilePath;
}

#endif  // Q_OS_WIN
