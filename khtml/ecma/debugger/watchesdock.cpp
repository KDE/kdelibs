#include <QVBoxLayout>
#include <QListWidget>

#include "watchesdock.h"
#include "watchesdock.moc"

WatchesDock::WatchesDock(QWidget *parent)
    : QDockWidget("Watches", parent)
{
    QListWidget *widget = new QListWidget(this);
    widget->addItem("Test 1");
    widget->addItem("Test 2");
    widget->addItem("Test 3");
    widget->addItem("Test 4");
    widget->addItem("Test 5");
    widget->addItem("Test 6");
    widget->addItem("Test 7");
    setWidget(widget);
}

WatchesDock::~WatchesDock()
{
}

