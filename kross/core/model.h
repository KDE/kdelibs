/***************************************************************************
 * model.h
 * This file is part of the KDE project
 * copyright (C) 2006-2007 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_MODEL_H
#define KROSS_MODEL_H

#include "krossconfig.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

namespace Kross {

    // Forward declarations.
    class Action;
    class ActionCollection;

    /**
     * The ActionCollectionModel class implements a QAbstractItemModel to provide
     * a model for views of a \a ActionCollection instance that manages a
     * collection of \a Action instances.
     */
    class KROSSCORE_EXPORT ActionCollectionModel : public QAbstractItemModel
    {
            Q_OBJECT
        public:
            enum Mode {
                None = 0,
                Icons = 1,
                ToolTips = 2,
                UserCheckable = 4
                //Editable = 8
            };

            explicit ActionCollectionModel(QObject* parent, ActionCollection* collection = 0, Mode mode = Mode(Icons|ToolTips));
            virtual ~ActionCollectionModel();

            virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
            virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex parent(const QModelIndex& index) const;
            virtual Qt::ItemFlags flags(const QModelIndex &index) const;
            virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
            virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

            virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
            virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
            virtual bool insertColumns(int column, int count, const QModelIndex& parent = QModelIndex());
            virtual bool removeColumns(int column, int count, const QModelIndex& parent = QModelIndex());

            //Qt::DropActions supportedDragActions() const;
            virtual QStringList mimeTypes() const;
            virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
            virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

            virtual Qt::DropActions supportedDropActions() const;

            /**
            * \return the \a Action instance the as argument passed QModelIndex
            * represents or NULL if the QModelIndex is not a \a Action .
            */
            static Action* action(const QModelIndex& index);

            /**
            * \return the \a ActionCollection instance the as argument passed QModelIndex
            * represents or NULL if the QModelIndex is not a \a ActionCollection .
            */
            static ActionCollection* collection(const QModelIndex& index);

        private Q_SLOTS:
            void slotUpdated();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * The ActionCollectionProxyModel class implements a QSortFilterProxyModel 
     * for a \a ActionCollectionModel instance.
     */
    class KROSSCORE_EXPORT ActionCollectionProxyModel : public QSortFilterProxyModel
    {
        public:
            ActionCollectionProxyModel(QObject* parent, ActionCollectionModel* model = 0);
            virtual ~ActionCollectionProxyModel();

        private:
            /// Set the \a ActionCollectionModel source model we are proxy for.
            virtual void setSourceModel(QAbstractItemModel* sourceModel);
            /// Implements a filter for the QModelIndex instances.
            virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    };

}

#endif

