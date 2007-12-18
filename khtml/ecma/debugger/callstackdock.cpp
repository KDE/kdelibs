/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Matt Broadstone (mbroadst@gmail.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "callstackdock.h"
#include "callstackdock.moc"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include "debugdocument.h"

namespace KJSDebugger {

CallStackDock::CallStackDock(QWidget *parent)
    : QDockWidget(i18n("Call Stack"), parent)
{
    setFeatures(DockWidgetMovable | DockWidgetFloatable);
    m_view = new QTableWidget(0, 2);
    m_view->setHorizontalHeaderLabels(QStringList() << i18n("Call") << i18n("Line"));
    m_view->verticalHeader()->hide();
    m_view->setShowGrid(false);
    m_view->setAlternatingRowColors(true);

    setWidget(m_view);
}

CallStackDock::~CallStackDock()
{
}

void CallStackDock::addCall(const QString &function, int lineNumber)
{
    CallStackEntry entry;
    entry.name = function;
    entry.lineNumber = lineNumber;

    m_callStack.append(entry);
}

void CallStackDock::removeCall()
{
    m_callStack.pop_back();
}

void CallStackDock::updateCall(int line)
{
    m_callStack.last().lineNumber = line;
    // Expects displayStack to be called to redraw.
}

void CallStackDock::setGlobalFrame(const QString& url)
{
    m_callStack.clear();
    addCall(KUrl(url).fileName(), 0);
}

void CallStackDock::displayStack()
{
    m_view->clearContents();
    m_view->setRowCount(m_callStack.count());

    int row = 0;
    foreach (CallStackEntry entry, m_callStack)
    {
        int displayRow = m_callStack.count() - row - 1; //Want newest entry on top
        QTableWidgetItem *function = new QTableWidgetItem(entry.name);
        function->setFlags(Qt::ItemIsEnabled);
        m_view->setItem(displayRow, 0, function);
        QTableWidgetItem *lineNumber = new QTableWidgetItem(QString::number(entry.lineNumber));
        lineNumber->setFlags(Qt::ItemIsEnabled);
        m_view->setItem(displayRow, 1, lineNumber);
        row++;
    }
    m_view->resizeColumnsToContents();
    m_view->resizeRowsToContents();
//    m_view->setColumnWidth(1, 20);
}

}
