/* This file is part of the KDE libraries
    Copyright (c) 2002 KDE Team

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// $Id$

#ifndef _KDE_VERSION_H_
#define _KDE_VERSION_H_

#define KDE_VERSION 310
#define KDE_VERSION_STRING "3.1.0 (KDE 3.1 RC4)"
#define KDE_VERSION_MAJOR 3
#define KDE_VERSION_MINOR 1
#define KDE_VERSION_RELEASE 0

/**
 * Namespace for general KDE functions.
 */
namespace KDE
{
    /**
     * Returns the major number of KDE's version, e.g.
     * 3 for KDE 3.1.2. 
     * @return the major version number
     * @since 3.1
     */
    unsigned int versionMajor();
    /**
     * Returns the minor number of KDE's version, e.g.
     * 1 for KDE 3.1.2. 
     * @return the minor version number
     * @since 3.1
     */
    unsigned int versionMinor();
    /**
     * Returns the release of KDE's version, e.g.
     * 2 for KDE 3.1.2. 
     * @return the release number
     * @since 3.1
     */
    unsigned int versionRelease();
    /**
     * Returns the KDE version as string, e.g. "3.1.2".
     * @return the KDE version. You can keep the string forever
     * @since 3.1
     */
    const char *versionString();
};

#endif // _KDE_VERSION_H_
