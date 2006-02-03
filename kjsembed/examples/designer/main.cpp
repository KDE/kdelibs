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
