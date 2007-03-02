/***************************************************************************
 * scriptmanagereditor.h
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
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

            /**
            * Constructor.
            * \param action The \a Action instance this editor should
            * operate on.
            * \param parent The parent widget this widget is child of.
            */
            ScriptManagerPropertiesEditor(Action* action, QWidget* parent);

            /**
            * Destructor.
            */
            virtual ~ScriptManagerPropertiesEditor();

            /**
            * This method got called if the changes done in the editor
            * should be saved aka committed to the \a Action instance.
            */
            void commit();

        private Q_SLOTS:
            /// This slot got called if the selection changed.
            void slotSelectionChanged();
            /// This slot got called if a new property should be added.
            void slotAdd();
            /// This slot got called if the selected property should be removed.
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

            /**
            * Constructor.
            * \param action The \a Action instance this editor should
            * operate on.
            * \param parent The parent widget this widget is child of.
            */
            ScriptManagerEditor(Action* action, QWidget* parent);

            /**
            * Constructor.
            * \param collection The \a ActionCollection instance this
            * editor should operate on.
            * \param parent The parent widget this widget is child of.
            */
            ScriptManagerEditor(ActionCollection* collection, QWidget* parent);

            /**
            * Destructor.
            */
            virtual ~ScriptManagerEditor();

            /**
            * \return the \a Action instance this editor operates on or
            * NULL if this editor does not operate on a \a Action instance.
            */
            Action* action() const;

            /**
            * \return the \a ActionCollection instance this editor operates on or
            * NULL if this editor does not operate on a \a ActionCollection instance.
            */
            ActionCollection* collection() const;

            /**
            * \return true if the values within the editor fields are valid.
            */
            bool isValid();

            /**
            * This method got called if the changes done in the editor
            * should be saved aka committed to the \a Action or
            * \a ActionCollection instance.
            */
            void commit();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;

            /// Initialize the GUI.
            void initGui();
    };

}

#endif
