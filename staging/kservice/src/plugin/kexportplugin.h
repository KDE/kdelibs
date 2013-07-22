/*  This file is part of the KDE project
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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
#ifndef KEXPORTPLUGIN_H
#define KEXPORTPLUGIN_H

#include <QtCore/QPluginLoader>
#include <QtCore/QtPlugin>
#include <kservice_export.h>
#include <kservice_version.h>

/**
 * \internal
 * Stores KDE version information in a plugin library.
 * \see K_PLUGIN_VERIFICATION_DATA
 */
struct KDEPluginVerificationData
{
    enum { PluginVerificationDataVersion = 1 };

    /**
     * \internal
     * The version of this structure. Do not ever delete or change a field.
     * Increase it in the K_EXPORT_PLUGIN_VERSION macro whenever you
     * append a field to this structure.
     */
    quint8 dataVersion;
    quint32 KDEVersion;
    const char *KDEVersionString;
};

/**
 * \internal
 * Used to export the KDE version a plugin was compiled against.
 * \see KDEPluginVerificationData
 */
#define K_PLUGIN_VERIFICATION_DATA \
Q_EXTERN_C KSERVICE_EXPORT const KDEPluginVerificationData kde_plugin_verification_data = \
{ KDEPluginVerificationData::PluginVerificationDataVersion, KSERVICE_VERSION, KSERVICE_VERSION_STRING };

/**
 * \relates KPluginLoader
 * Use this macro if you want to give your plugin a version number.
 * You can later access the version number with KPluginLoader::pluginVersion()
 */
#define K_EXPORT_PLUGIN_VERSION(version) \
Q_EXTERN_C KSERVICE_EXPORT const quint32 kde_plugin_version = version;


/**
 * \relates KPluginLoader
 * This macro exports the main object of the plugin. Most times, this will be a KPluginFactory
 * or derived class, but any QObject derived class can be used.
 * Take a look at the documentation of Q_EXPORT_PLUGIN2 for some details.
 */

#if defined (Q_OS_WIN32) && defined(Q_CC_BOR)
#define Q_STANDARD_CALL __stdcall
#else
#define Q_STANDARD_CALL

// Hack to warn when this code is built, spitting out something that gives a hint for porting
class KSERVICE_DEPRECATED_EXPORT K_EXPORT_PLUGIN_is_deprecated_port_to_K_PLUGIN_HEADER
{
};

#define K_EXPORT_PLUGIN(factory) \
            K_EXPORT_PLUGIN_is_deprecated_port_to_K_PLUGIN_HEADER dummy; \
            K_PLUGIN_VERIFICATION_DATA

#endif

#endif // KEXPORTPLUGIN_H

