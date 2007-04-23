/* This file is part of the KDE project
   Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KUNDOSTACK_H
#define KUNDOSTACK_H

#include <kdeui_export.h>

#include <QtGui/QUndoStack>

class KActionCollection;

/**
 * Extends QUndoStack with functions that creates actions with KDE's default icons and shortcuts.
 * See QUndoStack for more information.
 */
class KDEUI_EXPORT KUndoStack : public QUndoStack
{
    Q_OBJECT
    public:
        /**
         * Constructs a KUndoStack with @p parent as parent
         * @param parent parent of the object
         */
        KUndoStack(QObject* parent = 0);

        /**
         * Creates an redo action with the default shortcut and icon and adds it to @p actionCollection
         * @param actionCollection the KActionCollection that should be the parent of the action
         * @param actionName the created action's object name, empty string will set it to the KDE default
         * @return the created action.
         */
        QAction* createRedoAction(KActionCollection* actionCollection, const QString& actionName = QString());
        /**
         * Creates an undo action with the default KDE shortcut and icon and adds it to @p actionCollection
         * @param actionCollection the KActionCollection that should be the parent of the action
         * @param actionName the created action's object name, empty string will set it to the KDE default
         * @return the created action.
         */
        QAction* createUndoAction(KActionCollection* actionCollection, const QString& actionName = QString());
};

#endif
