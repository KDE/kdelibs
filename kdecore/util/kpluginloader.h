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
#ifndef KDECORE_KPLUGINLOADER_H
#define KDECORE_KPLUGINLOADER_H

#include <kglobal.h>
#include <kdeversion.h>

#include <QtCore/QPluginLoader>
#include <QtCore/QtPlugin>

struct KDEPluginVerificationData
{
    quint8 dataVersion;
    quint32 KDEVersion;
    const char *KDEVersionString;
};

#define K_PLUGIN_VERIFICATION_DATA \
Q_EXTERN_C KDE_EXPORT const KDEPluginVerificationData kde_plugin_verification_data = \
{ 1, KDE_VERSION, KDE_VERSION_STRING };

#define K_EXPORT_PLUGIN_VERSION(version) \
Q_EXTERN_C KDE_EXPORT const quint32 kde_plugin_version = version;

#define K_EXPORT_PLUGIN(factory) \
Q_EXPORT_PLUGIN(factory) \
K_PLUGIN_VERIFICATION_DATA

class KComponentData;
class KPluginFactory;
class KService;

class KPluginLoaderPrivate;
class KDECORE_EXPORT KPluginLoader : public QPluginLoader
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString pluginName READ fileName)
public:
    explicit KPluginLoader(const QString &plugin, const KComponentData &componentdata = KGlobal::mainComponent(), QObject *parent = 0);
    explicit KPluginLoader(const KService &service, const KComponentData &componentdata = KGlobal::mainComponent(), QObject *parent = 0);
    ~KPluginLoader();

    KPluginFactory *factory();

    QString pluginName() const;

    quint32 pluginVersion() const;

    QString errorString() const;

protected:
    bool load();
private:
    Q_DECLARE_PRIVATE(KPluginLoader)
    Q_DISABLE_COPY(KPluginLoader)

    using QPluginLoader::setFileName;

    KPluginLoaderPrivate *const d_ptr;
};


#endif
