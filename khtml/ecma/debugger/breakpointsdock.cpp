#include <QVBoxLayout>
#include <QListWidget>

#include "breakpointsdock.h"
#include "breakpointsdock.moc"

BreakpointsDock::BreakpointsDock(QWidget *parent)
    : QDockWidget("Breakpoints", parent)
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

BreakpointsDock::~BreakpointsDock()
{
}
