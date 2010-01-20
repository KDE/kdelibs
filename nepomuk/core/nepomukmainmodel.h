/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2008-2010 Sebastian Trueg <trueg@kde.org>
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

#ifndef _NEPOMUK_MAIN_MODEL_H_
#define _NEPOMUK_MAIN_MODEL_H_

#include <Soprano/Model>
#include <Soprano/Vocabulary/RDFS>

#include <QtCore/QDateTime>

namespace Nepomuk {
    /**
     * \class MainModel nepomukmainmodel.h Nepomuk/MainModel
     *
     * \brief The main %Nepomuk data storage model.
     *
     * All user data is stored in the %Nepomuk main model.
     * This is a wrapper model which communicates all commands
     * to the %Nepomuk server.
     *
     * Usage is very simple. Just create an instance and start
     * using it. Use the isValid method to check the connection
     * to the server. All communication details are handled transparently.
     *
     * It is perfectly alright to create several instances of MainModel
     * in one application as internally all is mapped to a single global
     * instance which is also reused in ResourceManager::mainModel.
     *
     * Provides a set of convenience methods
     * for maintaining resource properties.
     *
     * It does automatic NRL named graph handling, i.e. provedance
     * data is created and deleted automatically.
     *
     * \warning This model assumes that no property value is stored twice,
     * i.e. in two different named graphs.
     *
     * \author Sebastian Trueg <trueg@kde.org>
     *
     * \since 4.2
     *
     * \sa ResourceManager::mainModel()
     */
    class MainModel : public Soprano::Model
    {
        Q_OBJECT

    public:
        /**
         * Create a new main model.
         */
        MainModel( QObject* parent = 0 );

        /**
         * Destructor.
         */
        ~MainModel();

        /**
         * Check the connection to the %Nepomuk server.
         * \return \p true if the connection is valid and commands can be issued,
         * \p false otherwise.
         */
        bool isValid() const;

        /**
         * Try connecting to the Nepomuk storage service. There is no need to call
         * this unless one wants to reconnect to a previously non-running system.
         * All methods below do connect automatically but do not retry after a
         * failure.
         */
        bool init();

        Soprano::StatementIterator listStatements( const Soprano::Statement &partial ) const;
        Soprano::NodeIterator listContexts() const;
        Soprano::QueryResultIterator executeQuery( const QString& query,
                                                   Soprano::Query::QueryLanguage language,
                                                   const QString& userQueryLanguage = QString() ) const;
        bool containsStatement( const Soprano::Statement &statement ) const;
        bool containsAnyStatement( const Soprano::Statement &statement ) const;
        bool isEmpty() const;
        int statementCount() const;
        Soprano::Error::ErrorCode addStatement( const Soprano::Statement& statement );
        Soprano::Error::ErrorCode removeStatement( const Soprano::Statement& statement );
        Soprano::Error::ErrorCode removeAllStatements( const Soprano::Statement& statement );
        Soprano::Node createBlankNode();

        using Model::addStatement;
        using Model::removeStatement;
        using Model::removeAllStatements;

    private:
        class Private;
        Private* const d;
    };
}

#endif
