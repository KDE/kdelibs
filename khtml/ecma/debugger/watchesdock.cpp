#include <QVBoxLayout>
#include <QListWidget>

#include "watchesdock.h"
#include "watchesdock.moc"

WatchesDock::WatchesDock(QWidget *parent)
    : QDockWidget("Watches", parent)
{
    QListWidget *widget = new QListWidget(this);
    setWidget(widget);
}

WatchesDock::~WatchesDock()
{
}

