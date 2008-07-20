/***************************************************************************
 * view.h
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

#ifndef KROSS_VIEW_H
#define KROSS_VIEW_H

#include <kross/core/krossconfig.h>

#include <QtCore/QObject>
#include <QtGui/QWidget>
#include <QtCore/QModelIndex>
#include <QtGui/QItemSelection>
#include <QtGui/QTreeView>

#include <kdialog.h>

class KLineEdit;
class QComboBox;
class QItemSelection;
class KActionCollection;
class KUrlRequester;

namespace Kross {

    class Action;
    class ActionCollection;

    /**
    * The ActionCollectionEditor class implements a general editor
    * for \a Action and \a ActionCollection instances.
    *
    * Example that shows how to display the editor with an \a Action ;
    * \code
    * KDialog d(this);
    * Action* a = new Action();
    * new ActionCollectionEditor(a, d.mainWidget());
    * d.exec();
    * \endcode
    *
    * Example that shows how to display the editor with an \a ActionCollection ;
    * \code
    * KDialog d(this);
    * ActionCollection* c = new ActionCollection();
    * new ActionCollectionEditor(c, d.mainWidget());
    * d.exec();
    * \endcode
    */
    class KROSSUI_EXPORT ActionCollectionEditor : public QWidget
    {
        public:

            /**
            * Constructor.
            * \param action The \a Action instance this editor should
            * operate on.
            * \param parent The optional parent widget this widget is child of.
            */
            explicit ActionCollectionEditor(Action* action, QWidget* parent = 0);

            /**
            * Constructor.
            * \param collection The \a ActionCollection instance this
            * editor should operate on.
            * \param parent The optional parent widget this widget is child of.
            */
            explicit ActionCollectionEditor(ActionCollection* collection, QWidget* parent = 0);

            /**
            * Destructor.
            */
            virtual ~ActionCollectionEditor();

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
            virtual bool isValid();

            /**
            * This method got called if the changes done in the editor
            * should be saved aka committed to the \a Action or
            * \a ActionCollection instance.
            */
            virtual void commit();

            /**
            * Following getters are providing access to the edit-widgets once the
            * \a initGui() was called by the constructors. Some widgets like e.g.
            * the \a fileEdit() may return NULL if they are not used / displayed
            * (what is for the fileEdit the case if this \a ActionCollectionEditor
            * instance has no \a Action ).
            */
            QLineEdit* nameEdit() const;
            QLineEdit* textEdit() const;
            QLineEdit* commentEdit() const;
            QLineEdit* iconEdit() const;
            QComboBox* interpreterEdit() const;
            KUrlRequester* fileEdit() const;

        protected:

            /**
            * Initialize the GUI. Called by the both constructors and does
            * create e.g. the \a nameEdit() and the \a textEdit() widgets.
            */
            virtual void initGui();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
    * The ActionCollectionView class shows a QTreeView where the content of a
    * \a ActionCollection is displayed and optional actions to run, stop,
    * add, edit and remove scripts are provided.
    *
    * Example how to create, fill and use an instance of a ActionCollectionView;
    * \code
    * // We like to show the widget in a dialog.
    * KDialog d(this);
    * // Create the view.
    * ActionCollectionView* v = new ActionCollectionView( d.mainWidget() );
    * // Create the model.
    * ActionCollectionModel::Mode mode( ActionCollectionModel::Icons | ActionCollectionModel::ToolTips | ActionCollectionModel::UserCheckable );
    * ActionCollectionModel* m = new ActionCollectionModel(view, Manager::self().actionCollection(), mode);
    * // Set the model the view should use.
    * v->setModel(m);
    * // Show the dialog.
    * d.exec();
    * \endcode
    */
    class KROSSUI_EXPORT ActionCollectionView : public QTreeView
    {
            Q_OBJECT
        public:

            /**
            * Constructor.
            * \param parent The optional parent widget this widget is child of.
            */
            explicit ActionCollectionView(QWidget* parent = 0);

            /**
            * Destructor.
            */
            virtual ~ActionCollectionView();

            /**
            * Set the model this view should use to \p model . Use an instance of
            * \a ActionCollectionModel or \a ActionCollectionProxyModel as model.
            */
            virtual void setModel(QAbstractItemModel* model);

            /**
            * \return true if the collection was modified.
            */
            bool isModified() const;

            /**
            * Set the internal modified state of the collection to \p modified .
            */
            void setModified(bool modified);

            /**
            * \return the KActionCollection which is filled with KAction
            * instances this view provides. Per default there are the
            * actions "run" to run a script, "stop" to stop execution, "edit"
            * to edit the selected item, "add" to add a new item or resource,
            * "remove" to remove the selected item and "manager" to call and
            * show the modal Script Manager dialog.
            */
            KActionCollection* actionCollection() const;

            /**
            * \return the KPushButton instance which has the actionname \p actionname
            * or NULL if there is not such button.
            */
            KPushButton* button(const QString& actionname) const;

            /**
            * Create and return a new KPushButton instance for the given actionname.
            * \param parentWidget The parent widget.
            * \param actionname The name of the action. Each button points to an action
            * from within the \a actionCollection() and triggers that action if the
            * button got clicked.
            * \return The new KPushButton instance or NULL if e.g. there exist no
            * such action with \p actionname .
            */
            KPushButton* createButton(QWidget* parentWidget, const QString& actionname);

        public Q_SLOTS:

            /**
            * Called if the "run" action was triggered and the selected script
            * should be executed.
            */
            virtual void slotRun();

            /**
            * Called if the "stop" action was triggered and the selected script
            * stops execution if running.
            */
            virtual void slotStop();

            /**
            * Called if the "edit" action was triggered and the select item should
            * be edited via the scripts manager editor dialog.
            */
            virtual void slotEdit();

            /**
            * Called if the "add" action was triggered and a new item should be added.
            */
            virtual void slotAdd();

            /**
            * Called if the "remove" action was triggered and the selected item
            * should be removed.
            */
            virtual void slotRemove();

        Q_SIGNALS:

            /**
            * This signal is emitted if the enabled/disabled state of an action
            * changed. This happens for example if the slotSelectionChanged() above
            * got called cause another item was selected.
            * \param actionname The name of the action that changed. You are able
            * to use \a actionCollection() to receive the to the name matching
            * \a KAction instance. You are able to use e.g. a QSignalMapper here to
            * map such changes direct to your e.g. \a KPushButton instances used
            * to display some of the actions provided with \a actionCollection() .
            */
            void enabledChanged(const QString& actionname);

        protected Q_SLOTS:

            /**
            * This slot got called if the data changed.
            */
            virtual void slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

            /**
            * This slot got called if the enable/disable state of an action changed.
            */
            virtual void slotEnabledChanged(const QString& actionname);

            /**
            * This slot got called if the selected item changed.
            */
            virtual void slotSelectionChanged();

        protected:

            /**
            * This method provides us access to the QItemSelection. Compared to
            * the selectionModel()->selection() method this method does also
            * map the selection to the source-model for the case e.g. the
            * \a ActionCollectionProxyModel proxy-model was used rather then
            * a \a ActionCollectionModel direct.
            */
            QItemSelection itemSelection() const;

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif
