#include <QVBoxLayout>
#include <QListWidget>

#include "breakpointsdock.h"
#include "breakpointsdock.moc"

BreakpointsDock::BreakpointsDock(QWidget *parent)
    : QDockWidget("Breakpoints", parent)
{
    QListWidget *widget = new QListWidget;
    setWidget(widget);
}

BreakpointsDock::~BreakpointsDock()
{
}
