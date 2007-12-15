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

#ifndef LOCALVARIABLEDOCK_H
#define LOCALVARIABLEDOCK_H

#include <QDockWidget>

class QTreeView;

namespace KJS {
    class ExecState;
}

namespace KJSDebugger {
class ExecStateModel;

class LocalVariablesDock : public QDockWidget
{
    Q_OBJECT
public:
    LocalVariablesDock(QWidget *parent = 0);
    ~LocalVariablesDock();

    void display(KJS::ExecState *exec);
    void clear();

private:
    QTreeView       *m_view;
    ExecStateModel  *m_execModel;

};

}

#endif
