/* This file is part of the KDE libraries
    Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "krun.h"
#include "krun_p.h"

#include <QDir>
#include <QFile>
#include <QWidget>

#include <windows.h>

// TODO move to a shared lib
static int runDll(WId windowId, const QString& libraryName, const QString& functionName, 
            const QString& arguments = QString())
{
  HMODULE libHandle = LoadLibraryA( QFile::encodeName(libraryName).constData() );
  if (!libHandle)
    return false;
  typedef int (WINAPI *FunctionType)(HWND, HMODULE, LPCSTR, int);
  FunctionType function 
    = (FunctionType)GetProcAddress( libHandle, functionName.toLatin1().constData() );
  if (!function)
    return false;
  int result = function((HWND)windowId, libHandle, (LPCSTR)arguments.toLatin1().constData(), SW_SHOW);
  FreeLibrary(libHandle);
  return result;
}

// TODO move to a shared lib
static int runDll(QWidget* parent, const QString& libraryName, const QString& functionName, 
            const QString& arguments = QString())
{
  return runDll(parent ? parent->winId() : 0, libraryName, functionName, arguments);
}


// Windows implementation using "OpenAs_RunDLL" entry
bool KRun::KRunPrivate::displayNativeOpenWithDialog( const KUrl::List& lst, QWidget* window, bool tempFiles,
                                               const QString& suggestedFileName, const QByteArray& asn )
{
    Q_UNUSED(tempFiles);
    Q_UNUSED(suggestedFileName);
    Q_UNUSED(asn);

    QStringList fnames;
    foreach( const KUrl& url, lst )
    {
      fnames += QFile::encodeName( 
        QDir::toNativeSeparators( url.path() ) ); // QFile::encodeName() needed because we will use toLatin1()
    }
    int result = runDll( window, QLatin1String("shell32.dll"), QLatin1String("OpenAs_RunDLL"), 
                         QFile::encodeName( fnames.join(" ") ) );
    return result == 0;
}
