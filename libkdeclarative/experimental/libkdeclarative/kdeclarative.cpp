/*
 *   Copyright 2010 Marco Martin <mart@gmail.com>
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

#include "kdeclarative.h"
#include "kdeclarativeprivate_p.h"
#include "engineaccess_p.h"

#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeExpression>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QWeakPointer>

#include <kdebug.h>

KDeclarativePrivate::KDeclarativePrivate()
    : initialized(false)
{
}

KDeclarative::KDeclarative()
    : d(new KDeclarativePrivate)
{
    /*
    //TODO: make possible to use a preexisting QDeclarativeEngine, due to the existence of the horrible qdeclarativeview class
    d->engine = new QDeclarativeEngine(this);
    d->component = new QDeclarativeComponent(d->engine, this);
    */
}

KDeclarative::~KDeclarative()
{
    delete d;
}


void KDeclarative::setDeclarativeEngine(QDeclarativeEngine *engine)
{
    if (d->declarativeEngine.data() == engine) {
        return;
    }
    d->initialized = false;
    d->declarativeEngine = engine;
}

QDeclarativeEngine *KDeclarative::declarativeEngine() const
{
    return d->declarativeEngine.data();
}

void KDeclarative::initialize()
{
    //Glorious hack:steal the engine
    //create the access object
    EngineAccess *engineAccess = new EngineAccess(this);
    d->declarativeEngine.data()->rootContext()->setContextProperty("__engineAccess", engineAccess);

    //make engineaccess set our d->scriptengine
    QDeclarativeExpression *expr = new QDeclarativeExpression(d->declarativeEngine.data()->rootContext(), d->declarativeEngine.data()->rootContext()->contextObject(), "__engineAccess.setEngine(this)");
    expr->evaluate();
    delete expr;
    engineAccess->deleteLater();

    //fail?
    if (!d->scriptEngine) {
        kWarning() << "Failed to get the script engine";
        return;
    }

    //change the old globalobject with a new read/write copy
    QScriptValue originalGlobalObject = d->scriptEngine.data()->globalObject();

    QScriptValue newGlobalObject = d->scriptEngine.data()->newObject();

    QString eval = QLatin1String("eval");
    QString version = QLatin1String("version");

    {
        QScriptValueIterator iter(originalGlobalObject);
        QVector<QString> names;
        QVector<QScriptValue> values;
        QVector<QScriptValue::PropertyFlags> flags;
        while (iter.hasNext()) {
            iter.next();

            QString name = iter.name();

            if (name == version) {
                continue;
            }

            if (name != eval) {
                names.append(name);
                values.append(iter.value());
                flags.append(iter.flags() | QScriptValue::Undeletable);
            }
            newGlobalObject.setProperty(iter.scriptName(), iter.value());

           // m_illegalNames.insert(name);
        }

    }

    d->scriptEngine.data()->setGlobalObject(newGlobalObject);

    d->initialized = true;
}

QScriptEngine *KDeclarative::scriptEngine() const
{
    return d->scriptEngine.data();
}
