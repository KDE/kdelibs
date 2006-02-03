#include <QWidget>
#include <QLayout>
#include <QAction>

#include "widgetfactory.h"

WidgetFactory::WidgetFactory()
    : QFormBuilder()
{
}

WidgetFactory::~WidgetFactory()
{
}

QWidget *WidgetFactory::widget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    return createWidget( widgetName, parentWidget, name );
}

QLayout *WidgetFactory::layout(const QString &layoutName, QObject *parent, const QString &name)
{
    return createLayout( layoutName, parent, name );
}

QAction *WidgetFactory::action(QObject *parent, const QString &name)
{
    return createAction( parent, name );
}
