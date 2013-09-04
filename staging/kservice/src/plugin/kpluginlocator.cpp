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


static QTextStream cout(stdout);


QStringList KPluginLocator::locatePlugin(const QString& name)
{
    QPluginLoader loader;

    //When loading the plugin, QPluginLoader searches in the current directory and in all plugin locations specified by
    const QStringList libraryPaths = QCoreApplication::libraryPaths();

    QStringList files;
    Q_FOREACH (const QString& dir, libraryPaths) {
        qDebug() << " ......" <<  files;
        cout << " + Looking into dir: " << dir << endl;
        //QDirIterator it(dir+"/kf5", QStringList() << "*.so", QDir::Files, QDirIterator::Subdirectories);
        QDirIterator it(dir+"/kf5", QStringList() << name+".so", QDir::Files);
        //QDirIterator it(dir, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            const QString _f = it.fileInfo().absoluteFilePath();
            if (QFile::exists(_f)) {
                cout << "    - " << _f << endl;
                files.append(_f);

                loader.setFileName(_f);
                qDebug() << loader.metaData();
                QVariantList argsWithMetaData;
                argsWithMetaData << loader.metaData().toVariantMap();
                KPluginInfo info(argsWithMetaData);
                cout << " Plugininfo reports: " << info.name() << ", " << info.icon() << endl;

            } else {
                cout << "skip" << _f << endl;
            }
        }
    }

    return files;
}

#include "kpluginlocator.moc"
