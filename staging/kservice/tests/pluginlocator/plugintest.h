/******************************************************************************
*   Copyright 2012 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef PLUGINTEST_H
#define PLUGINTEST_H

#include <QCoreApplication>

class QCommandLineParser;

class PluginTestPrivate;

class PluginTest : public QCoreApplication
{
    Q_OBJECT

    public:
        PluginTest(int& argc, char** argv, QCommandLineParser *parser);
        virtual ~PluginTest();

    public Q_SLOTS:
        void runMain();
        bool loadFromKService(const QString &name = "time");
        bool loadFromMetaData(const QString &pluginName = "plasma_engine_time");
        bool loadFromMetaData2(const QString &serviceType = "Plasma/DataEngine");
        void report(QList<qint64> timings, const QString &msg = "Test took ");

    private:
        PluginTestPrivate* d;
};

#endif
