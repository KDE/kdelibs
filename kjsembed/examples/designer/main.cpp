/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

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
#include <QApplication>
#include <QWidget>
#include <QPluginLoader>
#include <QHBoxLayout>
#include <QMetaObject>

#include <QtDesigner/QDesignerComponents>
#include <QtDesigner/QDesignerWidgetBoxInterface>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/QDesignerFormEditorPluginInterface>

#include <qdebug.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDesignerComponents::initializeResources();

    QWidget top;
    QDesignerFormEditorInterface *formEditor = QDesignerComponents::createFormEditor(&top);
    top.show();

    QObjectList kids = top.children();
    for (int i = 0; i < kids.size(); ++i) {
	qDebug() << kids.at(i)->objectName() << ", " << kids.at(i)->metaObject()->className() << endl;
    }

#if 0
    QList<QObject*>plugins = QPluginLoader::staticInstances();
    foreach (QObject *plugin, plugins) {

	qDebug() << plugin->objectName();
	qDebug() << plugin->metaObject()->className();

        if (QDesignerFormEditorPluginInterface *formEditorPlugin = qobject_cast<QDesignerFormEditorPluginInterface*>(plugin)) {
            if (!formEditorPlugin->isInitialized())
                formEditorPlugin->initialize( formEditor );
        }
    }
#endif

    QDesignerWidgetBoxInterface *widgetBox = QDesignerComponents::createWidgetBox(formEditor, 0);
    widgetBox->setFileName( "widgetbox.xml" );
    widgetBox->load();
    widgetBox->show();

    return app.exec();
}
