/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_DATAENGINECONSUMER_P_H
#define PLASMA_DATAENGINECONSUMER_P_H

#include <QtCore/QSet>
#include <QtCore/QUrl>

#include "plasma/dataenginemanager.h"
#include "servicejob.h"

namespace Plasma
{

class DataEngineConsumer;
class RemoteDataEngine;
class ServiceMonitor;

class DataEngineConsumerPrivate : public QObject
{
    Q_OBJECT

public:
    QSet<QString> loadedEngines;
    QMap<QPair<QString, QString>, RemoteDataEngine*> remoteEngines;
    QMap<Service*, QString> engineNameForService;
    DataEngine *remoteDataEngine(const QString &name, const QUrl &location);

public Q_SLOTS:
    void slotJobFinished(Plasma::ServiceJob *job);
    void slotServiceReady(Plasma::Service *service);
};

} // namespace Plasma

#endif

