#include <QVBoxLayout>
#include <QListWidget>

#include "scriptsdock.h"
#include "scriptsdock.moc"

ScriptsDock::ScriptsDock(QWidget *parent)
    : QDockWidget("Loaded Scripts", parent)
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

ScriptsDock::~ScriptsDock()
{
}

