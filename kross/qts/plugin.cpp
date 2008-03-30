/***************************************************************************
 * plugin.cpp
 * This file is part of the KDE project
 * copyright (C)2007 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "plugin.h"
#include "values_p.h"

#include "../core/manager.h"

#include <QDebug>

using namespace Kross;

Q_EXPORT_PLUGIN2(krossqtsplugin, Kross::EcmaPlugin)

namespace Kross {

    /// \internal d-pointer class.
    class EcmaPlugin::Private
    {
        public:
            QScriptValue manager;
    };

}

EcmaPlugin::EcmaPlugin(QObject* parent)
    : QScriptExtensionPlugin(parent)
    , d(new Private)
{
    //qDebug()<<QString("EcmaPlugin Ctor");
}

EcmaPlugin::~EcmaPlugin()
{
    //qDebug()<<QString("EcmaPlugin Dtor");
    delete d;
}

void EcmaPlugin::initialize(const QString& key, QScriptEngine* engine)
{
    if( key.toLower() == "kross" ) {
        QScriptValue global = engine->globalObject();

        //QScriptContext *context = engine->currentContext();
        d->manager = engine->newQObject( &Kross::Manager::self() );
        global.setProperty("Kross", d->manager);

        initializeCore(engine);
        initializeGui(engine);
    }
    else
        qDebug()<<QString("Plugin::initialize unhandled key=%1").arg(key);
}

QStringList EcmaPlugin::keys() const
{
    //qDebug()<<"> Plugin::keys";
    return QStringList() << "kross";
}

