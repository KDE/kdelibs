/*
   This file is part of the KDE libraries
   Copyright (c) 2006 Thomas Braxton <brax108@cox.net>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/QDebug>

#include "kglobal.h"
//#include "klocale.h"
#include "kpluginloader.h"
#include "kservicetypetrader.h"
//#include "kapplication.h"
#include "kconfig.h"
#include "kconfigbackend.h"
#include "kconfigini_p.h"
#include "kconfigdata.h"
#include "kdebug.h"
#include "kstandarddirs.h"
#include "kconfigbackend.moc"

typedef KSharedPtr<KConfigBackend> BackendPtr;

class KConfigBackend::Private
{
public:
    qint64 size;
    QDateTime lastModified;
    QString localFileName;

    static QString whatSystem(const QString& /*fileName*/)
    {
        return QLatin1String("INI");
    }
};


void KConfigBackend::registerMappings(const KEntryMap& /*entryMap*/)
{
}

BackendPtr KConfigBackend::create(const KComponentData& componentData, const QString& file,
                                  const QString& system)
{
    const QString upperSystem = (system.isEmpty() ?
            Private::whatSystem(file).toUpper() :
            system.toUpper());
    KConfigBackend* backend = 0;

    if (upperSystem == "INI")
        goto default_backend;
    else {
        KService::List offers = KServiceTypeTrader::self()->query("KConfigBackend");

        foreach (const KService::Ptr service, offers) {
            if (service->name().toUpper() != upperSystem ||
                service->library().isEmpty())
                continue;

            backend = KPluginLoader(*service, componentData).factory()->create<KConfigBackend>(QLatin1String("KConfigBackend"));
//             const char* libraryName = service->library().toLocal8Bit().constData();
//             int error = 0;

/*            backend = KLibLoader::createInstance<KConfigBackend>(
                                                   libraryName,
                                                   0,
                                                   QStringList(),
                                                   &error);*/
            if (backend) {
                backend->setFilePath(file);
                return BackendPtr(backend);
            } else {
                kDebug(181) << "Could not load config backend " << system
                            /*<< ". Error code: " << error*/ << endl;
                goto default_backend;
            }
        } // foreach offers
    }

default_backend:

    backend = new KConfigIniBackend;
    backend->setFilePath(file);
    return BackendPtr(backend);
}

KConfigBackend::KConfigBackend()
 : d(new Private)
{
}

KConfigBackend::~KConfigBackend()
{
    delete d;
}

QDateTime KConfigBackend::lastModified() const
{
    return d->lastModified;
}

void KConfigBackend::setLastModified(const QDateTime& dt)
{
    d->lastModified = dt;
}

qint64 KConfigBackend::size() const
{
    return d->size;
}

void KConfigBackend::setSize(qint64 sz)
{
    d->size = sz;
}

QString KConfigBackend::filePath() const
{
    return d->localFileName;
}

void KConfigBackend::setLocalFilePath(const QString& file)
{
    d->localFileName = file;
}
