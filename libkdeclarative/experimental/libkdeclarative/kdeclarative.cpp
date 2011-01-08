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
#include "engineaccess_p.h"

#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QScriptEngine>
#include <QWeakPointer>

class KDeclarativePrivate
{
public:
    KDeclarativePrivate();

    QWeakPointer<QDeclarativeEngine> declarativeEngine;
    QWeakPointer<QScriptEngine> scriptEngine;
    bool initialized;
};

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
    d->initialized = true;
}

QScriptEngine *KDeclarative::scriptEngine() const
{
    return d->scriptEngine.data();
}
