/***************************************************************************
 * scriptmanageradd.h
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

#ifndef KROSS_SCRIPTMANAGERADD_H
#define KROSS_SCRIPTMANAGERADD_H

#include <QObject>
#include <QWidget>

class QRadioButton;
class KAssistantDialog;
class KPageWidgetItem;

namespace Kross {

    //class Action;
    //class ActionCollection;
    class ScriptManagerCollection;
    class ScriptManagerAddWizard;
    class ScriptManagerEditor;
    class FormFileWidget;

    class ScriptManagerAddTypeWidget : public QWidget
    {
            Q_OBJECT
        public:
            ScriptManagerAddTypeWidget(ScriptManagerAddWizard* wizard, QWidget* parent);
        public Q_SLOTS:
            void slotUpdate();
        private:
            ScriptManagerAddWizard* m_wizard;
            QRadioButton *m_scriptCheckbox, *m_collectionCheckbox, *m_installCheckBox, *m_onlineCheckbox;
    };

    class ScriptManagerAddFileWidget : public QWidget
    {
            Q_OBJECT
        public:
            ScriptManagerAddFileWidget(ScriptManagerAddWizard* wizard, QWidget* parent, const QString& startDirOrVariable);
        public Q_SLOTS:
            void slotUpdate();
        private:
            ScriptManagerAddWizard* m_wizard;
            FormFileWidget* m_filewidget;
    };

    class ScriptManagerAddScriptWidget : public QWidget
    {
            Q_OBJECT
        public:
            ScriptManagerAddScriptWidget(ScriptManagerAddWizard* wizard, QWidget* parent);
        public Q_SLOTS:
            void slotUpdate();
        private:
            ScriptManagerAddWizard* m_wizard;
            ScriptManagerEditor* m_editor;
    };

    class ScriptManagerAddCollectionWidget : public QWidget
    {
            Q_OBJECT
        public:
            ScriptManagerAddCollectionWidget(ScriptManagerAddWizard* wizard, QWidget* parent);
        public Q_SLOTS:
            void slotUpdate();
        private:
            ScriptManagerAddWizard* m_wizard;
            ScriptManagerEditor* m_editor;
    };

    class ScriptManagerAddWizard : public QObject
    {
            Q_OBJECT
            friend class ScriptManagerAddTypeWidget;
            friend class ScriptManagerAddFileWidget;
            friend class ScriptManagerAddScriptWidget;
            friend class ScriptManagerAddCollectionWidget;
        public:
            explicit ScriptManagerAddWizard(QWidget* parent);
            virtual ~ScriptManagerAddWizard();
        public Q_SLOTS:
            int execWizard();
        private:
            KAssistantDialog* m_dialog;
            KPageWidgetItem *m_typeItem, *m_fileItem, *m_scriptItem, *m_collectionItem;
    };
}

#endif
