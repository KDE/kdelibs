/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KDE_WIN32_UTILS_H
#define KDE_WIN32_UTILS_H

#include <kdecore/kdelibs_export.h>

#ifdef  __cplusplus
#include <QString>
#include <QByteArray>

extern "C" {
#endif

#define fcopy_src_err -1
#define fcopy_dest_err -2

/**
 Copies @p src file to @p dest file.
 @return 0 on success, fcopy_src_err on source file error,
 fcopy_dest_err on destination file error.
*/
KDEWIN32_EXPORT int fcopy(const char *src, const char *dest);

/**
 Converts all backslashes to slashes in @p path.
 Converting is stopped on a null character or at @p maxlen character.
*/
KDEWIN32_EXPORT void win32_slashify(char *path, int maxlen);

#ifdef  __cplusplus
}

/**
 \return a value from MS Windows native registry for shell folder \a folder.
*/
KDEWIN32_EXPORT 
QString getWin32ShellFoldersPath(const QString& folder);

/**
 Shows native MS Windows file property dialog for a file \a fileName.
 Return true on success. Only works for local absolute paths.
 Used by KPropertiesDialog, if possible.
*/
KDEWIN32_EXPORT
bool showWin32FilePropertyDialog(const QString& fileName);

/**
 \return two-letter locale name (like "en" or "pl") taken from MS Windows native registry.
 Useful when we don't want to rely on KSyCoCa.
 Used e.g. by kbuildsycoca application.
*/
KDEWIN32_EXPORT
QByteArray getWin32LocaleName();

#endif //__cplusplus

#endif
