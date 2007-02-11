/***************************************************************************
 * scriptmanager.h
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 * copyright (C) 2006 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_SCRIPTMANAGEREDITOR_H
#define KROSS_SCRIPTMANAGEREDITOR_H

#include <QObject>
#include <QWidget>

namespace Kross {

    class Action;
    class ActionCollection;

    /**
    * The ScriptManagerPropertiesEditor class implements an editor
    * for properties an \a Action has.
    */
    class ScriptManagerPropertiesEditor : public QWidget
    {
            Q_OBJECT
        public:
            ScriptManagerPropertiesEditor(Action* action, QWidget* parent);
            virtual ~ScriptManagerPropertiesEditor();

            void commit();

        private Q_SLOTS:
            void slotSelectionChanged();
            void slotAdd();
            void slotRemove();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
    * The ScriptManagerEditor class implements a general editor
    * for \a Action and \a ActionCollection instances.
    */
    class ScriptManagerEditor : public QWidget
    {
        public:
            ScriptManagerEditor(Action* action, QWidget* parent);
            ScriptManagerEditor(ActionCollection* collection, QWidget* parent);
            virtual ~ScriptManagerEditor();

            void commit();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;

            void initGui();
    };

}

#endif
