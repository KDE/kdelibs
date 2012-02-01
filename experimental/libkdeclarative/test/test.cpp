/*
 *   Copyright 2011 Marco Martin <mart@gmail.com>
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

#include <QApplication>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QScriptEngine>

#include <kdeclarative.h>

#include "testobject_p.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDeclarativeView view;
    QDeclarativeContext *context = view.rootContext();
    context->setContextProperty("backgroundColor",
                                QColor(Qt::yellow));

    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(view.engine());
    kdeclarative.initialize();
    //binds things like kconfig and icons
    kdeclarative.setupBindings();

    //If all gone well, the QScriptEngine has been extracted
    QScriptEngine *scriptEngine = kdeclarative.scriptEngine();
    Q_ASSERT(scriptEngine);

    //Bind a test QObject in the "QtScript way"
    QScriptValue global = scriptEngine->globalObject();
    TestObject *testObject = new TestObject();
    QScriptValue testValue = scriptEngine->newQObject(testObject);
    testValue.setScope(global);
    global.setProperty("testObject", testValue);

    view.setSource(QUrl::fromLocalFile("test.qml"));
    view.show();

    return app.exec();
}

#include "moc_testobject_p.cpp"
