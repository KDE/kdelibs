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

#include "kkernel_win.h"

#include <config.h>
#include <QtCore/QBool>

#ifdef Q_OS_WIN
#include <kdebug.h>

#include <QtCore/QDir>
#include <QApplication>

#include <windows.h>
#include <shellapi.h>
#include <process.h>

#if defined(__MINGW32__)
# define WIN32_CAST_CHAR (const WCHAR*)
#else
# define WIN32_CAST_CHAR (LPCWSTR)
#endif

static HINSTANCE g_hInstance = NULL;

/**
 * The dll entry point - get the instance handle for GetModuleFleNameW
 * Maybe also some special initialization / cleanup can be done here
 **/
BOOL WINAPI DllMain ( HINSTANCE hinstDLL,DWORD fdwReason,LPVOID )
{
    switch ( fdwReason ) {
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
    if ( modFilePath.isEmpty() ) {
        wchar_t module_name[256];
        GetModuleFileNameW ( g_hInstance, module_name, sizeof ( module_name ) / sizeof ( wchar_t ) );
        modFilePath = QString::fromUtf16 ( ( ushort * ) module_name );
        int idx = modFilePath.lastIndexOf ( '\\' );
        if ( idx != -1 )
            modFilePath = modFilePath.left ( idx );
        modFilePath = QDir ( modFilePath + "/../" ).canonicalPath();
    }
    return modFilePath;
}

/**
 \return a value from MS Windows native registry.
 @param key is usually one of HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE
        constants defined in WinReg.h.
 @param subKey is a registry subkey defined as a path to a registry folder, eg.
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
        ('\' delimiter must be used)
 @param item is an item inside subKey or "" if default folder's value should be returned
 @param ok if not null, will be set to true on success and false on failure
*/
QString getWin32RegistryValue ( HKEY key, const QString& subKey, const QString& item, bool *ok = 0 )
{
#define FAILURE \
 { if (ok) \
  *ok = false; \
 return QString(); }

    if ( subKey.isEmpty() )
        FAILURE;
    HKEY hKey;
    TCHAR *lszValue;
    DWORD dwType=REG_SZ;
    DWORD dwSize;

    if ( ERROR_SUCCESS!=RegOpenKeyExW ( key, WIN32_CAST_CHAR subKey.utf16(), 0, KEY_READ, &hKey ) )
        FAILURE;

    if ( ERROR_SUCCESS!=RegQueryValueExW ( hKey, WIN32_CAST_CHAR item.utf16(), NULL, NULL, NULL, &dwSize ) )
        FAILURE;

    lszValue = new TCHAR[dwSize];

    if ( ERROR_SUCCESS!=RegQueryValueExW ( hKey, WIN32_CAST_CHAR item.utf16(), NULL, &dwType, ( LPBYTE ) lszValue, &dwSize ) ) {
        delete [] lszValue;
        FAILURE;
    }
    RegCloseKey ( hKey );

    QString res = QString::fromUtf16 ( ( const ushort* ) lszValue );
    delete [] lszValue;
    return res;
}


bool showWin32FilePropertyDialog ( const QString& fileName )
{
    QString path_ = QDir::convertSeparators ( QFileInfo ( fileName ).absoluteFilePath() );

    SHELLEXECUTEINFOW execInfo;
    memset ( &execInfo,0,sizeof ( execInfo ) );
    execInfo.cbSize = sizeof ( execInfo );
    execInfo.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    const QString verb ( QLatin1String ( "properties" ) );
    execInfo.lpVerb = WIN32_CAST_CHAR verb.utf16();
    execInfo.lpFile = WIN32_CAST_CHAR path_.utf16();
    return ShellExecuteExW ( &execInfo );
}

QByteArray getWin32LocaleName()
{
    bool ok;
    QString localeNumber = getWin32RegistryValue ( HKEY_CURRENT_USER,
                           QLatin1String("Control Panel\\International"),
                           "Locale", &ok );
    if ( !ok )
        return QByteArray();
    QString localeName = getWin32RegistryValue ( HKEY_LOCAL_MACHINE,
                         QLatin1String("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layout\\DosKeybCodes"),
                         localeNumber, &ok );
    if ( !ok )
        return QByteArray();
    return localeName.toLatin1();
}

/**
 \return a value from MS Windows native registry for shell folder \a folder.
*/
QString getWin32ShellFoldersPath ( const QString& folder )
{
    return getWin32RegistryValue ( HKEY_CURRENT_USER,
                                   QLatin1String("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
                                   folder );
}

static void kMessageOutput(QtMsgType type, const char *msg)
{
#if 1
    int BUFSIZE=4096;
    char buf[BUFSIZE];
    switch (type) {
        case QtDebugMsg:
            strlcpy(buf,"Qt Debug:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            break;
        case QtWarningMsg:
            strlcpy(buf,"Qt Warning:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            break;
        case QtCriticalMsg:
            strlcpy(buf,"Qt Critial:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            break;
        case QtFatalMsg:
            strlcpy(buf,"Qt Fatal:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            //abort();
            break;
    }
    strlcat(buf,"\n",BUFSIZE);
    OutputDebugString(buf);
#else
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "[%4d] Qt Debug: %s\n", getpid(), msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "[%4d] Qt Warning: %s\n", getpid(), msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[%4d] Qt Critical: %s\n", getpid(), msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "[%4d] Qt Fatal: %s\n", getpid(), msg);
        //abort();
    }
#endif
}

class kGlobalClass {
    public: 
        kGlobalClass() {
            qInstallMsgHandler(kMessageOutput);
        }
}; 

static kGlobalClass myGlobals; 

#endif  // Q_OS_WIN
