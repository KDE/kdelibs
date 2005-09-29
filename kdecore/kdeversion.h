/* This file is part of the KDE libraries
    Copyright (c) 2002-2005 KDE Team

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

#ifndef _KDE_VERSION_H_
#define _KDE_VERSION_H_

#include "kdelibs_export.h"

#define KDE_VERSION_STRING "3.4.91 (beta1, >= 20050910)"
#define KDE_VERSION_MAJOR 3
#define KDE_VERSION_MINOR 4
#define KDE_VERSION_RELEASE 91
#define KDE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

#define KDE_VERSION \
  KDE_MAKE_VERSION(KDE_VERSION_MAJOR,KDE_VERSION_MINOR,KDE_VERSION_RELEASE)

#define KDE_IS_VERSION(a,b,c) ( KDE_VERSION >= KDE_MAKE_VERSION(a,b,c) )

/**
 * Namespace for general KDE functions.
 */
namespace KDE
{
    /**
     * Returns the encoded number of KDE's version, see the KDE_VERSION macro.
     * In contrary to that macro this function returns the number of the actully
     * installed KDE version, not the number of the KDE version that was
     * installed when the program was compiled.
     * @return the version number, encoded in a single uint
     * @since 3.2
     */
    KDECORE_EXPORT unsigned int version();
    /**
     * Returns the major number of KDE's version, e.g.
     * 3 for KDE 3.1.2. 
     * @return the major version number
     * @since 3.1
     */
    KDECORE_EXPORT unsigned int versionMajor();
    /**
     * Returns the minor number of KDE's version, e.g.
     * 1 for KDE 3.1.2. 
     * @return the minor version number
     * @since 3.1
     */
    KDECORE_EXPORT unsigned int versionMinor();
    /**
     * Returns the release of KDE's version, e.g.
     * 2 for KDE 3.1.2. 
     * @return the release number
     * @since 3.1
     */
    KDECORE_EXPORT unsigned int versionRelease();
    /**
     * Returns the KDE version as string, e.g. "3.1.2".
     * @return the KDE version. You can keep the string forever
     * @since 3.1
     */
    KDECORE_EXPORT const char *versionString();
}

#endif // _KDE_VERSION_H_
