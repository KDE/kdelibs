/* This file is part of the KDE libraries
    Copyright (c) 2002 Simon Hausmann <hausmann@kde.org>
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>
    Copyright (c) 2003 Andreas Beckermann <b_mann@gmx.de>

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

#ifndef KDELIBS_KDEVERSION_H
#define KDELIBS_KDEVERSION_H

/**
 * @file kdeversion.h
 * @brief The file contains macros and functions related to the KDE version.
 */

#include <kdecore_export.h>

/**
 * @def KDE_VERSION_STRING
 * @ingroup KDEMacros
 * @brief Version of KDE as string, at compile time
 *
 * This macro contains the KDE version in string form. As it is a macro,
 * it contains the version at compile time. See versionString() if you need
 * the KDE version used at runtime.
 *
 * @note The version string might contain a section in parentheses,
 * especially for development versions of KDE.
 * If you use that macro directly for a file format (e.g. OASIS Open Document)
 * or for a protocol (e.g. http) be careful that it is appropriate.
 * (Fictional) example: "4.0.90 (>=20070101)"
 */
#cmakedefine KDE_VERSION_STRING "@KDE_VERSION_STRING@"

/**
 * @def KDE_VERSION_MAJOR
 * @ingroup KDEMacros
 * @brief Major version of KDE, at compile time
 */
#cmakedefine KDE_VERSION_MAJOR @KDE_VERSION_MAJOR@
/**
 * @def KDE_VERSION_MINOR
 * @ingroup KDEMacros
 * @brief Minor version of KDE, at compile time
 */
#cmakedefine KDE_VERSION_MINOR @KDE_VERSION_MINOR@
/**
 * @def KDE_VERSION_RELEASE
 * @ingroup KDEMacros
 * @brief Release version of KDE, at compile time
 */
#cmakedefine KDE_VERSION_RELEASE @KDE_VERSION_RELEASE@

/**
 * @ingroup KDEMacros
 * @brief Make a number from the major, minor and release number of a KDE version
 *
 * This function can be used for preprocessing when KDE_IS_VERSION is not
 * appropriate.
 */
#define KDE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

/**
 * @ingroup KDEMacros
 * @brief Version of KDE as number, at compile time
 *
 * This macro contains the KDE version in number form. As it is a macro,
 * it contains the version at compile time. See versionString() if you need
 * the KDE version used at runtime.
 */
#define KDE_VERSION \
  KDE_MAKE_VERSION(KDE_VERSION_MAJOR,KDE_VERSION_MINOR,KDE_VERSION_RELEASE)

/**
 * @ingroup KDEMacros
 * @brief Check if the KDE version matches a certain version or is higher
 *
 * This macro is typically used to compile conditionally a part of code:
 * @code
 * #if KDE_IS_VERSION(4,0,90)
 * // Code for KDE 4.1
 * #else
 * // Code for KDE 4.0
 * #endif
 * @endcode
 *
 * @warning Especially during development phases of KDE, be careful
 * when choosing the version number that you are checking against.
 * Otherwise you might risk to break the next KDE release.
 * Therefore be careful that development version have a
 * version number lower than the released version, so do not check 
 * e.g. for KDE 4.1 with KDE_IS_VERSION(4,1,0)
 * but with the actual version number at a time a needed feature was introduced.
 */
#define KDE_IS_VERSION(a,b,c) ( KDE_VERSION >= KDE_MAKE_VERSION(a,b,c) )

/**
 * Namespace for general KDE functions.
 */
namespace KDE
{
    /**
     * @brief Returns the encoded number of KDE's version, see the KDE_VERSION macro.
     *
     * In contrary to the macro KDE_VERSION
     * this function returns the number of the actually
     * installed KDE version, not the number of the KDE version that was
     * installed when the program was compiled.
     * @return the version number, encoded in a single uint
     */
    KDECORE_EXPORT unsigned int version();
    /**
     * @brief Returns the major number of KDE's version, e.g.
     * 4 for KDE 4.1.2. 
     * @return the major version number
     */
    KDECORE_EXPORT unsigned int versionMajor();
    /**
     * @brief Returns the minor number of KDE's version, e.g.
     * 1 for KDE 4.1.2. 
     * @return the minor version number
     */
    KDECORE_EXPORT unsigned int versionMinor();
    /**
     * @brief Returns the release of KDE's version, e.g.
     * 2 for KDE 4.1.2. 
     * @return the release number
     */
    KDECORE_EXPORT unsigned int versionRelease();
    /**
     * @brief Returns the KDE version as string, e.g. "4.1.2".
     *
     * On contrary to the macro KDE_VERSION_STRING this function returns
     * the version number of KDE at runtime.
     * @return the KDE version. You can keep the string forever
     */
    KDECORE_EXPORT const char *versionString();
}

#endif // KDELIBS_KDEVERSION_H
