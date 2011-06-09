/*
   This file is part of the KDE libraries
   Copyright (c) 2011 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2.1 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KFILESYSTEMTYPE_P_H
#define KFILESYSTEMTYPE_P_H

#include <QtCore/QString>

namespace KFileSystemType
{
   enum Type {
       Unknown,
       Nfs, // NFS or similar (autofs, subfs, cachefs)
       Fat,  // FAT or similar (msdos, fat, vfat)
       Other // ext, reiser, and so on. "Normal" filesystems.
   };

   Type fileSystemType(const QString& path);

};

#endif
