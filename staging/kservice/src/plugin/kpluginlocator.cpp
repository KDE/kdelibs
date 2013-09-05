/*
   Copyright 2013 Sebastian Kügler <sebas@kde.org>

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

#include "kpluginlocator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QJsonObject>
#include <QtCore/QPluginLoader>

#include <QDebug>

#include <kplugininfo.h>

KPluginInfo::List KPluginLocator::query(const QString& servicetype, const QString& constraint)
{
    QPluginLoader loader;
    const QStringList libraryPaths = QCoreApplication::libraryPaths();

    KPluginInfo::List services;
    //QStringList files;
    Q_FOREACH (const QString& dir, libraryPaths) {
        //QDirIterator it(dir+"/kf5", QStringList() << "plasma_engine_time.so", QDir::Files);
        QDirIterator it(dir+"/kf5", QStringList() << "*.so", QDir::Files);

        while (it.hasNext()) {
            it.next();
            const QString _f = it.fileInfo().absoluteFilePath();
            loader.setFileName(_f);
            const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();
            KPluginInfo info(argsWithMetaData);

            if (info.serviceTypes().contains(servicetype)) {
                qDebug() << "Found plugin with " << servicetype << " : " << info.name();
                info.setLibraryPath(_f);
                services.append(info);
            }
//             qDebug() << " Plugininfo reports: " << info.name() << ", " << info.icon() << info.serviceTypes() << endl;
        }
    }
    //qDebug() << "Found " << files.count();
    return services;

}

QStringList KPluginLocator::locatePlugin(const QString& name)
{
    QPluginLoader loader;
    //When loading the plugin, QPluginLoader searches in the current directory and in all plugin locations specified by
    const QStringList libraryPaths = QCoreApplication::libraryPaths();

    QStringList files;
    Q_FOREACH (const QString& dir, libraryPaths) {

        //QDirIterator it(dir, QDirIterator::Subdirectories);
        //QDirIterator it(dir, QStringList() << "*.so", QDir::Files, QDirIterator::Subdirectories);
        //QDirIterator it(dir+"/kf5", QStringList() << name+".so", QDir::Files);
        QDirIterator it(dir+"/kf5", QStringList() << "*.so", QDir::Files);

        while (it.hasNext()) {
            it.next();
            const QString _f = it.fileInfo().absoluteFilePath();
            files.append(_f);
            loader.setFileName(_f);
            //qDebug() << loader.metaData();
            QVariantList argsWithMetaData;
            argsWithMetaData << loader.metaData().toVariantMap();
            KPluginInfo info(argsWithMetaData);
            //qDebug() << " Plugininfo reports: " << info.name() << ", " << info.icon() << endl;
        }
    }
    //qDebug() << "Found " << files.count();
    return files;
}

#include "kpluginlocator.moc"
