/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2008-2009 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _NEPOMUK_GRAPH_WRAPPER_H_
#define _NEPOMUK_GRAPH_WRAPPER_H_

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QUrl>

namespace Soprano {
    class Model;
}

namespace Nepomuk {

    class ResourceManager;

    /**
     * Creates a new graph whenever the event loop is entered.
     * This way we do not create a new graph for each statement
     * added to the main model but group sets of added statements
     * into graphs.
     *
     * We need separate graphs to be able to track their creation time.
     *
     * IDEA: We actually need multiple graphs to be able to save the creation date of statements (annotations)
     *       At the same time we do not want to create one graph for each created statement as we do in KDE 4.1
     *       A timeout is a bad idea, too, since some batch operations may take longer than our timeout which
     *       would lead to different graphs for the same batch.
     *       Thus, why not using the event loop? Just use a timer whenever statements are added to
     *       trigger the creation of a new graph on the next adding. Long batch operations as done in the
     *       Akonadi feeders would result in a single graph while separate user actions in a GUI would always
     *       lead to separate graphs.
     */
    class GraphWrapper : public QObject
    {
        Q_OBJECT

    public:
        GraphWrapper( QObject* parent = 0 );
        ~GraphWrapper();

        /**
         * Only look at the graph.
         */
        QUrl lookupCurrentGraph() const;

        /**
         * Get the current graph. This will also store
         * the graph in the main model if not done already.
         */
        QUrl currentGraph();

        void setManager( ResourceManager* manager );

    private Q_SLOTS:
        void slotTimeout();

    private:
        void createNewGraph();
        void storeGraph( const QUrl& );

        QUrl m_currentGraph;
        bool m_currentGraphStored;
        QTimer m_timer;

        ResourceManager* m_manager;
    };
}

#endif
