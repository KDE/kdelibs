/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2007 Christian Ehrlicher <ch.ehrlicher@gmx.de>
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
   Copyright (C) 2008-2009 Ralf Habacker <ralf.habacker@freenet.de>

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
#include <QtCore/QTextCodec>

#ifdef Q_OS_WIN

#include "kglobal.h"
#include <klocale.h>

#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QLibrary>

#include <windows.h>
#include <shellapi.h>
#include <process.h>

// console related includes
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif

#if defined(__MINGW32__)
# define WIN32_CAST_CHAR (const WCHAR*)
#else
# define WIN32_CAST_CHAR (LPCWSTR)
#endif

#ifndef _WIN32_WCE
static HINSTANCE kdecoreDllInstance = NULL;
#else
static HANDLE kdecoreDllInstance = NULL;
#endif
static wchar_t kde4prefixUtf16[MAX_PATH + 2];
static QString *kde4Prefix = NULL;

void initKde4prefixUtf16()
{
    //the path is C:\some\path\kde4\bin\kdecore.dll
#ifndef _WIN32_WCE
    GetModuleFileNameW(kdecoreDllInstance, kde4prefixUtf16, MAX_PATH + 1);
#else
    GetModuleFileNameW((HMODULE)kdecoreDllInstance, kde4prefixUtf16, MAX_PATH + 1);
#endif
    int bs1 = 0, bs2 = 0;

    //we convert \ to / and remove \bin\kdecore.dll from the string
    int pos;
    for (pos = 0; pos < MAX_PATH + 1 && kde4prefixUtf16[pos] != 0; ++pos) {
        if (kde4prefixUtf16[pos] == '\\') {
            bs1 = bs2;
            bs2 = pos;
            kde4prefixUtf16[pos] = '/';
        }
    }
    Q_ASSERT(bs1);
    Q_ASSERT(pos < MAX_PATH + 1);
    kde4prefixUtf16[bs1] = '/';
    kde4prefixUtf16[bs1+1] = 0;
}

// can't use QCoreApplication::applicationDirPath() because sometimes we
// don't have an instantiated QCoreApplication
QString getKde4Prefix()
{
  // we can get called after DLL_PROCESS_DETACH!
  return kde4Prefix ? *kde4Prefix : QString::fromUtf16((ushort*) kde4prefixUtf16);
}

/**
 * The dll entry point - get the instance handle for GetModuleFleNameW
 * Maybe also some special initialization / cleanup can be done here
 **/
extern "C"
#ifndef _WIN32_WCE
BOOL WINAPI DllMain ( HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpReserved)
#else
BOOL WINAPI DllMain ( HANDLE hinstDLL,DWORD fdwReason,LPVOID lpReserved)
#endif
{
    switch ( fdwReason ) {
    case DLL_PROCESS_ATTACH:
        kdecoreDllInstance = hinstDLL;
        initKde4prefixUtf16();
        kde4Prefix = new QString( QString::fromUtf16((ushort*) kde4prefixUtf16) );
        break;
    case DLL_PROCESS_DETACH:
        /* msdn:
           When handling DLL_PROCESS_DETACH, a DLL should free resources such
           as heap memory only if the DLL is being unloaded dynamically (the
           lpReserved parameter is NULL). If the process is terminating (the
           lpvReserved parameter is non-NULL), all threads in the process except
           the current thread either have exited already or have been explicitly
           terminated by a call to the ExitProcess function, which might leave
           some process resources such as heaps in an inconsistent state. In this
           case, it is not safe for the DLL to clean up the resources. Instead,
           the DLL should allow the operating system to reclaim the memory.
         */
        if( lpReserved == NULL )
            delete kde4Prefix;
        kde4Prefix = 0;
        break;
    default:
        break;
    }
    return true;
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
QString getWin32RegistryValue ( HKEY key, const QString& subKey, const QString& item, bool *ok )
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

#ifndef _WIN32_WCE
    SHELLEXECUTEINFOW execInfo;
#else
    SHELLEXECUTEINFO execInfo;
#endif
    memset ( &execInfo,0,sizeof ( execInfo ) );
    execInfo.cbSize = sizeof ( execInfo );
#ifndef _WIN32_WCE
    execInfo.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
#else
    execInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
#endif
    const QString verb ( QLatin1String ( "properties" ) );
    execInfo.lpVerb = WIN32_CAST_CHAR verb.utf16();
    execInfo.lpFile = WIN32_CAST_CHAR path_.utf16();
#ifndef _WIN32_WCE
    return ShellExecuteExW ( &execInfo );
#else
    return ShellExecuteEx ( &execInfo );
    //There is no native file property dialog in wince
   // return false;
#endif
}

// note: QLocale().name().left(2).toLatin1() returns the same

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

/** 
  kde and qt debug message printer using windows debug message port
 */ 
static void kMessageOutputDebugString(QtMsgType type, const char *msg)
{
    int BUFSIZE=4096;
    char *buf = new char[BUFSIZE];
    switch (type) {
        case QtDebugMsg:
            strlcpy(buf,"Debug:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            break;
        case QtWarningMsg:
            strlcpy(buf,"Warning:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            break;
        case QtCriticalMsg:
            strlcpy(buf,"Critical:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            break;
        case QtFatalMsg:
            strlcpy(buf,"Fatal:",BUFSIZE);
            strlcat(buf,msg,BUFSIZE);
            //abort();
            break;
    }
    strlcat(buf,"\n",BUFSIZE);
    OutputDebugStringW( QString::fromAscii(buf).utf16());
    delete[] buf;
}

/** 
  kde and qt debug message printer using FILE pointer based output
 */ 
static void kMessageOutputFileIO(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg);
        //abort();
    }
}

/** 
  try to attach to the parents console
  \return true if console has been attached, false otherwise
*/
typedef BOOL (WINAPI*attachConsolePtr)(DWORD dwProcessId);
static attachConsolePtr attachConsole = 0;
static bool attachConsoleResolved = false;
static bool attachToConsole()
{
    if(!attachConsoleResolved) {
      attachConsoleResolved = true;
      attachConsole = (attachConsolePtr)QLibrary::resolve(QLatin1String("kernel32"), "AttachConsole");
    }
    return attachConsole ? attachConsole(~0U) != 0 : false;
}

/**
  redirect stdout, stderr and
  cout, wcout, cin, wcin, wcerr, cerr, wclog and clog to console
*/ 
static void redirectToConsole()
{
//FIXME: for wince we cannot set stdio buffers
#ifndef _WIN32_WCE
    int hCrt;
    FILE *hf;
    int i;
    
    hCrt = _open_osfhandle((intptr_t) GetStdHandle(STD_INPUT_HANDLE),_O_TEXT);
    if(hCrt != -1) {
        hf = _fdopen( hCrt, "r" );
        *stdin = *hf;
        i = setvbuf( stdin, NULL, _IONBF, 0 );
    }

    hCrt = _open_osfhandle((intptr_t) GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);
    if(hCrt != -1) {
        hf = _fdopen( hCrt, "w" );
        *stdout = *hf;
        i = setvbuf( stdout, NULL, _IONBF, 0 );
    }

    hCrt = _open_osfhandle((intptr_t) GetStdHandle(STD_ERROR_HANDLE),_O_TEXT);
    if(hCrt != -1) {
        hf = _fdopen( hCrt, "w" );
        *stderr = *hf;
        i = setvbuf( stderr, NULL, _IONBF, 0 );
    }
    // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
    // point to console as well
    ios::sync_with_stdio();
#endif
}

#include <streambuf>

/** 
  ios related debug message printer for win32
*/
class debug_streambuf: public std::streambuf
{
    public:
        debug_streambuf(const char *prefix)
        {
            strcpy(buf,prefix);
            index = rindex = strlen(buf);
        }

    protected:
        virtual int overflow(int c = EOF)
        {
            if (c != EOF)
            {
                char cc = traits_type::to_char_type(c);
                // @TODO: buffer size checking
                buf[index++] = cc;
                if (cc == '\n')
                {
                    buf[index] = '\0';
                    OutputDebugStringW(QString::fromAscii(buf).utf16());
                    index = rindex;
                }
            }
            return traits_type::not_eof(c);
        }
    private:
        char buf[4096];
        int index, rindex;
};

/**
  retrieve type of win32 subsystem from the executable header 
  \return type of win32 subsystem - the subsystem types are defined at http://msdn.microsoft.com/en-us/library/ms680339(VS.85).aspx 
*/
static int subSystem()
{
#ifdef _WIN32_WCE
    // there is only one subsystem on Windows CE
    return IMAGE_SUBSYSTEM_WINDOWS_CE_GUI;
#else
    static int subSystem = -1;
    if (subSystem > -1)
        return subSystem; 

    // get base address of memory mapped executable
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
    PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS) ((char *)dosHeader + dosHeader->e_lfanew);
    if (ntHeader->Signature != 0x00004550) 
    {
        subSystem = IMAGE_SUBSYSTEM_UNKNOWN;
        return subSystem;
    }
    subSystem = ntHeader->OptionalHeader.Subsystem;
    return subSystem;
#endif
}
    
/**
 win32 debug and console output handling

 source type of output 
    1. kde/qt debug system  - kDebug(), kWarning(), kFatal(), kError(), qDebug(), qWarning(), qFatal() 
    2. ios  - cout, wcout, wcerr, cerr, wclog and clog
    3. FILE * - stdout,stderr

 application  console    ------------------ output -----------------
    type     available   qt/kde-debug         ios             FILE *   

    cui        yes        console           console         console
    cui        no        win32debug         win32debug      no output[1]

    gui        yes       win32debug         console         console
    gui        no        win32debug         win32debug      win32debug 

    win-ce     no        win32debug         win32debug      win32debug

[1]no redirect solution for FILE * based output yet

 TODO: report events to the windows event log system 
 http://msdn.microsoft.com/en-us/library/aa363680(VS.85).aspx
*/

/**
 setup up debug output 
*/ 
static class kMessageOutputInstaller {
    public:
        kMessageOutputInstaller() : stdoutBuffer("stdout:"), stderrBuffer("stderr:"), oldStdoutBuffer(0), oldStderrBuffer(0)
        {
            if (subSystem() == IMAGE_SUBSYSTEM_WINDOWS_CUI) {
                if (attachToConsole()) {
                    // setup kde and qt level 
                    qInstallMsgHandler(kMessageOutputFileIO);
                    // redirect ios and file io to console
                    redirectToConsole();
                }
                else {
                    // setup kde and qt level 
                    qInstallMsgHandler(kMessageOutputDebugString);
                    // redirect ios to debug message port 
                    oldStdoutBuffer = std::cout.rdbuf(&stdoutBuffer);
                    oldStderrBuffer = std::cerr.rdbuf(&stderrBuffer);
                }
            }
            else if (subSystem() == IMAGE_SUBSYSTEM_WINDOWS_GUI) {
                // setup kde and qt level 
                qInstallMsgHandler(kMessageOutputDebugString);
                // try to get a console
                if (attachToConsole()) {
                    redirectToConsole();
                }
                else {
                    // redirect ios to debug message port
                    oldStdoutBuffer = std::cout.rdbuf(&stdoutBuffer);
                    oldStderrBuffer = std::cerr.rdbuf(&stderrBuffer);
                    // TODO: redirect FILE * level to console, no idea how to do yet
                }
            } else if (subSystem() == IMAGE_SUBSYSTEM_WINDOWS_CE_GUI) {
                // do not try to get a console on WinCE systems
                qInstallMsgHandler(kMessageOutputDebugString);
                oldStdoutBuffer = std::cout.rdbuf(&stdoutBuffer);
                oldStderrBuffer = std::cerr.rdbuf(&stderrBuffer);
            }
            else
                qWarning("unknown subsystem %d detected, could not setup qt message handler",subSystem());
        }
        ~kMessageOutputInstaller()
        {
            if (oldStdoutBuffer) 
                std::cout.rdbuf(oldStdoutBuffer);
            if (oldStderrBuffer) 
                std::cerr.rdbuf(oldStderrBuffer);
        }
    
    private:
        debug_streambuf stdoutBuffer;
        debug_streambuf stderrBuffer;
        std::streambuf* oldStdoutBuffer;
        std::streambuf* oldStderrBuffer;

} kMessageOutputInstallerInstance;


bool isExecutable(const QString &file)
{
  return ( file.endsWith( QLatin1String( ".exe" ) ) ||
           file.endsWith( QLatin1String( ".com" ) ) ||
           file.endsWith( QLatin1String( ".bat" ) ) ||
           file.endsWith( QLatin1String( ".sln" ) ) ||
           file.endsWith( QLatin1String( ".lnk" ) ) );

}

#endif  // Q_OS_WIN
