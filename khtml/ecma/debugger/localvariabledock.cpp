#include <QVBoxLayout>
#include <QListWidget>

#include "localvariabledock.h"
#include "localvariabledock.moc"

LocalVariablesDock::LocalVariablesDock(QWidget *parent)
    : QDockWidget("Local Variables", parent)
{
    QListWidget *widget = new QListWidget;
    widget->addItem("Test 1");
    widget->addItem("Test 2");
    widget->addItem("Test 3");
    widget->addItem("Test 4");
    widget->addItem("Test 5");
    widget->addItem("Test 6");
    widget->addItem("Test 7");
    setWidget(widget);
}

LocalVariablesDock::~LocalVariablesDock()
{
}
