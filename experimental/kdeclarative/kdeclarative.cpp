/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
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
#include "private/kdeclarative_p.h"
#include "private/rootcontext_p.h"
#include "private/kiconprovider_p.h"

#include <QCoreApplication>
#include <QtQml/QQmlComponent>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlExpression>
#include <QtQml/QQmlDebuggingEnabler>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

KDeclarativePrivate::KDeclarativePrivate()
    : initialized(false)
{
}

KDeclarative::KDeclarative()
    : d(new KDeclarativePrivate)
{
}

KDeclarative::~KDeclarative()
{
    delete d;
}


void KDeclarative::setDeclarativeEngine(QQmlEngine *engine)
{
    if (d->declarativeEngine.data() == engine) {
        return;
    }
    d->initialized = false;
    d->declarativeEngine = engine;
}

QQmlEngine *KDeclarative::declarativeEngine() const
{
    return d->declarativeEngine.data();
}

void KDeclarative::initialize()
{
    //FIXME: remove this or find a similar hack for qml2

    d->initialized = true;
}

void KDeclarative::setupBindings()
{
    /*Create a context object for the root qml context.
      in this way we can register global functions, in this case the i18n() family*/
    RootContext *contextObj = new RootContext(d->declarativeEngine.data());
    d->declarativeEngine.data()->rootContext()->setContextObject(contextObj);

    /*tell the engine to search for import in the kde4 plugin dirs.
    addImportPath adds the path at the beginning, so to honour user's
    paths we need to traverse the list in reverse order*/

    const QStringList pluginPathList = QCoreApplication::libraryPaths();
    QStringListIterator pluginPathIterator(pluginPathList);
    pluginPathIterator.toBack();
    while (pluginPathIterator.hasPrevious()) {
        d->declarativeEngine.data()->addImportPath(pluginPathIterator.previous() + "/imports");
    }

    const QString target = componentsTarget();
    if (target != defaultComponentsTarget()) {
        const QStringList paths = pluginPathList;
        QStringListIterator it(paths);
        it.toBack();
        while (it.hasPrevious()) {
            d->declarativeEngine.data()->addImportPath(it.previous() + "/platformimports/" + target);
        }
    }

    // setup ImageProvider for KDE icons
    d->declarativeEngine.data()->addImageProvider(QString("icon"), new KIconProvider);
}

void KDeclarative::setupQmlJsDebugger()
{
    if (QCoreApplication::arguments().contains(QLatin1String("-qmljsdebugger"))) {
        QQmlDebuggingEnabler enabler;
    }
}

QString KDeclarative::defaultComponentsTarget()
{
    return QLatin1String("desktop");
}

QString KDeclarative::componentsTarget()
{
    const QStringList platform = runtimePlatform();
    if (platform.isEmpty()) {
        return defaultComponentsTarget();
    }

    return platform.last();
}

QStringList KDeclarative::runtimePlatform()
{
    static QStringList *runtimePlatform = 0;
    if (!runtimePlatform) {
        const QString env = getenv("PLASMA_PLATFORM");
        runtimePlatform = new QStringList(env.split(":", QString::SkipEmptyParts));
        if (runtimePlatform->isEmpty()) {
            KConfigGroup cg(KSharedConfig::openConfig(), "General");
            *runtimePlatform = cg.readEntry("runtimePlatform", *runtimePlatform);
        }
    }

    return *runtimePlatform;
}

