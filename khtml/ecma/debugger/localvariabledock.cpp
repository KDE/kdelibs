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

#include "localvariabledock.h"
#include "localvariabledock.moc"

#include <QVBoxLayout>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QEventLoop>

#include <kjs/interpreter.h>
#include <kjs/PropertyNameArray.h>
#include <kjs/context.h>
#include <kjs/scope_chain.h>
#include <kjs/object.h>
#include <kdebug.h>
#include <klocale.h>

#include "objectmodel.h"
#include "execstatemodel.h"

LocalVariablesDock::LocalVariablesDock(QWidget *parent)
    : QDockWidget(i18n("Local Variables"), parent), m_execModel(0)
{
    setFeatures(DockWidgetMovable | DockWidgetFloatable);
    m_view = new QTreeView;
//    m_model = new ObjectModel;
//    m_view->setModel(m_model);

    setWidget(m_view);
}

LocalVariablesDock::~LocalVariablesDock()
{
}

void LocalVariablesDock::clear()
{
    if (m_execModel)
        delete m_execModel;
}

void LocalVariablesDock::display(KJS::ExecState *exec)
{
    if (m_execModel)
        delete m_execModel;

    m_execModel = new ExecStateModel(exec);
    m_view->setModel(m_execModel);
}

