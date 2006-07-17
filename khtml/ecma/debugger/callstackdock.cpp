#include <QVBoxLayout>
#include <QListWidget>

#include "callstackdock.h"
#include "callstackdock.moc"

CallStackDock::CallStackDock(QWidget *parent)
    : QDockWidget("Call Stack", parent)
{
    QListWidget *widget = new QListWidget;
    setWidget(widget);
}

CallStackDock::~CallStackDock()
{
}

