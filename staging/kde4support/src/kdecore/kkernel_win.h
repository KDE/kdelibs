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

#ifndef KKERNEL_WIN_H
#define KKERNEL_WIN_H

#include <QtCore/QString>

#ifdef Q_OS_WIN

#include <windows.h>
#include <kdecore_export.h>

/**
 * @short Windows-specific functions needed in kdecore
 * @author Christian Ehrlicher <ch.ehrlicher@gmx.de>
 *
 * This file provides some Windows-specific functions which
 * Qt do not provide access to.
*/

/**
 \return a value from MS Windows native registry for shell folder \a folder.
*/
KDECORE_EXPORT QString getWin32ShellFoldersPath(const QString& folder);

/**
 Shows native MS Windows file property dialog for a file \a fileName.
 Return true on success. Only works for local absolute paths.
 Used by KPropertiesDialog, if possible.
*/
KDECORE_EXPORT bool showWin32FilePropertyDialog(const QString& fileName);

/**
 \return two-letter locale name (like "en" or "pl") taken from MS Windows native registry.
 Useful when we don't want to rely on KSyCoCa.
 Used e.g. by kbuildsycoca application.
*/
KDECORE_EXPORT QByteArray getWin32LocaleName();

/**
 \return true when the file is an executable on windows.
*/
KDECORE_EXPORT bool isExecutable(const QString &filename);

/**
 \return a value from MS Windows native registry.
*/
QString getWin32RegistryValue ( HKEY key, const QString& subKey, const QString& item, bool *ok = 0 );

#endif  // Q_OS_WIN
#endif  // KKERNEL_WIN_H
