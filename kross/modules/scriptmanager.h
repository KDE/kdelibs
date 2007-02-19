/***************************************************************************
 * scriptmanager.h
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KROSS_SCRIPTMANAGER_H
#define KROSS_SCRIPTMANAGER_H

#include <QObject>
#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QTreeView>

#include <kdialog.h>

namespace Kross {

    class GUIClient;
    class ScriptManagerModule;

    /**
    * The ScriptManagerCollection class shows a QListView where the content of a
    * \a ActionCollection is displayed and some buttons to run, stop, add, edit
    * and remove scripts.
    */
    class ScriptManagerCollection : public QWidget
    {
            Q_OBJECT
        public:

            /**
            * Constructor.
            * \param module The \a ScriptManagerModule that was used
            * to create this \a ScriptManagerCollection instance.
            * \param parent The parent widget this widget is child of.
            */
            ScriptManagerCollection(ScriptManagerModule* module, QWidget* parent);

            /**
            * Destructor.
            */
            virtual ~ScriptManagerCollection();

            /**
            * \return the \a ScriptManagerModule that was used to
            * create this \a ScriptManagerCollection instance.
            */
            ScriptManagerModule* module() const;

            /**
            * \return true if the collection was modified.
            */
            bool isModified() const;

        public Q_SLOTS:

            /**
            * Run the selected script.
            */
            void slotRun();

            /**
            * Stop the selected script if running.
            */
            void slotStop();

            /**
            * Edit the select item.
            */
            void slotEdit();

            /**
            * Add a new item.
            */
            void slotAdd();

            /**
            * Remove the selected item.
            */
            void slotRemove();

        private Q_SLOTS:
            /// The selected item changed.
            void slotSelectionChanged();
            /// The data changed.
            void slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
    * The ScriptManagerModule provides access to the Script Manager
    * functionality like the "Script Manager" KDialog.
    */
    class ScriptManagerModule : public QObject
    {
            Q_OBJECT
        public:
            explicit ScriptManagerModule();
            virtual ~ScriptManagerModule();

        public Q_SLOTS:

#if 0
            /**
            * Install the scriptpackage \p file . The scriptpackage should be a
            * tar.gz or tar.bzip archivefile.
            *
            * \param scriptpackagefile The local tar.gz or tar.bzip archivfile
            * which contains the files that should be installed.
            * \return true if installing was successfully else false.
            */
            bool installPackage(const QString& scriptpackagefile);

            /**
            * Uninstalls the scriptpackage \p action and removes all to the package
            * belonging files.
            *
            * \param action The \a Action that should be removed.
            * \return true if the uninstall was successfully else false.
            */
            bool uninstallPackage(Action* action);
#endif

            void showEditorDialog(QObject* object, QWidget* parent = 0);

            /**
            * Display the modal "Script Manager" dialog.
            */
            void showManagerDialog();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };
}

#endif
