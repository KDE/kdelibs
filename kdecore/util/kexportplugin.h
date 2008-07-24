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
#ifndef KDECORE_KEXPORTPLUGIN_H
#define KDECORE_KEXPORTPLUGIN_H

#include <kglobal.h>
#include <kdeversion.h>

#include <QtCore/QPluginLoader>
#include <QtCore/QtPlugin>

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
Q_EXTERN_C KDE_EXPORT const KDEPluginVerificationData kde_plugin_verification_data = \
{ KDEPluginVerificationData::PluginVerificationDataVersion, KDE_VERSION, KDE_VERSION_STRING };

/**
 * \relates KPluginLoader
 * Use this macro if you want to give your plugin a version number.
 * You can later access the version number with KPluginLoader::pluginVersion()
 */
#define K_EXPORT_PLUGIN_VERSION(version) \
Q_EXTERN_C KDE_EXPORT const quint32 kde_plugin_version = version;


/**
 * \relates KPluginLoader
 * This macro exports the main object of the plugin. Most times, this will be a KPluginFactory
 * or derived class, but any QObject derived class can be used.
 * Take a look at the documentation of Q_EXPORT_PLUGIN2 for some details.
 */
#define K_EXPORT_PLUGIN(factory) \
Q_EXPORT_PLUGIN(factory) \
K_PLUGIN_VERIFICATION_DATA

#endif // KDECORE_KEXPORTPLUGIN_H

