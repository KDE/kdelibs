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

#ifndef KKERNEL_MAC_H
#define KKERNEL_MAC_H

#include <QtCore/qglobal.h> // defines Q_OS_MACX

#ifdef Q_OS_MACX

#include <kde4support_export.h>
#include <QtCore/QString>

#include <CoreFoundation/CFString.h>

/**
 * @short Mac-specific functions needed in kdecore
 * @author Benjamin Reed <rangerrick@befunk.com>
 *
 * This file provides some Mac-specific functions which
 * are needed to work around OS issues.
  */

/**
 * Convert a CFString to a QString.
 * Don't export, is for internal kdelibs use only.
 */
QString convert_CFString_to_QString(CFStringRef str);

/**
 * Fork and reexec to work around CoreFoundation issues.
  */
KDE4SUPPORT_DEPRECATED_EXPORT void mac_fork_and_reexec_self();

/**
  * Initialize D-Bus Mac-specific stuff if necessary.
  */
KDE4SUPPORT_DEPRECATED_EXPORT void mac_initialize_dbus();

/**
  * Get the application name.
  */
KDE4SUPPORT_DEPRECATED_EXPORT QString mac_app_filename();

#endif  // Q_OS_MACX
#endif  // KKERNEL_MAC_H
